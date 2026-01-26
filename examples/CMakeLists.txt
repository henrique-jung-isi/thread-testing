cmake_minimum_required(VERSION 3.8)

project(example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20 REQUIRED)

# find main package if the example project is being build independently
if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
    find_package(thread_testing REQUIRED)
endif()

add_executable(${PROJECT_NAME} example.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/include)

target_link_libraries(${PROJECT_NAME} thread_testing)
