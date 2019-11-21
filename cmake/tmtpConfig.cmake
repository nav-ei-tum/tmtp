include(CMakeFindDependencyMacro)
find_dependency(Boost REQUIRED COMPONENTS system thread)

include("${CMAKE_CURRENT_LIST_DIR}/tmtpTargets.cmake")
