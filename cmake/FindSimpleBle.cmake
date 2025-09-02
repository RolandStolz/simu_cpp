# Note: This requires clang, since it uses the -fobjc-arc flag, which is called -fobjc-gc for gcc
include(FetchContent)
FetchContent_Declare(
    simpleble
    GIT_REPOSITORY https://github.com/OpenBluetoothToolbox/SimpleBLE
    GIT_TAG v0.10.3
    GIT_SHALLOW YES
)

# Note that here we manually do what FetchContent_MakeAvailable() would do,
# except to ensure that the dependency can also get what it needs, we add
# custom logic between the FetchContent_Populate() and add_subdirectory()
# calls.
# FetchContent_GetProperties(simpleble)
if(NOT simpleble_POPULATED)
    FetchContent_Populate(simpleble)
    list(APPEND CMAKE_MODULE_PATH "${simpleble_SOURCE_DIR}/cmake/find")
    add_subdirectory("${simpleble_SOURCE_DIR}/simpleble" "${simpleble_BINARY_DIR}")
endif()
# FetchContent_MakeAvailable(simpleble)

set(simpleble_FOUND 1)
