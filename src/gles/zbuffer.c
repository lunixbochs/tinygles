/*
 * Z buffer: 16 bits Z / 16 bits color
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "zbuffer.h"

ZBuffer *ZB_open(int xsize, int ysize, int mode,
                 int nb_colors,
                 unsigned char *color_indexes,
                 int *color_table,
                 void *frame_buffer) {
    ZBuffer *zb;
    int size;

    zb = malloc(sizeof(ZBuffer));
    if (zb == NULL)
        return NULL;

    zb->xsize = xsize;
    zb->ysize = ysize;
    zb->mode = mode;
    zb->linesize = (xsize * PSZB + 3) & ~3;

    switch (mode) {
#ifdef TGL_FEATURE_8_BITS
        case ZB_MODE_INDEX:
            ZB_initDither(zb, nb_colors, color_indexes, color_table);
            break;
#endif
#ifdef TGL_FEATURE_32_BITS
        case ZB_MODE_RGBA:
#endif
#ifdef TGL_FEATURE_24_BITS
        case ZB_MODE_RGB24:
#endif
        case ZB_MODE_5R6G5B:
            zb->nb_colors = 0;
            break;
        default:
            goto error;
    }

    size = zb->xsize * zb->ysize * sizeof(unsigned short);

    zb->zbuf = malloc(size);
    if (zb->zbuf == NULL)
        goto error;

    if (frame_buffer == NULL) {
        zb->pbuf = malloc(zb->ysize * zb->linesize);
        if (zb->pbuf == NULL) {
            free(zb->zbuf);
            goto error;
        }
        zb->frame_buffer_allocated = 1;
    } else {
        zb->frame_buffer_allocated = 0;
        zb->pbuf = frame_buffer;
    }

    zb->current_texture = NULL;
    return zb;
error:
    free(zb);
    return NULL;
}

void ZB_close(ZBuffer * zb) {
#ifdef TGL_FEATURE_8_BITS
    if (zb->mode == ZB_MODE_INDEX)
        ZB_closeDither(zb);
#endif

    if (zb->frame_buffer_allocated)
        free(zb->pbuf);

    free(zb->zbuf);
    free(zb);
}

void ZB_resize(ZBuffer * zb, void *frame_buffer, int xsize, int ysize) {
    int size;

    /* xsize must be a multiple of 4 */
    xsize = xsize & ~3;

    zb->xsize = xsize;
    zb->ysize = ysize;
    zb->linesize = (xsize * PSZB + 3) & ~3;

    size = zb->xsize * zb->ysize * sizeof(unsigned short);

    free(zb->zbuf);
    zb->zbuf = malloc(size);

    if (zb->frame_buffer_allocated)
        free(zb->pbuf);

    if (frame_buffer == NULL) {
        zb->pbuf = malloc(zb->ysize * zb->linesize);
        zb->frame_buffer_allocated = 1;
    } else {
        zb->pbuf = frame_buffer;
        zb->frame_buffer_allocated = 0;
    }
}

static void ZB_copyBuffer(ZBuffer * zb, void *buf, int linesize) {
    unsigned char *p1;
    PIXEL *q;
    int y, n;

    q = zb->pbuf;
    p1 = buf;
    n = zb->xsize * PSZB;
    for (y = 0; y < zb->ysize; y++) {
        memcpy(p1, q, n);
        p1 += linesize;
        q = (PIXEL *)((char *)q + zb->linesize);
    }
}

#define RGB32_TO_RGB16(v) \
  (((v >> 8) & 0xf800) | (((v) >> 5) & 0x07e0) | (((v) & 0xff) >> 3))

