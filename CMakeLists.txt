cmake_minimum_required(VERSION 3.10)
project(train_ticket_system_017)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimize for performance, avoid heavy debugging symbols
add_compile_options(-O2 -pipe -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare)

add_executable(code main.cpp)

# Ensure output binary is placed in project root (default for in-tree builds)
