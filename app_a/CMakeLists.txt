file(GLOB_RECURSE SOURCES
 ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(app_a ${SOURCES})

target_include_directories(app_a PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/vendor/
)
