#######################################################################
# A convenience target that carries out the following steps:
#   - Apply astyle to all source files of interest.
#   - Build & test in a chroot, comparable setup to CI/Autolanding
#     and ppa builders. Will fail if new files have not been added.
#   - Build & test for android.
#
# NOTE: This target is very sensitive to the availability of all
#       all required dependencies. For that, we prefer to fail the
#       target if deps are missing to make the problem very visible.
#
# TODO:
#   - Wire up the style-check target once we have reached a state
#     where trunk actually passes the style check.
#######################################################################
add_custom_target(
  pre-push

  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}  
)

#######################################################################
#      Add target for running astyle with the correct options         #
#######################################################################
find_program(ASTYLE_EXECUTABLE astyle)

if (ASTYLE_EXECUTABLE)
  add_custom_target(
    astyle 
    ${ASTYLE_EXECUTABLE} --style=allman -s4 --indent=spaces=4 --pad-header --align-pointer=type --recursive ${CMAKE_SOURCE_DIR}/include/*.h
    COMMAND ${ASTYLE_EXECUTABLE} --recursive --style=allman -s4 --indent=spaces=4 --pad-header --align-pointer=type ${CMAKE_SOURCE_DIR}/tests/*.cpp
    COMMAND ${ASTYLE_EXECUTABLE} --recursive --style=allman -s4 --indent=spaces=4 --pad-header --align-pointer=type ${CMAKE_SOURCE_DIR}/examples/*.h  ${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp
    VERBATIM
  )
endif (ASTYLE_EXECUTABLE)

#######################################################################
#      Add target for creating a source tarball with bzr export       #
#######################################################################
add_custom_target(
  pre-push-source-tarball

  COMMAND rm -rf pre-push-build-area
  COMMAND mkdir pre-push-build-area
  COMMAND bzr export --root pre-push pre-push-build-area/${PROJECT_NAME}_${DBUS_CPP_VERSION_MAJOR}.${DBUS_CPP_VERSION_MAJOR}.${DBUS_CPP_VERSION_MAJOR}.orig.tar.bz2 ${CMAKE_SOURCE_DIR}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Preparing source tarball for pre-push build & test"
)

#######################################################################
#      Add target for extracting source tarball for pdebuild          #
#######################################################################
add_custom_target(
  extract-pre-push-tarball
  COMMAND tar -xf {PROJECT_NAME}_${DBUS_CPP_VERSION_MAJOR}.${DBUS_CPP_VERSION_MAJOR}.${DBUS_CPP_VERSION_MAJOR}.orig.tar.bz2
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/pre-push-build-area VERBATIM
)

#######################################################################
#  Builds & tests the last committed revision of the current branch   #
#######################################################################
find_program(PDEBUILD_EXECUTABLE pdebuild)
if(NOT PDEBUILD_EXECUTABLE)
  message(STATUS "pdebuild NOT found, pre-push is going to FAIL")
endif()

add_custom_target(
  pdebuild
  COMMAND ${PDEBUILD_EXECUTABLE}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/pre-push-build-area/pre-push 
  COMMENT "Building & testing in chroot'd environment"
  VERBATIM  
)

#######################################################################
# pdebuild: make tarball -> extract to build area -> pdebuild         #
# android-build: invoke cross-compile script                          #
#######################################################################
add_dependencies(extract-pre-push-tarball pre-push-source-tarball)
add_dependencies(pdebuild extract-pre-push-tarball)

add_dependencies(pre-push pdebuild android-build)
