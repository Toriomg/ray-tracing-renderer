add_library(common STATIC)

target_sources(common 
    PRIVATE
        src/dataStructs/bvh.cpp
        src/utilities/random.cpp
        src/camera.cpp
        src/renderer.cpp
        src/scene_parser.cpp
        src/ppm_writer.cpp
        src/config_parser.cpp
)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(common PUBLIC cxx_std_20)

target_link_libraries(common PUBLIC Microsoft.GSL::GSL TBB::tbb)
