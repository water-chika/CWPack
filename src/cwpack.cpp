/*      CWPack - cwpack.c   */
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

#include "cwpack.hpp"

/*******************************   U N P A C K   **********************************/


int cw_unpack_context_init (cw_unpack_context* unpack_context, const void* data, unsigned long length, unpack_underflow_handler huu)
{
    unpack_context->start = unpack_context->current = (uint8_t*)data;
    unpack_context->end = unpack_context->start + length;
    unpack_context->return_code = cwpack::test_byte_order();
    unpack_context->err_no = 0;
    unpack_context->handle_unpack_underflow = huu;
    return unpack_context->return_code;
}


/*  Unpacking routines  ----------------------------------------------------------  */



void cw_unpack_next (cw_unpack_context* unpack_context)
{
    if (unpack_context->return_code)
        return;

    uint64_t    tmpu64;
    uint32_t    tmpu32;
    uint16_t    tmpu16;
    uint8_t*    p;

#define buffer_end_return_code  CWP_RC_END_OF_INPUT;
    cw_unpack_assert_space(1);
    uint8_t c = *p;
#undef buffer_end_return_code
#define buffer_end_return_code  CWP_RC_BUFFER_UNDERFLOW;
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
                    getDDItem(CWP_ITEM_POSITIVE_INTEGER, i64, c);       return;  // positive fixnum
        case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
        case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
                    getDDItem(CWP_ITEM_MAP, map.size, c & 0x0f);        return;  // fixmap
        case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
        case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
                    getDDItem(CWP_ITEM_ARRAY, array.size, c & 0x0f);    return;  // fixarray
        case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
        case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
        case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
        case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
                    getDDItem(CWP_ITEM_STR, str.length, c & 0x1f);              // fixraw
                    cw_unpack_assert_blob(str);
        case 0xc0:  unpack_context->item.type = CWP_ITEM_NIL;           return;  // nil
        case 0xc2:  getDDItem(CWP_ITEM_BOOLEAN, boolean, false);        return;  // false
        case 0xc3:  getDDItem(CWP_ITEM_BOOLEAN, boolean, true);         return;  // true
        case 0xc4:  getDDItem1(CWP_ITEM_BIN, bin.length, uint8_t);              // bin 8
                    cw_unpack_assert_blob(bin);
        case 0xc5:  getDDItem2(CWP_ITEM_BIN, bin.length, uint16_t);             // bin 16
                    cw_unpack_assert_blob(bin);
        case 0xc6:  getDDItem4(CWP_ITEM_BIN, bin.length, uint32_t);             // bin 32
                    cw_unpack_assert_blob(bin);
        case 0xc7:  getDDItem1(CWP_ITEM_EXT, ext.length, uint8_t);              // ext 8
                    cw_unpack_assert_space(1);
                    unpack_context->item.type = (cwpack_item_types)*(int8_t*)p;
                    if (unpack_context->item.type == CWP_ITEM_TIMESTAMP)
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
        case 0xc8:  getDDItem2(CWP_ITEM_EXT, ext.length, uint16_t);             // ext 16
                    cw_unpack_assert_space(1);
                    unpack_context->item.type = (cwpack_item_types)*(int8_t*)p;
                    cw_unpack_assert_blob(ext);
        case 0xc9:  getDDItem4(CWP_ITEM_EXT, ext.length, uint32_t);             // ext 32
                    cw_unpack_assert_space(1);
                    unpack_context->item.type = (cwpack_item_types)*(int8_t*)p;
                    cw_unpack_assert_blob(ext);
        case 0xca:  unpack_context->item.type = CWP_ITEM_FLOAT;                 // float
                    cw_unpack_assert_space(4);
                    cw_load32(p);
                    unpack_context->item.as.real = *(float*)&tmpu32;     return;
        case 0xcb:  getDDItem8(CWP_ITEM_DOUBLE);                         return;  // double
        case 0xcc:  getDDItem1(CWP_ITEM_POSITIVE_INTEGER, u64, uint8_t); return;  // unsigned int  8
        case 0xcd:  getDDItem2(CWP_ITEM_POSITIVE_INTEGER, u64, uint16_t); return; // unsigned int 16
        case 0xce:  getDDItem4(CWP_ITEM_POSITIVE_INTEGER, u64, uint32_t); return; // unsigned int 32
        case 0xcf:  getDDItem8(CWP_ITEM_POSITIVE_INTEGER);               return;  // unsigned int 64
        case 0xd0:  getDDItem1(CWP_ITEM_NEGATIVE_INTEGER, i64, int8_t);          // signed int  8
                    if (unpack_context->item.as.i64 >= 0)
                        unpack_context->item.type = CWP_ITEM_POSITIVE_INTEGER;
                    return;
        case 0xd1:  getDDItem2(CWP_ITEM_NEGATIVE_INTEGER, i64, int16_t);        // signed int 16
                    if (unpack_context->item.as.i64 >= 0)
                        unpack_context->item.type = CWP_ITEM_POSITIVE_INTEGER;
                    return;
        case 0xd2:  getDDItem4(CWP_ITEM_NEGATIVE_INTEGER, i64, int32_t);        // signed int 32
                    if (unpack_context->item.as.i64 >= 0)
                        unpack_context->item.type = CWP_ITEM_POSITIVE_INTEGER;
                    return;
        case 0xd3:  getDDItem8(CWP_ITEM_NEGATIVE_INTEGER);                      // signed int 64
                    if (unpack_context->item.as.i64 >= 0)
                        unpack_context->item.type = CWP_ITEM_POSITIVE_INTEGER;
                    return;
        case 0xd4:  getDDItemFix(1);                                            // fixext 1
        case 0xd5:  getDDItemFix(2);                                            // fixext 2
        case 0xd6:  getDDItemFix(4);                                            // fixext 4
        case 0xd7:  getDDItemFix(8);                                            // fixext 8
        case 0xd8:  getDDItemFix(16);                                           // fixext 16
        case 0xd9:  getDDItem1(CWP_ITEM_STR, str.length, uint8_t);              // str 8
                    cw_unpack_assert_blob(str);
        case 0xda:  getDDItem2(CWP_ITEM_STR, str.length, uint16_t);             // str 16
                    cw_unpack_assert_blob(str);
        case 0xdb:  getDDItem4(CWP_ITEM_STR, str.length, uint32_t);             // str 32
                    cw_unpack_assert_blob(str);
        case 0xdc:  getDDItem2(CWP_ITEM_ARRAY, array.size, uint16_t);   return;  // array 16
        case 0xdd:  getDDItem4(CWP_ITEM_ARRAY, array.size, uint32_t);   return;  // array 32
        case 0xde:  getDDItem2(CWP_ITEM_MAP, map.size, uint16_t);       return;  // map 16
        case 0xdf:  getDDItem4(CWP_ITEM_MAP, map.size, uint32_t);       return;  // map 32
        case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
        case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
        case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
        case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
                    getDDItem(CWP_ITEM_NEGATIVE_INTEGER, i64, (int8_t)c); return;    // negative fixnum
        default:
                    UNPACK_ERROR(CWP_RC_MALFORMED_INPUT)
    }
}

