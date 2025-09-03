set(header_files
    ${PROJECT_SOURCE_DIR}/include/simu_lib/lib.h
    ${PROJECT_SOURCE_DIR}/include/simu_lib/handler.h
    ${PROJECT_SOURCE_DIR}/include/simu_lib/metawear_transport_simpleble.h
    ${PROJECT_SOURCE_DIR}/include/simu_lib/utils.h
    ${PROJECT_SOURCE_DIR}/include/simu_lib/metawear_async.h
)

set(source_files
    # lib.cpp
    handler.cpp
    metawear_transport_simpleble.cpp
    metawear_async.cpp
)

add_library(${cpp_lib_name}
    ${source_files} ${header_files}
)

target_compile_features(${cpp_lib_name} PRIVATE cxx_std_17)

include(FindMetawear)
include(FindSimpleBle)
find_package(simpleble REQUIRED)

target_link_libraries(${cpp_lib_name} PUBLIC Metawear::metawear simpleble)

target_include_directories(${cpp_lib_name}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)
