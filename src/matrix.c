#include "zgl.h"

void gl_print_matrix( const float *m)
{
   int i;

   for (i=0;i<4;i++) {
      fprintf(stderr,"%f %f %f %f\n", m[i], m[4+i], m[8+i], m[12+i] );
   }
}

static inline void gl_matrix_update(GLContext *c)
{
  c->matrix_model_projection_updated=(c->matrix_mode<=1);
}

void glMatrixMode(int mode)
{
  GLContext *c = gl_get_context();
  switch(mode) {
  case GL_MODELVIEW:
    c->matrix_mode=0;
    break;
  case GL_PROJECTION:
    c->matrix_mode=1;
    break;
  case GL_TEXTURE:
    c->matrix_mode=2;
    break;
  default:
    assert(0);
  }
}

void glLoadMatrixf(const float *matrix)
{
  GLContext *c = gl_get_context();
  int i;
  M4 *m;
  m=c->matrix_stack_ptr[c->matrix_mode];

  int q = 0;
  for(i=0; i<4; i++) {
    m->m[0][i] = matrix[q + 0];
    m->m[1][i] = matrix[q + 1];
    m->m[2][i] = matrix[q + 2];
    m->m[3][i] = matrix[q + 3];
    q+=4;
  }

  gl_matrix_update(c);
}

void glLoadIdentity(void)
{
  GLContext *c = gl_get_context();
  gl_M4_Id(c->matrix_stack_ptr[c->matrix_mode]);
  gl_matrix_update(c);
}

void glMultMatrixf(const float *matrix)
{
  GLContext *c = gl_get_context();
  M4 m;
  int i;

  int q = 0;
  for(i=0;i<4;i++) {
    m.m[0][i] = matrix[q + 0];
    m.m[1][i] = matrix[q + 1];
    m.m[2][i] = matrix[q + 2];
    m.m[3][i] = matrix[q + 3];
    q+=4;
  }

  gl_M4_MulLeft(c->matrix_stack_ptr[c->matrix_mode],&m);
  gl_matrix_update(c);
}


void glPushMatrix()
{
  GLContext *c = gl_get_context();
  int n=c->matrix_mode;
  M4 *m;

  assert( (c->matrix_stack_ptr[n] - c->matrix_stack[n] + 1 )
	   < c->matrix_stack_depth_max[n] );

  m=++c->matrix_stack_ptr[n];
  
  gl_M4_Move(&m[0],&m[-1]);

  gl_matrix_update(c);
}

void glPopMatrix()
{
  GLContext *c = gl_get_context();
  int n=c->matrix_mode;

  assert( c->matrix_stack_ptr[n] > c->matrix_stack[n] );
  c->matrix_stack_ptr[n]--;
  gl_matrix_update(c);
}


void glRotatef(float angle, float x, float y, float z)
{
  GLContext *c = gl_get_context();
  M4 m;
  float u[3];
  int dir_code;

  angle = angle * M_PI / 180.0;
  u[0]= x;
  u[1]= y;
  u[2]= z;

  /* simple case detection */
  dir_code = ((u[0] != 0)<<2) | ((u[1] != 0)<<1) | (u[2] != 0);

  switch(dir_code) {
  case 0:
    gl_M4_Id(&m);
    break;
  case 4:
    if (u[0] < 0) angle=-angle;
    gl_M4_Rotate(&m,angle,0);
    break;
  case 2:
    if (u[1] < 0) angle=-angle;
    gl_M4_Rotate(&m,angle,1);
    break;
  case 1:
    if (u[2] < 0) angle=-angle;
    gl_M4_Rotate(&m,angle,2);
    break;
  default:
    {
      float cost, sint;

      /* normalize vector */
      float len = u[0]*u[0]+u[1]*u[1]+u[2]*u[2];
      if (len == 0.0f) return;
      len = 1.0f / sqrt(len);
      u[0] *= len;
      u[1] *= len;
      u[2] *= len;

      /* store cos and sin values */
      cost=cos(angle);
      sint=sin(angle);

      /* fill in the values */
      m.m[3][0]=m.m[3][1]=m.m[3][2]=
        m.m[0][3]=m.m[1][3]=m.m[2][3]=0.0f;
      m.m[3][3]=1.0f;

      /* do the math */
      m.m[0][0]=u[0]*u[0]+cost*(1-u[0]*u[0]);
      m.m[1][0]=u[0]*u[1]*(1-cost)-u[2]*sint;
      m.m[2][0]=u[2]*u[0]*(1-cost)+u[1]*sint;
      m.m[0][1]=u[0]*u[1]*(1-cost)+u[2]*sint;
      m.m[1][1]=u[1]*u[1]+cost*(1-u[1]*u[1]);
      m.m[2][1]=u[1]*u[2]*(1-cost)-u[0]*sint;
      m.m[0][2]=u[2]*u[0]*(1-cost)-u[1]*sint;
      m.m[1][2]=u[1]*u[2]*(1-cost)+u[0]*sint;
      m.m[2][2]=u[2]*u[2]+cost*(1-u[2]*u[2]);
    }
  }

  gl_M4_MulLeft(c->matrix_stack_ptr[c->matrix_mode],&m);

  gl_matrix_update(c);
}

void glScalef(float x, float y, float z)
{
  GLContext *c = gl_get_context();
  float *m;
  m=&c->matrix_stack_ptr[c->matrix_mode]->m[0][0];

  m[0] *= x;   m[1] *= y;   m[2]  *= z;
  m[4] *= x;   m[5] *= y;   m[6]  *= z;
  m[8] *= x;   m[9] *= y;   m[10] *= z;
  m[12] *= x;   m[13] *= y;   m[14] *= z;
  gl_matrix_update(c);
}

void glTranslatef(float x, float y, float z)
{
  GLContext *c = gl_get_context();
  float *m;
  m=&c->matrix_stack_ptr[c->matrix_mode]->m[0][0];

  m[3] = m[0] * x + m[1] * y + m[2]  * z + m[3];
  m[7] = m[4] * x + m[5] * y + m[6]  * z + m[7];
  m[11] = m[8] * x + m[9] * y + m[10] * z + m[11];
  m[15] = m[12] * x + m[13] * y + m[14] * z + m[15];

  gl_matrix_update(c);
}

void glFrustum(double left, double right, double bottom,
               double top, double near, double far)
{
  GLContext *c = gl_get_context();
  float *r;
  M4 m;
  float x,y,A,B,C,D;

  x = (2.0*near) / (right-left);
  y = (2.0*near) / (top-bottom);
  A = (right+left) / (right-left);
  B = (top+bottom) / (top-bottom);
  C = -(far+near) / ( far-near);
  D = -(2.0*far*near) / (far-near);

  r=&m.m[0][0];
  r[0]= x; r[1]=0; r[2]=A; r[3]=0;
  r[4]= 0; r[5]=y; r[6]=B; r[7]=0;
  r[8]= 0; r[9]=0; r[10]=C; r[11]=D;
  r[12]= 0; r[13]=0; r[14]=-1; r[15]=0;

  gl_M4_MulLeft(c->matrix_stack_ptr[c->matrix_mode],&m);

  gl_matrix_update(c);
}
  
