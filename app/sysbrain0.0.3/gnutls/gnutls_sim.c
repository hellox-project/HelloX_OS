/* Simulation of GUN TLS routines. */
#include <stdlib.h>
#include <string.h>
#include "gnutls.h"

gnutls_alloc_function gnutls_malloc = _hx_malloc;
gnutls_alloc_function gnutls_secure_malloc = _hx_malloc;
gnutls_realloc_function gnutls_realloc = _hx_realloc;
gnutls_calloc_function gnutls_calloc = _hx_calloc;
gnutls_free_function gnutls_free = _hx_free;
gnutls_memset_function gnutls_memset = memset;

int gnutls_init(gnutls_session_t * session, unsigned int flags)
{
	return 0;
}

void gnutls_deinit(gnutls_session_t session)
{
	return;
}

int gnutls_bye(gnutls_session_t session, gnutls_close_request_t how)
{
	return 0;
}

int gnutls_handshake(gnutls_session_t session)
{
	return 0;
}

void gnutls_handshake_set_timeout(gnutls_session_t session,
	unsigned int ms)
{
	return;
}

int gnutls_error_is_fatal(int error)
{
	return 0;
}

ssize_t gnutls_record_send(gnutls_session_t session, const void *data,
	size_t data_size)
{
	return 0;
}

ssize_t gnutls_record_recv(gnutls_session_t session, void *data,
	size_t data_size)
{
	return 0;
}

size_t gnutls_record_get_max_size(gnutls_session_t session)
{
	return 0;
}

int gnutls_session_ticket_enable_server(gnutls_session_t session,
	const gnutls_datum_t * key)
{
	return 0;
}

int gnutls_priority_set(gnutls_session_t session,
	gnutls_priority_t priority)
{
	return 0;
}

int gnutls_credentials_set(gnutls_session_t session,
	gnutls_credentials_type_t type, void *cred)
{
	return 0;
}

void gnutls_transport_set_int2(gnutls_session_t session, int r, int s)
{
	return;
}

void gnutls_certificate_server_set_request(gnutls_session_t session,
	gnutls_certificate_request_t req)
{
	return;
}

unsigned int gnutls_sec_param_to_pk_bits(gnutls_pk_algorithm_t algo,
	gnutls_sec_param_t param)
{
	return 0;
}

int gnutls_session_ticket_key_generate(gnutls_datum_t * key)
{
	return 0;
}

int gnutls_priority_init(gnutls_priority_t * priority_cache,
	const char *priorities, const char **err_pos)
{
	return 0;
}

void gnutls_priority_deinit(gnutls_priority_t priority_cache)
{
	return;
}

void
gnutls_certificate_free_credentials(gnutls_certificate_credentials_t sc)
{
	return;
}

int
gnutls_certificate_allocate_credentials(gnutls_certificate_credentials_t
	* res)
{
	return 0;
}

void gnutls_certificate_set_dh_params(gnutls_certificate_credentials_t res,
	gnutls_dh_params_t dh_params)
{
	return;
}

int
gnutls_certificate_set_x509_trust_file(gnutls_certificate_credentials_t
	cred, const char *cafile,
	gnutls_x509_crt_fmt_t type)
{
	return 0;
}

int
gnutls_certificate_set_x509_key_file2(gnutls_certificate_credentials_t
	res, const char *certfile,
	const char *keyfile,
	gnutls_x509_crt_fmt_t type,
	const char *pass,
	unsigned int flags)
{
	return 0;
}

int gnutls_global_init(void)
{
	return 0;
}

void gnutls_global_deinit(void)
{
	return;
}

int gnutls_dh_params_init(gnutls_dh_params_t * dh_params)
{
	return 0;
}

void gnutls_dh_params_deinit(gnutls_dh_params_t dh_params)
{
	return;
}

int gnutls_dh_params_generate2(gnutls_dh_params_t params,
	unsigned int bits)
{
	return 0;
}
