/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "utf.h"

#include <cstddef>
#include <cstring>

#include <limits>
#include <tuple>

namespace OHOS::Ace{


/*
 * MUtf-8
 *
 * U+0000 => C0 80
 *
 * N  Bits for     First        Last        Byte 1      Byte 2      Byte 3      Byte 4      Byte 5      Byte 6
 *    code point   code point   code point
 * 1  7            U+0000       U+007F      0xxxxxxx
 * 2  11           U+0080       U+07FF      110xxxxx    10xxxxxx
 * 3  16           U+0800       U+FFFF      1110xxxx    10xxxxxx    10xxxxxx
 * 6  21           U+10000      U+10FFFF    11101101    1010xxxx    10xxxxxx    11101101    1011xxxx    10xxxxxx
 * for U+10000 -- U+10FFFF encodes the following (value - 0x10000)
 */

/*
 * Convert mutf8 sequence to utf16 pair and return pair: [utf16 code point, mutf8 size].
 * In case of invalid sequence return first byte of it.
 */
std::pair<uint32_t, size_t> ConvertMUtf8ToUtf16Pair(const uint8_t *data, size_t max_bytes)
{
    // TODO(d.kovalneko): make the function safe
    uint8_t d0 = *data;
    if ((d0 & MASK1) == 0) {
        return {d0, 1};
    }

    if (max_bytes < CONST_2) {
        return {d0, 1};
    }
    uint8_t d1 = *(data+1);
    if ((d0 & MASK2) == 0) {
        return {((d0 & MASK_5BIT) << DATA_WIDTH) | (d1 & MASK_6BIT), 2};
    }

    if (max_bytes < CONST_3) {
        return {d0, 1};
    }
    uint8_t d2 = *(data+CONST_2);
    if ((d0 & MASK3) == 0) {
        return {((d0 & MASK_4BIT) << (DATA_WIDTH * CONST_2)) | ((d1 & MASK_6BIT) << DATA_WIDTH) | (d2 & MASK_6BIT),
                CONST_3};
    }

    if (max_bytes < CONST_4) {
        return {d0, 1};
    }
    uint8_t d3 = *(data+CONST_3);
    uint32_t code_point = ((d0 & MASK_4BIT) << (DATA_WIDTH * CONST_3)) | ((d1 & MASK_6BIT) << (DATA_WIDTH * CONST_2)) |
                          ((d2 & MASK_6BIT) << DATA_WIDTH) | (d3 & MASK_6BIT);

    uint32_t pair = 0;
    pair |= ((code_point >> (PAIR_ELEMENT_WIDTH - DATA_WIDTH)) + U16_LEAD) & MASK_16BIT;
    pair <<= PAIR_ELEMENT_WIDTH;
    pair |= (code_point & MASK_10BIT) + U16_TAIL;

    return {pair, CONST_4};
}


size_t ConvertRegionMUtf8ToUtf16(const uint8_t *mutf8_in, uint16_t *utf16_out, size_t mutf8_len, size_t utf16_len,
                                 size_t start)
{
    size_t in_pos = 0;
    size_t out_pos = 0;
    while (in_pos < mutf8_len) {
        auto [pair, nbytes] = ConvertMUtf8ToUtf16Pair(mutf8_in, mutf8_len - in_pos);
        auto [p_hi, p_lo] = SplitUtf16Pair(pair);

        mutf8_in += nbytes;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        in_pos += nbytes;
        if (start > 0) {
            start -= nbytes;
            continue;
        }

        if (p_hi != 0) {
            if (out_pos++ >= utf16_len - 1) {  // check for place for two uint16
                --out_pos;
                break;
            }
            *utf16_out++ = p_hi;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
        if (out_pos++ >= utf16_len) {
            --out_pos;
            break;
        }
        *utf16_out++ = p_lo;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    return out_pos;
}

bool IsUTF16HighSurrogate(uint16_t ch)
{
    return DECODE_LEAD_LOW <= ch && ch <= DECODE_LEAD_HIGH;
}

bool IsUTF16LowSurrogate(uint16_t ch)
{
    return DECODE_TRAIL_LOW <= ch && ch <= DECODE_TRAIL_HIGH;
}

size_t UTF8Length(uint32_t codepoint)
{
    if (codepoint <= UTF8_1B_MAX) {
        return UtfLength::ONE;
    }
    if (codepoint <= UTF8_2B_MAX) {
        return UtfLength::TWO;
    }
    if (codepoint <= UTF8_3B_MAX) {
        return UtfLength::THREE;
    }
    return UtfLength::FOUR;
}

// Methods for encode unicode to unicode
size_t EncodeUTF8(uint32_t codepoint, uint8_t* utf8, size_t len, size_t index)
{
    size_t size = UTF8Length(codepoint);
    if (index + size > len) {
        return 0;
    }
    for (size_t j = size - 1; j > 0; j--) {
        uint8_t cont = ((codepoint | byteMark) & byteMask);
        utf8[index + j] = cont;
        codepoint >>= UTF8_OFFSET;
    }
    utf8[index] = codepoint | firstByteMark[size];
    return size;
}

uint32_t HandleAndDecodeInvalidUTF16(uint16_t const *utf16, size_t len, size_t *index)
{
    uint16_t first = utf16[*index];
    // A valid surrogate pair should always start with a High Surrogate
    if (IsUTF16LowSurrogate(first)) {
        return UTF16_REPLACEMENT_CHARACTER;
    }
    if (IsUTF16HighSurrogate(first) || (first & SURROGATE_MASK) == DECODE_LEAD_LOW) {
        if (*index == len - 1) {
            // A High surrogate not paired with another surrogate
            return UTF16_REPLACEMENT_CHARACTER;
        }
        uint16_t second = utf16[*index + 1];
        if (!IsUTF16LowSurrogate(second)) {
            // A High surrogate not followed by a low surrogate
            return UTF16_REPLACEMENT_CHARACTER;
        }
        // A valid surrogate pair, decode normally
        (*index)++;
        return ((first - DECODE_LEAD_LOW) << UTF16_OFFSET) + (second - DECODE_TRAIL_LOW) + DECODE_SECOND_FACTOR;
    }
    // A unicode not fallen into the range of representing by surrogate pair, return as it is
    return first;
}

size_t DebuggerConvertRegionUtf16ToUtf8(const uint16_t *utf16In, uint8_t *utf8Out, size_t utf16Len, size_t utf8Len,
                                        size_t start, bool modify, bool isWriteBuffer)
{
    if (utf16In == nullptr || utf8Out == nullptr || utf8Len == 0) {
        return 0;
    }
    size_t utf8Pos = 0;
    size_t end = start + utf16Len;
    for (size_t i = start; i < end; ++i) {
        uint32_t codepoint = HandleAndDecodeInvalidUTF16(utf16In, end, &i);
        if (codepoint == 0) {
            if (isWriteBuffer) {
                utf8Out[utf8Pos++] = 0x00U;
                continue;
            }
            if (modify) {
                // special case for \u0000 ==> C080 - 1100'0000 1000'0000
                utf8Out[utf8Pos++] = UTF8_2B_FIRST;
                utf8Out[utf8Pos++] = UTF8_2B_SECOND;
            }
            continue;
        }
        utf8Pos += EncodeUTF8(codepoint, utf8Out, utf8Len, utf8Pos);
    }
    return utf8Pos;
}


}
