#include <stdbool.h>

#include "zgl.h"

void glScissor(GLint x, GLint y, GLsizei width, GLsizei height) {
    fprintf(stderr, "STUB: glScissor()\n");
}

void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
    fprintf(stderr, "STUB: glColorMask()\n");
}

void glDepthFunc(GLenum func) {
    fprintf(stderr, "STUB: glDepthFunc()\n");
}

void glClearDepthf(GLclampf depth) {
    fprintf(stderr, "STUB: glClearDepthf()\n");
}

void glDepthMask(GLboolean flag) {
    fprintf(stderr, "STUB: glDepthMask()\n");
}

void glFogfv(GLenum pname, const GLfloat *params) {
    fprintf(stderr, "STUB: glFogfv()\n");
}

void glFogf(GLenum pname, GLfloat param) {
    fprintf(stderr, "STUB: glFogf()\n");
}

void glLineWidth(GLfloat width) {
    fprintf(stderr, "STUB: glLineWidth\n");
}

void glPointSize(GLfloat size) {
    fprintf(stderr, "STUB: glPointSize\n");
}

void glFinish() {
    fprintf(stderr, "STUB: glFinish()\n");
}
