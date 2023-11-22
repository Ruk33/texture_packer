#include <stdlib.h> // calloc
#include <stdio.h>  // fprintf, FILE, fopen, fclose, fwrite

#define STBI_NO_SIMD

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char **argv) {
    if (argc == 1) {
        fprintf(stderr, 
                "usage: %s a.png b.png ...\n"
                "this generates two files:\n"
                "    output.png: image of 1024x1024 pixels containing a.png, b.png, etc in one file.\n"
                "    output.h: contains the coordinates and dimensions of all images.\n",
                argv[0]);
        return 0;
    }
    
    static stbrp_context context = {0};
    static stbrp_node nodes[1024] = {0};
    static stbrp_rect rects[1024] = {0};
    
    // Initialize with width and height of 1024
    stbrp_init_target(&context, 1024, 1024, nodes, 1024);
    
    // Create file with coordinates
    FILE *coordinates_file = fopen("output.h", "w+");
    if (!coordinates_file) {
        fprintf(stderr, "unable to create output.h file\n");
        return 0;
    }
    
    // Load the images
    int num_images = argc - 1;
    for (int i = 0; i < num_images; i++) {
        int n = 0;
        unsigned char *data = stbi_load(argv[i + 1], 
                                        &rects[i].w, 
                                        &rects[i].h, 
                                        &n, 
                                        0);
        free(data);
    }
    stbrp_pack_rects(&context, rects, num_images);
    
    // Start writing coordinates file
    fprintf(coordinates_file, 
            "static struct coordinates {\n"
            "    int x;\n"
            "    int y;\n"
            "    int w;\n"
            "    int h;\n"
            "    const char *path;\n"
            "} coordinates[] = {\n");
    
    // Create the output image
    unsigned char *output = calloc(1024 * 1024 * 4, sizeof(unsigned char));
    for (int i = 0; i < num_images; i++) {
        int x = rects[i].x;
        int y = rects[i].y;
        int w = rects[i].w;
        int h = rects[i].h;
        int n = 0;
        
        fprintf(coordinates_file, 
                "    {\n"
                "        %d,\n"
                "        %d,\n"
                "        %d,\n"
                "        %d,\n"
                "        \"%s\",\n"
                "    },\n",
                x,
                y,
                w,
                h,
                argv[i + 1]);
        
        unsigned char *data = stbi_load(argv[i + 1], &w, &h, &n, 0);
        for (int j = 0; j < h; j++) {
            for (int k = 0; k < w; k++) {
                int index = ((y + j) * 1024 + (x + k)) * 4;
                int data_index = (j * w + k) * n;
                output[index + 0] = data[data_index + 0];
                output[index + 1] = data[data_index + 1];
                output[index + 2] = data[data_index + 2];
                output[index + 3] = data[data_index + 3];
            }
        }
        free(data);
    }
    
    // Save the output image
    stbi_write_png("output.png", 1024, 1024, 4, output, 1024 * 4);
    
    fprintf(coordinates_file, "};\n");
    fclose(coordinates_file);
    
    return 0;
}
