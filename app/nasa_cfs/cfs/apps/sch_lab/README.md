![Static Analysis](https://github.com/nasa/sch_lab/workflows/Static%20Analysis/badge.svg)
![Format Check](https://github.com/nasa/sch_lab/workflows/Format%20Check/badge.svg)

# Core Flight System : Framework : App : Scheduler Lab

This repository contains NASA's Scheduler Lab (sch_lab), which is a framework component of the Core Flight System.

This lab application is a non-flight packet scheduler application for the cFS Bundle. It is intended to be located in the `apps/sch_lab` subdirectory of a cFS Mission Tree. The Core Flight System is bundled at <https://github.com/nasa/cFS> (which includes sch_lab as a submodule), which includes build and execution instructions.

sch_lab is a simple packet scheduler application with a one second resolution.

To change the list of packets that sch_lab sends out, edit the schedule table located in the platform include file: fsw/platform_inc/sch_lab_sched_tab.h

## Known issues

As a lab application, extensive testing is not performed prior to release and only minimal functionality is included.

## Getting Help

For best results, submit issues:questions or issues:help wanted requests at <https://github.com/nasa/cFS>.

Official cFS page: <http://cfs.gsfc.nasa.gov>
