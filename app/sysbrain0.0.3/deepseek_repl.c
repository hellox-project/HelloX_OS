/* deepseek_repl.c
 *
 * HelloX DeepSeek Chat REPL client.
 *
 * Provides a simple Read-Eval-Print-Loop that:
 * 1. Reads a line from stdin (user prompt)
 * 2. Builds a JSON request for DeepSeek Chat API
 * 3. Opens a TCP connection to api.deepseek.com:443
 * 4. Performs TLS 1.2 handshake via wolfSSL
 * 5. Sends HTTP POST request
 * 6. Reads and prints the AI response
 * 7. Loops back to step 1
 *
 * Dependencies: wolfSSL (TLS), cJSON (JSON), HelloX sockets/lwIP (TCP)
 */

#include <hellox.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sockets/sockets.h>

/* wolfSSL headers — rely on user_settings.h for full configuration */
#include <wolfssl/ssl.h>

/* cJSON for JSON building/parsing */
#include <cJSON/cJSON.h>

/* ===================================================================
 * Missing C library defines
 * =================================================================== */

#ifndef EOF
#define EOF (-1)
#endif

/* Use manually specified IP address. */
#define DEEPSEEK_API_ADDR "119.188.44.230"

/* Forward declarations for wolfSSL I/O callbacks (defined in wolfssl_platform.c) */
extern int HxIORecv(WOLFSSL* ssl, char* buf, int sz, void* ctx);
extern int HxIOSend(WOLFSSL* ssl, char* buf, int sz, void* ctx);

/* ===================================================================
 * Configuration
 * =================================================================== */

/* DeepSeek API endpoint */
#define DEEPSEEK_HOST        "api.deepseek.com"
#define DEEPSEEK_PORT        443
#define DEEPSEEK_API_PATH    "/chat/completions"

/* Maximum sizes */
#define MAX_LINE             4096        /* max user input line */
#define MAX_RESPONSE         (256 * 1024) /* max API response buffer */
#define API_KEY_MAX          256
#define REQUEST_BUF_SIZE     (64 * 1024) /* HTTP request buffer */
#define RESPONSE_BUF_SIZE    (1024 * 1024)

/* Default API key file path (on HelloX file system) */
#define API_KEY_FILE         "c:\\syscfg\\deepseek.key"
/* Default CA certificate file path */
#define CA_CERT_FILE         "c:\\syscfg\\ca.pem"

/* ===================================================================
 * DNS resolution — via HelloX system call SYSCALL_GETHOSTBYNAME
 *
 * HelloX provides a system call (int 0x7F, eax=0x209) for DNS
 * resolution. We wrap it here with a minimal interface.
 * =================================================================== */

struct hostent {
    char* h_name;        /* Official name of host */
    char** h_aliases;    /* Alias list */
    short h_addrtype;    /* Address type (AF_INET) */
    short h_length;      /* Length of address (4 for IPv4) */
    char** h_addr_list;  /* List of addresses (null-terminated) */
};

/* Minimal gethostbyname implementation for HelloX.
 * Returns a pointer to a static struct hostent on success, NULL on failure.
 * Uses HelloX system call 0x209. */
static struct hostent* HxGethostbyname(const char* name)
{
    static struct hostent he;
    static char* h_addr_list[2];
    static unsigned long addr;
    /* Buffer used by kernel to return DNS result */
    static char dns_buffer[256];

    if (!name || !*name) return NULL;

#if 0 /* Can not support yet. */
    /* System call: int 0x7F, eax=0x209 (SYSCALL_GETHOSTBYNAME)
     * Input:  name pointer in ebx, buffer in ecx, buffer size in edx
     * Output: eax = 0 on success, non-zero on failure
     *
     * We move values through memory to avoid register exhaustion
     * with the constrained clobber list.
     */
    {
        unsigned long result;
        unsigned long sc_num = SYSCALL_GETHOSTBYNAME;
        unsigned long buf_size = sizeof(dns_buffer);

        __asm__ volatile (
            "pushl %%ebx\n\t"
            "pushl %%ecx\n\t"
            "pushl %%edx\n\t"
            "movl  %1, %%ebx\n\t"   /* ebx = name pointer */
            "movl  %2, %%ecx\n\t"   /* ecx = dns_buffer */
            "movl  %3, %%edx\n\t"   /* edx = buffer size */
            "movl  %4, %%eax\n\t"   /* eax = syscall number */
            "int   $0x7F\n\t"
            "movl  %%eax, %0\n\t"   /* save return value */
            "popl  %%edx\n\t"
            "popl  %%ecx\n\t"
            "popl  %%ebx\n\t"
            : "=m"(result)
            : "m"(name), "m"(dns_buffer), "m"(buf_size), "m"(sc_num)
            : "eax", "ebx", "ecx", "edx", "memory"
        );

        if (result != 0) {
            return NULL;  /* DNS resolution failed */
        }
    }
#endif

