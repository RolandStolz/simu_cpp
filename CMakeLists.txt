cmake_minimum_required(VERSION 3.25)

# Define the project
project(
    simu_cpp
    VERSION 0.1.0
    LANGUAGES CXX
)

# Add cmake folder to the module path
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(cpp_lib_name "simu_lib")

add_subdirectory(src)
add_subdirectory(examples)

