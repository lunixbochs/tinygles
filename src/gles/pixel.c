#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pixel.h"
#include "gl_helpers.h"

static const colorlayout_t *get_color_map(GLenum format) {
    #define map(fmt, ...)                               \
        case fmt: {                                     \
        static colorlayout_t layout = {fmt, __VA_ARGS__}; \
        return &layout; }
    switch (format) {
        map(GL_ALPHA, -1, -1, -1, 0);
        map(GL_BGR, 2, 1, 0, -1);
        map(GL_BGRA, 2, 1, 0, 3);
        map(GL_LUMINANCE, 0, 0, 0, -1);
        map(GL_LUMINANCE_ALPHA, 0, 0, 0, 1);
        map(GL_RED, 0, -1, -1, -1);
        map(GL_RG, 0, 1, -1, -1);
        map(GL_RGB, 0, 1, 2, -1);
        map(GL_RGBA, 0, 1, 2, 3);
        default:
            printf("get_color_map(): Unknown pixel format 0x%x\n", format);
            break;
    }
    static colorlayout_t null = {0};
    return &null;
    #undef map
}

static inline
bool remap_pixel(const GLvoid *src, GLvoid *dst,
                 const colorlayout_t *src_color, GLenum src_type,
                 const colorlayout_t *dst_color, GLenum dst_type) {

    #define type_case(constant, type, ...)        \
        case constant: {                          \
            const type *s = (const type *)src;    \
            type *d = (type *)dst;                \
            type v = *s;                          \
            __VA_ARGS__                           \
            break;                                \
        }

    #define default(arr, amod, vmod, key, def) \
        key >= 0 ? arr[amod key] vmod : def

    #define carefully(arr, amod, key, value) \
        if (key >= 0) d[amod key] = value;

    #define read_each(amod, vmod)                                 \
        pixel.r = default(s, amod, vmod, src_color->red, 0);      \
        pixel.g = default(s, amod, vmod, src_color->green, 0);    \
        pixel.b = default(s, amod, vmod, src_color->blue, 0);     \
        pixel.a = default(s, amod, vmod, src_color->alpha, 1.0f);

    #define write_each(amod, vmod)                         \
        carefully(d, amod, dst_color->red, pixel.r vmod)   \
        carefully(d, amod, dst_color->green, pixel.g vmod) \
        carefully(d, amod, dst_color->blue, pixel.b vmod)  \
        carefully(d, amod, dst_color->alpha, pixel.a vmod)

    // this pixel stores our intermediate color
    // it will be RGBA and normalized to between (0.0 - 1.0f)
    pixel_t pixel;
    switch (src_type) {
        type_case(GL_DOUBLE, GLdouble, read_each(,))
        type_case(GL_FLOAT, GLfloat, read_each(,))
        case GL_UNSIGNED_INT_8_8_8_8_REV:
        type_case(GL_UNSIGNED_BYTE, GLubyte, read_each(, / 255.0))
        type_case(GL_UNSIGNED_INT_8_8_8_8, GLubyte, read_each(3 - , / 255.0))
        type_case(GL_UNSIGNED_SHORT_1_5_5_5_REV, GLushort,
            s = (GLushort[]){
                v & 31,
                (v & 0x03e0 >> 5) / 31.0,
                (v & 0x7c00 >> 10) / 31.0,
                (v & 0x8000 >> 15) / 31.0,
            };
            read_each(,);
        )
        default:
            // TODO: add glSetError?
            printf("remap_pixel(): Unsupported source data type: %i\n", src_type);
            return false;
            break;
    }

    switch (dst_type) {
        type_case(GL_FLOAT, GLfloat, write_each(,))
        type_case(GL_UNSIGNED_BYTE, GLubyte, write_each(, * 255.0))
        // TODO: force 565 to RGB? then we can change [4] -> 3
        type_case(GL_UNSIGNED_SHORT_5_6_5, GLushort,
            GLfloat color[3];
            color[dst_color->red] = pixel.r;
            color[dst_color->green] = pixel.g;
            color[dst_color->blue] = pixel.b;
            *d = ((GLuint)(color[0] * 31) & 0x1f << 11) |
                 ((GLuint)(color[1] * 63) & 0x3f << 5) |
                 ((GLuint)(color[2] * 31) & 0x1f);
        )
        type_case(GL_UNSIGNED_SHORT_5_5_5_1, GLushort,
            GLfloat color[4];
            color[dst_color->red] = pixel.r;
            color[dst_color->green] = pixel.g;
            color[dst_color->blue] = pixel.b;
            color[dst_color->alpha] = pixel.a;
            // TODO: can I macro this or something? it follows a pretty strict form.
            *d = ((GLuint)(color[0] * 31) & 0x1f << 0) |
                 ((GLuint)(color[1] * 31) & 0x1f << 5) |
                 ((GLuint)(color[2] * 31) & 0x1f << 10)  |
                 ((GLuint)(color[3] * 1)  & 0x01 << 15);
        )
        default:
            printf("remap_pixel(): Unsupported target data type: 0x%x\n", dst_type);
            return false;
            break;
    }
    return true;

    #undef type_case
    #undef default
    #undef carefully
    #undef read_each
    #undef write_each
}