#define cw_skip_bytes(n)                                \
    cw_unpack_assert_space((n));                          \
    break;

void cw_skip_items (cw_unpack_context* unpack_context, long item_count)
{
    if (unpack_context->return_code)
        return;

    uint32_t    tmpu32;
    uint16_t    tmpu16;
    uint8_t*    p;

    while (item_count-- > 0)
    {
#undef buffer_end_return_code
#define buffer_end_return_code  CWP_RC_END_OF_INPUT;
        cw_unpack_assert_space(1);
        uint8_t c = *p;

#undef buffer_end_return_code
#define buffer_end_return_code  CWP_RC_BUFFER_UNDERFLOW;
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
cwpack_item_types cw_look_ahead (cw_unpack_context* unpack_context)
{
    if (unpack_context->return_code)
        return CWP_NOT_AN_ITEM;

    uint8_t*    p;
#undef buffer_end_return_code
#define buffer_end_return_code  CWP_RC_END_OF_INPUT;
    cw_unpack_assert_space_sub(1,CWP_NOT_AN_ITEM);
    unpack_context->current -= 1;    //step back
    uint8_t c = *p;
#undef buffer_end_return_code
#define buffer_end_return_code  CWP_RC_BUFFER_UNDERFLOW;
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
                                                                                        return CWP_ITEM_POSITIVE_INTEGER;
        case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
        case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
                                                                                        return CWP_ITEM_MAP;
        case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
        case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
                                                                                        return CWP_ITEM_ARRAY;
        case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
        case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
        case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
        case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
                                                                                        return CWP_ITEM_STR;
        case 0xc0:                                                                      return CWP_ITEM_NIL;
        case 0xc2: case 0xc3:                                                           return CWP_ITEM_BOOLEAN;
        case 0xc4: case 0xc5: case 0xc6:                                                return CWP_ITEM_BIN;
        case 0xc7:
            cw_unpack_assert_space_sub(3,CWP_NOT_AN_ITEM);
            unpack_context->current -= 3;
            if ((cwpack_item_types)*(p+2) == CWP_ITEM_TIMESTAMP)                        return CWP_ITEM_TIMESTAMP;
            else                                                                        return (cwpack_item_types)*(int8_t*)(p+2);
        case 0xc8:
            cw_unpack_assert_space_sub(4,CWP_NOT_AN_ITEM);
            unpack_context->current -= 4;                                               return (cwpack_item_types)*(int8_t*)(p+3);
        case 0xc9:
            cw_unpack_assert_space_sub(6,CWP_NOT_AN_ITEM);
            unpack_context->current -= 6;                                               return (cwpack_item_types)*(int8_t*)(p+5);
        case 0xca:                                                                      return CWP_ITEM_FLOAT;
        case 0xcb:                                                                      return CWP_ITEM_DOUBLE;
        case 0xcc: case 0xcd: case 0xce: case 0xcf:                                     return CWP_ITEM_POSITIVE_INTEGER;
        case 0xd0: case 0xd1: case 0xd2: case 0xd3:                                     return CWP_ITEM_NEGATIVE_INTEGER;
        case 0xd4: case 0xd5: case 0xd6: case 0xd7: case 0xd8:
            cw_unpack_assert_space_sub(2,CWP_NOT_AN_ITEM);
            unpack_context->current -= 2;                                               return (cwpack_item_types)*(int8_t*)(p+1);
        case 0xd9: case 0xda: case 0xdb:                                                return CWP_ITEM_STR;
        case 0xdc: case 0xdd:                                                           return CWP_ITEM_ARRAY;
        case 0xde: case 0xdf:                                                           return CWP_ITEM_MAP;
        case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
        case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
        case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
        case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
                                                                                        return CWP_ITEM_NEGATIVE_INTEGER;
        default:
            return CWP_NOT_AN_ITEM;
    }
}

/* end cwpack.c */
