cmake_minimum_required(VERSION 3.7.0)

project(HexEditor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(HexEditor
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/hexwidget.cpp
    src/hexwidget.h
    src/gotodialog.cpp
    src/gotodialog.h
)

target_link_libraries(HexEditor Qt5::Widgets)
