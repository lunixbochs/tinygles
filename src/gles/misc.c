#include "zgl.h"
#include "msghandling.h"

void glViewport(GLint x, GLint y, GLint width, GLint height) {
    GLContext *c = gl_get_context();
    int xsize, ysize, xmin, ymin, xsize_req, ysize_req;

    xmin = x;
    ymin = y;
    xsize = width;
    ysize = height;

    /* we may need to resize the zbuffer */

    if (c->viewport.xmin != xmin ||
            c->viewport.ymin != ymin ||
            c->viewport.xsize != xsize ||
            c->viewport.ysize != ysize) {

        xsize_req = xmin + xsize;
        ysize_req = ymin + ysize;

        if (c->gl_resize_viewport && c->gl_resize_viewport(c, &xsize_req, &ysize_req) != 0) {
            fprintf(stderr, "glViewport: error while resizing display");
        }

        xsize = xsize_req - xmin;
        ysize = ysize_req - ymin;
        if (xsize <= 0 || ysize <= 0) {
            fprintf(stderr, "glViewport: size too small");
        }

        tgl_trace("glViewport: %d %d %d %d\n", xmin, ymin, xsize, ysize);
        c->viewport.xmin  = xmin;
        c->viewport.ymin  = ymin;
        c->viewport.xsize = xsize;
        c->viewport.ysize = ysize;

        c->viewport.updated = 1;
    }
}

void glopEnableDisable(GLenum code, int v) {
    GLContext *c = gl_get_context();
    switch(code) {
        case GL_CULL_FACE:
            c->cull_face_enabled = v;
            break;
        case GL_LIGHTING:
            c->lighting_enabled = v;
            break;
        case GL_COLOR_MATERIAL:
            c->color_material_enabled = v;
            break;
        case GL_TEXTURE_2D:
            c->texture_2d_enabled = v;
            break;
        case GL_NORMALIZE:
            c->normalize_enabled = v;
            break;
        case GL_DEPTH_TEST:
            c->depth_test = v;
            break;
        case GL_POLYGON_OFFSET_FILL:
            if (v) c->offset_states |= TGL_OFFSET_FILL;
            else c->offset_states &= ~TGL_OFFSET_FILL;
            break;
        case GL_POLYGON_OFFSET_POINT:
            if (v) c->offset_states |= TGL_OFFSET_POINT;
            else c->offset_states &= ~TGL_OFFSET_POINT;
            break;
        case GL_POLYGON_OFFSET_LINE:
            if (v) c->offset_states |= TGL_OFFSET_LINE;
            else c->offset_states &= ~TGL_OFFSET_LINE;
            break;
        default:
            if (code >= GL_LIGHT0 && code < GL_LIGHT0 + MAX_LIGHTS) {
                gl_enable_disable_light(c, code - GL_LIGHT0, v);
            } else {
                /*
                   fprintf(stderr, "glEnableDisable: 0x%X not supported.\n", code);
                   */
            }
            break;
    }
}

void glEnable(GLenum cap) {
    glopEnableDisable(cap, 1);
}

void glDisable(GLenum cap) {
    glopEnableDisable(cap, 0);
}

void glShadeModel(GLenum mode) {
    GLContext *c = gl_get_context();
    assert(mode == GL_FLAT || mode == GL_SMOOTH);
    c->current_shade_model = mode;
}

void glCullFace(GLenum mode) {
    GLContext *c = gl_get_context();
    assert(mode == GL_BACK || mode == GL_FRONT || mode == GL_FRONT_AND_BACK);
    c->current_cull_face = mode;
}

void glFrontFace(GLenum mode) {
    GLContext *c = gl_get_context();
    assert(mode == GL_CCW || mode == GL_CW);
    c->current_front_face = (mode != GL_CCW);
}

void glPolygonMode(GLenum face, GLenum mode) {
    GLContext *c = gl_get_context();
    assert(face == GL_BACK || face == GL_FRONT || face == GL_FRONT_AND_BACK);
    assert(mode == GL_POINT || mode == GL_LINE || mode == GL_FILL);

    switch(face) {
        case GL_BACK:
            c->polygon_mode_back = mode;
            break;
        case GL_FRONT:
            c->polygon_mode_front = mode;
            break;
        case GL_FRONT_AND_BACK:
            c->polygon_mode_front = mode;
            c->polygon_mode_back = mode;
            break;
        default:
            assert(0);
    }
}

void glHint(GLenum target, GLenum mode) {
#if 0
    int target = p[1].i;
    int mode = p[2].i;

    /* do nothing */
#endif
}

void glPolygonOffset(GLfloat factor, GLfloat units) {
    GLContext *c = gl_get_context();
    c->offset_factor = factor;
    c->offset_units = units;
}
