#include "zgl.h"
#include "pixel.h"

void glRasterPos3f(GLfloat x, GLfloat y, GLfloat z) {
    GLContext *c = gl_get_context();
    GLRasterPos *pos = &c->raster_pos;
    pos->x = x;
    pos->y = y;
    // TODO: update the zbuffer?
    pos->z = z;
}

const uintptr_t pbuf_pos(ZBuffer *zb, int x, int y) {
    return (uintptr_t)zb->pbuf + (y * zb->linesize) + x * PSZB;
}

void glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig,
              GLfloat xmove, GLfloat ymove, const GLubyte *bitmap) {
    GLContext *c = gl_get_context();
    GLRasterPos *pos = &c->raster_pos;
    ZBuffer *zb = c->zb;
    // TODO: shouldn't be drawn if the raster pos is outside the viewport?
    // TODO: negative width/height mirrors bitmap?
    if (!width && !height) {
        pos->x += xmove;
        pos->y -= ymove;
        return;
    }

    const GLubyte *from;
    // TODO: support 16-bit pixel size?
    uintptr_t to;
    int x, y;

    // copy to pixel data
    // TODO: strip blank lines and mirror vertically?
    for (y = 0; y < height; y++) {
        to = pbuf_pos(zb, pos->x, pos->y - y);
        from = bitmap + (y * 2);
        for (x = 0; x < width; x += 8) {
            if (pos->x + x > zb->xsize || pos->y + y > zb->ysize)
                continue;

            GLubyte b = *from++;
            for (int j = 8; j--; ) {
                GLuint value = (b & (1 << j)) ? 0xFFFFFFFF : 0;
#if TGL_FEATURE_RENDER_BITS == 32
                *(GLuint *)to = value;
#elif TGL_FEATURE_RENDER_BITS == 16
                *(GLushort *)to = value;
#endif
                to += PSZB;
            }
        }
    }

    pos->x += xmove;
    pos->y += ymove;
}

void glDrawPixels(GLsizei width, GLsizei height, GLenum format,
                  GLenum type, const GLvoid *data) {
    GLContext *c = gl_get_context();
    GLRasterPos *pos = &c->raster_pos;
    ZBuffer *zb = c->zb;
    GLViewport *viewport = &c->viewport;

    GLvoid *pixels, *from, *to;
    GLvoid *dst = NULL;

    // TODO: convert+blit directly into the pbuffer to save a copy?

    if (! pixel_convert(data, &dst, width, height, format, type, GL_BGRA,
#if TGL_FEATURE_RENDER_BITS == 32
                GL_UNSIGNED_BYTE
#elif TGL_FEATURE_RENDER_BITS == 16
                GL_UNSIGNED_SHORT_5_6_5
#endif
            )) {
        return;
    }
    pixels = (GLubyte *)dst;

    // shrink our pixel ranges to stay inside the viewport
    int ystart = MAX(0, -pos->y);
    height = MIN(viewport->ysize - pos->y, height);

    int xstart = MAX(0, -pos->x);
    int screen_width = MIN(viewport->xsize - pos->x, width);

    for (int y = ystart; y < height; y++) {
        to = (GLubyte *)pbuf_pos(zb, pos->x, pos->y - y);
        from = pixels + PSZB * (xstart + y * width);
        memcpy(to, from, PSZB * screen_width);
    }
    if (pixels != data)
        free(pixels);
}
