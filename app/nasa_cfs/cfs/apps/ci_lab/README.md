![Static Analysis](https://github.com/nasa/ci_lab/workflows/Static%20Analysis/badge.svg)
![Format Check](https://github.com/nasa/ci_lab/workflows/Format%20Check/badge.svg)

# Core Flight System : Framework : App : Command Ingest Lab

This repository contains NASA's Command Ingest Lab (ci_lab), which is a framework component of the Core Flight System.

This lab application is a non-flight utility to send commands to the cFS Bundle. It is intended to be located in the `apps/ci_lab` subdirectory of a cFS Mission Tree. The Core Flight System is bundled at <https://github.com/nasa/cFS> (which includes ci_lab as a submodule), which includes build and execution instructions.

ci_lab is a simple command uplink application that accepts CCSDS telecommand packets over a UDP/IP port. It does not provide a full CCSDS Telecommand stack implementation.

## Known issues

Dependence on cfe platform config header is undesirable, and the check is not endian safe. As a lab application, extensive testing is not performed prior to release and only minimal functionality is included.

## Getting Help

For best results, submit issues:questions or issues:help wanted requests at <https://github.com/nasa/cFS>.

Official cFS page: <http://cfs.gsfc.nasa.gov>
