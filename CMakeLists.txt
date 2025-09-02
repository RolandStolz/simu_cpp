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
set(exe_name "simu_app")

add_subdirectory(src)

add_executable(${exe_name} main.cpp)
target_compile_features(${exe_name} PRIVATE cxx_std_17)
target_link_libraries(${exe_name} PRIVATE ${cpp_lib_name})

# Necessary for the dynamic linker to find the metawear library during execution.
# https://stackoverflow.com/questions/13656033/how-do-you-add-an-rpath-to-an-executable-in-cmake-on-build-not-install-on-osx
add_custom_command(
    TARGET ${exe_name} POST_BUILD
    COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change libmetawear.dylib.0 /usr/local/lib/libmetawear.dylib.0 $<TARGET_FILE:${exe_name}>
)
