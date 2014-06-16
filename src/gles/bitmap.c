#include "zgl.h"
#include "pixel.h"

GLubyte *raster = NULL;

void glRasterPos3f(GLfloat x, GLfloat y, GLfloat z) {
    GLContext *c = gl_get_context();
    GLRasterPos *pos = &c->raster_pos;
    pos->x = x;
    pos->y = y;
    pos->z = z;
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
    // TODO: support 16-bit zbuffer
    GLuint *to;
    int x, y;

    // copy to pixel data
    // TODO: strip blank lines and mirror vertically?
    for (y = 0; y < height; y++) {
        to = (GLuint *)raster + (GLuint)(pos->x + ((pos->y - y) * zb->xsize));
        from = bitmap + (y * 2);
        for (x = 0; x < width; x += 8) {
            if (pos->x + x > zb->xsize || pos->y + y > zb->ysize)
                continue;

            GLubyte b = *from++;
            for (int j = 8; j--; ) {
                *to++ = (b & (1 << j)) ? 0xFFFFFFFF : 0;
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

    GLubyte *pixels, *from, *to;
    GLvoid *dst = NULL;

    if (! pixel_convert(data, &dst, width, height,
                        format, type, GL_RGBA, GL_UNSIGNED_BYTE)) {
        return;
    }
    pixels = (GLubyte *)dst;

    // shrink our pixel ranges to stay inside the viewport
    int ystart = MAX(0, -pos->y);
    height = MIN(viewport->ysize - pos->y, height);

    int xstart = MAX(0, -pos->x);
    int screen_width = MIN(viewport->xsize - pos->x, width);

    for (int y = ystart; y < height; y++) {
        to = raster + 4 * (GLuint)(pos->x + ((pos->y - y) * zb->xsize));
        from = pixels + 4 * (xstart + y * width);
        memcpy(to, from, 4 * screen_width);
    }
    if (pixels != data)
        free(pixels);
}