    /* Parse the kernel response from dns_buffer.
     * The exact format depends on HelloX kernel implementation.
     * For now, we assume the resolved IP address is at the start of dns_buffer. */
    //addr = *(unsigned long*)dns_buffer;
    ipaddr_aton(DEEPSEEK_API_ADDR, &addr);
    if (addr == 0 || addr == 0xFFFFFFFF) {
        return NULL;
    }

    /* Populate hostent structure */
    he.h_name = (char*)name;
    he.h_aliases = NULL;
    he.h_addrtype = AF_INET;
    he.h_length = 4;
    h_addr_list[0] = (char*)&addr;
    h_addr_list[1] = NULL;
    he.h_addr_list = h_addr_list;

    return &he;
}

/* ===================================================================
 * API Key management
 * =================================================================== */

static char g_api_key[API_KEY_MAX];

/* Load API key from file.
 * Returns 0 on success, -1 on failure. */
static int LoadApiKey(const char* filepath)
{
    HANDLE hFile;
    DWORD read_bytes;

    hFile = CreateFile((LPSTR)filepath, FILE_ACCESS_READ, 0, NULL);
    if (INVALID_HANDLE_VALUE == hFile) {
        printf("Error: Cannot open API key file: %s\r\n", filepath);
        return -1;
    }

    if (!ReadFile(hFile, API_KEY_MAX - 1, g_api_key, &read_bytes)) {
        printf("Error: Cannot read API key file.\r\n");
        CloseFile(hFile);
        return -1;
    }
    CloseFile(hFile);

    /* Trim trailing whitespace/CR/LF */
    while (read_bytes > 0 &&
           (g_api_key[read_bytes - 1] == '\r' ||
            g_api_key[read_bytes - 1] == '\n' ||
            g_api_key[read_bytes - 1] == ' ')) {
        read_bytes--;
    }
    g_api_key[read_bytes] = '\0';

    if (read_bytes == 0) {
        printf("Error: API key file is empty.\r\n");
        return -1;
    }

    return 0;
}

/* ===================================================================
 * TCP connection helper
 * =================================================================== */

/* Resolve hostname and connect.
 * Returns socket fd on success, -1 on failure. */
static int TcpConnect(const char* host, unsigned short port)
{
    struct sockaddr_in server;
    struct hostent* he;
    int sock;

    /* Resolve DNS */
    printf("Resolving %s...\r\n", host);
    he = HxGethostbyname(host);
    if (he == NULL) {
        printf("Error: DNS resolution failed for %s\r\n", host);
        return -1;
    }

    /* Create TCP socket */
    sock = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Error: Cannot create TCP socket.\r\n");
        return -1;
    }

    /* Prepare server address */
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    memcpy(&server.sin_addr.s_addr, he->h_addr_list[0], he->h_length);

    printf("Connecting to %s:%d...\r\n", host, port);
    if (lwip_connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("Error: TCP connection failed.\r\n");
        lwip_close(sock);
        return -1;
    }

    return sock;
}

/* ===================================================================
 * HTTP/TLS request/response handling
 * =================================================================== */

/* Build the HTTP POST request for DeepSeek Chat.
 * Returns number of bytes written to request_buf, or negative on error. */
