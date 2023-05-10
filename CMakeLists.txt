cmake_minimum_required(VERSION 3.16)
project(io_tcp)


add_executable(io_tcp main.c server.c)

target_link_libraries(io_tcp uring)

