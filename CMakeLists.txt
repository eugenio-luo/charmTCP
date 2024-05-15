cmake_minimum_required(VERSION 3.16)
project(charmTCP) 

file(GLOB_RECURSE sources      src/*.cpp src/*.h)
file(GLOB_RECURSE sources_test src/test/*.cpp)

add_executable(charmTCP ${sources})

target_compile_options(charmTCP PUBLIC -std=c++17 -Wall)

set_target_properties(charmTCP PROPERTIES RUNTIME_OUTPUT_DIRECTORY "bin")

find_package(GTest)

if(GTEST_FOUND)

  add_executable(unit_tests ${sources_test} ${sources})

  target_compile_definitions(unit_tests PUBLIC UNIT_TESTS)

  set_target_properties(charmTCP PROPERTIES ENABLE_EXPORTS on)

  target_link_libraries(unit_tests PUBLIC
    ${GTEST_BOTH_LIBRARIES}
    charmTCP
  )

  target_include_directories(unit_tests PUBLIC
    ${GTEST_INCLUDE_DIRS} # doesn't do anything on linux
  )
  
endif()

install(TARGETS charmTCP DESTINATION build)
set(CPACK_PACKAGE_NAME "charmTCP")
set(CPACK_PACKAGE_VERSION "0.0.1")
set(CPACK_MONOLITHIC_INSTALL 1)

include(CPack)
