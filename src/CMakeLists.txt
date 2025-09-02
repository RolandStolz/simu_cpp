set(header_files
    ${PROJECT_SOURCE_DIR}/include/simu_lib/lib.h
    ${PROJECT_SOURCE_DIR}/include/simu_lib/metawear_transport_simpleble.h
)

set(source_files
    # lib.cpp
    metawear_transport_simpleble.cpp
)

add_library(${cpp_lib_name}
    ${source_files} ${header_files}
)

target_compile_features(${cpp_lib_name} PRIVATE cxx_std_17)

include(FindMetawear)
include(FindSimpleBle)
find_package(simpleble REQUIRED)

target_link_libraries(${cpp_lib_name} PUBLIC ${metawear_lib} simpleble)

target_include_directories(${cpp_lib_name}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${metawear_header_path}
)
