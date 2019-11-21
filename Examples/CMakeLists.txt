cmake_minimum_required(VERSION 3.10)

project(tmtpexamples VERSION 0.1.0 LANGUAGES CXX)

if(NOT TARGET tmtp::tmtp)
    find_package(tmtp REQUIRED)
endif()

if(NOT TARGET ticp::ticp)
    find_package(ticp REQUIRED)
endif()

add_executable(sender Sender_Example.cpp)
target_link_libraries(sender PRIVATE tmtp::tmtp ticp::ticp)
set_property(TARGET sender PROPERTY CXX_STANDARD 11)

add_executable(receiver Receiver_Example.cpp)
target_link_libraries(receiver PRIVATE tmtp::tmtp ticp::ticp)
set_property(TARGET sender PROPERTY CXX_STANDARD 11)
