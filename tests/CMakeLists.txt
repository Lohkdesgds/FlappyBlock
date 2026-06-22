file(WRITE "${CMAKE_BINARY_DIR}/DartConfiguration.tcl" "")

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS src/*.cpp)
message(STATUS "Got the following tests: ${TEST_SOURCES}")

foreach(test ${TEST_SOURCES})
    get_filename_component(test_name ${test} NAME_WE)
    message(STATUS "- Building test: ${test_name}")

    add_executable(${test_name} ${test})

    target_link_libraries(${test_name} PRIVATE
        core
    )

    target_compile_definitions(${test_name} PRIVATE TESTING_FLAG)

    add_test(NAME ${test_name} COMMAND ${test_name})
    message(STATUS "- Added test: ${test_name}")
endforeach()