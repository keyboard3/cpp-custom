cmake_minimum_required(VERSION 3.19)

project(curlDemo)

include_directories(/usr/local/include)
link_libraries(curl)

add_executable(https https.c)
add_executable(simple simple.c)
add_executable(parseurl parseurl.c)
add_executable(simplepost simplepost.c)
add_executable(htmltidy htmltidy.c)

target_link_libraries(htmltidy tidy)