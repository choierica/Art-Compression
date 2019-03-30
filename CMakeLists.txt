cmake_minimum_required(VERSION 3.13)
project("pa3")

set(CMAKE_CXX_STANDARD 11)

#set(CMAKE_CXX_COMPILER "clang++")
#ADD_COMPILE_OPTIONS(-std=c++1y -stdlib=libc++ -c -g -O0 -Wall -Wextra -pedantic)
#ADD_LINK_OPTIONS(-std=c++1y -stdlib=libc++ -lc++abi -lpthread -lm)


set(EXE_FILES
        cs221util/PNG.cpp
        cs221util/HSLAPixel.cpp
        cs221util/lodepng/lodepng.cpp
        toqutree.cpp
        stats.cpp)
add_executable(pa3test testComp.cpp ${EXE_FILES})
add_executable(pa3 main.cpp ${EXE_FILES})