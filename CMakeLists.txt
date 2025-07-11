cmake_minimum_required(VERSION 3.10)
project(ParticleSimulator)

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

add_executable(ParticleSimulator
  src/main.c
  src/simulation.c
  src/render/shader.c
  src/render/texture.c
  src/render/camera.c
  src/physics/object.c
  src/physics/particle.c
)
target_include_directories(ParticleSimulator PRIVATE include)

target_include_directories(ParticleSimulator PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(ParticleSimulator PRIVATE glad glfw ${CMAKE_DL_LIBS})

if(APPLE)
  find_library(COCOA_LIBRARY Cocoa)
  find_library(IOKIT_LIBRARY IOKit)
  find_library(COREVIDEO_LIBRARY CoreVideo)
  target_link_libraries(ParticleSimulator PRIVATE
    ${COCOA_LIBRARY}
    ${IOKIT_LIBRARY}
    ${COREVIDEO_LIBRARY}
  )
endif()
