/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2017 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/mango.hpp>

using namespace mango;

// -----------------------------------------------------------------
// pipelined jpeg reader
// -----------------------------------------------------------------

void test_jpeg(const std::string& folder)
{
    ConcurrentQueue q("jpeg reader testloop");
    //SerialQueue q("jpeg reader testloop");

    Path path(folder);
    const size_t count = path.size();

    std::atomic<size_t> image_bytes { 0 };

    for (size_t i = 0; i < count; ++i)
    {
        const auto& node = path[i];
        if (!node.isDirectory())
        {
            std::string filename = node.name;
            q.enqueue([&path, filename, i, count, &image_bytes] {
                printf("filename: %s (%zu / %zu) begin.\n", filename.c_str(), i + 1, count);
                File file(path, filename);
                Bitmap bitmap(file, filename);
                image_bytes += bitmap.width * bitmap.height * 4;
                printf("filename: %s (%zu / %zu) done.\n", filename.c_str(), i + 1, count);
            });
        }
    }

    q.wait();
    printf("image: %zu MB\n", image_bytes / (1024 * 1024));
}

// -----------------------------------------------------------------
// main
// -----------------------------------------------------------------

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        printf("Too few arguments. Usage: %s <folder>\n", argv[0]);
        return 1;
    }

    test_jpeg(argv[1]);
    printf("* done *\n");
}
