/*      CWPack - cwpack.h   */
/*
 The MIT License (MIT)

 Copyright (c) 2017 Claes Wihlborg

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify,
 merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <cstdint>
#include <ctime>
#include <cstring>

#include <functional>

#include "cwpack_internals.hpp"

/*******************************   Return Codes   *****************************/

enum CWP_RC {
    CWP_RC_OK                      =   0,
    CWP_RC_END_OF_INPUT            =  -1,
    CWP_RC_BUFFER_OVERFLOW         =  -2,
    CWP_RC_BUFFER_UNDERFLOW        =  -3,
    CWP_RC_MALFORMED_INPUT         =  -4,
    CWP_RC_WRONG_BYTE_ORDER        =  -5,
    CWP_RC_ERROR_IN_HANDLER        =  -6,
    CWP_RC_ILLEGAL_CALL            =  -7,
    CWP_RC_MALLOC_ERROR            =  -8,
    CWP_RC_STOPPED                 =  -9,
    CWP_RC_TYPE_ERROR              = -10,
    CWP_RC_VALUE_ERROR             = -11,
    CWP_RC_WRONG_TIMESTAMP_LENGTH  = -12,
};

namespace cwpack {

static int test_byte_order(void)
{
#ifdef COMPILE_FOR_BIG_ENDIAN
    const char *endianness = "1234";
    if (*(uint32_t*)endianness != 0x31323334UL)
        return CWP_RC_WRONG_BYTE_ORDER;
#else

#ifdef COMPILE_FOR_LITTLE_ENDIAN
    const char *endianness = "1234";
    if (*(uint32_t*)endianness != 0x34333231UL)
        return CWP_RC_WRONG_BYTE_ORDER;
#endif
#endif
    return CWP_RC_OK;
}

struct context {
public:
    using overflow_handler = std::function<int (context*, unsigned long)>;
    using flush_handler = std::function<int (context*)>;

    context() = default;
    context(uint8_t* data, unsigned long length, overflow_handler overflow_handler)
        :
            current{data},
            start{data},
            end{data + length},
            be_compatible{false},
            return_code{test_byte_order()},
            err_no{},
            handle_pack_overflow{overflow_handler},
            handle_flush{}
    {}
    ~context() = default;

    uint8_t*                current;
    uint8_t*                start;
    uint8_t*                end;
    bool                    be_compatible;
    int                     return_code;
    int                     err_no;          /* handlers can save error here */
    std::function<int (context*, unsigned long)> handle_pack_overflow;
    std::function<int (context*)> handle_flush;
};

}

using cw_pack_context = cwpack::context;
typedef int (*pack_flush_handler)(cw_pack_context*);

inline static int cw_pack_context_init (cw_pack_context* pack_context, void* data, unsigned long length, cwpack::context::overflow_handler hpo) {
    new (pack_context) cw_pack_context{reinterpret_cast<uint8_t*>(data), length, hpo};

    return pack_context->err_no;
}

inline static void cw_pack_set_compatibility (cw_pack_context* pack_context, bool be_compatible) {
    pack_context->be_compatible = be_compatible;
}

inline static void cw_pack_set_flush_handler (cw_pack_context* pack_context, pack_flush_handler handle_flush) {
    pack_context->handle_flush = handle_flush;
}

inline static void cw_pack_flush (cw_pack_context* pack_context)
{
    if (pack_context->return_code == CWP_RC_OK)
        pack_context->return_code =
            pack_context->handle_flush ?
                pack_context->handle_flush(pack_context) :
                CWP_RC_ILLEGAL_CALL;
}

inline static void cw_pack_nil(cw_pack_context* pack_context)
{
    if (pack_context->return_code)
        return;

    tryMove0(0xc0);
}
inline static void cw_pack_true (cw_pack_context* pack_context)
{
    if (pack_context->return_code)
        return;

    tryMove0(0xc3);
}


inline void cw_pack_false (cw_pack_context* pack_context)
{
    if (pack_context->return_code)
        return;

    tryMove0(0xc2);
}
inline void cw_pack_boolean(cw_pack_context* pack_context, bool b)
{
    if (pack_context->return_code)
        return;

    tryMove0(b? 0xc3: 0xc2);
}


