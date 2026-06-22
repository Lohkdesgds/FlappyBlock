cmake_minimum_required(VERSION 3.28)
# CMake made 2026/06/22+

project(FlappyBlock
    VERSION 1.0
    DESCRIPTION "FlappyBlock is a copy of a popular game made in C/C++ with Allegro 5"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_ROOT_DIR "${CMAKE_SOURCE_DIR}")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -DUNICODE -D_UNICODE")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNICODE -D_UNICODE")


if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
endif()


if (MINGW)
    include(cmake/MinGWFix.cmake)
endif()

include(cmake/installBatteryEmbed.cmake)
include(cmake/installAllegro.cmake)


# ===== Shared core code ===== #

file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
    ${PROJECT_ROOT_DIR}/src/*.cpp
)

list(REMOVE_ITEM CORE_SOURCES
    "${PROJECT_ROOT_DIR}/src/main.cpp"
)

add_library(core STATIC ${CORE_SOURCES})

target_include_directories(core PUBLIC 
    ${ALLEGRO_INCLUDE_DIRS}
    ${PROJECT_ROOT_DIR}/include
)

target_link_libraries(core PUBLIC
    ${ALLEGRO_LIBRARIES}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")

target_compile_options(core PRIVATE
    $<$<CONFIG:Release>:-O3>
)


# ===== Main project code ===== #

add_executable(${PROJECT_NAME} 
    "${PROJECT_ROOT_DIR}/src/main.cpp"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    core
)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:-O3>
)

b_embed(core resources/icon_src.ico)
b_embed(core resources/atlas.png)

# ===== Enable testing ===== #

include(CTest)
enable_testing()
add_subdirectory(tests)