/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <xmmintrin.h>
#include <emmintrin.h>
#include <cassert>
#include "Mem.h"

#if defined(_MSC_VER)
  #define RESTRICT __restrict
#elif defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
  #define RESTRICT __restrict__
#elif 
  #define RESTRICT
#endif

#define USE_SSE 1

inline void vadd(const float* v0, const float* v1, float* vOut)
{
    assert(IsAligned<16>(v0));
    assert(IsAligned<16>(v1));
    assert(IsAligned<16>(vOut));
    __m128 f0 = _mm_load_ps(v0);
    __m128 f1 = _mm_load_ps(v1);
    _mm_store_ps(vOut, _mm_add_ps(f0, f1));
}

inline void vsub(const float* v0, const float* v1, float* vOut)
{
    assert(IsAligned<16>(v0));
    assert(IsAligned<16>(v1));
    assert(IsAligned<16>(vOut));
    __m128 f0 = _mm_load_ps(v0);
    __m128 f1 = _mm_load_ps(v1);
    _mm_store_ps(vOut, _mm_sub_ps(f0, f1));
}

inline void vmul(const float* v0, const float* v1, float* vOut)
{
    assert(IsAligned<16>(v0));
    assert(IsAligned<16>(v1));
    assert(IsAligned<16>(vOut));
    __m128 f0 = _mm_load_ps(v0);
    __m128 f1 = _mm_load_ps(v1);
    _mm_store_ps(vOut, _mm_mul_ps(f0, f1));
}

inline void vmul(const float* v0, const float s0, float* vOut)
{
    assert(IsAligned<16>(v0));
    assert(IsAligned<16>(vOut));
    __m128 f0 = _mm_load_ps(v0);
    __m128 f1 = _mm_set_ps1(s0);
    _mm_store_ps(vOut, _mm_mul_ps(f0, f1));
}

inline void vdiv(const float* v0, float s0, float* vOut)
{
    assert(IsAligned<16>(v0));
    assert(IsAligned<16>(vOut));
    __m128 f0 = _mm_load_ps(v0);
    __m128 f1 = _mm_set_ps1(s0);
    _mm_store_ps(vOut, _mm_div_ps(f0, f1));
}

inline uint32_t vcvt_rgba(const float* v0)
{
    assert(IsAligned<16>(v0));
    __m128 f0 = _mm_mul_ps(_mm_load_ps(v0), _mm_set_ps1(255.0f));
    __m128i i0 = _mm_cvtps_epi32(f0);
    i0 = _mm_packus_epi16(_mm_packus_epi32(i0, _mm_setzero_si128()), _mm_setzero_si128());
    return (uint32_t)_mm_cvtsi128_si32(i0);
}

inline void vcvt_rgba(uint32_t v0, float* vOut)
{
    assert(IsAligned<16>(vOut));
    __m128i i0 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128((int&)v0));
    __m128 f0 = _mm_cvtepi32_ps(i0);
    f0 = _mm_div_ps(f0, _mm_set_ps1(255.0f));
    _mm_store_ps(vOut, f0);
}

inline uint32_t vcvt_rgba_to_bgra(const float* v0)
{
    assert(IsAligned<16>(v0));
    __m128 f0 = _mm_load_ps(v0);
    f0 = _mm_shuffle_ps(f0, f0, _MM_SHUFFLE(3, 0, 1, 2));
    f0 = _mm_mul_ps(f0, _mm_set_ps1(255.0f));
    __m128i i0 = _mm_cvtps_epi32(f0);
    i0 = _mm_packus_epi16(_mm_packus_epi32(i0, _mm_setzero_si128()), _mm_setzero_si128());
    return (uint32_t)_mm_cvtsi128_si32(i0);
}

inline void vcvt_bgra_to_rgba(uint32_t v0, float* vOut)
{
    assert(IsAligned<16>(vOut));
    __m128i i0 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(v0));
    __m128 f0 = _mm_cvtepi32_ps(i0);
    f0 = _mm_div_ps(f0, _mm_set_ps1(255.0f));
    f0 = _mm_shuffle_ps(f0, f0, _MM_SHUFFLE(3, 0, 1, 2));
    _mm_store_ps(vOut, f0);
}

inline void vclamp(const float* v0, float lower, float upper, float* vOut)
{
    assert(IsAligned<16>(v0));
    assert(IsAligned<16>(vOut));
    __m128 f0 = _mm_min_ps(_mm_max_ps(
        _mm_load_ps(v0),
        _mm_set_ps1(lower)),
        _mm_set_ps1(upper));
    _mm_store_ps(vOut, f0);
}

