cmake_minimum_required(VERSION 3.21)
project(Final_year_Project)

set(CMAKE_CXX_STANDARD 17)

add_executable(Final_year_Project main.cpp)

add_executable(GBC_Single_Thread GBC_Single_Thread.cpp)

add_executable(GBC GBC.cpp)
add_executable(BetweennessCentrality BetweennessCentrality.cpp)