cmake_minimum_required(VERSION 3.4)
project(feedacat)
set(CMAKE_PROJECT_VERSION 3.4)
set(CMAKE_CXX_STANDARD 20)
add_subdirectory(libdatachannel)
add_subdirectory(paho.mqtt.cpp)

add_executable(${PROJECT_NAME} main.cpp
        mqtt.h
)
add_executable(client client.cpp
        mqtt.h
)


#set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/opencv-mobile-4.10.0-ubuntu-2204/lib/cmake/opencv4)
#find_package(OpenCV REQUIRED)
#target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
#include_directories(${OpenCV_INCLUDE_DIRS})


find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED libavutil libavcodec libavformat libswscale)

target_link_libraries(${PROJECT_NAME} PUBLIC ${FFMPEG_LIBRARIES})


target_include_directories(${PROJECT_NAME} PRIVATE ${GST_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${GST_LIBRARIES})


target_link_libraries(${PROJECT_NAME} PRIVATE LibDataChannel::LibDataChannel)
target_link_libraries(${PROJECT_NAME} PUBLIC PahoMqttCpp::paho-mqttpp3)

target_link_libraries(client PRIVATE LibDataChannel::LibDataChannel)
target_link_libraries(client PUBLIC PahoMqttCpp::paho-mqttpp3)

