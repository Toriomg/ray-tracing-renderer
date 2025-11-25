# Create SOA library for testing
add_library(soa STATIC)
target_sources(soa
    PRIVATE
      src/image_soa.cpp
)
target_include_directories(soa PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(soa PUBLIC Microsoft.GSL::GSL common)

# Create executable
add_executable(render-soa)
target_sources(render-soa 
    PRIVATE 
      src/main.cpp
)
target_include_directories(render-soa PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(render-soa PRIVATE Microsoft.GSL::GSL common soa)

