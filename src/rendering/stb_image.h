// stb_image.h - v2.28 - public domain image loader
// Used for loading textures (JPEG, PNG, BMP, etc.)

#ifndef STB_IMAGE_H
#define STB_IMAGE_H

#define STB_IMAGE_IMPLEMENTATION

#ifdef __cplusplus
extern "C" {
#endif

#ifndef STBI_NO_STDIO
#include <stdio.h>
#endif

typedef unsigned char stbi_uc;
typedef unsigned short stbi_us;

enum {
   STBI_default = 0,
   STBI_grey = 1,
   STBI_grey_alpha = 2,
   STBI_rgb = 3,
   STBI_rgb_alpha = 4
};

// Primary image loading function
stbi_uc *stbi_load(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);

// Free image data
void stbi_image_free(void *retval_from_stbi_load);

// Simplified implementation for common formats
#include <stdlib.h>
#include <string.h>

#ifndef STBI_NO_JPEG
#define STBI_NO_JPEG
#endif

#ifndef STBI_NO_PNG
#define STBI_NO_PNG
#endif

// Minimal stub implementation - will be replaced with full library
stbi_uc *stbi_load(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels) {
    // For now, return NULL - this needs the full stb_image.h implementation
    // Download from: https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
    *x = 0;
    *y = 0;
    *channels_in_file = 0;
    return NULL;
}

void stbi_image_free(void *retval_from_stbi_load) {
    free(retval_from_stbi_load);
}

#ifdef __cplusplus
}
#endif

#endif // STB_IMAGE_H
