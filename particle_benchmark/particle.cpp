/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2018 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/mango.hpp>
#include <algorithm>
#include <random>

using namespace mango;

template <typename T>
using AlignedVector = std::vector<T, AlignedAllocator<T>>;

// ----------------------------------------------------------------------
// helpers
// ----------------------------------------------------------------------

namespace {

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(-1.0, 1.0);

    inline float4 random_float4(float w)
    {
        float x = dist(mt);
        float y = dist(mt);
        float z = dist(mt);
        return float4(x, y, z, w);
    }

} // namespace

// ----------------------------------------------------------------------
// method1: AoS - Array of Structures
// ----------------------------------------------------------------------

/*
    This method wastes memory bandwidth for reading data which is not needed
    and writing data that doesn't change.
*/

namespace method1
{

    struct Particle
    {
        float4 position; // this is the only mutable data in the transform() method
        float4 velocity;
        uint32 color;
        float radius;
        float rotation;
    };

    struct Scene
    {
        AlignedVector<Particle> particles;

        Scene(int count)
            : particles(count)
        {
            for (auto &particle : particles)
            {
                particle.position = random_float4(1.0f);
                particle.velocity = random_float4(0.0f);
            }
        }

        void transform()
        {
            for (auto &particle : particles)
            {
                particle.position += particle.velocity;
            }
        }
    };

} // namespace

// ----------------------------------------------------------------------
// method2: SoA - Structure of Arrays
// ----------------------------------------------------------------------

/*
    This method is slightly better as it writes to continuous memory w/o any holes.
*/

namespace method2
{

    struct Scene
    {
        AlignedVector<float4> positions;
        AlignedVector<float4> velocities;
        std::vector<uint32> colors;
        std::vector<float> radiuses;
        std::vector<float> rotations;

        Scene(int count)
            : positions(count)
            , velocities(count)
            , colors(count)
            , radiuses(count)
            , rotations(count)
        {
            for (auto &position : positions)
            {
                position = random_float4(1.0f);
            }

            for (auto &velocity : velocities)
            {
                velocity = random_float4(1.0f);
            }
        }

        void transform()
        {
            const size_t count = positions.size();
            for (size_t i = 0; i < count; ++i)
            {
                positions[i] += velocities[i];
            }
        }
    };

} // namespace

// ----------------------------------------------------------------------
// method3: SoA w/ 100% SIMD register utilization
// ----------------------------------------------------------------------

/*
    This method does not waste 25% of ram for storing the w-coordinate which
    is a constant value. It was stored in the earlier methods for alignment.
*/

namespace method3
{

    struct Scene
    {
        AlignedVector<float4> xpositions;
        AlignedVector<float4> ypositions;
        AlignedVector<float4> zpositions;
        AlignedVector<float4> xvelocities;
        AlignedVector<float4> yvelocities;
        AlignedVector<float4> zvelocities;
        std::vector<uint32> colors;
        std::vector<float> radiuses;
        std::vector<float> rotations;

        Scene(int count)
            : xpositions(count / 4)
            , ypositions(count / 4)
            , zpositions(count / 4)
            , xvelocities(count / 4)
            , yvelocities(count / 4)
            , zvelocities(count / 4)
            , colors(count)
            , radiuses(count)
            , rotations(count)
        {
            count /= 4;
            for (int i = 0; i < count; ++i)
            {
                xpositions[i] = random_float4(1.0f);
                ypositions[i] = random_float4(1.0f);
                zpositions[i] = random_float4(2.0f);
                xvelocities[i] = random_float4(1.0f);
                yvelocities[i] = random_float4(2.0f);
                zvelocities[i] = random_float4(5.0f);
            }
        }

        void transform()
        {
            const int count = xpositions.size();
            for (int i = 0; i < count; ++i)
            {
                xpositions[i] += xvelocities[i];
                ypositions[i] += yvelocities[i];
                zpositions[i] += zvelocities[i];
            }
        }
    };

} // namespace