inline static void cw_pack_signed(cw_pack_context* pack_context, int64_t i)
{
    if (pack_context->return_code)
        return;

    if (i >127)
    {
        if (i < 256)
            tryMove1(0xcc, i);

        if (i < 0x10000L)
            tryMove2(0xcd, i);

        if (i < 0x100000000LL)
            tryMove4(0xce, i);

        tryMove8(0xcf,i);
    }

    if (i >= -32)
        tryMove0(i);

    if (i >= -128)
        tryMove1(0xd0, i);

    if (i >= -32768)
        tryMove2(0xd1,i);

    if (i >= (int64_t)0xffffffff80000000LL)
        tryMove4(0xd2,i);

    tryMove8(0xd3,i);
}
inline static void cw_pack_unsigned(cw_pack_context* pack_context, uint64_t i)
{
    if (pack_context->return_code)
        return;

    if (i < 128)
        tryMove0(i);

    if (i < 256)
        tryMove1(0xcc, i);

    if (i < 0x10000L)
    {
        tryMove2(0xcd, i);
    }
    if (i < 0x100000000LL)
        tryMove4(0xce, i);

    tryMove8(0xcf,i);
}

inline static void cw_pack_float(cw_pack_context* pack_context, float f)
{
    if (pack_context->return_code)
        return;

    uint32_t tmp = *((uint32_t*)&f);
    tryMove4(0xca,tmp);
}

inline static void cw_pack_double(cw_pack_context* pack_context, double d)
{
    if (pack_context->return_code)
        return;

    uint64_t tmp = *((uint64_t*)&d);
    tryMove8(0xcb,tmp);
}
/* void cw_pack_real (cw_pack_context* pack_context, double d);   moved to cwpack_utils */

inline static void cw_pack_array_size(cw_pack_context* pack_context, uint32_t n)
{
    if (pack_context->return_code)
        return;

    if (n < 16)
        tryMove0(0x90 | n);

    if (n < 65536)
        tryMove2(0xdc, n);

    tryMove4(0xdd, n);
}

inline static void cw_pack_map_size(cw_pack_context* pack_context, uint32_t n)
{
    if (pack_context->return_code)
        return;

    if (n < 16)
        tryMove0(0x80 | n);

    if (n < 65536)
        tryMove2(0xde, n);

    tryMove4(0xdf, n);
}

inline static void cw_pack_str(cw_pack_context* pack_context, const char* v, uint32_t l)
{
    if (pack_context->return_code)
        return;

    uint8_t *p;

    if (l < 32)             // Fixstr
    {
        cw_pack_reserve_space(l+1);
        *p = (uint8_t)(0xa0 + l);
        memcpy(p+1,v,l);
        return;
    }
    if (l < 256 && !pack_context->be_compatible)       // Str 8
    {
        cw_pack_reserve_space(l+2);
        *p++ = (uint8_t)(0xd9);
        *p = (uint8_t)(l);
        memcpy(p+1,v,l);
        return;
    }
    if (l < 65536)     // Str 16
    {
        cw_pack_reserve_space(l+3)
        *p++ = (uint8_t)0xda;
        cw_store16(l);
        memcpy(p+2,v,l);
        return;
    }
    // Str 32
    cw_pack_reserve_space(l+5)
    *p++ = (uint8_t)0xdb;
    cw_store32(l);
    memcpy(p+4,v,l);
    return;
}

inline static void cw_pack_bin(cw_pack_context* pack_context, const void* v, uint32_t l)
{
    if (pack_context->return_code)
        return;

    if (pack_context->be_compatible)
    {
        cw_pack_str( pack_context, (const char*)v, l);
        return;
    }

    uint8_t *p;

    if (l < 256)            // Bin 8
    {
        cw_pack_reserve_space(l+2);
        *p++ = (uint8_t)(0xc4);
        *p = (uint8_t)(l);
        memcpy(p+1,v,l);
        return;
    }
    if (l < 65536)     // Bin 16
    {
        cw_pack_reserve_space(l+3)
        *p++ = (uint8_t)0xc5;
        cw_store16(l);
        memcpy(p+2,v,l);
        return;
    }
    // Bin 32
    cw_pack_reserve_space(l+5)
    *p++ = (uint8_t)0xc6;
    cw_store32(l);
    memcpy(p+4,v,l);
    return;
}

