/* 
 * Float point related constants,definitions,routine protypes,and other structures
 * or functions.
 */

#ifndef __FLOAT_H__
#define __FLOAT_H__

 //-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 //
 // Constants
 //
 //-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#define DBL_DECIMAL_DIG  17                      // # of decimal digits of rounding precision
#define DBL_DIG          15                      // # of decimal digits of precision
#define DBL_EPSILON      2.2204460492503131e-016 // smallest such that 1.0+DBL_EPSILON != 1.0
#define DBL_HAS_SUBNORM  1                       // type does support subnormal numbers
#define DBL_MANT_DIG     53                      // # of bits in mantissa
#define DBL_MAX          1.7976931348623158e+308 // max value
#define DBL_MAX_10_EXP   308                     // max decimal exponent
#define DBL_MAX_EXP      1024                    // max binary exponent
#define DBL_MIN          2.2250738585072014e-308 // min positive value
#define DBL_MIN_10_EXP   (-307)                  // min decimal exponent
#define DBL_MIN_EXP      (-1021)                 // min binary exponent
#define _DBL_RADIX       2                       // exponent radix
#define DBL_TRUE_MIN     4.9406564584124654e-324 // min positive value

#define FLT_DECIMAL_DIG  9                       // # of decimal digits of rounding precision
#define FLT_DIG          6                       // # of decimal digits of precision
#define FLT_EPSILON      1.192092896e-07F        // smallest such that 1.0+FLT_EPSILON != 1.0
#define FLT_HAS_SUBNORM  1                       // type does support subnormal numbers
#define FLT_GUARD        0
#define FLT_MANT_DIG     24                      // # of bits in mantissa
#define FLT_MAX          3.402823466e+38F        // max value
#define FLT_MAX_10_EXP   38                      // max decimal exponent
#define FLT_MAX_EXP      128                     // max binary exponent
#define FLT_MIN          1.175494351e-38F        // min normalized positive value
#define FLT_MIN_10_EXP   (-37)                   // min decimal exponent
#define FLT_MIN_EXP      (-125)                  // min binary exponent
#define FLT_NORMALIZE    0
#define FLT_RADIX        2                       // exponent radix
#define FLT_TRUE_MIN     1.401298464e-45F        // min positive value

#define LDBL_DIG         DBL_DIG                 // # of decimal digits of precision
#define LDBL_EPSILON     DBL_EPSILON             // smallest such that 1.0+LDBL_EPSILON != 1.0
#define LDBL_HAS_SUBNORM DBL_HAS_SUBNORM         // type does support subnormal numbers
#define LDBL_MANT_DIG    DBL_MANT_DIG            // # of bits in mantissa
#define LDBL_MAX         DBL_MAX                 // max value
#define LDBL_MAX_10_EXP  DBL_MAX_10_EXP          // max decimal exponent
#define LDBL_MAX_EXP     DBL_MAX_EXP             // max binary exponent
#define LDBL_MIN         DBL_MIN                 // min normalized positive value
#define LDBL_MIN_10_EXP  DBL_MIN_10_EXP          // min decimal exponent
#define LDBL_MIN_EXP     DBL_MIN_EXP             // min binary exponent
#define _LDBL_RADIX      _DBL_RADIX              // exponent radix
#define LDBL_TRUE_MIN    DBL_TRUE_MIN            // min positive value

#define DECIMAL_DIG      DBL_DECIMAL_DIG

 //-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 //
 // Flags
 //
 //-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#define _SW_INEXACT     0x00000001              // Inexact (precision)
#define _SW_UNDERFLOW   0x00000002              // Underflow
#define _SW_OVERFLOW    0x00000004              // Overflow
#define _SW_ZERODIVIDE  0x00000008              // Divide by zero
#define _SW_INVALID     0x00000010              // Invalid
#define _SW_DENORMAL    0x00080000              // Denormal status bit

 // New Control Bit that specifies the ambiguity in control word.
#define _EM_AMBIGUIOUS  0x80000000 // For backwards compatibility
#define _EM_AMBIGUOUS   0x80000000

 // Abstract User Control Word Mask and bit definitions
#define _MCW_EM         0x0008001f              // Interrupt Exception Masks
#define _EM_INEXACT     0x00000001              //     inexact (precision)
#define _EM_UNDERFLOW   0x00000002              //     underflow
#define _EM_OVERFLOW    0x00000004              //     overflow
#define _EM_ZERODIVIDE  0x00000008              //     zero divide
#define _EM_INVALID     0x00000010              //     invalid
#define _EM_DENORMAL    0x00080000              // Denormal exception mask (_control87 only)

#define _MCW_RC         0x00000300              // Rounding Control
#define _RC_NEAR        0x00000000              //     near
#define _RC_DOWN        0x00000100              //     down
#define _RC_UP          0x00000200              //     up
#define _RC_CHOP        0x00000300              //     chop

 // i386 specific definitions
#define _MCW_PC         0x00030000              // Precision Control
#define _PC_64          0x00000000              //     64 bits
#define _PC_53          0x00010000              //     53 bits
#define _PC_24          0x00020000              //     24 bits

#define _MCW_IC         0x00040000              // Infinity Control
#define _IC_AFFINE      0x00040000              //     affine
#define _IC_PROJECTIVE  0x00000000              //     projective

 // RISC specific definitions
#define _MCW_DN         0x03000000              // Denormal Control
#define _DN_SAVE        0x00000000              //   save denormal results and operands
#define _DN_FLUSH       0x01000000              //   flush denormal results and operands to zero
#define _DN_FLUSH_OPERANDS_SAVE_RESULTS 0x02000000  // flush operands to zero and save results
#define _DN_SAVE_OPERANDS_FLUSH_RESULTS 0x03000000  // save operands and flush results to zero

 // Invalid subconditions (_SW_INVALID also set)
#define _SW_UNEMULATED          0x0040  // Unemulated instruction
#define _SW_SQRTNEG             0x0080  // Square root of a negative number
#define _SW_STACKOVERFLOW       0x0200  // FP stack overflow
#define _SW_STACKUNDERFLOW      0x0400  // FP stack underflow

 // Floating point error signals and return codes
#define _FPE_INVALID            0x81
#define _FPE_DENORMAL           0x82
#define _FPE_ZERODIVIDE         0x83
#define _FPE_OVERFLOW           0x84
#define _FPE_UNDERFLOW          0x85
#define _FPE_INEXACT            0x86

#define _FPE_UNEMULATED         0x87
#define _FPE_SQRTNEG            0x88
#define _FPE_STACKOVERFLOW      0x8a
#define _FPE_STACKUNDERFLOW     0x8b

#define _FPE_EXPLICITGEN        0x8c // raise(SIGFPE);

 // On x86 with arch:SSE2, the OS returns these exceptions
#define _FPE_MULTIPLE_TRAPS     0x8d
#define _FPE_MULTIPLE_FAULTS    0x8e

#define _FPCLASS_SNAN  0x0001  // signaling NaN
#define _FPCLASS_QNAN  0x0002  // quiet NaN
#define _FPCLASS_NINF  0x0004  // negative infinity
#define _FPCLASS_NN    0x0008  // negative normal
#define _FPCLASS_ND    0x0010  // negative denormal
#define _FPCLASS_NZ    0x0020  // -0
#define _FPCLASS_PZ    0x0040  // +0
#define _FPCLASS_PD    0x0080  // positive denormal
#define _FPCLASS_PN    0x0100  // positive normal
#define _FPCLASS_PINF  0x0200  // positive infinity

#endif //__FLOAT_H__