bool pixel_convert(const GLvoid *src, GLvoid **dst,
                   GLuint width, GLuint height,
                   GLenum src_format, GLenum src_type,
                   GLenum dst_format, GLenum dst_type) {
    if (src_format == GL_RGBA && dst_format == GL_BGRA && src_type == GL_UNSIGNED_BYTE && dst_type == GL_UNSIGNED_SHORT_5_6_5) {
        int size = (width * height);
        GLushort *out = *dst = malloc(size * 2);
        const GLubyte *pixels = (const GLubyte *)src;
        for (int i = 0; i < size; i++) {
            char r, g, b;
            r = pixels[0] * 0x1f / 0xff;
            g = pixels[1] * 0x3f / 0xff;
            b = pixels[2] * 0x1f / 0xff;

            *out++ = ((r & 0x1f) << 11) | ((g & 0x3f) << 5) | (b & 0x1f);
            pixels += 4;
        }
        return true;
    }
    if (src_format == GL_RGBA && dst_format == GL_BGRA && src_type == dst_type && src_type == GL_UNSIGNED_BYTE) {
        int size = (width * height);
        GLubyte *out = *dst = malloc(size * 4);
#ifdef __ARM_NEON__
        asm volatile (
            "mov r0, #0\n"
            ".loop:\n"
                "pld [%0]\n"
                "vld4.8 {d0, d1, d2, d3}, [%0]\n"
                "vswp d0, d2\n"
                "vst4.8 {d0, d1, d2, d3}, [%1]\n"

                "add %0, %0, #32\n"
                "add %1, %1, #32\n"

                "add r0, r0, #8\n"
                "cmp r0, %2\n"
                "blt .loop\n"
            :
            : "r"(src), "r"(out), "r"(width * height)
            : "r0", "d0", "d1", "d2", "d3"
        );
        /*
        asm volatile (
            "pld [%0]\n"

            "mov r0, #0xFF000000\n"
            "vdup.32 q3, r0\n"
            "mov r0, #0x0000FF00\n"
            "vdup.32 q4, r0\n"
            "lsl r0, r0, #8\n"
            "orr r0, r0, #0xFF\n"
            "vdup.32 q5, r0\n"

            "mov r0, %2\n"
            ".loop:\n"
                "vld1.32 {d0, d1}, [%0]\n"
                "vld1.32 {d2, d3}, [%0]\n"
                "vld1.32 {d4, d5}, [%0]\n"

                "vand.u32 q0, q3\n"
                "vand.u32 q1, q4\n"
                "vand.u32 q2, q5\n"

                "vshr.u32 q0, q0, #16\n"
                "vshl.u32 q1, q1, #16\n"

                "vorr q0, q0, q1\n"
                "vorr q0, q0, q2\n"

                "vst1.32 {d0, d1}, [%1]\n"

                "add %0, %0, #4\n"
                "add %1, %1, #4\n"

                "sub r0, r0, #1\n"
                "cmp r0, #0\n"
                "bgt .loop\n"
            :
            : "r"(src), "r"(out), "r"(width * height - 2)
            : "r0", "d0", "d1", "d2", "d3"
        );
        */
#else
        const GLubyte *pixels = (const GLubyte *)src;
        for (int i = 0; i < size; i++) {
            out[0] = pixels[2];
            out[1] = pixels[1];
            out[2] = pixels[0];
            out[3] = pixels[3];
            out += 4;
            pixels += 4;
        }
#endif
        return true;
    }
    const colorlayout_t *src_color, *dst_color;
    GLuint pixels = width * height;
    GLuint dst_size = pixels * gl_pixel_sizeof(dst_format, dst_type);

    // printf("pixel conversion: %ix%i - %i, %i -> %i, %i\n", width, height, src_format, src_type, dst_format, dst_type);
    src_color = get_color_map(src_format);
    dst_color = get_color_map(dst_format);
    if (!dst_size || !gl_pixel_sizeof(src_format, src_type)
        || !src_color->type || !dst_color->type)
        return false;

    if (src_type == dst_type && src_color->type == dst_color->type) {
        if (*dst != src) {
            *dst = malloc(dst_size);
            memcpy(*dst, src, dst_size);
            return true;
        }
    } else {
        GLsizei src_stride = gl_pixel_sizeof(src_format, src_type);
        GLsizei dst_stride = gl_pixel_sizeof(dst_format, dst_type);
        *dst = malloc(dst_size);
        uintptr_t src_pos = (uintptr_t)src;
        uintptr_t dst_pos = (uintptr_t)*dst;
        for (int i = 0; i < pixels; i++) {
            if (! remap_pixel((const GLvoid *)src_pos, (GLvoid *)dst_pos,
                              src_color, src_type, dst_color, dst_type)) {
                // checking a boolean for each pixel like this might be a slowdown?
                // probably depends on how well branch prediction performs
                return false;
            }
            src_pos += src_stride;
            dst_pos += dst_stride;
        }
        return true;
    }
    return false;
}

