file(GLOB_RECURSE SOURCES
 ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(app_c ${SOURCES})

target_include_directories(app_c PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/vendor/svg-cpp-plot-master
)