inline static void cw_pack_ext (cw_pack_context* pack_context, int8_t type, const void* v, uint32_t l)
{
    if (pack_context->return_code)
        return;

    if (pack_context->be_compatible)
        PACK_ERROR(CWP_RC_ILLEGAL_CALL);

    uint8_t *p;

    switch (l)
    {
        case 1:                                         // Fixext 1
            cw_pack_reserve_space(3);
            *p++ = (uint8_t)0xd4;
            *p++ = (uint8_t)type;
            *p++ = *(uint8_t*)v;
            return;
        case 2:                                         // Fixext 2
            cw_pack_reserve_space(4);
            *p++ = (uint8_t)0xd5;
            break;
        case 4:                                         // Fixext 4
            cw_pack_reserve_space(6);
            *p++ = (uint8_t)0xd6;
            break;
        case 8:                                         // Fixext 8
            cw_pack_reserve_space(10);
            *p++ = (uint8_t)0xd7;
            break;
        case 16:                                        // Fixext16
            cw_pack_reserve_space(18);
            *p++ = (uint8_t)0xd8;
            break;
        default:
            if (l < 256)                                // Ext 8
            {
                cw_pack_reserve_space(l+3);
                *p++ = (uint8_t)0xc7;
                *p++ = (uint8_t)(l);
            }
            else if (l < 65536)                         // Ext 16
            {
                cw_pack_reserve_space(l+4)
                *p++ = (uint8_t)0xc8;
                cw_store16(l);
                p += 2;
            }
            else                                        // Ext 32
            {
                cw_pack_reserve_space(l+6)
                *p++ = (uint8_t)0xc9;
                cw_store32(l);
                p += 4;
            }
    }
    *p++ = (uint8_t)type;
    memcpy(p,v,l);
}

inline static void cw_pack_time (cw_pack_context* pack_context, int64_t sec, uint32_t nsec)
{
    if (pack_context->return_code)
        return;

    if (pack_context->be_compatible)
        PACK_ERROR(CWP_RC_ILLEGAL_CALL);

    if (nsec >= 1000000000)
        PACK_ERROR(CWP_RC_VALUE_ERROR);

    uint8_t *p;

    if ((uint64_t)sec & 0xfffffffc00000000LL) {
        // timestamp 96
        //serialize(0xc7, 12, -1, nsec, sec)
        cw_pack_reserve_space(15);
        *p++ = (uint8_t)0xc7;
        *p++ = (uint8_t)12;
        *p++ = (uint8_t)0xff;
        cw_store32(nsec); p += 4;
        cw_store64(sec);
    }
    else {
        uint64_t data64 = (((uint64_t)nsec << 34) | (uint64_t)sec);
        if (data64 & 0xffffffff00000000LL) {
            // timestamp 64
            //serialize(0xd7, -1, data64)
            cw_pack_reserve_space(10);
            *p++ = (uint8_t)0xd7;
            *p++ = (uint8_t)0xff;
            cw_store64(data64);
        }
        else {
            // timestamp 32
            uint32_t data32 = (uint32_t)data64;
            //serialize(0xd6, -1, data32)
            cw_pack_reserve_space(6);
            *p++ = (uint8_t)0xd6;
            *p++ = (uint8_t)0xff;
            cw_store32(data32);
        }
    }
}

inline static void cw_pack_insert (cw_pack_context* pack_context, const void* v, uint32_t l)
{
    uint8_t *p;
    cw_pack_reserve_space(l);
    memcpy(p,v,l);
}



