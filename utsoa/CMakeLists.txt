set(COMMON_SRC_FILES
  "${CMAKE_SOURCE_DIR}/soa/include/image_soa.hpp"
)

set(CURRENT_DIR_SRC_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/test_image_soa.cpp"
)

add_unit_test_target(
  TARGET_NAME utsoa
  SOURCE_FILES ${COMMON_SRC_FILES} ${CURRENT_DIR_SRC_FILES}
  LIBRARY_FILTER soa
  COVERAGE_DIR coverage-soa
  LIBRARY_TO_LINK common soa
  INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/soa/include
)