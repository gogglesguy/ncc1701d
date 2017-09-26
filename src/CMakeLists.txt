
find_package(OpenGL)
find_package(PkgConfig)
pkg_check_modules(FOX fox17>=1.7.50 REQUIRED)
pkg_check_modules(EPOXY epoxy REQUIRED)

project(ncc1701d VERSION 0.1.0.0)


set(SOURCES buffer.cpp
buffer.h
io.cpp
io.h
stream.cpp
stream.h
image.h
texture.cpp
texture.h
cinepak.cpp
cinepak.h
smc.cpp
smc.h
fourcc.cpp
fourcc.h
mp4.cpp
mp4.h
qtvr.cpp
qtvr.h
qtvr_scene.cpp
qtvr_scene.h
)

add_executable(ncc1701d ${SOURCES} fxpanorama.cpp)
target_compile_features(ncc1701d PUBLIC cxx_std_17)
target_link_libraries(ncc1701d PRIVATE ${FOX_LIBRARIES} ${EPOXY_LIBRARIES} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})
target_include_directories(ncc1701d PRIVATE ${FOX_INCLUDE_DIRS} ${OPENGL_gl_INCLUDE})


