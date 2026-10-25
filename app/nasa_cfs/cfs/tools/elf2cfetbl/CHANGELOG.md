# Changelog

## Development Build: v3.3.0-rc4+dev30
- Remove C++-style comments
- See <https://github.com/nasa/elf2cfetbl/pull/119>

## Development Build: v3.3.0-rc4+dev26
- Create CHANGELOG.md
- See <https://github.com/nasa/elf2cfetbl/pull/117>

## Development Build: v3.3.0-rc4+dev24

- Resolve uninit var static analysis warnings
- See <https://github.com/nasa/elf2cfetbl/pull/113> and <https://github.com/nasa/cFS/pull/492> 

## Development Build: v3.3.0-rc4+dev18

- Update Copyright Headers
- Standardize version.h 
- See <https://github.com/nasa/elf2cfetbl/pull/108> and <https://github.com/nasa/cFS/445>

## Development Build: v3.3.0-rc4+dev11

- Apply header guard standard 
- Remove explicit filename doxygen comments
- See <https://github.com/nasa/cFS/pull/432>

## Development Build: v3.3.0-rc4+dev4

- Fix Untrusted divisor (TAINTED_SCALAR) static analysis warning by checking `get_sh_entsize(SectionHeader)`
- Set baseline for cFS-Caelum-rc4: v3.3.0-rc4
- Update mission rev to use 0xFF for development version
- See <https://github.com/nasa/elf2cfetbl/pull/98> and <https://github.com/nasa/cfs/pull/390>

## Development Build: v3.2.0-rc1+dev30

-  Implement Coding Standard in CodeQL workflow
- See <https://github.com/nasa/elf2cfetbl/pull/81> and <https://github.com/nasa/cFS/pull/270>

## Development Build: v3.2.0-rc1+dev24

- Add Testing Tools to the Security Policy
- See <https://github.com/nasa/elf2cfetbl/pull/75>

## Development Build: v3.2.0+dev20

- Changes cpp-styled comments to c-style to meet coding standard
- See <https://github.com/nasa/elf2cfetbl/pull/72>

## Development Build: v3.2.0+dev9

- Restricts destination file permissions
- Squash int comparison warning
- Replace ctime (which generates LGTM warning) with ctime_r
- Deconflicts global/local parameters
- See <https://github.com/nasa/elf2cfetbl/pull/62>

## Development Build: v3.1.0+dev39

- Adds a null to the end of SrcFilename and DstFilename when using strncpy.
- Support ELF files that have all strings, including ELF section names, in one single ".strtab" section in the ELF file.
- Version reporting now uses the version numbers defined in elf_version.h and reports build number.
- See  <https://github.com/nasa/elf2cfetbl/pull/47>

## Development Build: 3.1.5

- Apply code style
- See <https://github.com/nasa/elf2cfetbl/pull/44>

## Development Build: 3.1.4

- Fix string termination warnings in GCC9
- See <https://github.com/nasa/elf2cfetbl/pull/41>

## Development Build: 3.1.3

- Builds for vxworks w/ 32-bit host
- See <https://github.com/nasa/elf2cfetbl/pull/40>

## Development Build: 3.1.2

- Minor bug fixes and documentation (see <https://github.com/nasa/elf2cfetbl/pull/25>)

## Development Build: 3.1.1

- Minor updates (see <https://github.com/nasa/elf2cfetbl/pull/19>)

## **_OFFICIAL RELEASE: 3.1.0 - Aquila_**

- Minor updates (see <https://github.com/nasa/elf2cfetbl/pull/13>)
- Not backwards compatible with OSAL 4.2.1
- Released as part of cFE 6.7.0, Apache 2.0

## **_OFFICIAL RELEASE: 3.0a_**

- Released as part of cFE 6.6.0a, Apache 2.0

NOTE - there are other parameter set management schemes used with the cFS (JSON, csv, etc) which may be more applicable for modern missions. Contact the community as detailed below for more information.

## Known issues

This ground utility was developed for a specific mission/configuration, and may not be applicable for general use. The Makefile and for_build/Makefile are no longer supported or tested.

## Getting Help

For best results, submit issues:questions or issues:help wanted requests at <https://github.com/nasa/cFS>.

Official cFS page: <http://cfs.gsfc.nasa.gov>