inline void vmulm(const float* v0, const float* m0, float* vOut)
{
    assert(IsAligned(v0, 16));
	assert(IsAligned(m0, 16));
    assert(IsAligned(vOut, 16));
    __m128 rv = _mm_load_ps(v0);
	__m128 cv1 = _mm_shuffle_ps(rv, rv, 0b00000000);
	__m128 cv2 = _mm_shuffle_ps(rv, rv, 0b01010101);
	__m128 cv3 = _mm_shuffle_ps(rv, rv, 0b10101010);
	__m128 cv4 = _mm_shuffle_ps(rv, rv, 0b11111111);
	__m128 a = _mm_mul_ps(cv1, _mm_load_ps(m0));
	__m128 b = _mm_mul_ps(cv2, _mm_load_ps(m0 + 4));
	__m128 c = _mm_mul_ps(cv3, _mm_load_ps(m0 + 8));
	__m128 d = _mm_mul_ps(cv4, _mm_load_ps(m0 + 12));
	_mm_store_ps(vOut, _mm_add_ps(_mm_add_ps(a, b), _mm_add_ps(c, d)));
}

inline void mmul(const float* m0, const float* m1, float* mOut)
{
    assert(IsAligned(m0, 16));
	assert(IsAligned(m1, 16));
	assert(IsAligned(mOut, 16));

	__m128 r1b = _mm_load_ps(m1);
	__m128 r2b = _mm_load_ps(m1 + 4);
	__m128 r3b = _mm_load_ps(m1 + 8);
	__m128 r4b = _mm_load_ps(m1 + 12);

	__m128 r1a = _mm_load_ps(m0);
	__m128 r1c =      _mm_mul_ps( _mm_shuffle_ps(r1a, r1a, 0b00000000), r1b);
	r1c = _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(r1a, r1a, 0b01010101), r2b), r1c);
	r1c = _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(r1a, r1a, 0b10101010), r3b), r1c);
	r1c = _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(r1a, r1a, 0b11111111), r4b), r1c);
	_mm_store_ps(mOut, r1c);

	__m128 r2a = _mm_load_ps(m0 + 4);
	__m128 r2c =      _mm_mul_ps( _mm_shuffle_ps(r2a, r2a, 0b00000000), r1b );
	r2c = _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(r2a, r2a, 0b01010101), r2b), r2c);
	r2c = _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(r2a, r2a, 0b10101010), r3b), r2c);
	r2c = _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(r2a, r2a, 0b11111111), r4b), r2c);
	_mm_store_ps(mOut + 4, r2c);

	__m128 r3a = _mm_load_ps(m0 + 8);
	__m128 r3c =      _mm_mul_ps( _mm_shuffle_ps(r3a, r3a, 0b00000000), r1b );
	r3c = _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(r3a, r3a, 0b01010101), r2b), r3c);
	r3c = _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(r3a, r3a, 0b10101010), r3b), r3c);
	r3c = _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(r3a, r3a, 0b11111111), r4b), r3c);
	_mm_store_ps(mOut + 8, r3c);

	__m128 r4a = _mm_load_ps(m0 + 12);
	__m128 r4c =      _mm_mul_ps( _mm_shuffle_ps(r4a, r4a, 0b00000000), r1b );
	r4c = _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(r4a, r4a, 0b01010101), r2b), r4c);
	r4c = _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(r4a, r4a, 0b10101010), r3b), r4c);
	r4c = _mm_add_ps( _mm_mul_ps( _mm_shuffle_ps(r4a, r4a, 0b11111111), r4b), r4c);
	_mm_store_ps(mOut + 12, r4c);
}

inline void vblend(const int (&v0)[4], const float* v1, float *vOut)
{
    assert(IsAligned(v1, 16));
	assert(IsAligned(vOut, 16));

    __m128 w = _mm_load_ps(v1);
    __m128 maxChan = _mm_set_ps1(255.0f);

    __m128 c0 = _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_cvtsi32_si128(v0[0])));
    c0 = _mm_div_ps(c0, maxChan);
    __m128 r = _mm_mul_ps(c0, _mm_shuffle_ps(w, w, 0b00000000));

    __m128 c1 = _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_cvtsi32_si128(v0[1])));
    c1 = _mm_div_ps(c1, maxChan);
    r = _mm_add_ps(_mm_mul_ps(c1, _mm_shuffle_ps(w, w, 0b01010101)), r);

    __m128 c2 = _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_cvtsi32_si128(v0[2])));
    c2 = _mm_div_ps(c2, maxChan);
    r = _mm_add_ps(_mm_mul_ps(c2, _mm_shuffle_ps(w, w, 0b10101010)), r);

    __m128 c3 = _mm_cvtepi32_ps(_mm_cvtepu8_epi32(_mm_cvtsi32_si128(v0[3])));
    c3 = _mm_div_ps(c3, maxChan);
    r = _mm_add_ps(_mm_mul_ps(c3, _mm_shuffle_ps(w, w, 0b11111111)), r);
    
    _mm_store_ps(vOut, r);
}
