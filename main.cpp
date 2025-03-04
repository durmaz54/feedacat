#include <iostream>
#include <memory>
#include <thread>
#include "rtc/rtc.hpp"
#include <chrono>
#include "mqtt.h"

using std::cout;

int main() {
    auto p2pcomm = Peer2PeerCommunication<PeerType::OFFER>();
    try {
        p2pcomm.connect();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    rtc::Configuration config;
    config.iceServers.emplace_back("stun:93.115.79.173:3478");

    auto pc = std::make_unique<rtc::PeerConnection>(config);

    std::optional<rtc::Description> my_sdp;
    pc->onLocalDescription([&my_sdp](rtc::Description sdp) {
        // std::cout << "Local Description oluşturuldu \n";
        my_sdp = std::move(sdp);
    });

    auto dc = pc->createDataChannel("test");

    dc->onOpen([&dc]() {
        // std::cout << "DataChannel açıldı: " << dc->label() << std::endl;
        dc->send("Merhaba from Offer!");
    });

    dc->onMessage([](std::variant<rtc::binary, rtc::string> message) {
        if (std::holds_alternative<rtc::string>(message)) {
            std::cout << "Mesaj alındı: " << std::get<rtc::string>(message) << std::endl;
        }
    });

    while (pc->state() != rtc::PeerConnection::State::Connected) {
        if (my_sdp) {
            p2pcomm.sendSdp(std::string(my_sdp.value()));
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        try {
            if (p2pcomm.getAnswerSdp()) {
                pc->setRemoteDescription(rtc::Description(p2pcomm.getAnswerSdp().value()));
                std::cout << "------------------- ALINAN SDP ----------\n";
                std::cout << p2pcomm.getAnswerSdp().value();
                std::cout << "-----------------------------------------\n";
                std::cout << "------------------- BENİM SDP ----------\n";
                std::cout << my_sdp.value();
                std::cout << "-----------------------------------------\n";
                p2pcomm.getAnswerSdp().reset();
            }
        } catch (const std::exception &e) {
            std::cout << "hata satırı 61" << std::endl;
            std::cerr << e.what() << std::endl;
        }
    }
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        dc->send("hello");
    }


    return 0;
}
