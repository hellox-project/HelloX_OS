//***********************************************************************/
//    Author                    : Garry&AI
//    Original Date             : Apr 12, 2026
//    Module Name               : bitscan_ai.h
//    Module Funciton           : 
//                                Pure C implementation of BitScan functions.
//                                No compiler builtins used.
//                                Implements MSVC-style BitScan functions.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                
//    Lines number              :
//***********************************************************************/

#ifndef __BITSCAN_AI_H__
#define __BITSCAN_AI_H__

#include <stdint.h>
#include <stdbool.h>

/* Use fixed-width types */
typedef uint32_t bitscan32_t;
typedef uint64_t bitscan64_t;

/* ============================================== */
/* Helper Functions                               */
/* ============================================== */

/**
 * @brief Count trailing zeros (find first set bit) - 32-bit
 * @param x 32-bit value
 * @return Number of trailing zeros (0-32), 32 if x is 0
 */
static inline unsigned int ctz32(bitscan32_t x) {
	if (x == 0) return 32;

	/* Isolate the lowest set bit: x & -x */
	bitscan32_t lowest = x & -x;

	/* Use De Bruijn sequence method for fast lookup */
	static const unsigned int deBruijn32[32] = {
		0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
		31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
	};

	return deBruijn32[((bitscan32_t)(lowest * 0x077CB531U)) >> 27];
}

/**
 * @brief Count leading zeros (find last set bit) - 32-bit
 * @param x 32-bit value
 * @return Number of leading zeros (0-32), 32 if x is 0
 */
static inline unsigned int clz32(bitscan32_t x) {
	if (x == 0) return 32;

	/* Count leading zeros using binary search */
	unsigned int n = 0;
	if (x <= 0x0000FFFF) { n += 16; x <<= 16; }
	if (x <= 0x00FFFFFF) { n += 8;  x <<= 8; }
	if (x <= 0x0FFFFFFF) { n += 4;  x <<= 4; }
	if (x <= 0x3FFFFFFF) { n += 2;  x <<= 2; }
	if (x <= 0x7FFFFFFF) { n += 1;  x <<= 1; }

	return n;
}

/**
 * @brief Count trailing zeros (find first set bit) - 64-bit
 * @param x 64-bit value
 * @return Number of trailing zeros (0-64), 64 if x is 0
 */
static inline unsigned int ctz64(bitscan64_t x) {
	if (x == 0) return 64;

	/* Check lower 32 bits first */
	bitscan32_t low = (bitscan32_t)x;
	if (low != 0) {
		return ctz32(low);
	}

	/* Check upper 32 bits */
	bitscan32_t high = (bitscan32_t)(x >> 32);
	return 32 + ctz32(high);
}

/**
 * @brief Count leading zeros (find last set bit) - 64-bit
 * @param x 64-bit value
 * @return Number of leading zeros (0-64), 64 if x is 0
 */
static inline unsigned int clz64(bitscan64_t x) {
	if (x == 0) return 64;

	/* Check upper 32 bits first */
	bitscan32_t high = (bitscan32_t)(x >> 32);
	if (high != 0) {
		return clz32(high);
	}

	/* Check lower 32 bits */
	bitscan32_t low = (bitscan32_t)x;
	return 32 + clz32(low);
}

/**
 * @brief Population count (number of set bits) - 32-bit
 * @param x 32-bit value
 * @return Number of set bits (0-32)
 */
static inline unsigned int popcount32(bitscan32_t x) {
	/* Hamming weight algorithm */
	x = x - ((x >> 1) & 0x55555555);
	x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
	x = (x + (x >> 4)) & 0x0F0F0F0F;
	x = (x * 0x01010101) >> 24;
	return (unsigned int)x;
}

/**
 * @brief Population count (number of set bits) - 64-bit
 * @param x 64-bit value
 * @return Number of set bits (0-64)
 */
static inline unsigned int popcount64(bitscan64_t x) {
	/* 64-bit Hamming weight */
	x = x - ((x >> 1) & 0x5555555555555555ULL);
	x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
	x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
	x = (x * 0x0101010101010101ULL) >> 56;
	return (unsigned int)x;
}

/* ============================================== */
/* Basic BitScan Functions (32-bit)               */
/* ============================================== */

/**
 * @brief Find the index of the first set bit (LSB) in a 32-bit value
 * @param index Pointer to store the bit index (0-31)
 * @param mask 32-bit value to scan
 * @return true if a set bit was found, false if mask is zero
 */
