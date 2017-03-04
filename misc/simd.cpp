/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2017 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/simd/simd.hpp>

using namespace mango;

/*
    Portable low level SIMD abstraction. Uses a simple functional API for
    getting the job done. Supports multiple architectures using a common interface.
    Does only expose functionalty that is efficient and common for all architectures.

    The higher-level short vector math library is written on top of the low level
    code to be more user-friendly. This abstracts all of the platform specific
    minute details into it's own neat compartment for easier maintenance. This also
    allows to add more platforms easier; we already have quite a few targets:
    - Intel (SSE, SSE2, SSE3, SSE 4.1, AVX, AVX2)
    - ARM neon
    - PPC Altivec / SPU

*/
void example1()
{
    simd::float32x4 a = simd::float32x4_set4(1.0f, 2.0f, 2.0f, 1.0f);
    simd::float32x4 b = simd::float32x4_set4(0.0f, 1.0f, 0.5f, 0.5f);
    simd::float32x4 c = simd::add(a, b);
    simd::float32x4 d = simd::mul(c, b);
    simd::float32x4 mask = simd::compare_gt(a, b);
    simd::float32x4 e = simd::select(mask, d, c);
}

// previous example using higher-level "Short Vector Math" abstraction:
void example2()
{
    float4 a(1.0f, 2.0f, 2.0f, 1.0f);
    float4 b(0.0f, 1.0f, 0.5f, 0.5f);
    float4 c = a + b;
    float4 d = c * b;
    float4 e = select(a > b, d, c);
}

simd::float32x4 example3(simd::float32x4 v)
{
    // SIMD optimized trigonometric functions (courtesy of Sleef)
    return simd::sin(v);
}
