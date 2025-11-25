# Create AOS library for testing
add_library(aos STATIC)
target_sources(aos
    PRIVATE
      src/image_aos.cpp
)
target_include_directories(aos PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(aos PUBLIC Microsoft.GSL::GSL common)

# Create executable
add_executable(render-aos)
target_sources(render-aos 
    PRIVATE 
      src/main.cpp
)
target_include_directories(render-aos PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(render-aos PRIVATE Microsoft.GSL::GSL common aos)
