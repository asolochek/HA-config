/**
*   @file util.h
*
*   @date 2/3/2014 3:21:28 PM
*   @author Aaron Solochek
*
*   @brief General utility macros, definitions, and inline functions
*/


#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>

#ifndef M_PI
#define M_PI 3.141592653
#endif

#ifdef __GNUC__
#include <limits.h>
#else // IAR
#define UINT_MAX __UNSIGNED_INT_MAX__
#endif // __GNUC__

 /// @return the number of elements in array a
#define NELEM(a) sizeof(a)/sizeof(a[0])

/// Compile time ASSERT to check the size of array is at least size s.
#define ASSERT_STRUCT_SIZE(a, s) typedef char __tmp_struct[( !!(sizeof(a) <= s) )*2-1 ]

///
/// Bit twiddling macros
///
#ifndef _BV
#define _BV(b)                  (1UL << b) ///< Return a number with the bth bit set
#endif
#define SET_B(r, b)             (r |= _BV(b)) ///< Set bit \#b in location r
#define CLEAR_B(r, b)           (r &= ~_BV(b)) ///< Clear bit \#b in location r

#define SET_M(r, m)             (r |= m) ///< Set bit in r specified by bitmask m
#define CLEAR_M(r, m)           (r &= ~m) ///< Clear bit in r specified by bitmask m

#define BIT_IS_SET(r, b)        ((r & _BV(b)) ? 1 : 0) ///< True if bit \#b is set at location r
#define BIT_IS_CLEAR(r, b)      ((r & _BV(b)) ? 0 : 1) ///< True if bit \#b is clear at location r

#define BITS_ARE_SET(r, m)      ((r & m) ? 1 : 0) ///< True if any bits in m are set in r
#define BITS_ARE_CLEAR(r, m)    ((r & m) ? 0 : 1) ///< True if no bits of m are set in r

#define HIGH_BYTE(w)            ((w >> 8) & 0xFF) ///< Evaluates to the high byte of a 16bit value
#define LOW_BYTE(w)             (w & 0xFF) ///< Evaluates to the low byte of a 16bit value


///
/// Byte swapping functions
///

/// Byte swap unsigned short
static inline uint16_t swap_uint16(uint16_t val) {
    return (val << 8) | (val >> 8 );
}

/// Byte swap short
static inline int16_t swap_int16(int16_t val) {
    return (val << 8) | ((val >> 8) & 0xFF);
}

/// Byte swap unsigned int
static inline uint32_t swap_uint32(uint32_t val) {
    val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF ); 
    return (val << 16) | (val >> 16);
}

/// Byte swap int
static inline int32_t swap_int32(int32_t val) {
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF ); 
    return (val << 16) | ((val >> 16) & 0xFFFF);
}


/// Null register reads are handled differently on GCC and IAR
#ifdef __GNUC__
#define READ_REG16(reg)          (void) *(volatile uint16_t*) &reg
#else
extern volatile uint16_t dummy;
#define READ_REG16(reg)          dummy = reg
#endif



/// Evaluates to the largest unsigned value that can fit in t
#define U_MAX(t)                (((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | \
                                 (0xFULL << ((sizeof(t) * 8ULL) - 4ULL)))

/// Evaluates to the largest signed value that can fit in t
#define S_MAX(t)                (((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | \
                                 (0x7ULL << ((sizeof(t) * 8ULL) - 4ULL)))

// Cause a compile error if e is not 0
//#define ERROR_IF_NON_ZERO(e) (sizeof(struct { int:-!!(e); }))
//#define ERROR_IF_NON_NULL(e, m) ((void *)sizeof(struct { int:-!!(e); }))

#define ERROR_IF_ZERO(e, m)     (sizeof(struct {int m:e; })) ///< Cause a compiler error if e is 0
#define ERROR_IF_NULL(e, m)     ((void *)sizeof(struct {int m:e; })) ///< Cause a compiler error if e is NULL