static int BuildHttpRequest(char* request_buf, int buf_size,
                            const char* api_key, const char* user_message,
                            int stream)
{
    cJSON* root;
    cJSON* messages;
    cJSON* msg;
    char* json_str;
    int content_length;
    int written;

    /* Build JSON: {"model":"deepseek-chat","messages":[{"role":"user","content":"..."}],"stream":false} */
    root = cJSON_CreateObject();
    if (!root) return -1;

    cJSON_AddStringToObject(root, "model", "deepseek-chat");

    messages = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "messages", messages);

    msg = cJSON_CreateObject();
    cJSON_AddStringToObject(msg, "role", "user");
    cJSON_AddStringToObject(msg, "content", user_message);
    cJSON_AddItemToArray(messages, msg);

    cJSON_AddBoolToObject(root, "stream", stream ? 1 : 0);

    json_str = cJSON_PrintUnformatted(root);
    if (!json_str) {
        cJSON_Delete(root);
        return -1;
    }

    content_length = strlen(json_str);

    /* Build HTTP headers + body */
    written = snprintf(request_buf, buf_size,
        "POST %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Content-Type: application/json\r\n"
        "Authorization: Bearer %s\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        DEEPSEEK_API_PATH,
        DEEPSEEK_HOST,
        api_key,
        content_length,
        json_str);

    cJSON_free(json_str);
    cJSON_Delete(root);

    return written;
}

/* Send HTTP request over TLS connection.
 * Returns number of bytes sent, or negative on error. */
static int SendHttpRequest(WOLFSSL* ssl, const char* request, int len)
{
    int total_sent = 0;
    int ret;

    while (total_sent < len) {
        ret = wolfSSL_write(ssl, request + total_sent, len - total_sent);
        if (ret <= 0) {
            int err = wolfSSL_get_error(ssl, ret);
            printf("TLS write error: %d, tot_tx: %d\r\n", 
                err, total_sent);
            return -1;
        }
        total_sent += ret;
    }

    return total_sent;
}

/* Read HTTP response from TLS connection.
 * Returns number of bytes read into response_buf, or negative on error.
 * The response may include HTTP headers and body. */
static int ReadHttpResponse(WOLFSSL* ssl, char* response_buf, int buf_size)
{
    int total_read = 0;
    int ret;

    while (total_read < buf_size - 1) {
        ret = wolfSSL_read(ssl, response_buf + total_read,
                           buf_size - 1 - total_read);
        if (ret <= 0) {
            int err = wolfSSL_get_error(ssl, ret);
            if (err == WOLFSSL_ERROR_WANT_READ ||
                err == WOLFSSL_ERROR_ZERO_RETURN) {
                /* Connection closed cleanly */
                break;
            }
            /* Real error */
            if (err != 0) {
                /* Normal closure */
                break;
            }
            break;
        }
        total_read += ret;

        /* If we got a complete HTTP response, we could stop earlier,
         * but for simplicity we just read until the connection closes. */
    }

    response_buf[total_read] = '\0';
    return total_read;
}

/* Extract JSON body from HTTP response (after \r\n\r\n).
 * Returns pointer to start of body within response, or NULL on failure.
 * Handles Transfer-Encoding: chunked by skipping the chunk-size line. */
static const char* ExtractHttpBody(const char* response)
{
    const char* body = strstr(response, "\r\n\r\n");

    if (body == NULL) {
        body = strstr(response, "\n\n");
        if (body == NULL) {
            return NULL;
        }
        body += 2;
    } else {
        body += 4;
    }                                                                                                                                                                         

#if 0
    /* Skip chunk size line (hex digits + \r\n) — Transfer-Encoding: chunked */       
    if ((*body >= '0' && *body <= '9') || (*body >= 'a' && *body <= 'f') || (*body >= 'A' && *body <= 'F'))
    {
        const char* eol = strstr(body, "\r\n");
        if (eol){
            body = eol + 2;
        }
    }
#endif 

    return body;

#if 0
    const char* body = strstr(response, "\r\n\r\n");
    if (body == NULL) {
        body = strstr(response, "\n\n");
        if (body == NULL) {
            return NULL;
        }
        body += 2;
    } else {
        body += 4;
    }

    /* Skip chunked transfer encoding header (hex chunk size + \r\n).
     * Check if first char of body is a hex digit (chunk size). */
    if ((*body >= '0' && *body <= '9') ||
        (*body >= 'a' && *body <= 'f') ||
        (*body >= 'A' && *body <= 'F')) {
        const char* eol = strstr(body, "\r\n");
        if (eol != NULL) {
            body = eol + 2;  /* skip past hex size line + \r\n */
        }
    }

    return body;
#endif
}

/* De-chunk a Transfer-Encoding: chunked body in-place.
 * Returns pointer to the start of the decoded body (still in the same buffer). */
