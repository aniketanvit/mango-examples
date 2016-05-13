/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2016 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/mango.hpp>
#include <mango/vulkan/vulkan.hpp>

using namespace mango::vulkan;

/*
    Vulkan mini-engine examples. The goal is that these utility classes
    make life of a Vulkan developer a life worth living. The resemblance
    to existing APIs is result of reader's unconstrained imagination.

    Here's the basic pattern the design is following throughout:

    namespace internal {
        class VertexBuffer
        { ... };
    }

    using VertexBuffer = std::shared_ptr<internal::VertexBuffer>;

    In other words, everything is reference counted and lives in the heap.
    This creates some overhead but on the plus side it is less fuss to
    juggle the objects around in multiple threads with various lifetimes.
*/

VertexBuffer example1(Device& device, const std::vector<float>& fv)
{
    size_t bytes = fv.size() * sizeof(float);
    VertexBuffer vb = device.createVertexBuffer(bytes);

    float* ptr = vb->lock<float>();
    memcpy(ptr, fv.data(), bytes);
    vb->unlock();

    return vb;
}
