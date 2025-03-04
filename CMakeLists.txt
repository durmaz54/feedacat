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
target_link_libraries(client PRIVATE LibDataChannel::LibDataChannel)
target_link_libraries(client PUBLIC PahoMqttCpp::paho-mqttpp3)


target_link_libraries(${PROJECT_NAME} PRIVATE LibDataChannel::LibDataChannel)
target_link_libraries(${PROJECT_NAME} PUBLIC PahoMqttCpp::paho-mqttpp3)

