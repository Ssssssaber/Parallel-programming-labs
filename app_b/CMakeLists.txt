file(GLOB_RECURSE SOURCES
 ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(app_b ${SOURCES})

target_include_directories(app_b PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/vendor/
)