/*****************************   U N P A C K   ********************************/
namespace cwpack {
enum class item_type : int16_t
{
    MIN_RESERVED_EXT       = -128,
    TIMESTAMP              = -1,
    MAX_RESERVED_EXT       = -1,
    MIN_USER_EXT           = 0,
    USER_EXT_0             = 0,
    USER_EXT_1             = 1,
    USER_EXT_2             = 2,
    USER_EXT_3             = 3,
    USER_EXT_4             = 4,
    USER_EXT_5             = 5,
    USER_EXT_6             = 6,
    USER_EXT_7             = 7,
    USER_EXT_8             = 8,
    USER_EXT_9             = 9,
    USER_EXT_10            = 10,
    USER_EXT_11            = 11,
    USER_EXT_12            = 12,
    USER_EXT_13            = 13,
    USER_EXT_14            = 14,
    USER_EXT_15            = 15,
    USER_EXT_16            = 16,
    USER_EXT_17            = 17,
    USER_EXT_18            = 18,
    USER_EXT_19            = 19,
    USER_EXT_20            = 20,
    USER_EXT_21            = 21,
    USER_EXT_22            = 22,
    USER_EXT_23            = 23,
    USER_EXT_24            = 24,
    USER_EXT_25            = 25,
    USER_EXT_26            = 26,
    USER_EXT_27            = 27,
    USER_EXT_28            = 28,
    USER_EXT_29            = 29,
    USER_EXT_30            = 30,
    USER_EXT_31            = 31,
    USER_EXT_32            = 32,
    USER_EXT_33            = 33,
    USER_EXT_34            = 34,
    USER_EXT_35            = 35,
    USER_EXT_36            = 36,
    USER_EXT_37            = 37,
    USER_EXT_38            = 38,
    USER_EXT_39            = 39,
    USER_EXT_40            = 40,
    USER_EXT_41            = 41,
    USER_EXT_42            = 42,
    USER_EXT_43            = 43,
    USER_EXT_44            = 44,
    USER_EXT_45            = 45,
    USER_EXT_46            = 46,
    USER_EXT_47            = 47,
    USER_EXT_48            = 48,
    USER_EXT_49            = 49,
    USER_EXT_50            = 50,
    USER_EXT_51            = 51,
    USER_EXT_52            = 52,
    USER_EXT_53            = 53,
    USER_EXT_54            = 54,
    USER_EXT_55            = 55,
    USER_EXT_56            = 56,
    USER_EXT_57            = 57,
    USER_EXT_58            = 58,
    USER_EXT_59            = 59,
    USER_EXT_60            = 60,
    USER_EXT_61            = 61,
    USER_EXT_62            = 62,
    USER_EXT_63            = 63,
    USER_EXT_64            = 64,
    USER_EXT_65            = 65,
    USER_EXT_66            = 66,
    USER_EXT_67            = 67,
    USER_EXT_68            = 68,
    USER_EXT_69            = 69,
    USER_EXT_70            = 70,
    USER_EXT_71            = 71,
    USER_EXT_72            = 72,
    USER_EXT_73            = 73,
    USER_EXT_74            = 74,
    USER_EXT_75            = 75,
    USER_EXT_76            = 76,
    USER_EXT_77            = 77,
    USER_EXT_78            = 78,
    USER_EXT_79            = 79,
    USER_EXT_80            = 80,
    USER_EXT_81            = 81,
    USER_EXT_82            = 82,
    USER_EXT_83            = 83,
    USER_EXT_84            = 84,
    USER_EXT_85            = 85,
    USER_EXT_86            = 86,
    USER_EXT_87            = 87,
    USER_EXT_88            = 88,
    USER_EXT_89            = 89,
    USER_EXT_90            = 90,
    USER_EXT_91            = 91,
    USER_EXT_92            = 92,
    USER_EXT_93            = 93,
    USER_EXT_94            = 94,
    USER_EXT_95            = 95,
    USER_EXT_96            = 96,
    USER_EXT_97            = 97,
    USER_EXT_98            = 98,
    USER_EXT_99            = 99,
    USER_EXT_100           = 100,
    USER_EXT_101           = 101,
    USER_EXT_102           = 102,
    USER_EXT_103           = 103,
    USER_EXT_104           = 104,
    USER_EXT_105           = 105,
    USER_EXT_106           = 106,
    USER_EXT_107           = 107,
    USER_EXT_108           = 108,
    USER_EXT_109           = 109,
    USER_EXT_110           = 110,
    USER_EXT_111           = 111,
    USER_EXT_112           = 112,
    USER_EXT_113           = 113,
    USER_EXT_114           = 114,
    USER_EXT_115           = 115,
    USER_EXT_116           = 116,
    USER_EXT_117           = 117,
    USER_EXT_118           = 118,
    USER_EXT_119           = 119,
    USER_EXT_120           = 120,
    USER_EXT_121           = 121,
    USER_EXT_122           = 122,
    USER_EXT_123           = 123,
    USER_EXT_124           = 124,
    USER_EXT_125           = 125,
    USER_EXT_126           = 126,
    USER_EXT_127           = 127,
    MAX_USER_EXT           = 127,
    
