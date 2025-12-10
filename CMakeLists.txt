cmake_minimum_required(VERSION 3.10)
project(GlobeMercatorBlendDemo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(include external/glad/include external/stb_image/include external/glm/include)

aux_source_directory(src MAIN_SRC_FILE)
aux_source_directory(external/glad/src MAIN_SRC_FILE)
aux_source_directory(external/stb_image/src MAIN_SRC_FILE)

add_executable(GlobeMercatorBlendDemo ${MAIN_SRC_FILE}
    src/GlobeProjection.h
    src/ShaderManager.h
    src/TileRenderer.h
    src/Application.h)

target_link_libraries(GlobeMercatorBlendDemo OpenGL::GL glfw ${CMAKE_DL_LIBS})
