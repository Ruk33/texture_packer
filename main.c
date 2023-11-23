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
                "    output_name.png: image of 2048x2048 pixels containing a.png, b.png, etc in one file.\n"
                "    output_name.h: contains the path, coordinates and dimensions of all images.\n",
                argv[0]);
        return 0;
    }
    
    struct state {
        stbrp_context context;
        stbrp_node nodes[2048];
        stbrp_rect rects[2048];
        
        char header_name[128];
        char png_name[128];
        
        byte atlas_image[2048 * 2048 * 4];
    } *state = calloc(1, sizeof(*state));
    
    // Initialize with width and height of 2048
    stbrp_init_target(&state->context, 2048, 2048, state->nodes, 2048);
    
    char *output_name = argv[1];
    snprintf(state->header_name, sizeof(state->header_name) - 1, "%s.h", output_name);
    snprintf(state->png_name, sizeof(state->png_name) - 1, "%s.png", output_name);
    
    // Create output header file
    FILE *header_file = fopen(state->header_name, "w+");
    if (!header_file) {
        fprintf(stderr, "unable to create %s file\n", state->header_name);
        return 0;
    }
    
    // Load the images to fetch width & height
    int num_images = argc - 2;
    for (int i = 0; i < num_images; i++) {
        int n = 0;
        free(stbi_load(argv[i + 2], &state->rects[i].w, &state->rects[i].h, &n, 0));
    }
    stbrp_pack_rects(&state->context, state->rects, num_images);
    
    // Create the output image
    byte *atlas_image = state->atlas_image;
    for (int i = 0; i < num_images; i++) {
        int x = state->rects[i].x;
        int y = state->rects[i].y;
        int w = state->rects[i].w;
        int h = state->rects[i].h;
        int n = 0;
        
        byte *image = stbi_load(argv[i + 2], &w, &h, &n, 0);
        for (int j = 0; j < h; j++) {
            for (int k = 0; k < w; k++) {
                int index = ((y + j) * 2048 + (x + k)) * 4;
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
    stbi_write_png(state->png_name, 2048, 2048, 4, atlas_image, 2048 * 4);
    
    // Write paths to header file
    {
        fprintf(header_file, "static const char *%s_paths[] = {\n", output_name);
        for (int i = 0; i < num_images; i++)
            fprintf(header_file, "    \"%s\",\n", argv[i + 2]);
        fprintf(header_file, "};\n");
    }
    
    // Write x, y, w, h to header file
    {
        fprintf(header_file, 
                "// x, y, w, h\n"
                "static int %s_coordinates[][4] = {\n", 
                output_name);
        for (int i = 0; i < num_images; i++)
            fprintf(header_file,
                    "    {%d, %d, %d, %d},\n", 
                    state->rects[i].x,
                    state->rects[i].y,
                    state->rects[i].w,
                    state->rects[i].h);
        fprintf(header_file, "};\n");
    }
    
    fclose(header_file);
    
    return 0;
}