    NIL                    = 300,
    BOOLEAN                = 301,
    POSITIVE_INTEGER       = 302,
    NEGATIVE_INTEGER       = 303,
    FLOAT                  = 304,
    DOUBLE                 = 305,
    STR                    = 306,
    BIN                    = 307,
    ARRAY                  = 308,
    MAP                    = 309,
    EXT                    = 310,
    NOT_AN_ITEM                 = 999
};

typedef struct {
    const void*     start;
    uint32_t        length;
} blob;


typedef struct {
    uint32_t    size;
} container;


typedef struct {
    int64_t     tv_sec;
    uint32_t    tv_nsec;
} timespec;


struct item_as {
    item_type type;
    union
    {
        bool            boolean;
        uint64_t        u64;
        int64_t         i64;
        float           real;
        double          long_real;
        container array;
        container map;
        blob     str;
        blob     bin;
        blob     ext;
        timespec time;
    } as;
};


struct unpack_context {
public:
    using underflow_handler = std::function<int (unpack_context*, unsigned long)>;

    item_as                     item;
    uint8_t*                    start;
    uint8_t*                    current;
    uint8_t*                    end;             /* logical end of buffer */
    int                         return_code;
    int                         err_no;          /* handlers can save error here */
    underflow_handler    handle_unpack_underflow;
};

}

using cwpack_item_types = cwpack::item_type;

using cwpack_blob = cwpack::blob;

using cwpack_container = cwpack::container;


using cwpack_timespec = cwpack::timespec;

using cwpack_item = cwpack::item_as;

using cw_unpack_context = cwpack::unpack_context;

inline static int cw_unpack_context_init (cw_unpack_context* unpack_context, const void* data, unsigned long length, cwpack::unpack_context::underflow_handler huu)
{
    unpack_context->start = unpack_context->current = (uint8_t*)data;
    unpack_context->end = unpack_context->start + length;
    unpack_context->return_code = cwpack::test_byte_order();
    unpack_context->err_no = 0;
    unpack_context->handle_unpack_underflow = huu;
    return unpack_context->return_code;
}

