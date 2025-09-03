
set(exe_name "simu_app")
add_executable(${exe_name} main.cpp)
target_compile_features(${exe_name} PRIVATE cxx_std_17)
target_link_libraries(${exe_name} PRIVATE ${cpp_lib_name})

set(exe_name "test")
add_executable(${exe_name} test.cpp)
target_compile_features(${exe_name} PRIVATE cxx_std_17)
target_link_libraries(${exe_name} PRIVATE ${cpp_lib_name}) 