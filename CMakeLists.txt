cmake_minimum_required(VERSION 3.10)
project(DesktopOrgUtility CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find Windows SDK components
find_package(Threads REQUIRED)

# Add source files
# Updated to include new source files.
# Note: main.cpp will need to include Persistence.h and DataModels.h (indirectly)
# and potentially create a PersistenceManager instance.
add_executable(DesktopOrgUtility WIN32
    main.cpp
    src/HubWindow.cpp
    # src/Persistence.cpp # If we add a .cpp file for PersistenceManager methods
)

# Include directories for our own source files
include_directories(src third_party) # Ensure third_party is also here if json.hpp is directly included by HubWindow.cpp
# Link libraries
target_link_libraries(DesktopOrgUtility PRIVATE
    gdi32
    user32
    d2d1
    dwmapi
    Comctl32 # For Common Controls used in HubWindow
    # Ole32.lib for COM, etc.
)

# Ensure proper settings for a Windows application
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

# Include directories if needed (e.g., for third-party libraries)
# include_directories(path/to/includes)

# Add preprocessor definitions if needed
# target_compile_definitions(DesktopOrgUtility PRIVATE SOME_DEFINITION)
