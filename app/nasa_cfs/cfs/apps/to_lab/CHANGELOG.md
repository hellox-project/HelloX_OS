# Changelog

## Development Build: v2.5.0-rc4+dev45
- adjust subscription table based on apps present
- See <https://github.com/nasa/to_lab/pull/153>

## Development Build: v2.5.0-rc4+dev41
- update cmake recipe
- See <https://github.com/nasa/to_lab/pull/150>

## Development Build: v2.5.0-rc4+dev35
- Adds format changes in to_lab_app.c
- See <https://github.com/nasa/to_lab/pull/142>

## Development Build: v2.5.0-rc4+dev31
- Remove unused PktSize variable.
- Standardize naming of TO_LAB functions/macros etc.
- Create CHANGELOG.md
- See <https://github.com/nasa/to_lab/pull/127>, <https://github.com/nasa/to_lab/pull/126>, and <https://github.com/nasa/to_lab/pull/129>

## Development Build: v2.5.0-rc4+dev23
- Update cmake mimimum required to something more recent
- See <https://github.com/nasa/to_lab/pull/124>

## Development Build: v2.5.0-rc4+dev21

- Remove registration of empty EVS filters
- Update codeql workflow for reusable updates
- See <https://github.com/nasa/cFS/pull/505>


## Development Build: v2.5.0-rc4+dev16

- Update Copyright Headers
- Standardize version.h 
- See <https://github.com/nasa/to_lab/pull/119> and <https://github.com/nasa/cFS/445>

## Development Build: v2.5.0-rc4+dev9

- Apply header guard standard
- See <https://github.com/nasa/cFS/pull/432>

## Development Build: v2.5.0-rc4+dev4

- Use CFE_MSG_PTR conversion macro
- Set new baseline for cFS-Caelum-rc4: v2.5.0-rc4
- See <https://github.com/nasa/to_lab/pull/112> and <https://github.com/nasa/cFS/pull/390>

## Development Build: v2.4.0-rc1+dev58

- Apply CFE_SB_ValueToMsgId where required
- See <https://github.com/nasa/to_lab/pull/105> and <https://github.com/nasa/cFS/pull/359>

## Development Build: v2.4.0-rc1+dev49

-  Implement Coding Standard in CodeQL workflow
- See <https://github.com/nasa/to_lab/pull/100> and <https://github.com/nasa/cFS/pull/270>

## Development Build: v2.4.0-rc1+dev47

- Removes app registration call, `CFE_ES_RegisterApp()` since applications do not need to register themselves.
- See <https://github.com/nasa/to_lab/pull/97>

## Development Build: v2.4.0-rc1+dev41

-  Use `cfe.h` header file
- See <https://github.com/nasa/to_lab/pull/91>

## Development Build: v2.4.0-rc1+dev38

- Remove numeric pipe ID from event printf
- Add Testing Tools to the Security Policy
- See <https://github.com/nasa/to_lab/pull/89>

## Development Build: v.2.4.0-rc1+dev32

- Removes end-of-function comments in `to_lab_app.c`
- Adds static analysis and code format check to continuous integration workflow. Updates workflow status badges in ReadMe
- Adds CodeQL analysis to continuous integration workflow
- See <https://github.com/nasa/to_lab/pull/84>

## Development Build: v.2.4.0-rc1+dev21

- TO remains command-able after a "remove all subscriptions" command; the command now only removes all subscriptions to the Tlm_pipe
- See <https://github.com/nasa/to_lab/pull/75>

## Development Build: v.2.4.0-rc1+dev17

- Aligns messages according to changes in cFE <https://github.com/nasa/cFE/issues/1009>. Uses the "raw" message cmd/tlm types in definition
- See <https://github.com/nasa/to_lab/pull/70>

## Development Build: v.2.4.0-rc1+dev13

- Replaces deprecated SB API's with MSG
- See <https://github.com/nasa/to_lab/pull/65>

## Development Build: v.2.4.0-rc1+dev9

- Update the TLMsockid field to be `osal_id_t` instead of uint32
- Set revision number to 99 to indicate development status in telemetry
- See <https://github.com/nasa/to_lab/pull/59>

## Development Build: v.2.4.0-rc1+dev6

- Adds header guard to `to_lab_sub_table.h`
- See <https://github.com/nasa/to_lab/pull/59>

## Development Build: v.2.4.0-rc1+dev3

- Remove reference to deprecated `CFE_ES_SHELL_TLM_MID`.
- See <https://github.com/nasa/to_lab/pull/58>

## Development Build: v2.3.0+dev45

- Fixes bug where an unset address values caused subscriptions to MsgId 0 over 200 times. Added a `TO_UNUSED` entry at the end of the subscription list and a break in the subscription loop when `TO_UNUSED` found. No more subscriptions on the unused table slots (no MsgId 0 subscriptions).
- Corrects return value of `TO_LAB_init()` to be `int32` instead of `int`. Declaration now matches definition, and app builds without errors.
- Add build number and baseline to version reporting.
- See <https://github.com/nasa/to_lab/pull/53>

## Development Build: v2.3.7

- Makes the `TO_LAB_Subs` table into a CFE_TBL-managed table.
- See <https://github.com/nasa/to_lab/pull/46>


## Development Build: v2.3.6

- Replace references to `ccsds.h` types with the `cfe_sb.h`-provided type.
- See <https://github.com/nasa/to_lab/pull/44>

## Development Build: v2.3.5

- Apply code style
- See <https://github.com/nasa/to_lab/pull/43>

## Development Build: v2.3.4

- Configure the maximum depth supported by OSAL, rather than a hard coded 64.
- See <https://github.com/nasa/to_lab/pull/39>

## Development Build: v2.3.3

- Apply the CFE_SB_MsgIdToValue() and CFE_SB_ValueToMsgId() routines where compatibility with an integer MsgId is necessary - syslog prints, events, compile-time MID #define values.
- Deprecates shell tlm subscription
- Changes to documentation
- See <https://github.com/nasa/to_lab/pull/38>

## Development Build: v2.3.2

- Use OSAL socket API instead of BSD Sockets

- Use global namespace to isolate variables

- Minor updates (see <https://github.com/nasa/to_lab/pull/27>)

## Development Build: v2.3.1

- Fix for a clean build with OMIT_DEPRECATED
- Minor updates (see <https://github.com/nasa/to_lab/pull/26>)

## _**OFFICIAL RELEASE: 2.3.0 - Aquila**_

- Minor updates (see <https://github.com/nasa/to_lab/pull/13>)

- Not backwards compatible with OSAL 4.2.1

- Released as part of cFE 6.7.0, Apache 2.0

## _**OFFICIAL RELEASE: 2.2.0a**_

- Released as part of cFE 6.6.0a, Apache 2.0

## Known issues

As a lab application, extensive testing is not performed prior to release and only minimal functionality is included.

## Getting Help

For best results, submit issues:questions or issues:help wanted requests at <https://github.com/nasa/cFS>.

Official cFS page: <http://cfs.gsfc.nasa.gov>
