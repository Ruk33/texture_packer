#include <stdlib.h> // calloc
#include <stdio.h>  // fprintf, FILE, fopen, fclose, fwrite

#define STBI_NO_SIMD

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

typedef unsigned char byte;

int main(int argc, char **argv) {
    if (argc == 1) {
        fprintf(stderr, 
                "usage: %s output_name a.png b.png ...\n"
                "this generates two files:\n"
                "    output_name.png: image of 1024x1024 pixels containing a.png, b.png, etc in one file.\n"
                "    output_name.h: contains the path, coordinates and dimensions of all images.\n",
                argv[0]);
        return 0;
    }
    
    static stbrp_context context = {0};
    static stbrp_node nodes[1024] = {0};
    static stbrp_rect rects[1024] = {0};
    
    // Initialize with width and height of 1024
    stbrp_init_target(&context, 1024, 1024, nodes, 1024);
    
    // Create file with coordinates
    char *output_name = argv[1];
    char header_name[64] = {0};
    char png_name[64] = {0};
    snprintf(header_name, sizeof(header_name) - 1, "%s.h", output_name);
    snprintf(png_name, sizeof(png_name) - 1, "%s.png", output_name);
    
    FILE *header_file = fopen(header_name, "w+");
    if (!header_file) {
        fprintf(stderr, "unable to create %s file\n", header_name);
        return 0;
    }
    
    // Load the images to fetch width & height.
    int num_images = argc - 2;
    for (int i = 0; i < num_images; i++) {
        int n = 0;
        free(stbi_load(argv[i + 2], &rects[i].w, &rects[i].h, &n, 0));
    }
    stbrp_pack_rects(&context, rects, num_images);
    
    // Create the output image
    byte *atlas_image = calloc(1024 * 1024 * 4, sizeof(byte));
    for (int i = 0; i < num_images; i++) {
        int x = rects[i].x;
        int y = rects[i].y;
        int w = rects[i].w;
        int h = rects[i].h;
        int n = 0;
        
        byte *image = stbi_load(argv[i + 2], &w, &h, &n, 0);
        for (int j = 0; j < h; j++) {
            for (int k = 0; k < w; k++) {
                int index = ((y + j) * 1024 + (x + k)) * 4;
                int pixel_index = (j * w + k) * n;
                atlas_image[index + 0] = image[pixel_index + 0];
                atlas_image[index + 1] = image[pixel_index + 1];
                atlas_image[index + 2] = image[pixel_index + 2];
                atlas_image[index + 3] = image[pixel_index + 3];
            }
        }
        free(image);
    }
    
    // Save the output image
    stbi_write_png(png_name, 1024, 1024, 4, atlas_image, 1024 * 4);
    
    // Write paths
    fprintf(header_file, "static const char *%s_paths[] = {\n", output_name);
    for (int i = 0; i < num_images; i++)
        fprintf(header_file, "    \"%s\",\n", argv[i + 2]);
    fprintf(header_file, "};\n");
    
    // Write x, y, w, h.
    fprintf(header_file, 
            "// x, y, w, h\n"
            "static int %s_coordinates[][4] = {\n", 
            output_name);
    for (int i = 0; i < num_images; i++)
        fprintf(header_file,
                "    {%d, %d, %d, %d},\n", 
                rects[i].x,
                rects[i].y,
                rects[i].w,
                rects[i].h);
    fprintf(header_file, "};\n");
    
    fclose(header_file);
    
    return 0;
}