inline static void cw_unpack_next(cw_unpack_context* unpack_context)
{
    if (unpack_context->return_code)
        return;

    uint64_t    tmpu64;
    uint32_t    tmpu32;
    uint16_t    tmpu16;
    uint8_t*    p;

    {
        constexpr auto buffer_end_return_code = CWP_RC_END_OF_INPUT;
        cw_unpack_assert_space(1);
    }
    uint8_t c = *p;
    constexpr auto buffer_end_return_code = CWP_RC_BUFFER_UNDERFLOW;
    switch (c)
    {
        case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
        case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
        case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
        case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
        case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
        case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
        case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
        case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
        case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
        case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
        case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
        case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
        case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
        case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
        case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
        case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
                    getDDItem(cwpack::item_type::POSITIVE_INTEGER, i64, c);       return;  // positive fixnum
        case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
        case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
                    getDDItem(cwpack::item_type::MAP, map.size, c & 0x0f);        return;  // fixmap
        case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
        case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
                    getDDItem(cwpack::item_type::ARRAY, array.size, c & 0x0f);    return;  // fixarray
        case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
        case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
        case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
        case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
                    getDDItem(cwpack::item_type::STR, str.length, c & 0x1f);              // fixraw
                    cw_unpack_assert_blob(str);
        case 0xc0:  unpack_context->item.type = cwpack::item_type::NIL;           return;  // nil
        case 0xc2:  getDDItem(cwpack::item_type::BOOLEAN, boolean, false);        return;  // false
        case 0xc3:  getDDItem(cwpack::item_type::BOOLEAN, boolean, true);         return;  // true
        case 0xc4:  getDDItem1(cwpack::item_type::BIN, bin.length, uint8_t);              // bin 8
                    cw_unpack_assert_blob(bin);
        case 0xc5:  getDDItem2(cwpack::item_type::BIN, bin.length, uint16_t);             // bin 16
                    cw_unpack_assert_blob(bin);
        case 0xc6:  getDDItem4(cwpack::item_type::BIN, bin.length, uint32_t);             // bin 32
                    cw_unpack_assert_blob(bin);
        case 0xc7:  getDDItem1(cwpack::item_type::EXT, ext.length, uint8_t);              // ext 8
                    cw_unpack_assert_space(1);
                    unpack_context->item.type = (cwpack_item_types)*(int8_t*)p;
                    if (unpack_context->item.type == cwpack::item_type::TIMESTAMP)
                    {
                        if (unpack_context->item.as.ext.length == 12)
                        {
                            cw_unpack_assert_space(4);
                            cw_load32(p);
                            unpack_context->item.as.time.tv_nsec = tmpu32;
                            cw_unpack_assert_space(8);
                            cw_load64(p,tmpu64);
                            unpack_context->item.as.time.tv_sec = (int64_t)tmpu64;
                            return;
                        }
                        UNPACK_ERROR(CWP_RC_WRONG_TIMESTAMP_LENGTH)
                    }
                    cw_unpack_assert_blob(ext);
        case 0xc8:  getDDItem2(cwpack::item_type::EXT, ext.length, uint16_t);             // ext 16
                    cw_unpack_assert_space(1);
                    unpack_context->item.type = (cwpack_item_types)*(int8_t*)p;
                    cw_unpack_assert_blob(ext);
        case 0xc9:  getDDItem4(cwpack::item_type::EXT, ext.length, uint32_t);             // ext 32
                    cw_unpack_assert_space(1);
                    unpack_context->item.type = (cwpack_item_types)*(int8_t*)p;
                    cw_unpack_assert_blob(ext);
        case 0xca:  unpack_context->item.type = cwpack::item_type::FLOAT;                 // float
                    cw_unpack_assert_space(4);
                    cw_load32(p);
                    unpack_context->item.as.real = *(float*)&tmpu32;     return;
        case 0xcb:  getDDItem8(cwpack::item_type::DOUBLE);                         return;  // double
        case 0xcc:  getDDItem1(cwpack::item_type::POSITIVE_INTEGER, u64, uint8_t); return;  // unsigned int  8
        case 0xcd:  getDDItem2(cwpack::item_type::POSITIVE_INTEGER, u64, uint16_t); return; // unsigned int 16
        case 0xce:  getDDItem4(cwpack::item_type::POSITIVE_INTEGER, u64, uint32_t); return; // unsigned int 32
        case 0xcf:  getDDItem8(cwpack::item_type::POSITIVE_INTEGER);               return;  // unsigned int 64
        case 0xd0:  getDDItem1(cwpack::item_type::NEGATIVE_INTEGER, i64, int8_t);          // signed int  8
                    if (unpack_context->item.as.i64 >= 0)
                        unpack_context->item.type = cwpack::item_type::POSITIVE_INTEGER;
                    return;
        case 0xd1:  getDDItem2(cwpack::item_type::NEGATIVE_INTEGER, i64, int16_t);        // signed int 16
                    if (unpack_context->item.as.i64 >= 0)
                        unpack_context->item.type = cwpack::item_type::POSITIVE_INTEGER;
                    return;
        case 0xd2:  getDDItem4(cwpack::item_type::NEGATIVE_INTEGER, i64, int32_t);        // signed int 32
                    if (unpack_context->item.as.i64 >= 0)
                        unpack_context->item.type = cwpack::item_type::POSITIVE_INTEGER;
                    return;
        case 0xd3:  getDDItem8(cwpack::item_type::NEGATIVE_INTEGER);                      // signed int 64
                    if (unpack_context->item.as.i64 >= 0)
                        unpack_context->item.type = cwpack::item_type::POSITIVE_INTEGER;
                    return;
        case 0xd4:  getDDItemFix(1);                                            // fixext 1
        case 0xd5:  getDDItemFix(2);                                            // fixext 2
        case 0xd6:  getDDItemFix(4);                                            // fixext 4
        case 0xd7:  getDDItemFix(8);                                            // fixext 8
        case 0xd8:  getDDItemFix(16);                                           // fixext 16
        case 0xd9:  getDDItem1(cwpack::item_type::STR, str.length, uint8_t);              // str 8
                    cw_unpack_assert_blob(str);
        case 0xda:  getDDItem2(cwpack::item_type::STR, str.length, uint16_t);             // str 16
                    cw_unpack_assert_blob(str);
        case 0xdb:  getDDItem4(cwpack::item_type::STR, str.length, uint32_t);             // str 32
                    cw_unpack_assert_blob(str);
        case 0xdc:  getDDItem2(cwpack::item_type::ARRAY, array.size, uint16_t);   return;  // array 16
        case 0xdd:  getDDItem4(cwpack::item_type::ARRAY, array.size, uint32_t);   return;  // array 32
        case 0xde:  getDDItem2(cwpack::item_type::MAP, map.size, uint16_t);       return;  // map 16
        case 0xdf:  getDDItem4(cwpack::item_type::MAP, map.size, uint32_t);       return;  // map 32
        case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
        case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
        case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
        case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
                    getDDItem(cwpack::item_type::NEGATIVE_INTEGER, i64, (int8_t)c); return;    // negative fixnum
        default:
                    UNPACK_ERROR(CWP_RC_MALFORMED_INPUT)
    }
}

