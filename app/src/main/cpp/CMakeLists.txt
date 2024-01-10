# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

project("myapplication")

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/vulkan_cpp)
include_directories(${CMAKE_SOURCE_DIR}/vulkan_object)
include_directories(${CMAKE_SOURCE_DIR}/inc)
include_directories(${CMAKE_SOURCE_DIR}/inc/shaderc)
include_directories(${CMAKE_SOURCE_DIR}/inc/stb)

add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)

link_directories(${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI})

FILE(GLOB vulkan_src
        ${CMAKE_SOURCE_DIR}/vulkan_cpp/*.cpp
        ${CMAKE_SOURCE_DIR}/vulkan_object/*.cpp)

add_library(${CMAKE_PROJECT_NAME} SHARED
        ${vulkan_src}
        vulkan_object/vulkan_object.cpp
        create_info_factory.cpp
        tutorial.cpp
        native-lib.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME}
        shaderc
        android
        vulkan
        log)