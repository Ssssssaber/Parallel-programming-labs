cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)
project(parallel_2 VERSION 0.1.0 LANGUAGES C CXX)

set(OSBitness 32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(OSBitness 64)
endif()
set (FullOutputDir "${CMAKE_SOURCE_DIR}/bin/${CMAKE_SYSTEM_NAME}${OSBitness}/${CMAKE_BUILD_TYPE}")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${FullOutputDir}")

add_subdirectory(app_a)
add_subdirectory(app_b)

# add_custom_command(
#   TARGET parallel_2
#   POST_BUILD
#   COMMAND ${CMAKE_COMMAND}
#   ARGS -E copy_directory_if_different "${CMAKE_CURRENT_SOURCE_DIR}/images/" "${FullOutputDir}/images/"
#   COMMENT "---- Copying Images -----"
# )

