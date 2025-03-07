#include <iostream>
#include <memory>
#include <thread>
#include "rtc/rtc.hpp"
#include <chrono>
#include "mqtt.h"

int main() {
    std::atomic<bool> is_rdp_transfer = false;

    auto p2pcomm = Peer2PeerCommunication<PeerType::ANSWER>();
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
        std::cout << "Local Description oluşturuldu \n";
        my_sdp = std::move(sdp);
    });

    pc->onLocalCandidate([&p2pcomm](rtc::Candidate candidate) {
    });

    pc->onStateChange([](rtc::PeerConnection::State state) {
        std::cout << "State: " << state << std::endl;
    });

    pc->onGatheringStateChange([](rtc::PeerConnection::GatheringState state) {
        std::cout << "Gathering state: " << state << std::endl;
    });

    // std::shared_ptr<rtc::DataChannel> dc;
    // pc->onDataChannel([&dc](std::shared_ptr<rtc::DataChannel> incoming) {
    //     dc = incoming;
    //     dc->onMessage([](std::variant<rtc::binary, rtc::string> message) {
    //         if (std::holds_alternative<rtc::string>(message)) {
    //             std::cout << "Received: " << get<rtc::string>(message) << std::endl;
    //         }
    //     });
    // });
    rtc::Description::Video media("video", rtc::Description::Direction::RecvOnly);
    media.addH264Codec(96);
    media.setBitrate(3000);
    auto track = pc->addTrack(media);

    auto session = std::make_shared<rtc::RtcpReceivingSession>();
    track->setMediaHandler(session);

    track->onMessage(
        [](rtc::binary message) {
            std::cout << "received message: " << message.size() << std::endl;
    },nullptr);
    // pc->setLocalDescription();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    while (pc->state() != rtc::PeerConnection::State::Connected) {
        if (p2pcomm.getAnswerSdp()) {
            pc->setRemoteDescription(rtc::Description(p2pcomm.getAnswerSdp().value()));
            p2pcomm.getAnswerSdp().reset();
        }
        if (my_sdp) {
            p2pcomm.sendSdp(std::string(my_sdp.value()));
        }
    }
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
