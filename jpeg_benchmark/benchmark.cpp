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

Surface load_jpeg(const char* filename)
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
    unsigned char *data = new u8[dataSize];;
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

void save_jpeg(const char* filename, const Surface& surface)
{
    struct jpeg_compress_struct cinfo;
    jpeg_create_compress(&cinfo);

    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);

    FILE * outfile;
    if ((outfile = fopen(filename, "wb")) == NULL)
    {
        fprintf(stderr, "can't open %s\n", filename);
        exit(1);
    }
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = surface.width;
    cinfo.image_height = surface.height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    int quality = 95;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    JSAMPROW row_pointer[1];

    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = surface.image + cinfo.next_scanline * surface.stride;
        int x = jpeg_write_scanlines(&cinfo, row_pointer, 1);
        (void) x;
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);

    delete[] surface.image;
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

    uint64 time0 = timer.ms();
    printf("load libjpeg: ");

    Surface s = load_jpeg(argv[1]);

    uint64 time1 = timer.ms();
    printf("%d ms\n", int(time1 - time0));
    printf("load mango:   ");

    Bitmap bitmap(argv[1]);

    uint64 time2 = timer.ms();
    printf("%d ms\n", int(time2 - time1));
    printf("save libjpeg: ");

    save_jpeg("output-libjpeg.jpg", s);

    uint64 time3 = timer.ms();
    printf("%d ms\n", int(time3 - time2));
    printf("save mango:   ");

    bitmap.save("output-mango.jpg");

    uint64 time4 = timer.ms();
    printf("%d ms\n", int(time4 - time3));
}
