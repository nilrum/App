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
        )
set(BASE_CPP
        App.cpp
        Timer.cpp
        Progress.cpp
        MenuTree.cpp
        Translator.cpp
        )

try_compile(HAS_STD_FS
        "${CMAKE_BINARY_DIR}/temp"
        "${LIB_DIR}/FileSystem/TestStdFileSystem.cpp"
        CMAKE_FLAGS -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON
        )

try_compile(HAS_STD_EXPER_FS
        "${CMAKE_BINARY_DIR}/temp"
        "${LIB_DIR}/FileSystem/TestExperFileSystem.cpp"
        CMAKE_FLAGS -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON
        LINK_LIBRARIES stdc++fs)

if(HAS_STD_FS)
    message("Using std::filesystem")
    add_definitions("-DFILESYSTEM_STD")
    set(PROPERTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Property CACHE FILEPATH "PROPERTY_DIR")
elseif(HAS_STD_EXPER_FS)
    message("Using std::experimental::filesystem")
    add_definitions("-DFILESYSTEM_EXPER")
else()
    message("Using own filesystem")
    add_definitions("-DFILESYSTEM_OWN")
    set(BASE_CPP ${BASE_CPP} ${LIB_DIR}/FileSystem/OwnFileSystem.cpp)

endif()

add_library(App STATIC ${BASE_CPP})
target_include_directories(App PUBLIC ${FETCH_DIR} ${App_DEPS_INCL})
target_link_libraries(App PUBLIC Property)

if(${BUILD_TEST})
    add_executable(AppTest
            tests/AppTest.cpp
            )
    target_include_directories(AppTest
            PUBLIC ${FETCH_DIR}     #Подключаем каталог Libs
            ${App_DEPS_INCL}        #Подключаем каталог Property и все каталоги от которых оно зависит
            )
    target_link_libraries(AppTest PUBLIC App gtest gtest_main gmock)
    enable_testing()
    add_test(NAME AppTest COMMAND AppTest)
endif()

#Включаем зависимости для корневого проекта
DepsAvailable()