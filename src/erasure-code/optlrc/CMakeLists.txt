# lrc plugin

set(optlrc_objs
  ErasureCodeOptLrc.cc
  ErasureCodePluginOptLrc.cc 
  $<TARGET_OBJECTS:erasure_code_objs>
  ${CMAKE_SOURCE_DIR}/src/common/str_map.cc
)

add_library(optlrc_objs OBJECT ${optlrc_srcs})

add_library(ec_optlrc SHARED
  $<TARGET_OBJECTS:jerasure_objs>
  $<TARGET_OBJECTS:optlrc_objs>)

add_dependencies(ec_optlrc ${CMAKE_SOURCE_DIR}/src/ceph_ver.h)
target_link_libraries(ec_optlrc crush json_spirit)
set_target_properties(ec_optlrc PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS ec_optlrc DESTINATION lib/erasure-code)
