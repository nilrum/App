cmake_minimum_required(VERSION 3.14)
project(App)

set(CMAKE_CXX_STANDARD 17)

include(C:/CMakeProjects/Fetch.cmake)

MakeAvailable(Property github.com/nilrum/Property2 .)

set(BASE_HDR
        App.h
        Timer.h
        Progress.h
        MenuTree.h
        Translator.h
        EditorWidget.h
        )
set(BASE_CPP
        App.cpp
        Timer.cpp
        Progress.cpp
        MenuTree.cpp
        Translator.cpp
        EditorWidget.cpp
        )

if(NOT TARGET App)
    add_library(App STATIC ${BASE_CPP})
    target_include_directories(App PUBLIC ${FETCH_DIR})
    target_link_libraries(App Property)
    target_compile_definitions(App PUBLIC APP_NAME="${CMAKE_PROJECT_NAME}")
    target_compile_definitions(App PUBLIC APP_VER="${APP_VER}")
endif()

if(${BUILD_TEST})
    add_executable(AppTest
            tests/AppTest.cpp
            )
    target_link_libraries(AppTest App gtest gtest_main gmock)
    enable_testing()
    add_test(NAME AppTest COMMAND AppTest)
endif()

if(NOT DEFINED QT_GUI)
    set(QT_GUI ON)
endif()

#Если включен QT как GUI
if(${QT_GUI})
    if(MSVC)
        set(CMAKE_PREFIX_PATH D:/DiskC/Qt/Qt5.15.0/5.15.0/msvc2019/lib/cmake)
    else()
        set(CMAKE_PREFIX_PATH D:/DiskC/Qt/Qt5.15.0/5.15.0/mingw81_32/lib/cmake)
    endif()

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    find_package(Qt5 COMPONENTS Core REQUIRED)
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
    find_package(Qt5 COMPONENTS PrintSupport REQUIRED)

    set(GUI_SRC
            ${LIB_DIR}/Qt/AppQt.cpp
            ${LIB_DIR}/Qt/MainWindowQt.cpp
            ${LIB_DIR}/Qt/TimerQt.cpp
            ${LIB_DIR}/Qt/ObjTreeQt.cpp
            ${LIB_DIR}/Qt/EditorWidgetQt.cpp
            ${LIB_DIR}/Qt/resources.qrc
            )
    set(GUI_LIBS
            Qt5::Core
            Qt5::Widgets
            Qt5::PrintSupport
            )
    set(GUI_DIRS PUBLIC
            ${LIB_DIR}/Qt
            )
    set(GUI_INIT_HDR "\"Qt/InitApp.h\"")
    set(Qt5Widgets_INCLUDE_DIRS ${Qt5Widgets_INCLUDE_DIRS} PARENT_SCOPE)
endif()

if(GUI_INIT_HDR)
    add_library(GuiLib OBJECT)
    target_link_libraries(GuiLib ${GUI_LIBS})

    add_library(GuiApp OBJECT ${GUI_SRC})
    target_include_directories(GuiApp PUBLIC
            ${FETCH_DIR}
            ${GUI_DIRS}
            )
    target_link_libraries(GuiApp App ${GUI_LIBS})
    target_compile_definitions(GuiApp PUBLIC GUI_INIT_HDR=${GUI_INIT_HDR})

    add_executable(TestGuiApp main.cpp)
    target_link_libraries(TestGuiApp GuiApp)
endif()