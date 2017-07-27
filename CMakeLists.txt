cmake_minimum_required(VERSION 3.8)
project(othello)

set(SOURCES othello.c othello.h)

add_executable(othello_bench ${SOURCES} othello_bench.c)
add_executable(othello_test ${SOURCES} othello_test.c)
add_executable(othello_text ${SOURCES} text_othello.c)

if(WIN32)
    add_executable(othello_windows WIN32 ${SOURCES} win_othello.c win_othello_res.h win_othello_res.rc)
endif()
