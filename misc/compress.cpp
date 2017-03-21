/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2016 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/core/core.hpp>

using namespace mango;

void example1(const uint8* data, size_t size)
{
    // compute (maximum) compressed buffer size
    size_t compressed_size = lz4::bound(size);

    // allocate compressed buffer
    Buffer buffer(compressed_size);

    // compress with maximum compression rate (10)
    size_t compressed = lz4::compress(buffer, Memory(data, size), 10);

    // print results :)
    printf("compressed %zu bytes to %zu bytes.\n", size, compressed);
}

void example2(Memory compressed, size_t size)
{
    // allocate buffer for decompressed data
    Buffer buffer(size);

    // decompress :)
    lz4::decompress(buffer, compressed);
}

/*
    It is important to notice that the compress/decompress do not
    allocate memory for the result. The Memory object only describes
    the input and output memory blocks. Notice how the example1 does
    first compure the bound() so that the correct amount of memory can
    be allocated. All memory management is left to the caller.
*/
