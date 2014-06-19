#include "zgl.h"

void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) {
    printf("STUB: glScissor()\n");
}

void glAlphaFunc(GLenum func, GLclampf ref) {
    printf("STUB: glAlphaFunc()\n");
}

void glBlendFunc(GLenum sfactor, GLenum dfactor) {
    printf("STUB: glAlphaFunc()\n");
}

void glLogicOp(GLenum opcode) {
    printf("STUB: glLogicOp()\n");
}

void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
    printf("STUB: glColorMask()\n");
}

void glDepthFunc(GLenum func) {
    printf("STUB: glDepthFunc()\n");
}

void glClearDepthf(GLclampf depth) {
    printf("STUB: glClearDepthf()\n");
}

void glDepthMask(GLboolean flag) {
    printf("STUB: glDepthMask()\n");
}

void glFogfv(GLenum pname, const GLfloat *params) {
    printf("STUB: glFogfv()\n");
}

void glFogf(GLenum pname, GLfloat param) {
    printf("STUB: glFogf()\n");
}

void glLineWidth(GLfloat width) {
    printf("STUB: glLineWidth\n");
}

void glPointSize(GLfloat size) {
    printf("STUB: glPointSize\n");
}

void glFinish() {
    printf("STUB: glFinish()\n");
}

GLboolean glIsEnabled(GLenum cap) {
    printf("STUB: glIsEnabled(%x)\n", cap);
    return false;
}
