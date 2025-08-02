cmake_minimum_required(VERSION 3.10)
project(PhysicsEngine)

set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# GLAD
add_library(glad STATIC libs/glad.c)
target_include_directories(glad PUBLIC include)

# GLFW
add_library(glfw STATIC IMPORTED)
set_target_properties(glfw PROPERTIES
  IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libs/libglfw3.a
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/libs
)

add_executable(PhysicsEngine
  src/main.c
  src/simulation.c
  src/render/shader.c
  src/render/texture.c
  src/render/camera.c
  src/physics/object.c
  src/physics/objects/floor.c
  src/physics/objects/sphere.c
  src/physics/objects/cube.c
  src/physics/objects/tetrahedron.c
  src/utils/cJSON.c
  src/utils/parse.c
)

target_compile_options(PhysicsEngine PRIVATE -fsanitize=address -g)
target_link_options(PhysicsEngine PRIVATE -fsanitize=address)

target_include_directories(PhysicsEngine PRIVATE include)

target_include_directories(PhysicsEngine PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(PhysicsEngine PRIVATE glad glfw ${CMAKE_DL_LIBS})

if(APPLE)
  find_library(COCOA_LIBRARY Cocoa)
  find_library(IOKIT_LIBRARY IOKit)
  find_library(COREVIDEO_LIBRARY CoreVideo)
  target_link_libraries(PhysicsEngine PRIVATE
    ${COCOA_LIBRARY}
    ${IOKIT_LIBRARY}
    ${COREVIDEO_LIBRARY}
  )
endif()
