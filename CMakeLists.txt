cmake_minimum_required(VERSION 4.2)

project(aurion-engine VERSION 0.1.0)

add_subdirectory(AurionEngine)

set(CPACK_PACKAGE_NAME "AurionEngine")
set(CPACK_GENERATOR "ZIP")

include(CPACK)