#define cw_skip_bytes(n)                                \
    cw_unpack_assert_space((n));                          \
    break;

inline static void cw_skip_items (cw_unpack_context* unpack_context, long item_count)
{
    if (unpack_context->return_code)
        return;

    uint32_t    tmpu32;
    uint16_t    tmpu16;
    uint8_t*    p;

    while (item_count-- > 0)
    {
        {
            constexpr auto buffer_end_return_code = CWP_RC_END_OF_INPUT;
            cw_unpack_assert_space(1);
        }
        uint8_t c = *p;

        constexpr auto buffer_end_return_code = CWP_RC_BUFFER_UNDERFLOW;
        switch (c)
        {
            case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
            case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
            case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
            case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
            case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
            case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
            case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
            case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
            case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
            case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
            case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
            case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
            case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
            case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
            case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
            case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
                                                                // unsigned fixint
            case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
            case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
            case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
            case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
                                                                // signed fixint
            case 0xc0:                                          // nil
            case 0xc2:                                          // false
            case 0xc3:  break;                                  // true
            case 0xcc:                                          // unsigned int  8
            case 0xd0:	cw_skip_bytes(1);                       // signed int  8
            case 0xcd:                                          // unsigned int 16
            case 0xd1:                                          // signed int 16
            case 0xd4:  cw_skip_bytes(2);                       // fixext 1
            case 0xd5:  cw_skip_bytes(3);                       // fixext 2
            case 0xca:                                          // float
            case 0xce:                                          // unsigned int 32
            case 0xd2:  cw_skip_bytes(4);                       // signed int 32
            case 0xd6:  cw_skip_bytes(5);                       // fixext 4
            case 0xcb:                                          // double
            case 0xcf:                                          // unsigned int 64
            case 0xd3:  cw_skip_bytes(8);                       // signed int 64
            case 0xd7:  cw_skip_bytes(9);                       // fixext 8
            case 0xd8:  cw_skip_bytes(17);                      // fixext 16
            case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
            case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
            case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
            case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
                cw_skip_bytes(c & 0x1f);                        // fixstr
            case 0xd9:                                          // str 8
            case 0xc4:                                          // bin 8
                cw_unpack_assert_space(1);
                tmpu32 = *p;
                cw_skip_bytes(tmpu32);

            case 0xda:                                          // str 16
            case 0xc5:                                          // bin 16
                cw_unpack_assert_space(2);
                cw_load16(p);
                cw_skip_bytes(tmpu16);

            case 0xdb:                                          // str 32
            case 0xc6:                                          // bin 32
                cw_unpack_assert_space(4);
                cw_load32(p);
                cw_skip_bytes(tmpu32);

            case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
            case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
                item_count += 2*(c & 15);                       // FixMap
                break;

            case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
            case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
                item_count += c & 15;                           // FixArray
                break;

            case 0xdc:                                          // array 16
                cw_unpack_assert_space(2);
                cw_load16(p);
                item_count += tmpu16;
                break;

            case 0xde:                                          // map 16
                cw_unpack_assert_space(2);
                cw_load16(p);
                item_count += 2*tmpu16;
                break;

            case 0xdd:                                          // array 32
                cw_unpack_assert_space(4);
                cw_load32(p);
                item_count += tmpu32;
                break;

            case 0xdf:                                          // map 32
                cw_unpack_assert_space(4);
                cw_load32(p);
                item_count += 2*tmpu32;
                break;

            case 0xc7:                                          // ext 8
                cw_unpack_assert_space(1);
                tmpu32 = *p;
                cw_skip_bytes(tmpu32 +1);

            case 0xc8:                                          // ext 16
                cw_unpack_assert_space(2);
                cw_load16(p);
                cw_skip_bytes(tmpu16 +1);

            case 0xc9:                                          // ext 32
                cw_unpack_assert_space(4);
                cw_load32(p);
                cw_skip_bytes(tmpu32 +1);

            default:                                            // illegal
                UNPACK_ERROR(CWP_RC_MALFORMED_INPUT)
        }
    }
}

