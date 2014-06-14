set(COMMON glu.c x11.c)

link_libraries(GLESv1_CM X11)
add_executable(texobj ${COMMON} texobj.c)
add_executable(spin EXCLUDE_FROM_ALL ${COMMON} spin.c)
add_executable(gears EXCLUDE_FROM_ALL ${COMMON} mech.c)
add_executable(mech EXCLUDE_FROM_ALL ${COMMON} gears.c)
