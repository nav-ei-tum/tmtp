#############################################
# Create library target and set properties

set(LIBTMTP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/GroundOcfServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GroundPacketServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/myErrors.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/NetProtConf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OcfServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PacketServer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SpacePacketConf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TestProtConf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TmFrameTimestamp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TmFrameBitrate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TmMasterChannel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TmOcf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TmPhysicalChannel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TmTransferFrame.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TmVirtualChannel.cpp
)

set(LIBTMTP_HEADERS
    ${PROJECT_SOURCE_DIR}/include/tmtp/GroundOcfServer.h
    ${PROJECT_SOURCE_DIR}/include/tmtp/GroundPacketServer.h
    ${PROJECT_SOURCE_DIR}/include/tmtp/myErrors.h
    ${PROJECT_SOURCE_DIR}/include/tmtp/NetProtConf.h
    ${PROJECT_SOURCE_DIR}/include/tmtp/OcfServer.h
    ${PROJECT_SOURCE_DIR}/include/tmtp/PacketServer.h
    ${PROJECT_SOURCE_DIR}/include/tmtp/SpacePacketConf.h
    ${PROJECT_SOURCE_DIR}/include/tmtp/TestProtConf.h
    ${PROJECT_SOURCE_DIR}/include/tmtp/TmFrameBitrate.h
    ${PROJECT_SOURCE_DIR}/include/tmtp/TmFrameTimestamp.h
    ${PROJECT_SOURCE_DIR}/include/tmtp/TmMasterChannel.h
    ${PROJECT_SOURCE_DIR}/include/tmtp/TmOcf.h
    ${PROJECT_SOURCE_DIR}/include/tmtp/TmPhysicalChannel.h
    ${PROJECT_SOURCE_DIR}/include/tmtp/Tmtp.h
    ${PROJECT_SOURCE_DIR}/include/tmtp/TmtpOcf.h
    ${PROJECT_SOURCE_DIR}/include/tmtp/TmtpPacket.h
    ${PROJECT_SOURCE_DIR}/include/tmtp/TmTransferFrame.h
    ${PROJECT_SOURCE_DIR}/include/tmtp/TmVirtualChannel.h
)

set(Boost_USE_MULTITHREADED On)
find_package(Boost REQUIRED COMPONENTS system thread)

add_library(tmtp SHARED ${LIBTMTP_SOURCES} ${LIBTMTP_HEADERS})
target_link_libraries(tmtp PUBLIC Boost::thread PRIVATE Boost::system)

# Explicitly mark all symbols to be exported
#set_property(TARGET tmtp PROPERTY C_VISIBILITY_PRESET default)
#set_property(TARGET tmtp PROPERTY VISIBILITY_INLINES_HIDDEN 1)

target_include_directories(tmtp
    INTERFACE
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/tmtp
)

# Relax some compile time warnings
# target_compile_options(tmtp
#     PRIVATE
#         $<$<OR:$<C_COMPILER_ID:Clang>,$<C_COMPILER_ID:AppleClang>,$<C_COMPILER_ID:GNU>>:-fpermissive>
# )
target_compile_options(tmtp PRIVATE -fpermissive -Wno-static-float-init)

set_property(TARGET tmtp PROPERTY CXX_STANDARD 11)

#############################################
# Add an alias so that library can be used inside the build tree, e.g. when testing

add_library(tmtp::tmtp ALIAS tmtp)
message(STATUS "created ALIAS target tmtp::tmtp")

#############################################
# Install instruction


message(STATUS "Files will be installed to ${CMAKE_INSTALL_PREFIX}")

install(
    FILES
        ${LIBTMTP_HEADERS}
    DESTINATION include/tmtp
)

# export target
install(TARGETS tmtp EXPORT tmtpTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(EXPORT tmtpTargets
    FILE tmtpTargets.cmake
    NAMESPACE tmtp::
    DESTINATION lib/cmake/tmtp
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file("tmtpConfigVersion.cmake"
    VERSION ${tmtp_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/tmtpConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/tmtpConfigVersion.cmake"
    DESTINATION lib/cmake/tmtp
)
