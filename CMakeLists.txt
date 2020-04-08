cmake_minimum_required(VERSION 3.5)
project(grafika)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -fpermissive -Wno-write-strings")

find_package (PkgConfig REQUIRED)
find_package (OpenGL REQUIRED)
find_package (GLUT REQUIRED)
find_package (GLEW REQUIRED)

include_directories(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIRS} $(GLEW_INCLUDE_DIRS))
link_libraries(${GLEW_LIBRARIES})

file(GLOB SOURCE_FILES
        src/*.cpp
        src/*.h
        )

add_executable(main ${SOURCE_FILES})

target_link_libraries (main ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} m dl X11 Xxf86vm pthread)
