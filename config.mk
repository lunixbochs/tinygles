#####################################################################
# C compiler

# linux
CC= gcc
CFLAGS= -g -Wall -O2
LFLAGS=

#####################################################################
# TinyGL configuration 

#####################################################################
# Select window API for TinyGL: 

# standard X11 GLX like API 
TINYGL_USE_GLX=y

#####################################################################
# X11 configuration (for the examples only)

ifdef TINYGL_USE_GLX
# Linux
UI_LIBS= -L/usr/X11R6/lib -lX11 -lXext
UI_INCLUDES= 
UI_OBJS=x11.o
endif

#####################################################################
# OpenGL configuration (for the examples only)

# use TinyGL 
GL_LIBS= -L../lib -lTinyGL 
GL_INCLUDES= -I../include
GL_DEPS= ../lib/libTinyGL.a

# use Mesa
#GL_LIBS= -lMesaGL 
#GL_INCLUDES= 
#GL_DEPS=

# use OpenGL
#GL_LIBS= -lGL 
#GL_INCLUDES= 
#GL_DEPS=

####################################################################
# Compile and link control

# UNIX systems
DIRS= src examples
