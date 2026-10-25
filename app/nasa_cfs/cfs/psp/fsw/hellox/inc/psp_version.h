/************************************************************************
 * HelloX Platform Support Package version identifiers
 ************************************************************************/

#ifndef PSP_VERSION_H
#define PSP_VERSION_H

#define CFE_PSP_IMPL_BUILD_NUMBER   1
#define CFE_PSP_IMPL_BUILD_BASELINE "v1.4.0"

#define CFE_PSP_IMPL_MAJOR_VERSION 1
#define CFE_PSP_IMPL_MINOR_VERSION 4
#define CFE_PSP_IMPL_REVISION      99

#define CFE_PSP_IMPL_MISSION_REV 0xFF

#define CFE_PSP_IMPL_CODENAME "HelloX"

#define CFE_PSP_IMPL_STR_HELPER(x) #x
#define CFE_PSP_IMPL_STR(x)        CFE_PSP_IMPL_STR_HELPER(x)

#define CFE_PSP_IMPL_VERSION CFE_PSP_IMPL_BUILD_BASELINE "+dev" CFE_PSP_IMPL_STR(CFE_PSP_IMPL_BUILD_NUMBER)

#define CFE_PSP_IMPL_VERSION_STRING                \
    " PSP DEVELOPMENT BUILD " CFE_PSP_IMPL_VERSION \
    ", Last Official Release: psp v1.4.0"

#endif
