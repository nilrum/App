cmake_minimum_required(VERSION 3.14)
project(App)

set(CMAKE_CXX_STANDARD 17)

if(NOT DEFINED FETCH_DIR)
    set(FETCH_DIR ${CMAKE_SOURCE_DIR}/Libs)
endif()

set(BUILD_TEST ON)

set(PROPERTY_DIR ${FETCH_DIR}/Property)

include(FetchContent)
FetchContent_Declare(
        Property
        GIT_REPOSITORY https://github.com/nilrum/Property2.git
        SOURCE_DIR ${PROPERTY_DIR}
)

FetchContent_Populate(Property)
add_subdirectory(${PROPERTY_DIR})

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
        "${CMAKE_SOURCE_DIR}/FileSystem/TestStdFileSystem.cpp"
        CMAKE_FLAGS -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON
        )

try_compile(HAS_STD_EXPER_FS
        "${CMAKE_BINARY_DIR}/temp"
        "${CMAKE_SOURCE_DIR}/FileSystem/TestExperFileSystem.cpp"
        CMAKE_FLAGS -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON
        LINK_LIBRARIES stdc++fs)

if(HAS_STD_FS)
    message("Using std::filesystem")
    add_definitions("-DFILESYSTEM_STD")
elseif(HAS_STD_EXPER_FS)
    message("Using std::experimental::filesystem")
    add_definitions("-DFILESYSTEM_EXPER")
else()
    message("Using own filesystem")
    add_definitions("-DFILESYSTEM_OWN")
    set(BASE_CPP ${BASE_CPP} FileSystem/OwnFileSystem.cpp)

endif()


if(${BUILD_TEST})
    add_executable(AppTest
            tests/AppTest.cpp
            ${BASE_CPP}
            )
    target_include_directories(AppTest
            PUBLIC ${FETCH_DIR}
            )
    target_link_libraries(AppTest PUBLIC Property gtest gtest_main gmock)
    enable_testing()
    add_test(NAME AppTest COMMAND AppTest)
endif()

