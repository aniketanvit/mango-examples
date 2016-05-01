/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2016 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/core/core.hpp>

using namespace mango;

void example1(const uint8* data, size_t size)
{
    // compute (maximum) compressed buffer size
    size_t compressedSize = lz4::bound(size);

    // allocate compressed buffer
    Buffer buffer(compressedSize);

    // compress with maximum compression rate (10)
    Memory compressed = lz4::compress(buffer, Memory(data, size), 10);

    // print results :)
    printf("compressed %d bytes to %d bytes.\n",
        int(size),
        int(compressed.size));
}

void example2(const Memory& compressed, size_t size)
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
    be allocated. Likewise, the returned Memory object tells how many
    bytes the compressed data consume. All memory management is left
    to the caller.
*/
