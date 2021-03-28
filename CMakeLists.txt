cmake_minimum_required(VERSION 3.12)

project(TU VERSION 0.1.0 LANGUAGES CXX)

enable_testing()
add_subdirectory(typesafe_units)
add_subdirectory(test)

# Create package version file and copy it to source folder.
include(CMakePackageConfigHelpers)
write_basic_package_version_file(${CMAKE_BINARY_DIR}/TUConfigVersion.cmake VERSION ${CMAKE_PROJECT_VERSION} COMPATIBILITY ExactVersion)
file(COPY ${CMAKE_BINARY_DIR}/TUConfigVersion.cmake DESTINATION ${CMAKE_SOURCE_DIR})