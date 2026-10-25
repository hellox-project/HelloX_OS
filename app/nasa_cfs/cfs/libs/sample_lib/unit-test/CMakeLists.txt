##################################################################
#
# Coverage Unit Test build recipe
#
# This CMake file contains the recipe for building the sample unit tests.
# It is invoked from the parent directory when unit tests are enabled.
#
##################################################################

#
# NOTE on the subdirectory structures here:
#
# - "inc" provides local header files shared between the coveragetest,
#    wrappers, and overrides source code units
# - "coveragetest" contains source code for the actual unit test cases
#    The primary objective is to get line/path coverage on the FSW 
#    code units.
# - "overrides" provides implementation of LOCAL stub functions to
#    that replace external calls.  This is for use cases where the
#    normal link-time  replacements is not sufficient/possible, and
#    a different implementation needs to be called at compile-time 
#    instead. 
#
# IMPORTANT: Most UT test cases do not need the "overrides" feature,
# it is primarily for cases where a C library function needs to be
# overridden.  It's use-case is included here as an example.

# This is also allowed to directly include files in the "fsw/src"
# directory that are normally private to the implementation
include_directories(${PROJECT_SOURCE_DIR}/fsw/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

# The "overrides" target provides replacements for C library
# calls that cannot be handled at link time.  Most UT test
# cases will NOT need this feature.
add_cfe_coverage_stubs(sample_lib_overrides
    override_src/libc_string_stubs.c
)

# Add a coverage test executable called "sample_lib-ALL" that 
# covers all of the functions in sample_lib.  
#
# Also note in a more complex app/lib the coverage test can also
# be broken down into smaller units (in which case one should use
# a unique suffix other than "ALL" for each unit).  For example,
# OSAL implements a separate coverage test per source unit.
add_cfe_coverage_test(sample_lib ALL 
    "coveragetest/coveragetest_sample_lib.c"
    "${CFE_SAMPLE_LIB_SOURCE_DIR}/fsw/src/sample_lib.c"
)

# For the object target only, the "override" includes should be injected
# into the include path.  Note it is important that this is only included 
# for the specific unit under test (object lib) not the coverage
# test executable or test cases, since these typically need the real
# version of these functions.
add_cfe_coverage_unit_include(sample_lib ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/override_inc
)


# The sample_lib stubs out a C library function so must be linked
# with the local "overrides" library (this is mainly just an example of how this 
# can be done).
add_cfe_coverage_dependency(sample_lib ALL 
    sample_lib_overrides
)