/* XXX: not optimized */
void ZB_copyFrameBuffer5R6G5B(ZBuffer *zb, void *buf, int linesize) {
#if 0
    asm volatile (
        "mov r0, #0\n"
        ".outer:\n"
            "mov r1, %3\n"
            "mov r2, %0\n"

            ".inner:\n"
                "pld [%1]\n"
                "vld4.8 {d0, d1, d2, d3}, [%1]\n"

                "vshll.u8 q3, d1, #8\n"
                "vshll.u8 q5, d0, #8\n"
                "vshll.u8 q4, d2, #8\n"

                "vsri.u16 q5, q3, #5\n"
                "vsri.u16 q5, q4, #11\n"

                "vst1.32 {d10, d11}, [r2]\n"

                "add %1, %1, #32\n"
                "add r2, r2, #16\n"

                "sub r1, r1, #2\n"
                "cmp r1, #0\n"
                "bgt .inner\n"

            "add %0, %0, %4\n"
            "add r0, r0, #1\n"
            "cmp r0, %2\n"
            "blt .outer\n"

        :
        : "r"(buf), "r"(zb->pbuf), "r"(zb->ysize), "r"(zb->xsize >> 2), "r"(linesize)
        : "r0", "r1", "r2", "q0", "q1", "q2", "q3", "q4", "q5"
    );
    /*
    asm volatile (
        "mov r0, #0xf800\n"
        "vdup.32 d6, r0\n"
        "mov r0, #0x07e0\n"
        "vdup.32 d7, r0\n"
        "mov r0, #0x1f\n"
        "vdup.32 d8, r0\n"

        "mov r0, #0\n"
        ".outer:\n"
            "mov r1, %3\n"
            "mov r2, %0\n"

            ".inner:\n"
                "pld [%1]\n"
                "vld1.32 {d0, d1}, [%1]\n"
                "vld1.32 {d2, d3}, [%1]\n"
                "vld1.32 {d4, d5}, [%1]\n"

                "vshrn.u32 d0, q0, #8\n"
                "vshrn.u32 d1, q1, #5\n"
                "vshrn.u32 d2, q2, #3\n"

                "vand.u16 d0, d6\n"
                "vand.u16 d1, d7\n"
                "vand.u16 d2, d8\n"
                "vorr d0, d0, d1\n"
                "vorr d0, d0, d2\n"

                "vst1.32 d0, [r2]\n"

                "add %1, %1, #16\n"
                "add r2, r2, #8\n"

                "sub r1, r1, #1\n"
                "cmp r1, #0\n"
                "bgt .inner\n"

            "add %0, %0, %4\n"
            "add r0, r0, #1\n"
            "cmp r0, %2\n"
            "blt .outer\n"

        :
        : "r"(buf), "r"(zb->pbuf), "r"(zb->ysize), "r"(zb->xsize >> 2), "r"(linesize)
        : "r0", "r1", "r2", "q0", "q1", "q2", "q3", "q4", "q5", "q6"
    );
    */
#else
    PIXEL *q = zb->pbuf;
    unsigned short *p, *p1 = (unsigned short *) buf;
    for (int y = 0; y < zb->ysize; y++) {
        int n = zb->xsize >> 2;
        p = p1;
        do {
            p[0] = RGB32_TO_RGB16(q[0]);
            p[1] = RGB32_TO_RGB16(q[1]);
            p[2] = RGB32_TO_RGB16(q[2]);
            p[3] = RGB32_TO_RGB16(q[3]);
            q += 4;
            p += 4;
        } while (--n > 0);
        p1 = (unsigned short *)((char *)p1 + linesize);
    }
#endif
}

void ZB_copyFrameBuffer(ZBuffer * zb, void *buf, int linesize) {
    switch (zb->mode) {
#ifdef TGL_FEATURE_16_BITS
        case ZB_MODE_5R6G5B:
            ZB_copyFrameBuffer5R6G5B(zb, buf, linesize);
            break;
#endif
#ifdef TGL_FEATURE_32_BITS
        case ZB_MODE_RGBA:
            ZB_copyBuffer(zb, buf, linesize);
            break;
#endif
        default:
            assert(0);
    }
}

/*
 * adr must be aligned on an 'int'
 */
static void memset_short(void *adr, int val, int count) {
    int i, n, v;
    unsigned int *p;
    unsigned short *q;

    p = adr;
    v = val | (val << 16);

    n = count >> 3;
    for (i = 0; i < n; i++) {
        p[0] = v;
        p[1] = v;
        p[2] = v;
        p[3] = v;
        p += 4;
    }

    q = (unsigned short *) p;
    n = count & 7;
    for (i = 0; i < n; i++)
        *q++ = val;
}

static void memset_long(void *adr, int val, int count) {
    int i, n, v;
    unsigned int *p;

    p = adr;
    v = val;
    n = count >> 2;
    for (i = 0; i < n; i++) {
        p[0] = v;
        p[1] = v;
        p[2] = v;
        p[3] = v;
        p += 4;
    }

    n = count & 3;
    for (i = 0; i < n; i++)
        *p++ = val;
}

/* count must be a multiple of 4 and >= 4 */
void memset_RGB24(void *adr, int r, int v, int b, long count) {
    long i, n;
    register long v1, v2, v3,*pt = (long *)(adr);
    unsigned char *p, R = (unsigned char)r, V = (unsigned char)v , B = (unsigned char)b;

    p=(unsigned char *)adr;
    *p++ = R;
    *p++ = V;
    *p++ = B;
    *p++ = R;
    *p++ = V;
    *p++ = B;
    *p++ = R;
    *p++ = V;
    *p++ = B;
    *p++ = R;
    *p++ = V;
    *p++ = B;
    v1 = *pt++;
    v2 = *pt++;
    v3 = *pt++;
    n = count >> 2;
    for(int i = 1; i < n; i++) {
        *pt++ = v1;
        *pt++ = v2;
        *pt++ = v3;
    }
}

void ZB_clear(ZBuffer * zb, int clear_z, int z, int clear_color, int r, int g, int b) {
    int color;
    int y;
    PIXEL *pp;

    if (clear_z) {
        memset_short(zb->zbuf, z, zb->xsize * zb->ysize);
    }
    if (clear_color) {
        pp = zb->pbuf;
        for (y = 0; y < zb->ysize; y++) {
            color = RGB_TO_PIXEL(r, g, b);
            memset_long(pp, color, zb->xsize);
            pp = (PIXEL *) ((char *)pp + zb->linesize);
        }
    }
}
