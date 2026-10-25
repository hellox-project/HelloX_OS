/* hellox_module_list.c - PSP module list for the HelloX port. */
#include "cfe_psp_module.h"

extern CFE_PSP_ModuleApi_t CFE_PSP_soft_timebase_API;
extern CFE_PSP_ModuleApi_t CFE_PSP_timebase_posix_clock_API;
extern CFE_PSP_ModuleApi_t CFE_PSP_eeprom_notimpl_API;
extern CFE_PSP_ModuleApi_t CFE_PSP_ram_notimpl_API;
extern CFE_PSP_ModuleApi_t CFE_PSP_port_notimpl_API;
extern CFE_PSP_ModuleApi_t CFE_PSP_iodriver_API;

CFE_StaticModuleLoadEntry_t CFE_PSP_BASE_MODULE_LIST[] = {
    { .Name = "soft_timebase",          .Api = &CFE_PSP_soft_timebase_API },
    { .Name = "timebase_posix_clock",   .Api = &CFE_PSP_timebase_posix_clock_API },
    { .Name = "eeprom_notimpl",         .Api = &CFE_PSP_eeprom_notimpl_API },
    { .Name = "ram_notimpl",            .Api = &CFE_PSP_ram_notimpl_API },
    { .Name = "port_notimpl",           .Api = &CFE_PSP_port_notimpl_API },
    { .Name = "iodriver",               .Api = &CFE_PSP_iodriver_API },
    { NULL }
};

/* User (mission-specific) PSP module list; empty for HelloX. */
CFE_StaticModuleLoadEntry_t CFE_PSP_MODULE_LIST[] = {
    { NULL }
};
