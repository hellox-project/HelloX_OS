/*
 * osconfig.h - OSAL configuration for the HelloX port.
 * Generated manually from osconfig.h.in (no CMake).
 */

#ifndef OSCONFIG_H
#define OSCONFIG_H

/*
 * OSAL feature selection
 *   - Dynamic loader: OFF (apps are statically linked)
 *   - Network:        ON  (ground system communication)
 *   - Static loader:  ON  (static symbol table)
 *   - Shell:          OFF (HelloX has no fork/exec)
 *   - Console async:  ON
 */
/* #define OSAL_CONFIG_INCLUDE_DYNAMIC_LOADER */
#define OSAL_CONFIG_INCLUDE_NETWORK
#define OSAL_CONFIG_INCLUDE_STATIC_LOADER
/* #define OSAL_CONFIG_INCLUDE_SHELL */
#define OSAL_CONFIG_DEBUG_PRINTF
#define OSAL_CONFIG_DEBUG_PERMISSIVE_MODE
#define OSAL_CONFIG_CONSOLE_ASYNC

/* #define OSAL_CONFIG_BUGCHECK_DISABLE */
/* #define OSAL_CONFIG_BUGCHECK_STRICT */

/*
 * OSAL resource limits
 */
#define OS_MAX_TASKS                    64
#define OS_MAX_QUEUES                   64
#define OS_MAX_COUNT_SEMAPHORES         20
#define OS_MAX_BIN_SEMAPHORES           20
#define OS_MAX_MUTEXES                  20
#define OS_MAX_CONDVARS                  4
#define OS_MAX_MODULES                  20
#define OS_MAX_TIMEBASES                 5
#define OS_MAX_TIMERS                   10
#define OS_MAX_NUM_OPEN_FILES           50
#define OS_MAX_NUM_OPEN_DIRS             4
#define OS_MAX_FILE_SYSTEMS             14
#define OS_MAX_SYM_LEN                  64
#define OS_MAX_FILE_NAME                20
#define OS_MAX_PATH_LEN                 64
#define OS_MAX_API_NAME                 20
#define OS_SOCKADDR_MAX_LEN             28
#define OS_BUFFER_SIZE                 172
#define OS_BUFFER_MSG_DEPTH            100
#define OS_UTILITYTASK_PRIORITY        245
#define OS_UTILITYTASK_STACK_SIZE     2048
#define OS_MAX_CMD_LEN                1000
#define OS_QUEUE_MAX_DEPTH              50
#define OS_SHELL_CMD_INPUT_FILE_NAME   "/ram/ShellCmd.txt"
#define OS_PRINTF_CONSOLE_NAME         "hellox"
#define OS_ADD_TASK_FLAGS                0

/*
 * OSAL fixed resource limits
 */
#define OS_MAX_CONSOLES                  1
#define OS_MODULE_FILE_EXTENSION        ".exe"

/* Device / volume name lengths */
#define OS_FS_DEV_NAME_LEN   32
#define OS_FS_PHYS_NAME_LEN  64
#define OS_FS_VOL_NAME_LEN   32

#endif  /* OSCONFIG_H */
