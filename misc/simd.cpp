/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2016 Twilight Finland 3D Oy Ltd. All rights reserved.
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
    - Intel (sse, sse2, sse3, sse 4.x, avx, avx2)
    - ARM neon
    - PPC Altivec / SPU

*/
void example1()
{
    simd4f a = simd4f_set4(1.0f, 2.0f, 2.0f, 1.0f);
    simd4f b = simd4f_set4(0.0f, 1.0f, 0.5f, 0.5f);
    simd4f c = simd4f_add(a, b);
    simd4f d = simd4f_mul(c, b);
    simd4f mask = simd4f_gt(a, b); // greater-than compare
    simd4f e = simd4f_select(c, d, mask);
}

// previous example using higher-level "short vector math" abstraction:
void example2()
{
    float4 a(1.0f, 2.0f, 2.0f, 1.0f);
    float4 b(0.0f, 1.0f, 0.5f, 0.5f);
    float4 c = a + b;
    float4 d = c * b;
    float4 e = select(a > b, d, c);
}
