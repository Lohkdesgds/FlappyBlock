message(STATUS "Applying patch for MINGW / Windows")

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00")

message(STATUS "Patch for MINGW / Windows applied")