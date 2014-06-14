file(GLOB_RECURSE GLES_SOURCES gles/*.c)
add_library(GLESv1_CM SHARED ${GLES_SOURCES})
target_link_libraries(GLESv1_CM X11 Xext)

add_subdirectory(examples)
