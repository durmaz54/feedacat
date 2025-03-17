/**
 * libdatachannel media sender example
 * Copyright (c) 2020 Staz Modrzynski
 * Copyright (c) 2020 Paul-Louis Ageneau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */


#include <cstddef>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "rtc/rtc.hpp"
#include <nlohmann/json.hpp>
#include "mqtt.h"

typedef int SOCKET;

using nlohmann::json;

const int BUFFER_SIZE = 2048;

int main() {
    auto p2pcomm = Peer2PeerCommunication<PeerType::OFFER>();
    try {
        p2pcomm.connect();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));


    rtc::Configuration config;
    config.iceServers.emplace_back("stun:93.115.79.173:3478");

    rtc::InitLogger(rtc::LogLevel::Error);
    auto pc = std::make_shared<rtc::PeerConnection>(config);

    pc->onLocalDescription([](rtc::Description sdp) {
        std::cout << "Local Description oluşturuldu \n";
    });

    pc->onLocalCandidate([&p2pcomm](rtc::Candidate candidate) {
        // Send the candidate to the remote peer
        json message = {
            {"candidate", candidate.candidate()},
            {"mid", candidate.mid()}
        };
        p2pcomm.sendCandidate(nlohmann::to_string(message));
    });


    std::atomic<rtc::PeerConnection::State> m_state = rtc::PeerConnection::State::New;
    pc->onStateChange(
        [&m_state](rtc::PeerConnection::State state) {
            std::cout << "State: " << state << std::endl;
            m_state = state;
        });

    std::atomic<rtc::PeerConnection::GatheringState> m_gathering_state;
    pc->onGatheringStateChange([&m_gathering_state, &p2pcomm, &pc](rtc::PeerConnection::GatheringState state) {
        std::cout << "Gathering State: " << state << std::endl;
        m_gathering_state = state;
                if (m_gathering_state == rtc::PeerConnection::GatheringState::Complete) {
            auto description = pc->localDescription();
            json message = {
                {"type", description->typeString()},
                {"sdp", std::string(description.value())}
            };
            std::cout << message << std::endl;
            p2pcomm.sendSdp(to_string(message));
        }
    });

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(6000);

    if (bind(sock, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr)) < 0)
        throw std::runtime_error("Failed to bind UDP socket on 127.0.0.1:6000");

    int rcvBufSize = 212992;
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char *>(&rcvBufSize),
               sizeof(rcvBufSize));

    const rtc::SSRC ssrc = 42;
    rtc::Description::Video media("video", rtc::Description::Direction::SendOnly);
    media.addH264Codec(96); // Must match the payload type of the external h264 RTP stream
    media.addSSRC(ssrc, "video-send");
    auto track = pc->addTrack(media);

    pc->setLocalDescription();

    while (m_state != rtc::PeerConnection::State::Connected) {

        if (p2pcomm.getAnswerSdp()) {
            json j = json::parse(p2pcomm.getAnswerSdp().value());
            rtc::Description answer(j["sdp"].get<std::string>(), j["type"].get<std::string>());
            pc->setRemoteDescription(answer);
            p2pcomm.getAnswerSdp().reset();
        }
        if (p2pcomm.getAnswerCandidate()) {
            json j = json::parse(p2pcomm.getAnswerCandidate().value());
            auto m_candidate = rtc::Candidate(j["candidate"].get<std::string>());
            pc->addRemoteCandidate(m_candidate);
            p2pcomm.getAnswerCandidate().reset();
        }
    }

    // Receive from UDP
    char buffer[BUFFER_SIZE];
    int len;
    while ((len = recv(sock, buffer, BUFFER_SIZE, 0)) >= 0) {
        if (len < sizeof(rtc::RtpHeader) || !track->isOpen())
            continue;

        auto rtp = reinterpret_cast<rtc::RtpHeader *>(buffer);
        rtp->setSsrc(ssrc);

        track->send(reinterpret_cast<const std::byte *>(buffer), len);
    }
}
