# Create Par library for testing
add_library(par STATIC)
target_sources(par
    PRIVATE
      src/image_par.cpp
)
target_include_directories(par PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(par PUBLIC Microsoft.GSL::GSL common)

# Create executable
add_executable(render-par)
target_sources(render-par 
    PRIVATE 
      src/main.cpp
)
target_include_directories(render-par PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(render-par PRIVATE Microsoft.GSL::GSL common par)