bool pixel_scale(const GLvoid *old, GLvoid **new,
                 GLuint width, GLuint height,
                 GLfloat ratio,
                 GLenum format, GLenum type) {
    GLuint pixel_size, new_width, new_height;
    new_width = width * ratio;
    new_height = height * ratio;
    printf("scaling %ux%u -> %ux%u\n", width, height, new_width, new_height);
    GLvoid *dst;
    uintptr_t src, pos, pixel;

    pixel_size = gl_pixel_sizeof(format, type);
    dst = malloc(pixel_size * new_width * new_height);
    src = (uintptr_t)old;
    pos = (uintptr_t)dst;
    for (int x = 0; x < new_width; x++) {
        for (int y = 0; y < new_height; y++) {
            pixel = src + (x / ratio) +
                          (y / ratio) * width;
            memcpy((GLvoid *)pos, (GLvoid *)pixel, pixel_size);
            pos += pixel_size;
        }
    }
    *new = dst;
    return true;
}

bool pixel_to_ppm(const GLvoid *pixels, GLuint width, GLuint height,
                  GLenum format, GLenum type, GLuint name) {
    if (! pixels)
        return false;

    const GLvoid *src;
    char filename[64];
    int size = 4 * 3 * width * height;
    if (format == GL_RGB && type == GL_UNSIGNED_BYTE) {
        src = pixels;
    } else {
        if (! pixel_convert(pixels, (GLvoid **)&src, width, height, format, type, GL_RGB, GL_UNSIGNED_BYTE)) {
            return false;
        }
    }

    snprintf(filename, 64, "/tmp/tex.%d.ppm", name);
    FILE *fd = fopen(filename, "w");
    fprintf(fd, "P6 %d %d %d\n", width, height, 255);
    fwrite(src, 1, size, fd);
    fclose(fd);
    return true;
}
