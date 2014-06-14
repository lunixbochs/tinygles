set(COMMON glu.c x11.c)

link_libraries(GL X11)
add_executable(texobj ${COMMON} texobj.c)
add_executable(spin ${COMMON} spin.c)
add_executable(gears ${COMMON} gears.c)
add_executable(mech ${COMMON} mech.c)
