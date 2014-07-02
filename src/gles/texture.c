/*
 * Texture Manager
 */

#include "zgl.h"

static GLTexture *find_texture(GLContext *c, int h) {
  GLTexture *t;

  t = c->shared_state.texture_hash_table[h % TEXTURE_HASH_TABLE_SIZE];
  while (t != NULL) {
    if (t->handle == h) return t;
    t = t->next;
  }
  return NULL;
}

static void free_texture(GLContext *c, int h) {
    GLTexture *t, **ht;
    GLImage *im;

    t=find_texture(c, h);
    if (t->prev == NULL) {
        ht = &c->shared_state.texture_hash_table[t->handle % TEXTURE_HASH_TABLE_SIZE];
        *ht = t->next;
    } else {
        t->prev->next = t->next;
    }
    if (t->next != NULL) t->next->prev = t->prev;

    for (int i = 0; i < MAX_TEXTURE_LEVELS; i++) {
        im = &t->images[i];
        if (im->pixmap != NULL) free(im->pixmap);
    }

    free(t);
}

GLTexture *alloc_texture(GLContext *c, int h) {
    GLTexture *t, **ht;

    t = calloc(1, sizeof(GLTexture));

    ht = &c->shared_state.texture_hash_table[h % TEXTURE_HASH_TABLE_SIZE];

    t->next = *ht;
    t->prev = NULL;
    if (t->next != NULL) t->next->prev = t;
    *ht = t;

    t->handle = h;

    return t;
}


void glInitTextures(GLContext *c) {
  /* textures */

  c->texture_2d_enabled = 0;
  c->current_texture = find_texture(c,0);
}

void glGenTextures(int n, unsigned int *textures) {
    GLContext *c = gl_get_context();
    int max;
    GLTexture *t;

    max=0;
    for (int i = 0; i < TEXTURE_HASH_TABLE_SIZE; i++) {
        t = c->shared_state.texture_hash_table[i];
        while (t != NULL) {
            if (t->handle > max) max = t->handle;
            t = t->next;
        }

    }
    for (int i = 0; i < n; i++) {
        textures[i] = max + i + 1;
    }
}


void glDeleteTextures(GLsizei n, const GLuint *textures) {
    GLContext *c = gl_get_context();
    GLTexture *t;

    for (int i = 0; i < n; i++) {
        t = find_texture(c, textures[i]);
        if (t != NULL && t != 0) {
            if (t == c->current_texture) {
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            free_texture(c, textures[i]);
        }
    }
}


void glBindTexture(GLenum target, GLuint texture) {
    GLContext *c = gl_get_context();
    GLTexture *t;

    assert(target == GL_TEXTURE_2D && texture >= 0);

    t = find_texture(c, texture);
    if (t == NULL) {
        t = alloc_texture(c, texture);
    }
    c->current_texture = t;
}


GLboolean glIsTexture(GLuint texture) {
    GLContext *c = gl_get_context();
    GLTexture *t = find_texture(c, texture);
    return t != NULL;
}


void glTexImage2D(GLenum target, GLint level, GLint internalFormat,
                  GLsizei width, GLsizei height, GLint border,
                  GLenum format, GLenum type, const GLvoid *pixels) {
    GLContext *c = gl_get_context();
    GLImage *im;
    unsigned char *pixels1;
    int do_free;

    if (!(target == GL_TEXTURE_2D && level == 0 && internalFormat == GL_RGB &&
            border == 0 && format == GL_RGB &&
            type == GL_UNSIGNED_BYTE)) {
        fprintf(stderr, "glTexImage2D: combination of parameters not handled");
    }

    do_free = 0;
    if (width != 256 || height != 256) {
        pixels1 = malloc(256 * 256 * 3);
        /* no interpolation is done here to respect the original image aliasing ! */
        // TODO: better type conversion
        gl_resizeImageNoInterpolate(pixels1, 256, 256, (GLubyte *)pixels, width, height);
        do_free = 1;
        width = 256;
        height = 256;
    } else {
        // TODO: better type conversion
        pixels1 = (GLubyte *)pixels;
    }

    im = &c->current_texture->images[level];
    im->xsize = width;
    im->ysize = height;
    if (im->pixmap != NULL) free(im->pixmap);
#if TGL_FEATURE_RENDER_BITS == 32
    im->pixmap = malloc(width * height * 4);
    if (im->pixmap) {
        gl_convertRGB_to_8A8R8G8B(im->pixmap, pixels1, width, height);
    }
#elif TGL_FEATURE_RENDER_BITS == 16
    im->pixmap = malloc(width * height * 2);
    if (im->pixmap) {
        gl_convertRGB_to_5R6G5B(im->pixmap, pixels1, width, height);
    }
#else
#error TODO
#endif
    if (do_free) free(pixels1);
}


void glTexEnvf(GLenum target, GLenum pname, GLfloat param) {
    printf("STUB: glTexEnvf(%x, %x, %.2f)\n", target, pname, param);
}

/* TODO: not all tests are done */
void glTexEnvi(GLenum target, GLenum pname, GLint param) {
    if (target != GL_TEXTURE_ENV || pname != GL_TEXTURE_ENV_MODE || param != GL_DECAL) {
        fprintf(stderr, "glTexEnvi: unsupported option\n");
    }
}


/* TODO: not all tests are done */
void glTexParameteri(GLenum target, GLenum pname, GLint param) {
    if (target != GL_TEXTURE_2D) {
        fprintf(stderr, "glTexParameter: unsupported option\n");
    }

    switch(pname) {
        case GL_TEXTURE_WRAP_S:
        case GL_TEXTURE_WRAP_T:
            if (param != GL_REPEAT)
                fprintf(stderr, "glTexParameter: unsupported option\n");
            break;
    }
}

void glPixelStorei(GLenum pname, GLint param) {
    if (pname != GL_UNPACK_ALIGNMENT || param != 1) {
        fprintf(stderr, "glPixelStore: unsupported option: %x\n", pname);
    }
}
