#set(metawear_lib_path "/Users/roland/Development/personal/sports_imu/MetaWear-SDK-Cpp/dist/release/lib/arm")
set(metawear_lib_path "/usr/local/lib")
set(metawear_header_path "../../MetaWear-SDK-Cpp/src")

find_library(metawear_lib metawear PATHS ${metawear_lib_path})

install(FILES ${metawear_lib} DESTINATION /usr/local/lib)

set(CMAKE_MACOSX_RPATH ON)
set(CMAKE_INSTALL_RPATH "${metawear_lib_path}")

message(STATUS "Metawear libs: ${metawear_lib}")