static char* DechunkBody(char* body)
{
    char* src = body;
    char* dst = body;
    
    while (*src) {
        /* Parse chunk size (hex) */
        int size = 0;
        if (*src >= '0' && *src <= '9') size = *src - '0';
        else if (*src >= 'a' && *src <= 'f') size = *src - 'a' + 10;
        else if (*src >= 'A' && *src <= 'F') size = *src - 'A' + 10;
        else break;  /* Not a valid chunk header */
        
        src++;
        while ((*src >= '0' && *src <= '9') || 
               (*src >= 'a' && *src <= 'f') || 
               (*src >= 'A' && *src <= 'F')) {
            size = (size << 4) | (*src >= '0' && *src <= '9' ? *src - '0' :
                   *src >= 'a' && *src <= 'f' ? *src - 'a' + 10 : *src - 'A' + 10);
            src++;
        }
        
        /* Skip \r\n after chunk size */
        if (*src == '\r') src++;
        if (*src == '\n') src++;
        
        if (size == 0) break;  /* Last chunk */
        
        /* Copy chunk data */
        for (int i = 0; i < size; i++) {
            *dst++ = *src++;
        }
        
        /* Skip \r\n after chunk data */
        if (*src == '\r') src++;
        if (*src == '\n') src++;
    }
    
    *dst = '\0';
    return body;
}

/* Parse DeepSeek response JSON and print the AI's reply.
 * Expects: {"choices":[{"message":{"content":"..."}}]}
 * For non-streaming responses. */
static void PrintAiReply(const char* body)
{
    cJSON* root;
    cJSON* choices;
    cJSON* choice;
    cJSON* message;
    cJSON* content;

    root = cJSON_Parse(body);
    if (!root) {
        printf("(JSON parse error)\r\n");
        /* Print raw response as fallback */
        printf("%s\r\n", body);
        return;
    }

    /* Check for error */
    cJSON* error = cJSON_GetObjectItem(root, "error");
    if (error) {
        cJSON* err_msg = cJSON_GetObjectItem(error, "message");
        if (err_msg && cJSON_IsString(err_msg)) {
            printf("API Error: %s\r\n", err_msg->valuestring);
        } else {
            printf("Unknown API error.\r\n");
        }
        cJSON_Delete(root);
        return;
    }

    choices = cJSON_GetObjectItem(root, "choices");
    if (!choices || !cJSON_IsArray(choices) || cJSON_GetArraySize(choices) == 0) {
        printf("(No choices in response)\r\n");
        cJSON_Delete(root);
        return;
    }

    choice = cJSON_GetArrayItem(choices, 0);
    if (!choice) {
        cJSON_Delete(root);
        return;
    }

    /* Try delta content (streaming format — works for non-streaming too) */
    cJSON* delta = cJSON_GetObjectItem(choice, "delta");
    if (delta) {
        content = cJSON_GetObjectItem(delta, "content");
        if (content && cJSON_IsString(content) && content->valuestring) {
            printf("%s", content->valuestring);
            printf("\r\n");
            cJSON_Delete(root);
            return;
        }
    }

    /* Standard non-streaming format */
    message = cJSON_GetObjectItem(choice, "message");
    if (message) {
        content = cJSON_GetObjectItem(message, "content");
        if (content && cJSON_IsString(content) && content->valuestring) {
            printf("%s\r\n", content->valuestring);
            cJSON_Delete(root);
            return;
        }
    }

    /* Try text field */
    content = cJSON_GetObjectItem(choice, "text");
    if (content && cJSON_IsString(content) && content->valuestring) {
        printf("%s\r\n", content->valuestring);
        cJSON_Delete(root);
        return;
    }

    printf("(Unexpected response format)\r\n");
    cJSON_Delete(root);
}

/* ===================================================================
 * ReadLine — simple line reader from stdin
 * =================================================================== */

/* Read a line from stdin into buffer. Returns buffer on success, NULL on EOF.
 * Uses HelloX's gets() or fgets equivalent.
 * Note: HelloX user mode's stdin is serial console input. */
