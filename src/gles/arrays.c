#include "zgl.h"
#include <assert.h>
#include <stdio.h>

#define VERTEX_ARRAY   0x0001
#define COLOR_ARRAY    0x0002
#define NORMAL_ARRAY   0x0004
#define TEXCOORD_ARRAY 0x0008

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices) {
    glBegin(mode);
    if (type != GL_UNSIGNED_SHORT) {
        fprintf(stderr, "tinygles: can't handle type 0x%04x in glDrawElements\n", type);
    }
    const GLshort *shorts = indices;
    for (int i = 0; i < count; i++) {
        glArrayElement(shorts[i]);
    }
    glEnd();
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count) {
    glBegin(mode);
    for (int i = first; i < first + count; i++) {
        glArrayElement(i);
    }
    glEnd();
}

void glArrayElement(GLint idx) {
    GLContext *c = gl_get_context();
    int i;
    int states = c->client_states;

    if (states & COLOR_ARRAY) {
        int size = c->array.color.size;
        float *ptr = c->array.color.p;
        i = idx * (size + c->array.color.stride);
        glColor4f(
            ptr[i],
            ptr[i+1],
            ptr[i+2],
            size > 3 ? ptr[i+3] : 1.0f
        );
    }
    if (states & NORMAL_ARRAY) {
        i = idx * (3 + c->array.normal.stride);
        c->current.normal.X = c->array.normal.p[i];
        c->current.normal.Y = c->array.normal.p[i+1];
        c->current.normal.Z = c->array.normal.p[i+2];
        c->current.normal.Z = 0.0f;
    }
    if (states & TEXCOORD_ARRAY) {
        int size = c->array.tex_coord.size;
        i = idx * (size + c->array.tex_coord.stride);
        c->current.tex_coord.X = c->array.tex_coord.p[i];
        c->current.tex_coord.Y = c->array.tex_coord.p[i+1];
        c->current.tex_coord.Z = size > 2 ? c->array.tex_coord.p[i+2] : 0.0f;
        c->current.tex_coord.W = size > 3 ? c->array.tex_coord.p[i+3] : 1.0f;
    }
    if (states & VERTEX_ARRAY) {
        int size = c->array.vertex.size;
        i = idx * (size + c->array.vertex.stride);
        glVertex4f(
            c->array.vertex.p[i],
            c->array.vertex.p[i+1],
            size > 2 ? c->array.vertex.p[i+2] : 0.0f,
            size > 3 ? c->array.vertex.p[i+3] : 1.0f
        );
    }
}

void glEnableClientState(GLenum array) {
    GLContext *c = gl_get_context();
    GLenum bit;
    switch(array) {
        case GL_VERTEX_ARRAY:
            bit = VERTEX_ARRAY;
            break;
        case GL_NORMAL_ARRAY:
            bit = NORMAL_ARRAY;
            break;
        case GL_COLOR_ARRAY:
            bit = COLOR_ARRAY;
            break;
        case GL_TEXTURE_COORD_ARRAY:
            bit = TEXCOORD_ARRAY;
            break;
        default:
            assert(0);
            break;
    }
    c->client_states |= bit;
}

void glDisableClientState(GLenum array) {
    GLContext *c = gl_get_context();
    GLenum bit;
    switch(array) {
        case GL_VERTEX_ARRAY:
            bit = ~VERTEX_ARRAY;
            break;
        case GL_NORMAL_ARRAY:
            bit = ~NORMAL_ARRAY;
            break;
        case GL_COLOR_ARRAY:
            bit = ~COLOR_ARRAY;
            break;
        case GL_TEXTURE_COORD_ARRAY:
            bit = ~TEXCOORD_ARRAY;
            break;
        default:
            assert(0);
            break;
    }
    c->client_states &= bit;
}

// TODO: support other types? or do we want to assume glshim is helping us?
void glVertexPointer(GLint size, GLenum type, GLsizei stride,
                     const GLvoid *pointer) {
    GLContext *c = gl_get_context();
    c->array.vertex.size = size;
    c->array.vertex.stride = stride;
    c->array.vertex.p = pointer;
}

void glColorPointer(GLint size, GLenum type, GLsizei stride,
                    const GLvoid *pointer) {
    GLContext *c = gl_get_context();
    c->array.color.size = size;
    c->array.color.stride = stride;
    c->array.color.p = pointer;
}

void glNormalPointer(GLenum type, GLsizei stride,
                const GLvoid *pointer) {
    GLContext *c = gl_get_context();
    c->array.normal.stride = stride;
    c->array.normal.p = pointer;
}

void glTexCoordPointer(GLint size, GLenum type, GLsizei stride,
                       const GLvoid *pointer) {
    GLContext *c = gl_get_context();
    c->array.tex_coord.size = size;
    c->array.tex_coord.stride = stride;
    c->array.tex_coord.p = pointer;
}
