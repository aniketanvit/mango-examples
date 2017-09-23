/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2016 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/mango.hpp>

using namespace mango;

// ----------------------------------------------------------------------
// warmup()
// ----------------------------------------------------------------------

void warmup(const char* filename)
{
    File file(filename);
    Memory memory = file;
    std::vector<char> buffer(memory.size);
    std::memcpy(buffer.data(), memory.address, memory.size);
}

// ----------------------------------------------------------------------
// libjpeg
// ----------------------------------------------------------------------

#include <jpeglib.h>
#include <jerror.h>

Surface LoadJPEG(const char* filename)
{
    FILE *file = fopen(filename, "rb" );
    if ( file == NULL )
    {
        return Surface(0, 0, FORMAT_NONE, 0, NULL);
    }

    struct jpeg_decompress_struct info; //for our jpeg info
    struct jpeg_error_mgr err; //the error handler

    info.err = jpeg_std_error( &err );
    jpeg_create_decompress( &info ); //fills info structure

    jpeg_stdio_src( &info, file );
    jpeg_read_header( &info, true );

    jpeg_start_decompress( &info );

    int w = info.output_width;
    int h = info.output_height;
    int numChannels = info.num_components; // 3 = RGB, 4 = RGBA
    unsigned long dataSize = w * h * numChannels;

    // read scanlines one at a time & put bytes in jdata[] array (assumes an RGB image)
    unsigned char *data = (unsigned char *)malloc( dataSize );
    unsigned char *rowptr[ 1 ]; // array or pointers
    for ( ; info.output_scanline < info.output_height ; )
    {
        rowptr[ 0 ] = data + info.output_scanline * w * numChannels;
        jpeg_read_scanlines( &info, rowptr, 1 );
    }

    jpeg_finish_decompress( &info );

    fclose( file );

    // TODO: free data, format depends on numChannels
    return Surface(w, h, FORMAT_R8G8B8, w * numChannels, data);
}

// ----------------------------------------------------------------------
// main()
// ----------------------------------------------------------------------

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        printf("Too few arguments. usage: <filename.jpg>\n");
        exit(1);
    }

    warmup(argv[1]);

    Timer timer;

    double time0 = timer.time();
    Bitmap bitmap(argv[1]);
    //bitmap.save("output-mango.jpg");

    double time1 = timer.time();
    Surface s = LoadJPEG(argv[1]);
    //s.save("output-libjpeg.jpg");
    delete[] s.image;

    double time2 = timer.time();

    printf("mango:   %f ms\n", (time1 - time0) * 1000);
    printf("libjpeg: %f ms\n", (time2 - time1) * 1000);
    printf("processing time: %f %%\n", (time1-time0)/(time2-time1) * 100);
}