static char* ReadLine(char* buffer, int max_len)
{
    int ch;
    int pos = 0;

    while (pos < max_len - 1) {
        /* GetChar() reads one character from HelloX console input.
         * The exact function name might differ; let's use 'getchar()'
         * or inline assembly to read from serial port.
         */
        ch = getchar();

        if (ch == EOF || ch == 0x04) {  /* Ctrl+D = EOF */
            if (pos == 0) return NULL;  /* Empty line = EOF */
            break;
        }

        if (ch == '\r' || ch == '\n') {
            /* Echo newline and return */
            printf("\r\n");
            break;
        }

        if (ch == '\b' || ch == 0x7F) { /* Backspace or DEL */
            if (pos > 0) {
                pos--;
                printf("\b \b");  /* Erase character */
            }
            continue;
        }

        buffer[pos++] = (char)ch;
        printf("%c", ch);  /* Local echo */
    }

    buffer[pos] = '\0';
    return buffer;
}

/* ===================================================================
 * REPL main loop
 * =================================================================== */

int DeepSeekRepl(void)
{
    int ret = 0;
    WOLFSSL_CTX* ctx = NULL;
    WOLFSSL* ssl = NULL;
    int sock = -1;
    char line[MAX_LINE];
    char* request_buf = NULL;
    char* response_buf = NULL;

    printf("\r\n========================================\r\n");
    printf("  DeepSeek Chat REPL Client v0.0.3\r\n");
    printf("  (HelloX + wolfSSL + cJSON)\r\n");
    printf("========================================\r\n\r\n");

    /* Allocate buffers */
    request_buf = (char*)_hx_malloc(REQUEST_BUF_SIZE);
    response_buf = (char*)_hx_malloc(RESPONSE_BUF_SIZE);
    if (!request_buf || !response_buf) {
        printf("Error: Out of memory.\r\n");
        if (request_buf) _hx_free(request_buf);
        if (response_buf) _hx_free(response_buf);
        return -1;
    }

    /* Load API key */
    if (LoadApiKey(API_KEY_FILE) != 0) {
        printf("Place your DeepSeek API key in %s\r\n", API_KEY_FILE);
        _hx_free(request_buf);
        _hx_free(response_buf);
        return -1;
    }
    printf("[%s]load API key OK.\r\n", __FUNCTION__);

    /* Initialize wolfSSL library */
    wolfSSL_Init();
    wolfSSL_Debugging_ON();

    /* Create SSL context */
    ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
    if (!ctx) {
        printf("Error: Cannot create SSL context.\r\n");
        _hx_free(request_buf);
        _hx_free(response_buf);
        wolfSSL_Cleanup();
        return -1;
    }

    /* Load CA certificate — from file into memory buffer, then use
     * wolfSSL_CTX_load_verify_buffer() (the buffer version works
     * even when NO_FILESYSTEM is defined, unlike the file version). */
    {
        HANDLE hCertFile;
        DWORD fileSize, readSize;
        unsigned char* certBuf = NULL;

        hCertFile = CreateFile(CA_CERT_FILE, FILE_ACCESS_READ, 0, NULL);
        if (INVALID_HANDLE_VALUE != hCertFile) {
            fileSize = GetFileSize(hCertFile, NULL);
            if (fileSize > 0 && fileSize < (64 * 1024)) {
                certBuf = (unsigned char*)_hx_malloc(fileSize + 1);
                if (certBuf) {
                    if (ReadFile(hCertFile, fileSize, certBuf, &readSize)) {
                        certBuf[readSize] = '\0';
                        if (wolfSSL_CTX_load_verify_buffer(ctx, certBuf,
                                (long)readSize, WOLFSSL_FILETYPE_PEM) != SSL_SUCCESS) {
                            printf("Warning: Failed to load CA certificate.\r\n");
                            wolfSSL_CTX_set_verify(ctx, WOLFSSL_VERIFY_NONE, NULL);
                        } else {
                            printf("CA certificate loaded.\r\n");
                        }
                    }
                    _hx_free(certBuf);
                }
            }
            CloseFile(hCertFile);
        } else {
            printf("Warning: Cannot open CA cert file: %s\r\n"
                   "  TLS cert verification disabled.\r\n", CA_CERT_FILE);
            wolfSSL_CTX_set_verify(ctx, WOLFSSL_VERIFY_NONE, NULL);
        }
    }

    /* Enable SNI (Server Name Indication) */
    wolfSSL_CTX_UseSNI(ctx, WOLFSSL_SNI_HOST_NAME, DEEPSEEK_HOST,
                       (unsigned short)strlen(DEEPSEEK_HOST));

    printf("\r\nType your prompts. Enter 'exit' or 'quit' to exit.\r\n");
    printf("------------------------------------------------\r\n");

    /* ---- REPL Loop ---- */
    while (1) {
        /* Print prompt */
        printf("\r\n>>> ");

        /* Read user input */
        if (ReadLine(line, MAX_LINE) == NULL) {
            printf("\r\n");
            break;
        }

        /* Check for exit command */
        if (strcmp(line, "exit") == 0 || strcmp(line, "quit") == 0 ||
            strcmp(line, "q") == 0) {
            break;
        }

        /* Skip empty lines */
        if (line[0] == '\0') {
            continue;
        }

        /* ---- Build connection ---- */
        sock = TcpConnect(DEEPSEEK_HOST, DEEPSEEK_PORT);
        if (sock < 0) {
            printf("Error: Cannot connect to server.\r\n");
            continue;
        }

        /* ---- TLS Handshake ---- */
        ssl = wolfSSL_new(ctx);
        if (!ssl) {
            printf("Error: Cannot create SSL object.\r\n");
            lwip_close(sock);
            continue;
        }

        wolfSSL_set_fd(ssl, sock);

        /* Set I/O routines properly. */
        wolfSSL_SSLSetIORecv(ssl, HxIORecv);
        wolfSSL_SSLSetIOSend(ssl, HxIOSend);

        /* Set up I/O — use wolfSSL's default BSD socket I/O.
 * Since HelloX's sockets.h (with LWIP_COMPAT_SOCKETS) defines
 * send/recv as macros for lwip_send/lwip_recv, wolfSSL will
 * call them directly via wolfSSL_set_fd().
 *
 * (No need for custom callbacks — the default I/O wrapper works.) */

        ret = wolfSSL_connect(ssl);
        if (ret != SSL_SUCCESS) {
            int err = wolfSSL_get_error(ssl, ret);
            printf("TLS handshake failed (error %d).\r\n", err);
            wolfSSL_free(ssl);
            lwip_close(sock);
            ssl = NULL;
            sock = -1;
            continue;
        }

        /* ---- Send HTTP request ---- */
        ret = BuildHttpRequest(request_buf, REQUEST_BUF_SIZE,
                               g_api_key, line, 0); /* non-streaming */
        if (ret <= 0) {
            printf("Error: Cannot build HTTP request.\r\n");
            wolfSSL_free(ssl);
            lwip_close(sock);
            ssl = NULL;
            sock = -1;
            continue;
        }

        ret = SendHttpRequest(ssl, request_buf, ret);
        if (ret < 0) {
            printf("Error: Failed to send request.\r\n");
            wolfSSL_free(ssl);
            lwip_close(sock);
            ssl = NULL;
            sock = -1;
            continue;
        }

        /* ---- Receive response ---- */
        printf("Waiting for response...\r\n");
        ret = ReadHttpResponse(ssl, response_buf, RESPONSE_BUF_SIZE);
        //printf("=== Raw response (%d bytes) ===\r\n%s\r\n=== End ===\r\n", ret, response_buf);
        if (ret < 0) {
            printf("Error: Failed to read response.\r\n");
        } else if (ret == 0) {
            printf("(Empty response)\r\n");
        } else {
            /* Extract JSON body and print AI reply */
            const char* body = ExtractHttpBody(response_buf);
            if (body) {
                /* Skip all chunk size and print out. */
                body = DechunkBody((char*)body);
                PrintAiReply(body);
            } else {
                printf("(No HTTP body in response)\r\n");
                printf("Raw response:\r\n%s\r\n", response_buf);
            }
        }

        /* ---- Cleanup ---- */
        wolfSSL_free(ssl);
        ssl = NULL;
        lwip_close(sock);
        sock = -1;
    }

    /* ---- Cleanup on exit ---- */
    if (ssl) wolfSSL_free(ssl);
    if (sock >= 0) lwip_close(sock);
    if (ctx) wolfSSL_CTX_free(ctx);
    wolfSSL_Cleanup();

    if (request_buf) _hx_free(request_buf);
    if (response_buf) _hx_free(response_buf);

    printf("\r\nGoodbye!\r\n");
    return 0;
}

/* Main function. */
int main(int argc, char* argv[])
{
    /* Just invoke DeepSeek repl. */
    return DeepSeekRepl();
}
