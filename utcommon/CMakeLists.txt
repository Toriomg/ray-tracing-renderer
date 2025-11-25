set(COMMON_SRC_FILES 
  #"${CMAKE_SOURCE_DIR}/common/src/ray.cpp"
  #el fichero que se añade aqui
)

set(CURRENT_DIR_SRC_FILES 
  "${CMAKE_CURRENT_SOURCE_DIR}/test_vector.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_config_parser.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_scene_parser.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_renderer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_camera.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_random.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_ppm_writer.cpp"
  #aqui se añade el test del fichero puesto arriba
)

add_unit_test_target(
  TARGET_NAME utcommon
  SOURCE_FILES ${COMMON_SRC_FILES} ${CURRENT_DIR_SRC_FILES}
  LIBRARY_FILTER common
  COVERAGE_DIR coverage-common
  LIBRARY_TO_LINK common
)
