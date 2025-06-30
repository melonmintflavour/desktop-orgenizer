cmake_minimum_required(VERSION 3.10)
project(DesktopOverlay CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Fetch Dear ImGui using FetchContent
include(FetchContent)
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG docking # Using docking branch for potential future use, includes many backends
)
FetchContent_MakeAvailable(imgui)

# Fetch GLFW using FetchContent
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.3.8 # Specify a version
)
FetchContent_MakeAvailable(glfw)

# Add source files
add_executable(DesktopOverlay
  src/main.cpp
  src/imgui_impl_glfw.cpp
  src/imgui_impl_opengl3.cpp
  src/cJSON.c
)

# Link Dear ImGui and GLFW
target_link_libraries(DesktopOverlay PRIVATE imgui glfw)

# Add include directories for ImGui and GLFW
target_include_directories(DesktopOverlay PRIVATE
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends # For imgui_impl_xxx.h files if not copied locally but used from source
  ${glfw_SOURCE_DIR}/include # For glfw3.h
  src # To find our copied imgui_impl_glfw.h and imgui_impl_opengl3.h
)


# For Windows, link necessary system libraries for graphics and windowing
if(WIN32)
  target_link_libraries(DesktopOverlay PRIVATE opengl32 shell32 gdi32 user32)
endif()

# For Linux, GLFW might need additional libraries
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # Find X11 and link necessary X11 libraries for GLFW
  find_package(X11 REQUIRED)
  if(X11_FOUND)
    target_link_libraries(DesktopOverlay PRIVATE X11::X11)
  endif()
  # GLFW also needs pthreads and dl
  target_link_libraries(DesktopOverlay PRIVATE Threads::Threads dl)
endif()

# On macOS, GLFW needs Cocoa, IOKit, CoreFoundation
if(APPLE)
  target_link_libraries(DesktopOverlay PRIVATE "-framework Cocoa -framework IOKit -framework CoreFoundation")
endif()