// ----------------------------------------------------------------------
// method4: SoA with vector types
// ----------------------------------------------------------------------

namespace method4
{

    // configure SIMD vector type
    using VectorType = float32x4;

    // 3-dimensional vector of VectorType
    using PackedVector = Vector<VectorType, 3>;

    constexpr int N = VectorType::VectorSize;

    VectorType vrandom()
    {
        VectorType v;
        for (int i = 0; i < N; ++i)
        {
            v[i] = dist(mt);
        }
        return v;
    }

    struct Scene
    {
        AlignedVector<PackedVector> positions;
        AlignedVector<PackedVector> velocities;
        std::vector<uint32> colors;
        std::vector<float> radiuses;
        std::vector<float> rotations;

        Scene(int count)
            : positions(count / N)
            , velocities(count / N)
            , colors(count)
            , radiuses(count)
            , rotations(count)
        {
            count /= N;
            for (int i = 0; i < count; ++i)
            {
                positions[i].x = vrandom();
                positions[i].y = vrandom();
                positions[i].z = vrandom();
                velocities[i].x = vrandom();
                velocities[i].y = vrandom();
                velocities[i].z = vrandom();
            }
        }

        void transform()
        {
            const int count = positions.size();
            for (int i = 0; i < count; ++i)
            {
                positions[i].x += velocities[i].x;
                positions[i].y += velocities[i].y;
                positions[i].z += velocities[i].z;
            }

            /* generated code with g++ 7.1

            .L631:
            movaps  (%rax), %xmm0
            addq    $48, %rax
            addq    $48, %rcx
            addps   -48(%rcx), %xmm0
            movaps  %xmm0, -48(%rax)
            movaps  -32(%rax), %xmm0
            addps   -32(%rcx), %xmm0
            movaps  %xmm0, -32(%rax)
            movaps  -16(%rax), %xmm0
            addps   -16(%rcx), %xmm0
            movaps  %xmm0, -16(%rax)
            cmpq    %rax, %rsi
            jne     .L631

            */
        }
    };

} // namespace

// ----------------------------------------------------------------------
// main()
// ----------------------------------------------------------------------

/*
    Timings on i7 3770K processor using AVX instructions for 1,000,000 particles and 60 frames:

    method1: 263 ms (228 fps)
    method2: 151 ms (397 fps)
    method3: 121 ms (495 fps)
    method4: 110 ms (545 fps)

    Conclusion: the effects of memory layout can double the performance.
*/

int main(int argc, const char* argv[])
{
    const int count = 1000 * 1000;

    method1::Scene scene1(count);
    method2::Scene scene2(count);
    method3::Scene scene3(count);
    method4::Scene scene4(count);

    uint64 time1 = 0;
    uint64 time2 = 0;
    uint64 time3 = 0;
    uint64 time4 = 0;

    Timer timer;

    const int frames = 60;
    for (int i = 0; i < frames; ++i)
    {
        uint64 s0 = timer.ms();
        scene1.transform();

        uint64 s1 = timer.ms();
        scene2.transform();

        uint64 s2 = timer.ms();
        scene3.transform();

        uint64 s3 = timer.ms();
        scene4.transform();

        uint64 s4 = timer.ms();

        time1 += (s1 - s0);
        time2 += (s2 - s1);
        time3 += (s3 - s2);
        time4 += (s4 - s3);
    }

    printf("Rendered %d frames in...\n", frames);
    printf("time: %d ms (%d fps)\n", int(time1), int(frames * 1000 / time1));
    printf("time: %d ms (%d fps)\n", int(time2), int(frames * 1000 / time2));
    printf("time: %d ms (%d fps)\n", int(time3), int(frames * 1000 / time3));
    printf("time: %d ms (%d fps)\n", int(time4), int(frames * 1000 / time4));
}
