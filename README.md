TinyGLES (c) 2014 Ryan Hileman

Based on TinyGL 0.4 (c) 1997-2002 Fabrice Bellard.

General Description:
--------------------

TinyGLES is a software OpenGL ES driver, intended for use
with [glshim](https://github.com/lunixbochs/glshim).

The main features of TinyGLES are:

- ABI-compatible with OpenGL ES

- Zlib-like licence for easy integration in commercial designs (read
the LICENCE file).

- Subset of GLX for easy testing with X Window.

- OpenGL-like lighting.

- Complete OpenGL selection mode handling for object picking.

- 16 bit Z buffer. 16/32 bit RGB rendering. High speed dithering to
paletted 8 bits if needed.

- Partially NEON-optimized.

- Fast Gouraud shadding optimized for 16 bit RGB.

- Fast texture mapping capabilities, with perspective correction and
texture objects.

- 32 bit float only arithmetic.

- Very small: compiled code size of about 61 kB on ARM. The file
  src/zfeatures.h can be used to remove some unused features from
  TinyGLES.

- C sources for GCC on 32/64 bit architectures.

Examples:
---------

I took three simple examples from the Mesa package to test the main
functions of TinyGLES. They link against libGL, so you will need
glshim to use them.

- texobj illustrates the use of texture objects. Its shows the speed
of TinyGLES in this case.

- glutmech comes from the GLUT packages. It is much bigger and slower
because it uses the lighting. I have just included some GLU
functions and suppressed the GLUT related code to make it work. It
shows the display list handling of TinyGL in particular. You can look
at the source code to learn the keys to move the robot. The key 't'
toggles between shaded rendering and wire frame.

Architecture:
-------------

TinyGLES is made up four main modules:

- Mathematical routines (zmath).

- OpenGL-like emulation (zgl).

- Z buffer and rasterisation (zbuffer).

- GLX interface (zglx).

Notes - limitations:
--------------------

- See the file 'LIMITATIONS' to see the current functions supported by the API.

- The multithreading could be easily implemented since no global state
is maintainted. The library gets the current context with a function
which can be modified.

- The lighting is not very fast. I supposed that in most games the
lighting is computed by the 3D engine.

- Some changes are needed for 64 bit pointers for the handling of
arrays of float with the GLParam union.

- List sharing is partialy supported in the source, but not by the
current TinyGLX implementation (is it really useful ?).

- No user clipping planes are supported.

- No color index mode (no longer useful!)

- The mipmapping is not implemented.

- The perspective correction in the mapping code does not use W but
1/Z. In any 'normal scene' it should work.

- The resizing of the viewport in TinyGLX ensures that the width and
the height are multiples of 4. This is not optimal because some pixels
of the window may not be refreshed.

Why?
-----

TinyGLES was developed because some operations on a mobile GPU (namely
texture uploads) can be slower than rendering the entire scene yourself
in software.
