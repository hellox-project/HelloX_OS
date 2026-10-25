OVERRIDE DIRECTORY FOR UNIT TESTS

This directory is inserted into the compilation include path prior to any other
include paths when compiling FSW code modules for unit testing.  This may provide
alternate definitions of any dependent header files, in case the UT needs to
be directed to alternate implementations that cannot be substituted at link time.
For instance, this may provide alternate definitions of the C library calls.

This directory may be empty if the UT does not require this feature.

