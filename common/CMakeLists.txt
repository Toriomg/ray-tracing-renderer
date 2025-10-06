add_library(common STATIC)

target_sources(common 
    PRIVATE 
        src/vector.cpp
        src/camera.cpp
        src/renderer.cpp
        src/scene_parser.cpp
)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(common PUBLIC Microsoft.GSL::GSL)
