/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2016 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/core/core.hpp>

using namespace mango;

void example1(const uint8* p)
{
    // unaligned little-endian loads
    // address must be manually computed and updated
    uint16 a = uload16le(p + 0);
    uint16 b = uload16le(p + 2);
    uint32 c = uload32le(p + 4);
    p += 8;
}

void example2(const uint8* address)
{
    // same as example1 but using "endian aware pointers"
    LittleEndianConstPointer p = address;
    uint16 a = p.read16();
    uint16 b = p.read16();
    uint32 c = p.read32();
}

void example3(LittleEndianConstPointer& p)
{
    float f = p.read32f();
    p += 20; // skip 20 bytes
    half h = p.read16f();
    uint64 a = p.read64();
}

void example4(Stream& stream)
{
    // Initialize endian aware streaming translator
    LittleEndianStream s = stream;

    // The translator extends Stream interface to understand endianess..
    uint32 a = s.read32();
    float b = s.read32f();
    s.seek(20, Stream::CURRENT); // skip 20 bytes

    // read a block of memory
    uint32 size = s.read32();
    char* buffer = new char[size];
    s.read(buffer, size);
    delete[] buffer;
}

// Wait! There's more! If you clone MANGO from github.com now you will get endianess-aware types for FREE!
// NO ADDITIONAL CHARGE!

struct SomeHeader
{
    uint16be a;
    uint16be b;
    uint32be c;
};

void example5(const uint8* p)
{
    // reinterpret raw storage as SomeHeader
    const SomeHeader& header = *reinterpret_cast<const SomeHeader *>(p);

    // convert-on-read endianess conversion
    int a = header.a;
    int b = header.b;
    int c = header.c;
}

/*
    All of the streaming interfaces are super light-weight and
    every decision is done at compile time; the correct code is called
    directly without any dispatching logic.

    This line of code:
    uint32 x = p.read32();

    Would see that we are reading from, say, Little Endian pointer and
    that we are compiling for little endian architecture, thus no
    translation will be required. The data will be fetched using
    unaligned memory read operation. The load operation is selected using
    compile time information about the architecture and how it handles
    alignment. It will just work. However, if the client can guarantee
    correct alignment the memory load operation could potentially be
    more efficient.

    This is equal to doing this:
    char* p = ...;
    uint32 x = *reinterpret_cast<uint32 *>(p);
    p += sizeof(uint32);

    The difference is that our way is less typing and always guaranteed
    to be correct and use the best available implementation.
*/
