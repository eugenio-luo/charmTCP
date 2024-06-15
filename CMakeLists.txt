cmake_minimum_required(VERSION 3.16)

project(charmTCP VERSION 0.0.1 DESCRIPTION "small user-space TCP implementation") 

option(RUN_TESTS "build and run tests" ON) 

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE sources      src/*.cpp src/*.h)

add_library(charmTCP SHARED ${sources})

target_compile_options(charmTCP PUBLIC -Wall -I${CMAKE_CURRENT_SOURCE_DIR}/src/include)

set_target_properties(charmTCP PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

install(TARGETS charmTCP 
        LIBRARY DESTINATION build
        PUBLIC_HEADER DESTINATION build)

if (RUN_TESTS)
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
#[[

file(GLOB_RECURSE sources      src/*.cpp src/*.h)
file(GLOB_RECURSE sources_test src/test/*.cpp)

if (RUN_TESTS) 
    add_executable(charmTCP ${sources})
else()
    add_executable(charmTCP ${sources}${sources_test})
endif()

target_compile_options(charmTCP PUBLIC -Wall -I${CMAKE_CURRENT_SOURCE_DIR}/src/include)

set_target_properties(charmTCP PROPERTIES RUNTIME_OUTPUT_DIRECTORY "bin")


if (RUN_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    target_link_libraries(charmTCP PRIVATE gtest_main)

    include(GoogleTest)
    gtest_discover_tests(charmTCP)

else() 

    install(TARGETS charmTCP DESTINATION build)
    set(CPACK_PACKAGE_NAME "charmTCP")
    set(CPACK_PACKAGE_VERSION "0.0.1")
    set(CPACK_MONOLITHIC_INSTALL 1)
    
    include(CPack)

endif()
]]

