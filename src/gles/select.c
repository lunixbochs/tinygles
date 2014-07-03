#include "zgl.h"

int glRenderMode(GLenum mode) {
    GLContext *c = gl_get_context();
    int result = 0;

    switch(c->render_mode) {
        case GL_RENDER:
            break;
        case GL_SELECT:
            if (c->select.overflow) {
                result = -c->select.hits;
            } else {
                result = c->select.hits;
            }
            c->select.overflow = 0;
            c->select.ptr = c->select.buffer;
            c->name.stack_size = 0;
            break;
        default:
            assert(0);
    }
    switch(mode) {
        case GL_RENDER:
            c->render_mode = GL_RENDER;
            break;
        case GL_SELECT:
            c->render_mode = GL_SELECT;
            assert(c->select.buffer != NULL);
            c->select.ptr = c->select.buffer;
            c->select.hits = 0;
            c->select.overflow = 0;
            c->select.hit = NULL;
            break;
        default:
            assert(0);
    }
    return result;
}

void glSelectBuffer(GLsizei size, GLuint *buf) {
    GLContext *c = gl_get_context();

    assert(c->render_mode != GL_SELECT);

    c->select.buffer = buf;
    c->select.size = size;
}


void glInitNames() {
    GLContext *c = gl_get_context();
    if (c->render_mode == GL_SELECT) {
        c->name.stack_size = 0;
        c->select.hit = NULL;
    }
}

void glPushName(GLuint name) {
    GLContext *c = gl_get_context();
    if (c->render_mode == GL_SELECT) {
        assert(c->name.stack_size < MAX_NAME_STACK_DEPTH);
        c->name.stack[c->name.stack_size++] = name;
        c->select.hit = NULL;
    }
}

void glPopName() {
    GLContext *c = gl_get_context();
    if (c->render_mode == GL_SELECT) {
        assert(c->name.stack_size>0);
        c->name.stack_size--;
        c->select.hit = NULL;
    }
}

void glLoadName(GLuint name) {
    GLContext *c = gl_get_context();
    if (c->render_mode == GL_SELECT) {
        assert(c->name.stack_size > 0);
        c->name.stack[c->name.stack_size - 1] = name;
        c->select.hit = NULL;
    }
}

void gl_add_select(GLContext *c, unsigned int zmin, unsigned int zmax) {
    unsigned int *ptr;
    int n, i;

    if (!c->select.overflow) {
        if (c->select.hit == NULL) {
            n = c->name.stack_size;
            if ((c->select.ptr - c->select.buffer + 3 + n) > c->select.size) {
                c->select.overflow = 1;
            } else {
                ptr = c->select.ptr;
                c->select.hit = ptr;
                *ptr++ = c->name.stack_size;
                *ptr++ = zmin;
                *ptr++ = zmax;
                for(i = 0; i < n; i++) *ptr++ = c->name.stack[i];
                c->select.ptr = ptr;
                c->select.hits++;
            }
        } else {
            if (zmin < c->select.hit[1]) c->select.hit[1] = zmin;
            if (zmax > c->select.hit[2]) c->select.hit[2] = zmax;
        }
    }
}
