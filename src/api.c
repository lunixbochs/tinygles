#include "zgl.h"

/* glVertex */
void glVertex2f(float x, float y) {
    glVertex4f(x,y,0,1);
}

void glVertex3f(float x, float y, float z) {
    glVertex4f(x,y,z,1);
}

void glVertex3fv(float *v) {
    glVertex4f(v[0], v[1], v[2], 1);
}

/* glNormal */
void glNormal3fv(float *v) {
    glNormal3f(v[0], v[1], v[2]);
}

/* glColor */
void glColor4fv(float *v) {
    glColor4f(v[0], v[1], v[2], v[3]);
}

void glColor3f(float x, float y, float z) {
    glColor4f(x, y, z, 1);
}

void glColor3fv(float *v) {
    glColor4f(v[0], v[1], v[2], 1);
}

/* TexCoord */
void glTexCoord2f(float s, float t) {
    glTexCoord4f(s, t, 0, 1);
}

void glTexCoord2fv(float *v) {
    glTexCoord4f(v[0], v[1], 0, 1);
}

/* Special Functions */
void glFlush() {
  /* nothing to do */
}

/* Non standard functions */
void glDebug(int mode) {
    GLContext *c = gl_get_context();
    c->print_flag = mode;
}
