![Static Analysis](https://github.com/nasa/to_lab/workflows/Static%20Analysis/badge.svg)
![Format Check](https://github.com/nasa/to_lab/workflows/Format%20Check/badge.svg)

# Core Flight System : Framework : App : Telemetry Output Lab

This repository contains NASA's Telemetry Output Lab (to_lab), which is a framework component of the Core Flight System.

This lab application is a non-flight utility to downlink telemetry from the cFS Bundle. It is intended to be located in the `apps/to_lab` subdirectory of a cFS Mission Tree. The Core Flight System is bundled at <https://github.com/nasa/cFS> (which includes to_lab as a submodule), which includes build and execution instructions.

to_lab is a simple telemetry downlink application that sends CCSDS telecommand packets over a UDP/IP port. The UDP port and IP address are specified in the "Enable Telemetry" command. It does not provide a full CCSDS Telecommand stack implementation.

To send telemetry to the "ground" or UDP/IP port, edit the subscription table in the platform include file: fsw/platform_inc/to_lab_sub_table.h. to_lab will subscribe to the packet IDs that are listed in this table and send the telemetry packets it receives to the UDP/IP port.

## Known issues

As a lab application, extensive testing is not performed prior to release and only minimal functionality is included.

## Getting Help

For best results, submit issues:questions or issues:help wanted requests at <https://github.com/nasa/cFS>.

Official cFS page: <http://cfs.gsfc.nasa.gov>
