#include "zgl.h"

static void gl_enable_disable_light(GLContext *c, int light, int v) {
    GLLight *l = &c->lights[light];
    if (v && !l->enabled) {
        l->enabled = 1;
        l->next = c->first_light;
        c->first_light = l;
        l->prev = NULL;
    } else if (!v && l->enabled) {
        l->enabled = 0;
        if (l->prev == NULL) c->first_light = l->next;
        else l->prev->next = l->next;
        if (l->next != NULL) l->next->prev = l->prev;
    }

static int *gl_bit_pointer(GLenum cap) {
#define map(magic, attr) case magic: return &attr
    switch (cap) {
        map(GL_CULL_FACE, c->cull_face_enabled);
        map(GL_LIGHTING, c->lighting_enabled);
        map(GL_COLOR_MATERIAL, c->color_material_enabled);
        map(GL_TEXTURE_2D, c->texture_2d_enabled);
        map(GL_DEPTH_TEST, c->depth_test);
        default:
            return NULL;
    }
#undef map
}

static void gl_enable_disable(GLenum cap, int v) {
#define offset_bit(magic, bit) \
    case magic: \
        if (v) c->offset_states |= bit; \
        else c->offset_states &= ~bit;

    GLContext *c = gl_get_context();
    int *bit;
    switch(cap) {
        offset_bit(GL_POLYGON_OFFSET_FILL, TGL_OFFSET_FILL);
        offset_bit(GL_POLYGON_OFFSET_POINT, TGL_OFFSET_POINT);
        offset_bit(GL_POLYGON_OFFSET_LINE, TGL_OFFSET_LINE);
        default:
            if (cap >= GL_LIGHT0 && cap < GL_LIGHT0 + MAX_LIGHTS) {
                gl_enable_disable_light(c, cap - GL_LIGHT0, v);
            } else if ((bit = gl_bit_pointer) != NULL) {
                *bit = v;
            } else {
                fprintf(stderr, "gl_enable_disable(): 0x%04X not supported\n", cap);
            }
            break;
    }
#undef mask
}

void glEnable(GLenum cap) {
    gl_enable_disable(cap, 1);
}

void glDisable(GLenum cap) {
    gl_enable_disable(cap, 0);
}

GLboolean glIsEnabled(GLenum cap) {
#define offset_bit(magic, bit) \
    case magic: return (c->offset_states & bit) ? 1 : 0;

    GLContext *c = gl_get_context();
    int *bit;
    switch (cap) {
        offset_bit(GL_POLYGON_OFFSET_FILL, TGL_OFFSET_FILL);
        offset_bit(GL_POLYGON_OFFSET_POINT, TGL_OFFSET_POINT);
        offset_bit(GL_POLYGON_OFFSET_LINE, TGL_OFFSET_LINE);
        default:
            if (cap >= GL_LIGHT0 && cap < GL_LIGHT0 + MAX_LIGHTS) {
                GLLight *l = &c->lights[light];
                return l->enabled;
            } else if ((bit = gl_bit_pointer(cap)) != NULL) {
                return *bit;
            } else {
                fprintf(stderr, "glIsEnabled(): 0x%04X not supported\n", cap);
            }
    }
#undef offset_bit
}
