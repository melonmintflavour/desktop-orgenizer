cmake_minimum_required(VERSION 3.16)
project(DesktopOverlay LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)

set(PROJECT_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/PageManager.cpp
    src/PageManager.h
    src/PageData.h
    src/PageData.cpp
    src/PageTabContentWidget.h
    src/PageTabContentWidget.cpp
    src/ZoneData.h
    src/ZoneData.cpp
    src/ZoneWidget.h
    src/ZoneWidget.cpp
    src/IconData.h
    src/IconData.cpp
    src/IconWidget.h
    src/IconWidget.cpp
    src/DatabaseManager.h
    src/DatabaseManager.cpp
    src/WidgetHostWindow.h
    src/WidgetHostWindow.cpp
    src/DraggableToolbar.h
    src/DraggableToolbar.cpp
    src/ClockWidget.h
    src/ClockWidget.cpp
    src/QuickAccessPanel.h
    src/QuickAccessPanel.cpp
    src/TodoData.h
    src/TodoWidget.h
    src/TodoWidget.cpp
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql)

# Define source group for better organization in IDEs
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${PROJECT_SOURCES})
