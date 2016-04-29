/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2016 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/math/math.hpp>

using namespace mango;

void example1()
{
    float4 a(1.0f, 2.0f, 3.0f, 4.0f);
    float4 b = sin(a);
    float4 c = cross(a, b) * 1.5f - a * 2.0f * dot(a, b * 3.0f);
}

float4 example2(const float4& a, const float4& b)
{
    float4 ab = a.xxyy * 2.0f - b * b.wwww;
    return ab / ab.xxxx;
}

float4 example3(const float4& a, const float4& b)
{
    // select() is a bit more exotic operation; it uses a mask to select between
    // two values. The mask is generated in the comparison operator. This is useful
    // for avoiding branching; it is sometimes more efficient to compute both
    // results and choose one depending on some predicate (in this case a > b).

    float4 result = select(a > b, sin(a), cos(b));
    return result;
}

float4 example4(const float4& a, const float4& b)
{
    float4 result;

    // Same as example3 but using scalars instead of select()
    for (int i = 0; i < 4; ++i)
    {
        result[i] = a[i] > b[i] ? sin(a[i]) : cos(b[i]);
    }

    return result;
}

float3 example5(float3 a, float3 b, float3 c)
{
    // compute triangle normal given three vertices (a, b, c)
    float3 normal = cross(a - b, a - c);
    return normalize(normal);
}

float4 example6(float4 a)
{
    // a slower way to do a.zwxy to demonstrate decomposing vectors
    float2 low = a.xy;
    float2 high = a.zw;
    return float4(high, low);
}

void example7(float3 normal, float dist)
{
    Plane plane(normal, dist);
    float3 p(20.0f, 0.0f, 0.0f);
    float distanceToPlane = plane.distance(p);
    if (distanceToPlane < 0)
    {
        // point p is behind the plane
    }
}

void example8(const Plane& plane, float3 point0, float3 point1)
{
    Ray ray(point0, point1);

    Intersect is;
    if (is.intersect(ray, plane))
    {
        // compute point of intersection
        float3 p = ray.origin + ray.direction * is.t0;
    }
}

void example9(const Sphere& sphere, const Ray& ray)
{
    IntersectRange is;
    if (is.intersect(ray, sphere))
    {
        // compute points where ray enters and leaves the sphere
        float3 enter = ray.origin + ray.direction * is.t0;
        float3 leave = ray.origin + ray.direction * is.t1;
    }
}

void example10(const std::vector<Box>& boxes, const Ray& ray)
{
    // FastRay has precomputed data to accelerate intersection computations
    // However, it has overhead for doing this computation so it is best used
    // when the same ray is intersected to a lot of different geometry
    FastRay fast(ray);

    for (auto& box : boxes)
    {
        IntersectRange is;
        if (is.intersect(box, fast))
        {
            float3 enter = ray.origin + ray.direction * is.t0;
            float3 leave = ray.origin + ray.direction * is.t1;
        }
    }
}

void example11()
{
    float4 linear(1.0f, 0.5f, 0.5f, 1.0f);
    float4 nonlinear = srgb_encode(linear);
    linear = srgb_decode(nonlinear);
}