/* Check next item type without consuming input */
inline static cwpack_item_types cw_look_ahead (cw_unpack_context* unpack_context)
{
    if (unpack_context->return_code)
        return cwpack::item_type::NOT_AN_ITEM;

    uint8_t*    p;
    {
        constexpr auto buffer_end_return_code = CWP_RC_END_OF_INPUT;
        cw_unpack_assert_space_sub(1,cwpack::item_type::NOT_AN_ITEM);
    }
    unpack_context->current -= 1;    //step back
    uint8_t c = *p;
    constexpr auto buffer_end_return_code = CWP_RC_BUFFER_UNDERFLOW;
    switch (c)
    {
        case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
        case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
        case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
        case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
        case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
        case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
        case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
        case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
        case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
        case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
        case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
        case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
        case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
        case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
        case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
        case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
                                                                                        return cwpack::item_type::POSITIVE_INTEGER;
        case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
        case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
                                                                                        return cwpack::item_type::MAP;
        case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
        case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
                                                                                        return cwpack::item_type::ARRAY;
        case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
        case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
        case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
        case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
                                                                                        return cwpack::item_type::STR;
        case 0xc0:                                                                      return cwpack::item_type::NIL;
        case 0xc2: case 0xc3:                                                           return cwpack::item_type::BOOLEAN;
        case 0xc4: case 0xc5: case 0xc6:                                                return cwpack::item_type::BIN;
        case 0xc7:
            cw_unpack_assert_space_sub(3,cwpack::item_type::NOT_AN_ITEM);
            unpack_context->current -= 3;
            if ((cwpack_item_types)*(p+2) == cwpack::item_type::TIMESTAMP)                        return cwpack::item_type::TIMESTAMP;
            else                                                                        return (cwpack_item_types)*(int8_t*)(p+2);
        case 0xc8:
            cw_unpack_assert_space_sub(4,cwpack::item_type::NOT_AN_ITEM);
            unpack_context->current -= 4;                                               return (cwpack_item_types)*(int8_t*)(p+3);
        case 0xc9:
            cw_unpack_assert_space_sub(6,cwpack::item_type::NOT_AN_ITEM);
            unpack_context->current -= 6;                                               return (cwpack_item_types)*(int8_t*)(p+5);
        case 0xca:                                                                      return cwpack::item_type::FLOAT;
        case 0xcb:                                                                      return cwpack::item_type::DOUBLE;
        case 0xcc: case 0xcd: case 0xce: case 0xcf:                                     return cwpack::item_type::POSITIVE_INTEGER;
        case 0xd0: case 0xd1: case 0xd2: case 0xd3:                                     return cwpack::item_type::NEGATIVE_INTEGER;
        case 0xd4: case 0xd5: case 0xd6: case 0xd7: case 0xd8:
            cw_unpack_assert_space_sub(2,cwpack::item_type::NOT_AN_ITEM);
            unpack_context->current -= 2;                                               return (cwpack_item_types)*(int8_t*)(p+1);
        case 0xd9: case 0xda: case 0xdb:                                                return cwpack::item_type::STR;
        case 0xdc: case 0xdd:                                                           return cwpack::item_type::ARRAY;
        case 0xde: case 0xdf:                                                           return cwpack::item_type::MAP;
        case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
        case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
        case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
        case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
                                                                                        return cwpack::item_type::NEGATIVE_INTEGER;
        default:
            return cwpack::item_type::NOT_AN_ITEM;
    }
}

