cmake_minimum_required(VERSION 3.16)

project(charmTCP VERSION 0.0.1 DESCRIPTION "small user-space TCP implementation") 

option(BUILD_TESTS "build and run tests" OFF) 
option(BUILD_LIBRARY "build it as library" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE sources      src/*.cpp src/*.h)

if (BUILD_TESTS)
    add_library(charmTCP SHARED ${sources})

    target_compile_options(charmTCP PUBLIC -Wall -I${CMAKE_CURRENT_SOURCE_DIR}/src/include)

    set_target_properties(charmTCP PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR})

    install(TARGETS charmTCP 
            LIBRARY DESTINATION build
            PUBLIC_HEADER DESTINATION build)
else()
    add_executable(charmTCP ${sources})

    target_compile_options(charmTCP PUBLIC -Wall -I${CMAKE_CURRENT_SOURCE_DIR}/src/include)
    
    set_target_properties(charmTCP PROPERTIES RUNTIME_OUTPUT_DIRECTORY "bin")
    install(TARGETS charmTCP DESTINATION build)
    set(CPACK_PACKAGE_NAME "charmTCP")
    set(CPACK_PACKAGE_VERSION "0.0.1")
    set(CPACK_MONOLITHIC_INSTALL 1)
    
    include(CPack)
endif()

if (BUILD_LIBRARY)
    file(GLOB_RECURSE sources_test test/*.cpp test/*.h)

    add_executable(charmTCPtests ${sources};${sources_test})

    target_compile_options(charmTCPtests PUBLIC -Wall -I${CMAKE_CURRENT_SOURCE_DIR}/src/include)

    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    target_link_libraries(charmTCPtests PRIVATE gtest_main)

    include(GoogleTest)
    gtest_discover_tests(charmTCPtests)
endif()

