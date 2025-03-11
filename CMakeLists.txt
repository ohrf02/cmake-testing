cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_TOOLCHAIN_FILE Windows.MSVC.toolchain.cmake)

project(HistoryReader)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/external/SQLiteCpp)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/external/json)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/https)

# Create an executable (or library) and link SQLite
add_executable(app ${CMAKE_CURRENT_LIST_DIR}/src/app/main.cpp)
target_link_libraries(app
  sqlite3
  nlohmann_json::nlohmann_json
  https_lib
  )

target_include_directories(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

# Optionally, define lightweight build options
target_compile_definitions(sqlite3 PRIVATE
    SQLITE_OMIT_LOAD_EXTENSION
    SQLITE_OMIT_DEPRECATED
    SQLITE_THREADSAFE=0
    SQLITE_OMIT_SHARED_CACHE
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/external/googletest)

    # Add test executable
    add_executable(json_tests ${CMAKE_CURRENT_LIST_DIR}/tests/test_json.cpp)

    # Link GTest
    target_link_libraries(json_tests PRIVATE gtest_main nlohmann_json::nlohmann_json)
    target_compile_options(json_tests PRIVATE $<$<CONFIG:Debug>:/MTd>)

    enable_testing()
    include(GoogleTest)
    gtest_discover_tests(json_tests)
endif()

set(CLANG_FORMAT "C:/Program Files/LLVM/bin/clang-format.exe" CACHE STRING "Path to clang-format")

if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_SOURCE_FILES
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_SOURCE_DIR}/src/*.h"
        "${CMAKE_SOURCE_DIR}/tests/*.cpp"
    )

    message(STATUS "Found clang-format: ${CLANG_FORMAT} files: ${ALL_SOURCE_FILES}")


    # Define a custom CMake target for formatting
    add_custom_target(run_format ALL
        COMMAND ${CLANG_FORMAT} -i ${ALL_SOURCE_FILES}
        VERBATIM
    )
else()
    message(WARNING "clang-format not found! Skipping format target.")
endif()