static inline bool BitScanForward32(unsigned long* index, bitscan32_t mask) {
	if (mask == 0) {
		return false;
	}
	*index = (unsigned long)ctz32(mask);
	return true;
}

/**
 * @brief Find the index of the last set bit (MSB) in a 32-bit value
 * @param index Pointer to store the bit index (0-31)
 * @param mask 32-bit value to scan
 * @return true if a set bit was found, false if mask is zero
 */
static inline bool BitScanReverse32(unsigned long* index, bitscan32_t mask) {
	if (mask == 0) {
		return false;
	}
	*index = (unsigned long)(31 - clz32(mask));
	return true;
}

/* ============================================== */
/* 64-bit BitScan Functions                       */
/* ============================================== */

/**
 * @brief Find the index of the first set bit (LSB) in a 64-bit value
 * @param index Pointer to store the bit index (0-63)
 * @param mask 64-bit value to scan
 * @return true if a set bit was found, false if mask is zero
 */
static inline bool BitScanForward64(unsigned long* index, bitscan64_t mask) {
	if (mask == 0) {
		return false;
	}
	*index = (unsigned long)ctz64(mask);
	return true;
}

/**
 * @brief Find the index of the last set bit (MSB) in a 64-bit value
 * @param index Pointer to store the bit index (0-63)
 * @param mask 64-bit value to scan
 * @return true if a set bit was found, false if mask is zero
 */
static inline bool BitScanReverse64(unsigned long* index, bitscan64_t mask) {
	if (mask == 0) {
		return false;
	}
	*index = (unsigned long)(63 - clz64(mask));
	return true;
}

/* ============================================== */
/* MSVC-compatible Interface                      */
/* ============================================== */

#ifdef _WIN32
/* On Windows, match MSVC's _BitScanForward/_BitScanReverse */
#define _BitScanForward  BitScanForward32
#define _BitScanReverse  BitScanReverse32
#define _BitScanForward64 BitScanForward64
#define _BitScanReverse64 BitScanReverse64
#endif

/* Generic macros that work on any platform */
#define BitScanForward  BitScanForward32
#define BitScanReverse  BitScanReverse32

/* ============================================== */
/* Additional Bit Manipulation Functions          */
/* ============================================== */

/**
 * @brief Count the number of set bits (population count) in a 32-bit value
 * @param x 32-bit value
 * @return Number of set bits (0-32)
 */
static inline unsigned int BitCount32(bitscan32_t x) {
	return popcount32(x);
}

/**
 * @brief Count the number of set bits (population count) in a 64-bit value
 * @param x 64-bit value
 * @return Number of set bits (0-64)
 */
static inline unsigned int BitCount64(bitscan64_t x) {
	return popcount64(x);
}

/**
 * @brief Find the index of the first clear bit (LSB) in a 32-bit value
 * @param index Pointer to store the bit index (0-31)
 * @param mask 32-bit value to scan
 * @return true if a clear bit was found, false if mask is all ones
 */
static inline bool BitScanForwardClear32(unsigned long* index, bitscan32_t mask) {
	bitscan32_t inverted = ~mask;
	if (inverted == 0) {
		return false;
	}
	*index = (unsigned long)ctz32(inverted);
	return true;
}

/**
 * @brief Find the index of the last clear bit (MSB) in a 32-bit value
 * @param index Pointer to store the bit index (0-31)
 * @param mask 32-bit value to scan
 * @return true if a clear bit was found, false if mask is all ones
 */
static inline bool BitScanReverseClear32(unsigned long* index, bitscan32_t mask) {
	bitscan32_t inverted = ~mask;
	if (inverted == 0) {
		return false;
	}
	*index = (unsigned long)(31 - clz32(inverted));
	return true;
}

/**
 * @brief Rotate a 32-bit value left by n bits
 * @param x Value to rotate
 * @param n Number of bits to rotate (0-31)
 * @return Rotated value
 */
static inline bitscan32_t RotateLeft32(bitscan32_t x, unsigned int n) {
	n &= 31;
	if (n == 0) return x;
	return (x << n) | (x >> (32 - n));
}

/**
 * @brief Rotate a 32-bit value right by n bits
 * @param x Value to rotate
 * @param n Number of bits to rotate (0-31)
 * @return Rotated value
 */
static inline bitscan32_t RotateRight32(bitscan32_t x, unsigned int n) {
	n &= 31;
	if (n == 0) return x;
	return (x >> n) | (x << (32 - n));
}

