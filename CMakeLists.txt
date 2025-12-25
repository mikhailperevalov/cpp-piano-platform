cmake_minimum_required(VERSION 3.20)
project(PianoPlatform VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt6 поддержка
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Gui 
    Widgets
    Multimedia
)

# Источники проекта
set(PROJECT_SOURCES
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/MidiPlayer.h
    src/MidiPlayer.cpp
    src/MidiParser.h
    src/MidiParser.cpp
    src/PianoKeyboardWidget.h
    src/PianoKeyboardWidget.cpp
)

# Исполняемый файл
add_executable(PianoPlatform ${PROJECT_SOURCES}
    src/PianoKeyboardWidget.h
    src/PianoKeyboardWidget.cpp)

# Qt линки
target_link_libraries(PianoPlatform PRIVATE 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets
    Qt6::Multimedia
)

# Платформо-специфичные линки для MIDI
if(WIN32)
    target_link_libraries(PianoPlatform PRIVATE winmm ole32)
    add_compile_definitions(__WINDOWS_MM__)
elseif(APPLE)
    target_link_libraries(PianoPlatform PRIVATE "-framework CoreMIDI" "-framework CoreAudio" "-framework CoreFoundation")
    add_compile_definitions(__MACOSX_CORE__)
elseif(UNIX)
    target_link_libraries(PianoPlatform PRIVATE asound pthread)
    add_compile_definitions(__LINUX_ALSA__)
endif()

# Включаемые директории
target_include_directories(PianoPlatform PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/rtmidi
)

# Оптимизация Release сборки
if(MSVC)
    target_compile_options(PianoPlatform PRIVATE /W4 /permissive-)
else()
    target_compile_options(PianoPlatform PRIVATE -Wall -Wextra -Wpedantic)
endif()
