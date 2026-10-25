/* cfe_static_symbol_list.c - static symbol table for the HelloX port.
 *
 * Registers the entry points of the statically-linked apps/libraries so
 * that OS_ModuleLoad_Static() / OS_SymbolLookup_Static() (via the
 * OSAL_CONFIG_INCLUDE_STATIC_LOADER feature) can resolve the symbols
 * referenced by the ES startup script (/cf/cfe_es_startup.scr).
 *
 * This overrides the default (mostly empty) OS_STATIC_SYMBOL_TABLE that
 * bsp_default_symtab.c would otherwise provide.
 */

#include "osapi-module.h"

/* app / library entry points (all statically linked into cfs.exe) */
extern void OS_Application_Startup(void);
extern void OS_Application_Run(void);
extern void SAMPLE_LIB_Init(void);
extern void SAMPLE_APP_Main(void);
extern void CI_Lab_AppMain(void);
extern void TO_LAB_AppMain(void);
extern void SCH_Lab_AppMain(void);

OS_static_symbol_record_t OS_STATIC_SYMBOL_TABLE[] = {
    {"OS_Application_Startup", OS_Application_Startup, NULL},
    {"OS_Application_Run", OS_Application_Run, NULL},
    {"SAMPLE_LIB_Init", SAMPLE_LIB_Init, "SAMPLE_LIB"},
    {"SAMPLE_APP_Main", SAMPLE_APP_Main, "SAMPLE_APP"},
    {"CI_Lab_AppMain", CI_Lab_AppMain, "CI_LAB_APP"},
    {"TO_LAB_AppMain", TO_LAB_AppMain, "TO_LAB_APP"},
    {"SCH_Lab_AppMain", SCH_Lab_AppMain, "SCH_LAB_APP"},
    {NULL, NULL, NULL}};
