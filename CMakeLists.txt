cmake_minimum_required(VERSION 3.8)

project(SimpleThreads LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)

# interface because the lib is header only
add_library(${PROJECT_NAME} INTERFACE)

# ================== Headers ==================
target_include_directories(
    ${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)

# ================== Dependencies ==================
target_link_libraries(${PROJECT_NAME} INTERFACE Threads::Threads)

# ================== Installation directives ==================
install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include DESTINATION include/${PROJECT_NAME})

# ================== directives to enable find_package ==================
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(INCLUDE_INSTALL_DIR
    "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}"
    CACHE PATH
    "Location of header files"
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/install.cmake.in"
    "${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION "lib/cmake/${PROJECT_NAME}"
    PATH_VARS INCLUDE_INSTALL_DIR
)

install(
    EXPORT "${PROJECT_NAME}Targets"
    FILE "${PROJECT_NAME}Targets.cmake"
    DESTINATION "lib/cmake/${PROJECT_NAME}"
)

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    DESTINATION "lib/cmake/${PROJECT_NAME}"
)

# ================== Uninstall target ==================
if(NOT TARGET uninstall)
    configure_file(
        "cmake/uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
        IMMEDIATE
        @ONLY
    )

    add_custom_target(
        uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake
    )
endif()

# ================== Build examples ==================
add_subdirectory(examples)
