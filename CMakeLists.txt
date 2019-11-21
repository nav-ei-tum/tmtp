cmake_minimum_required(VERSION 3.10)

project(tmtp VERSION 0.1.0 LANGUAGES CXX)

#############################################
# tmtp library

add_subdirectory(src)

#############################################
# Exports


# Add uninstall target
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()


#############################################
# Examples

option(TMTP_BUILD_EXAMPLES "Build example applications" Off)

if(TMTP_BUILD_EXAMPLES)
    add_subdirectory(Examples)
endif()

