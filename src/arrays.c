#include "zgl.h"
#include <assert.h>
#include <stdio.h>

#define VERTEX_ARRAY   0x0001
#define COLOR_ARRAY    0x0002
#define NORMAL_ARRAY   0x0004
#define TEXCOORD_ARRAY 0x0008

void glDrawArrays(GLenum mode, GLint first, GLsizei count) {
    glBegin(mode);
    for (int i = first; i < first + count; i++) {
        glArrayElement(i);
    }
    glEnd();
}

void glArrayElement(GLint idx)
{
    GLContext *c = gl_get_context();
    int i;
    int states = c->client_states;

    if (states & COLOR_ARRAY) {
        int size = c->color_array_size; 
        i = idx * (size + c->color_array_stride);
        glColor4f(
            c->color_array[i],
            c->color_array[i+1],
            c->color_array[i+2],
            size > 3 ? c->color_array[i+3] : 1.0f
        );
    }
    if (states & NORMAL_ARRAY) {
        i = idx * (3 + c->normal_array_stride);
        c->current_normal.X = c->normal_array[i];
        c->current_normal.Y = c->normal_array[i+1];
        c->current_normal.Z = c->normal_array[i+2];
        c->current_normal.Z = 0.0f;
    }
    if (states & TEXCOORD_ARRAY) {
        int size = c->texcoord_array_size;
        i = idx * (size + c->texcoord_array_stride);
        c->current_tex_coord.X = c->texcoord_array[i];
        c->current_tex_coord.Y = c->texcoord_array[i+1];
        c->current_tex_coord.Z = size > 2 ? c->texcoord_array[i+2] : 0.0f;
        c->current_tex_coord.W = size > 3 ? c->texcoord_array[i+3] : 1.0f;
    }
    if (states & VERTEX_ARRAY) {
        int size = c->vertex_array_size;
        i = idx * (size + c->vertex_array_stride);
        glVertex4f(
            c->vertex_array[i],
            c->vertex_array[i+1],
            size > 2 ? c->vertex_array[i+2] : 0.0f,
            size > 3 ? c->vertex_array[i+3] : 1.0f
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
    c->vertex_array_size = size;
    c->vertex_array_stride = stride;
    c->vertex_array = pointer;
}

void glColorPointer(GLint size, GLenum type, GLsizei stride,
                    const GLvoid *pointer) {
    GLContext *c = gl_get_context();
    c->color_array_size = size;
    c->color_array_stride = stride;
    c->color_array = pointer;
}

void glNormalPointer(GLenum type, GLsizei stride, 
                const GLvoid *pointer) {
    GLContext *c = gl_get_context();
    c->normal_array_stride = stride;
    c->normal_array = pointer;
}

void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, 
                       const GLvoid *pointer) {
    GLContext *c = gl_get_context();
    c->texcoord_array_size = size;
    c->texcoord_array_stride = stride;
    c->texcoord_array = pointer;
}