/// Cause a compiler error if the unsigned value e cannot fit in storage of type v
#define U_BOUNDS_CHECK(e,v)     (ERROR_IF_ZERO(!(e > U_MAX(v)), value_out_of_bounds) ? e : e)

/// Cause a compiler error if the signed value e cannot fit in storage of type v
#define S_BOUNDS_CHECK(e,v)     (ERROR_IF_ZERO(!(e > S_MAX(v)), value_out_of_bounds) ? e : e)

#define STR(x) #x

//////////////////////////////////////////////////////////////////////////
/// Enumeration for a fake BOOL type.
/// This 'type' has an undefined storage size
typedef enum {
    FALSE = 0,
    TRUE
} BOOL;


#define A_TO_I(x)               ((x)-'0') ///< Convert a decimal ascii character to it's numeric value
#define I_TO_A(x)               ((x)+'0') ///< Convert a numeric value less than 10 to it's ascii character


//////////////////////////////////////////////////////////////////////////
/// Get the ASCII hex digit representation of a nibble
/// @param[in]  b   Value to convert
/// @return     ASCII character hex digit
static inline char NibbleToAscii(uint8_t b) {
    b &= 0x0F;
    if(b < 10) {
        return b + '0';
    }
    return b - 10 + 'A';
}


//////////////////////////////////////////////////////////////////////////
/// Get the ASCII hex digit representation of an 8 bit value
/// @param[in]  n   Value to represent
/// @param[out] str Buffer to store the 2 ASCII digits in
static inline void Int8toAscii(uint8_t n, char *str) {
    str[0] = NibbleToAscii(n >> 4);
    str[1] = NibbleToAscii(n);
}


//////////////////////////////////////////////////////////////////////////
/// Get the ASCII hex digit representation of a 16 bit value
/// @param[in]  n   Value to represent
/// @param[out] str Buffer to store the 4 ASCII digits in
static inline void Int16toAscii(uint16_t n, char *str) {
    str[0] = NibbleToAscii(n >> 12);
    str[1] = NibbleToAscii(n >> 8);
    str[2] = NibbleToAscii(n >> 4);
    str[3] = NibbleToAscii(n);
}


//////////////////////////////////////////////////////////////////////////
/// Reverse the the bytes in the 16-bit word
/// @param[in]  hword word in which to reverse the bytes
/// @return     uint16_t with bytes swapped
static inline uint16_t SwapBytes16(uint16_t hword) {
    return (hword >> 8) | ((hword & 0xFF) << 8);
}


#ifndef htonl // thanks, Darwin
static inline uint32_t htonl(uint32_t x) {
    return ((x >> 24) &       0xff) |
           ((x >> 8 ) &     0xff00) |
           ((x << 8 ) &   0xff0000) |
           ((x << 24) & 0xff000000);
}
#endif


#ifndef ntohl // thanks, Darwin
static inline uint32_t ntohl(uint32_t x) {
    return htonl(x);
}
#endif


//////////////////////////////////////////////////////////////////////////
/// Read a 16 bit value out of a buffer
/// @param[in]  buf     Buffer containing the value to read
/// @param[in]  offset  Index into the buffer to read the value from
/// @return     Unsigned 16 bit value
static inline uint16_t Read16(char *buf, uint8_t offset) {
    union {
        uint16_t x;
        struct {
            uint8_t a;
            uint8_t b;
        };
    } out;

    out.b = buf[offset];
    out.a = buf[offset+1];
    return out.x;
}


//////////////////////////////////////////////////////////////////////////
/// Read a 32bit value out of a buffer
/// @param[in]  buf     Buffer containing the value to read
/// @param[in]  offset  Index into the buffer to read the value from
/// @return     Unsigned 32 bit value
static inline uint32_t Read32(char *buf, uint8_t offset) {
    union {
        uint32_t x;
        struct {
            uint8_t a;
            uint8_t b;
            uint8_t c;
            uint8_t d;
        };
    } out;

    out.d = buf[offset];
    out.c = buf[offset+1];
    out.b = buf[offset+2];
    out.a = buf[offset+3];
    return out.x;
}


