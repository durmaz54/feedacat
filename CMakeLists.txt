cmake_minimum_required(VERSION 3.4)
project(feedacat)
set(CMAKE_PROJECT_VERSION 3.4)
set(CMAKE_CXX_STANDARD 20)
add_subdirectory(libdatachannel)
add_subdirectory(paho.mqtt.cpp)

add_executable(${PROJECT_NAME} main.cpp
        mqtt.h
)

target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE LibDataChannel::LibDataChannel)
target_link_libraries(${PROJECT_NAME} PUBLIC PahoMqttCpp::paho-mqttpp3)

