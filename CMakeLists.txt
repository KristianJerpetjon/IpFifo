project(cross_cpu_queue)

cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_STANDARD 11)
aux_source_directory(. SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST} atomicring.h ringdescriptor.h ringinterface.h)
target_link_libraries(${PROJECT_NAME} pthread stdc++ )
