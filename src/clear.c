#include "zgl.h"

void glClearColor(float r,float g,float b,float a)
{
  GLContext *c = gl_get_context();
  c->clear_color.v[0] = r;
  c->clear_color.v[1] = g;
  c->clear_color.v[2] = b;
  c->clear_color.v[3] = a;
}

void glClearDepth(double depth)
{
  GLContext *c = gl_get_context();
  c->clear_depth = depth;
}

void glClear(int mask)
{
  GLContext *c = gl_get_context();
  int z=0;
  int r=(int)(c->clear_color.v[0]*65535);
  int g=(int)(c->clear_color.v[1]*65535);
  int b=(int)(c->clear_color.v[2]*65535);

  /* TODO : correct value of Z */

  ZB_clear(c->zb,mask & GL_DEPTH_BUFFER_BIT,z,
	   mask & GL_COLOR_BUFFER_BIT,r,g,b);
}