/**
 * @brief Test if a specific bit is set in a 32-bit value
 * @param x Value to test
 * @param bit Bit position to test (0-31)
 * @return true if bit is set, false otherwise
 */
static inline bool TestBit32(bitscan32_t x, unsigned int bit) {
	return (x & ((bitscan32_t)1 << bit)) != 0;
}

/**
 * @brief Set a specific bit in a 32-bit value
 * @param x Value to modify
 * @param bit Bit position to set (0-31)
 * @return Value with the bit set
 */
static inline bitscan32_t SetBit32(bitscan32_t x, unsigned int bit) {
	return x | ((bitscan32_t)1 << bit);
}

/**
 * @brief Clear a specific bit in a 32-bit value
 * @param x Value to modify
 * @param bit Bit position to clear (0-31)
 * @return Value with the bit cleared
 */
static inline bitscan32_t ClearBit32(bitscan32_t x, unsigned int bit) {
	return x & ~((bitscan32_t)1 << bit);
}

/**
 * @brief Toggle a specific bit in a 32-bit value
 * @param x Value to modify
 * @param bit Bit position to toggle (0-31)
 * @return Value with the bit toggled
 */
static inline bitscan32_t ToggleBit32(bitscan32_t x, unsigned int bit) {
	return x ^ ((bitscan32_t)1 << bit);
}

/* ============================================== */
/* Bit Field Operations                           */
/* ============================================== */

/**
 * @brief Extract a bit field from a 32-bit value
 * @param x Source value
 * @param start Starting bit position (0-31)
 * @param length Length of bit field (1-32)
 * @return Extracted bit field
 */
static inline bitscan32_t ExtractBitField32(bitscan32_t x,
	unsigned int start,
	unsigned int length) {
	if (length >= 32) return x;
	bitscan32_t mask = ((bitscan32_t)1 << length) - 1;
	return (x >> start) & mask;
}

/**
 * @brief Insert a bit field into a 32-bit value
 * @param x Destination value
 * @param field Value to insert
 * @param start Starting bit position (0-31)
 * @param length Length of bit field (1-32)
 * @return Value with bit field inserted
 */
static inline bitscan32_t InsertBitField32(bitscan32_t x,
	bitscan32_t field,
	unsigned int start,
	unsigned int length) {
	if (length >= 32) return field;
	bitscan32_t mask = ((bitscan32_t)1 << length) - 1;
	x &= ~(mask << start);
	x |= (field & mask) << start;
	return x;
}

/* ============================================== */
/* Utility Functions                              */
/* ============================================== */

/**
 * @brief Check if a value is a power of two
 * @param x Value to check
 * @return true if x is a power of two, false otherwise
 */
static inline bool IsPowerOfTwo32(bitscan32_t x) {
	return x != 0 && (x & (x - 1)) == 0;
}

/**
 * @brief Round up to the next power of two
 * @param x Value to round up
 * @return Next power of two (or 0 if x is 0)
 */
static inline bitscan32_t NextPowerOfTwo32(bitscan32_t x) {
	if (x == 0) return 1;
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}

/**
 * @brief Get the mask for the lowest set bit
 * @param x Value to process
 * @return Mask with only the lowest set bit
 */
static inline bitscan32_t LowestSetBitMask32(bitscan32_t x) {
	return x & -x;
}

/**
 * @brief Clear the lowest set bit
 * @param x Value to process
 * @return Value with lowest set bit cleared
 */
static inline bitscan32_t ClearLowestSetBit32(bitscan32_t x) {
	return x & (x - 1);
}

/**
 * @brief Simple implementation of ctz32 using loop (for clarity)
 * @param x 32-bit value
 * @return Number of trailing zeros
 */
static inline unsigned int ctz32_simple(bitscan32_t x) {
	if (x == 0) return 32;

	unsigned int count = 0;
	while ((x & 1) == 0) {
		x >>= 1;
		count++;
	}
	return count;
}

/**
 * @brief Simple implementation of clz32 using loop (for clarity)
 * @param x 32-bit value
 * @return Number of leading zeros
 */
static inline unsigned int clz32_simple(bitscan32_t x) {
	if (x == 0) return 32;

	unsigned int count = 0;
	while ((x & 0x80000000) == 0) {
		x <<= 1;
		count++;
	}
	return count;
}

#endif /* __BITSCAN_AI_H__ */
