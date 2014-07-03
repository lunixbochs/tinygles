#include "zgl.h"

void glAlphaFunc(GLenum func, GLclampf ref) {
    GLContext *c = gl_get_context();
    c->alpha.func = func;
    c->alpha.ref = ref;
}

void glBlendFunc(GLenum sfactor, GLenum dfactor) {
    GLContext *c = gl_get_context();
    c->blend.sfactor = sfactor;
    c->blend.dfactor = dfactor;
}

void glLogicOp(GLenum opcode) {
    GLContext *c = gl_get_context();
    c->logic.op = opcode;
}
