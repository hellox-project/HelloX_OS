project(CFE_SAMPLE_LIB C)

# Create the app module
add_cfe_app(sample_lib fsw/src/sample_lib.c)

# The API to this library (which may be invoked/referenced from other apps)
# is stored in fsw/public_inc.  Using "target_include_directories" is the 
# preferred method of indicating this (vs. directory-scope "include_directories").
target_include_directories(sample_lib PUBLIC fsw/public_inc)

if (ENABLE_UNIT_TESTS)
  add_subdirectory(ut-stubs)
  add_subdirectory(unit-test)
endif (ENABLE_UNIT_TESTS)


