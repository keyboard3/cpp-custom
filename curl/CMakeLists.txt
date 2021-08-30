cmake_minimum_required(VERSION 3.19)

project(curlDemo)

link_libraries(curl)

add_executable(https https.c)
add_executable(simple simple.c)