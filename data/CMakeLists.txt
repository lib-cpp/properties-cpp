configure_file(
  properties-cpp.pc.in properties-cpp.pc @ONLY
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/properties-cpp.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