//////////////////////////////////////////////////////////////////////////
/// Find the minimum of two unsigned numbers
/// @param[in]  x   First number to compare
/// @param[in]  y   Second number to compare
/// @return     The smaller of the two input numbers
static inline uint32_t min(uint32_t x, uint32_t y) {
    return (x<y) ? x : y;
}


//////////////////////////////////////////////////////////////////////////
/// Find the maximum of two unsigned numbers
/// @param[in]  x   First number to compare
/// @param[in]  y   Second number to compare
/// @return     The larger of the two input numbers
static inline uint32_t max(uint32_t x, uint32_t y) {
    return (x>y) ? x : y;
}


//////////////////////////////////////////////////////////////////////////
/// Determine if a byte is a printable ASCII character
/// @param[in]  c   Byte to consider
/// @return     1 if A-Za-z0-9, 0 otherwise
static inline int myisalnum(char c) {
    return ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9'));
}


//////////////////////////////////////////////////////////////////////////
/// Determine the index value of bitmask
/// @param[in]  bm  Bitmask with a single bit set
/// @return     Index of the set bit
static inline uint8_t maskToIndex(unsigned bm) {
    unsigned r = 0;
    while (bm >>= 1) {
        r++;
    }
    return r;
}


//////////////////////////////////////////////////////////////////////////
/// Round a number up to the nearest multiple of another number
/// @param[in]  mumber   Number to be rounded
/// @param[in]  multiple Multiple of which the final number will be
/// @return     number rounded up to the nearest multiple of multiple
static inline uint32_t roundUp32(uint32_t number, uint32_t multiple) {
    uint32_t remainder;
    if (multiple == 0) {
        return number;
    }

    remainder = number % multiple;

    if (remainder == 0) {
        return number;
    }
    return number + multiple - remainder;
}


//////////////////////////////////////////////////////////////////////////
/// Round a number to the nearest multiple of another number
/// @param[in]  mumber   Number to be rounded
/// @param[in]  multiple Multiple of which the final number will be
/// @return     number rounded to the nearest multiple of multiple
static inline uint32_t round32(uint32_t number, uint32_t multiple) {
    uint32_t remainder;
    if (multiple == 0) {
        return number;
    }

    remainder = number % multiple;

    // round up
    if ((remainder * 2) >= multiple) {
        return number + multiple - remainder;
    } else {
        return number - remainder;
    }
}


//////////////////////////////////////////////////////////////////////////
/// Test if a value is a power of two (aka, has one bit set).
/// @param[in] x_   Number to be tested
/// @return    true if x_ is a power of two, false otherwise
#define IS_POWER_OF_TWO(x_) (((x_) & ((x_)-1)) == 0)

//////////////////////////////////////////////////////////////////////////
/// Compare two uint32_t timers accounting for overflow
/// @param[in]  startTime  The first time
/// @param[in]  endTime    The second time
/// @retval     uint32_t   Difference in the two times
static inline uint32_t timeDiff32(uint32_t startTime, uint32_t endTime) {
    if (endTime >= startTime) {
        return endTime - startTime;
    } else {
        return endTime + UINT_MAX - startTime;
    }
}


//////////////////////////////////////////////////////////////////////////
/// Locate a variable in a section of memory
/// @param[in]  x location to place the variable (.sram, .bootdata, etc)
#ifdef __GNUC__
#if defined(TEST)
#define LOCATE(x)
#else
#define LOCATE(x) __attribute__((section(#x)))
#endif
#else // IAR
#define LOCATE(x) _Pragma(STR(location=#x))
#endif


#endif /* UTIL_H_ */
