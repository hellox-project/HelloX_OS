![Static Analysis](https://github.com/nasa/elf2cfetbl/workflows/Static%20Analysis/badge.svg)
![Format Check](https://github.com/nasa/elf2cfetbl/workflows/Format%20Check/badge.svg)

# Core Flight System : Framework : Tool : ELF to cFE Table Converter

This repository contains NASA's ELF to cFE Table Converter Tool (elf2cfetbl), which is a framework component of the Core Flight System.

This lab application is a ground utility to convert ELF to cFE binary tables for cFS. It is intended to be located in the `tools/elf2cfetbl` subdirectory of a cFS Mission Tree. The Core Flight System is bundled at <https://github.com/nasa/cFS>, which includes this tool as a submodule, and includes build and execution instructions.

See README.txt for more information.

## Known issues

This ground utility was developed for a specific mission/configuration, and may not be applicable for general use. The Makefile and for_build/Makefile are no longer supported or tested.

## Getting Help

For best results, submit issues:questions or issues:help wanted requests at <https://github.com/nasa/cFS>.

Official cFS page: <http://cfs.gsfc.nasa.gov>
