#set(metawear_lib_path "/Users/roland/Development/personal/sports_imu/MetaWear-SDK-Cpp/dist/release/lib/arm")
set(metawear_lib_path "/usr/local/lib")
set(metawear_header_path "../../MetaWear-SDK-Cpp/src")

find_library(metawear_lib metawear PATHS ${metawear_lib_path})

if(metawear_lib)
    # Create an imported target for the metawear library
    add_library(Metawear::metawear SHARED IMPORTED)
    
    # Set the location of the library
    set_target_properties(Metawear::metawear PROPERTIES
        IMPORTED_LOCATION "${metawear_lib}"
    )
    
    # Set include directories
    target_include_directories(Metawear::metawear INTERFACE ${metawear_header_path})
    
    message(STATUS "Found Metawear library: ${metawear_lib}")
    set(Metawear_FOUND TRUE)
else()
    message(FATAL_ERROR "Metawear library not found")
    set(Metawear_FOUND FALSE)
endif()

# Fix the library install name if needed
if(APPLE AND EXISTS "${metawear_lib}")
    # Get the real path (resolve symlinks) to fix the actual library file
    get_filename_component(REAL_METAWEAR_LIB "${metawear_lib}" REALPATH)
    execute_process(
        COMMAND ${CMAKE_INSTALL_NAME_TOOL} -id "${metawear_lib}" "${REAL_METAWEAR_LIB}"
        OUTPUT_QUIET
        ERROR_QUIET
    )
endif()

# Enable RPATH for macOS
set(CMAKE_MACOSX_RPATH ON)

# Legacy variable for backward compatibility
set(metawear_lib Metawear::metawear)
