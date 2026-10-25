# Changelog

## Development Build: v2.5.0-rc4+dev43
- update cmake recipe
- See <https://github.com/nasa/ci_lab/pull/136>

## Development Build: v2.5.0-rc4+dev39
- Remove redundant comments
- Create CHANGELOG.md
- See <https://github.com/nasa/ci_lab/pull/123> and <https://github.com/nasa/ci_lab/pull/120>

## Development Build: v2.5.0-rc4+dev30
- Remove unnecessary parentheses around return values.
- Remove 'return;' from last line of void functions.
- See <https://github.com/nasa/ci_lab/pull/116> and <https://github.com/nasa/ci_lab/pull/118>

## Development Build: v2.5.0-rc4+dev24
- Update cmake mimimum required to something more recent
- See <https://github.com/nasa/ci_lab/pull/113>

## Development Build: v2.5.0-rc4+dev22
- Remove registration of empty EVS filters
- Update codeql workflow for reusable updates
- See <https://github.com/nasa/cFS/pull/505> 

## Development Build: v2.5.0-rc4+dev17
- Update Copyright Headers
- Standardize version.h 
- See <https://github.com/nasa/ci_lab/pull/107> and <https://github.com/nasa/cFS/445>

## Development Build: v2.5.0-rc4+dev10
- Apply header guard standard
- See <https://github.com/nasa/cFS/pull/432>

## Development Build: v2.5.0-rc4+dev4
- Use CFE_MSG_PTR conversion macro
- Update baseline for cFS-Caelum-rc4 to v2.5.0-rc4
- See <https://github.com/nasa/ci_lab/pull/101> and <https://github.com/nasa/cFS/pull/390>

## Development Build: v2.4.0-rc1+dev46
- Apply CFE_SB_ValueToMsgId where required
- See <https://github.com/nasa/ci_lab/pull/93> and <https://github.com/nasa/cFS/pull/359>

## Development Build: v2.4.0-rc1+dev42
- Implement Coding Standard in CodeQL workflow
- See <https://github.com/nasa/ci_lab/pull/88> and <https://github.com/nasa/cFS/pull/270>

## Development Build: v2.4.0-rc1+dev39
- Removes unnecessary call to `CFE_ES_RegisterApp()` since app registration is done automatically.
- Demonstrates the use of the Zero Copy API. Adds a step that obtains a buffer prior to calling `OS_SocketRecvFrom` then transmits that same buffer directly rather than copying it.
- See <https://github.com/nasa/ci_lab/pull/85>

## Development Build: v2.4.0-rc1+dev30
- Use `cfe.h` instead of individual headers
- See <https://github.com/nasa/ci_lab/pull/78>

## Development Build: v2.4.0-rc1+dev25
- Add Testing Tools to the Security Policy
- See <https://github.com/nasa/ci_lab/pull/76>

## Development Build: v2.4.0-rc1+dev14
- Aligns messages according to changes in cFE <https://github.com/nasa/cFE/issues/1009>. Uses the "raw" message cmd/tlm types in definition
- See <https://github.com/nasa/ci_lab/pull/65>

## Development Build: v2.4.0-rc1+dev8
- Replaces deprecated SB API's with MSG
- No behavior impact, removes undesirable pattern use of `OS_PACK`
- See <https://github.com/nasa/ci_lab/pull/60>

## Development Build: v2.4.0-rc1+dev2
- Update the SocketID field to be `osal_id_t` instead of uint32
- Set Revision number to 99 for development version identifier in telemetry
- See <https://github.com/nasa/ci_lab/pull/56>

## Development Build: v2.3.0+dev36
- Add build name and build number to version reporting
- See <https://github.com/nasa/ci_lab/pull/53>

## Development Build: v2.3.5
- Replace references to `ccsds.h` types with the `cfe_sb.h`-provided type.
- See <https://github.com/nasa/ci_lab/pull/50>

## Development Build: v2.3.4
- Apply the CFE_SB_MsgIdToValue() and CFE_SB_ValueToMsgId() routines where compatibility with an integer MsgId is necessary - syslog prints, events, compile-time MID #define values.
- Minor changes, see <https://github.com/nasa/ci_lab/pull/47>

## Development Build: v2.3.3
- Offset UDP base port in multi-cpu configurations
- Minor changes, see <https://github.com/nasa/ci_lab/pull/44>

## Development Build: v2.3.2
- Use OSAL socket API instead of BSD sockets
- Remove PDU introspection code
- Update command and telemetry logic
- Collect globals as a single top-level global structure
- Minor changes, see <https://github.com/nasa/ci_lab/pull/38>

## Development Build: v2.3.1
- Code style and enforcement (see <https://github.com/nasa/ci_lab/pull/31>)

## _**OFFICIAL RELEASE: v2.3.0 - Aquila**_
- Minor updates (see <https://github.com/nasa/ci_lab/pull/12>)
- Not backwards compatible with OSAL 4.2.1
- Released as part of cFE 6.7.0, Apache 2.0

## _**OFFICIAL RELEASE: v2.2.0a**_
- Released as part of cFE 6.6.0a, Apache 2.0

## Known issues
Dependence on cfe platform config header is undesirable, and the check is not endian safe. As a lab application, extensive testing is not performed prior to release and only minimal functionality is included.

## Getting Help
For best results, submit issues:questions or issues:help wanted requests at <https://github.com/nasa/cFS>.

Official cFS page: <http://cfs.gsfc.nasa.gov>
