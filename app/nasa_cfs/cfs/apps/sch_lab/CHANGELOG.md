# Changelog

## Development Build: v2.5.0-rc4+dev57
- Add support for cmds w/args
- See <https://github.com/nasa/sch_lab/pull/144>

## Development Build: 2.5.0-rc4+dev53
- adjust table based on apps present
- See <https://github.com/nasa/sch_lab/pull/139>

## Development Build: 2.5.0-rc4+dev49
- update cmake recipe
- See <https://github.com/nasa/sch_lab/pull/136>

## Development Build: 2.5.0-rc4+dev45
- Remove redundant comments
- See <https://github.com/nasa/sch_lab/pull/126>

## Development Build: v2.5.0-rc4+dev41
- Create CHANGELOG.md
- See <https://github.com/nasa/sch_lab/pull/123>

## Development Build: v2.5.0-rc4:dev35
- Remove unnecessary parentheses around return values.
- See <https://github.com/nasa/sch_lab/pull/121>

## Development Build: v2.5.0-rc4+dev31
- Update cmake mimimum required to something more recent
- See <https://github.com/nasa/sch_lab/pull/118>

## Development Build: v2.5.0-rc4+dev29
- Resolve uninit var static analysis warnings
- See <https://github.com/nasa/sch_lab/pull/116> and <https://github.com/nasa/cFS/pull/492>

## Development Build: v2.5.0-rc4+dev23
- Update Copyright Headers
- Standardize version.h 
- See <https://github.com/nasa/sch_lab/pull/> and <https://github.com/nasa/cFS/445>

## Development Build: v2.5.0-rc4+dev16
- Apply header guard standard
- See <https://github.com/nasa/cFS/pull/432>

## Development Build: v2.5.0-rc4+dev13
- Add option to configure base tick rate
- Reuse CodeQL, Static Analyis, and Format Check
- See <https://github.com/nasa/sch_lab/pull/105> and <https://github.com/nasa/cFS/pull/410>

## Development Build: v2.5.0-rc4+dev6
- Use separate address variable
- Use CFE_MSG_PTR conversion macro
- Update baseline for cFS-Caelum-rc4: v2.5.0-rc4
- See <https://github.com/nasa/sch_lab/pull/102> and <https://github.com/nasa/cFS/pull/390>

## Development Build: v2.4.0-rc1+dev53
- Apply CFE_SB_ValueToMsgId where required
- See <https://github.com/nasa/sch_lab/pull/92> and <https://github.com/nasa/cFS/pull/359>

## Development Build: v2.4.0-rc1+dev47
- Allow command codes to be specified in sch_lab_table #85, add CmdCode to sch_lab_table
- See <https://github.com/nasa/sch_lab/pull/87> and <https://github.com/nasa/cFS/pull/351>

## Development Build: v2.4.0-rc1+dev40
-  Implement Coding Standard in CodeQL workflow
- See <https://github.com/nasa/sch_lab/pull/83> and <https://github.com/nasa/cFS/pull/270>

## Development Build: v2.4.0-rc1+dev36
- Fix infinite loop by making sch_lab exit by making "RunStatus" return ERROR if initialization fails. This causes the `CFE_ES_RunLoop` function to return false.
- See <https://github.com/nasa/sch_lab/pull/82> and <https://github.com/nasa/cFS/pull/263/>

## Development Build: v2.4.0-rc1+dev32
- Removes app registration call, `CFE_ES_RegisterApp()` since applications do not need to register themselves.
- See <https://github.com/nasa/sch_lab/pull/76>

## Development Build: v2.4.0-rc1+dev26
- Update sequence count in transmitted messages
- Add Testing Tools to the Security Policy
- See <https://github.com/nasa/sch_lab/pull/71>

## Development Build: v2.4.0-rc1+dev12
- Aligns messages according to changes in cFE <https://github.com/nasa/cFE/issues/1009>. Uses the "raw" message cmd/tlm types in definition
- See <https://github.com/nasa/sch_lab/pull/59>

## Development Build: v2.4.0-rc1+dev9
- Update to use MSG module. Replaces deprecated SB APIs with MSG
- See <https://github.com/nasa/sch_lab/pull/58>

## Development Build: v2.4.0-rc1+dev6
- Adds header guard (the other warning on the ticket was already resolved)
- See <https://github.com/nasa/sch_lab/pull/53>

## Development Build: v2.4.0-rc1+dev2
- Reorganize the sch_lab table so it matches the sample_app usage and format.
- See <https://github.com/nasa/sch_lab/pull/52>

## Development Build: v2.3.0+dev37
- Fixes schedule table documentation
- Add baseline and build number to version reporting
- See <https://github.com/nasa/sch_lab/pull/48>

## Development Build: v2.3.7
- Apply the CFE_SB_MsgIdToValue() and CFE_SB_ValueToMsgId() routines where compatibility with an integer MsgId is necessary - syslog prints, events, compile-time MID #define values.
- See <https://github.com/nasa/sch_lab/pull/41>

## Development Build: v2.3.6
- Apply the CFE_SB_MsgIdToValue() and CFE_SB_ValueToMsgId() routines where compatibility with an integer MsgId is necessary - syslog prints, events, compile-time MID #define values.
- See <https://github.com/nasa/sch_lab/pull/39>

## Development Build: v2.3.5
- Improved table handling
- sch_lab now builds on Raspbian OS
- Minor updates (see <https://github.com/nasa/sch_lab/pull/36>)

## Development Build: v2.3.4
- Fix for clean build with OMIT_DEPRECATED
- Minor updates (see <https://github.com/nasa/sch_lab/pull/35>)

## Development Build: v2.3.3
- Minor updates (see <https://github.com/nasa/sch_lab/pull/28>)

## Development Build: v2.3.2
- Table definition include update (see <https://github.com/nasa/sch_lab/pull/18>)

## Development Build: v2.3.1
- Minor updates (see <https://github.com/nasa/sch_lab/pull/16>)

## _**OFFICIAL RELEASE: v2.3.0 - Aquila**_
- Minor updates (see <https://github.com/nasa/sch_lab/pull/13>)
- Not backwards compatible with OSAL 4.2.1
- Released as part of cFE 6.7.0, Apache 2.0

## _**OFFICIAL RELEASE: 2.2.0a**_
- Released as part of cFE 6.6.0a, Apache 2.0

## Known issues
As a lab application, extensive testing is not performed prior to release and only minimal functionality is included.

## Getting Help
For best results, submit issues:questions or issues:help wanted requests at <https://github.com/nasa/cFS>.

Official cFS page: <http://cfs.gsfc.nasa.gov>
