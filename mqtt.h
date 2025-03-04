//
// Created by durmaz on 03.03.2025.
//

#ifndef MQTT_H
#define MQTT_H
#include "mqtt/client.h"
#include <ctime>
#include <oneapi/tbb/detail/_template_helpers.h>

std::string random_string() {
    srand(static_cast<unsigned int>(time(0)));
    auto value = rand();
    return std::to_string(rand());
}

static const std::string SERVER_ADDRESS = "ws://93.115.79.173:8083"; // WebSocket MQTT Broker Adresi
static const int QOS = 2; // Tam olarak bir kez gönderilir. En az bir kez teslim alınır.
static const long TIMEOUT = 500;

enum class PeerType {
    OFFER,
    ANSWER
};

template<PeerType type>
class Peer2PeerCommunication {
public:
    static constexpr const char *TOPIC_SDP = (type == PeerType::OFFER) ? "offer/sdp" : "answer/sdp";
    static constexpr const char *TOPIC_CANDIDATE = (type == PeerType::OFFER) ? "offer/candidate" : "answer/candidate";
    static constexpr const char *TOPIC_OTHER_SDP = (type == PeerType::OFFER) ? "answer/sdp" : "offer/sdp";
    static constexpr const char *TOPIC_OTHER_CANDIDATE = (type == PeerType::OFFER) ? "answer/candidate" : "offer/candidate";

    Peer2PeerCommunication() : m_client(SERVER_ADDRESS, random_string()),
                               m_sdp(std::nullopt), m_candidate(std::nullopt) {
    }

    void connect() {
        if (!m_client.connect(m_connOpts)->wait_for(TIMEOUT))
            throw std::runtime_error("MQTT Bağlantısı sağlanamadı");
        if (!m_client.subscribe(TOPIC_OTHER_SDP, QOS)->wait_for(TIMEOUT))
            throw std::runtime_error("MQTT Subscribe başarısız");
        if (!m_client.subscribe(TOPIC_OTHER_CANDIDATE, QOS)->wait_for(TIMEOUT))
            throw std::runtime_error("MQTT Subscribe başarısız");
        m_client.set_message_callback([&](mqtt::const_message_ptr payload) {
            if (payload->get_topic_ref().to_string() == TOPIC_OTHER_SDP) {
                m_sdp = payload->get_payload_str();
            } else if (payload->get_topic_ref().to_string() == TOPIC_OTHER_CANDIDATE) {
                m_candidate = payload->get_payload_str();
            } else {
                throw std::runtime_error("Bilinmeyen Topic'e abone olunmuş");
            }
        });
    }

    bool sendSdp(const std::string &sdp) {
        mqtt::message_ptr msg = mqtt::make_message(TOPIC_SDP, sdp);
        msg->set_qos(QOS);
        auto rtr = m_client.publish(msg)->wait_for(TIMEOUT);
        return rtr;
    }

    bool sendCandidate(const std::string &candidate) {
        mqtt::message_ptr msg = mqtt::make_message(TOPIC_CANDIDATE, candidate);
        msg->set_qos(QOS);
        bool rtr = m_client.publish(msg)->wait_for(TIMEOUT);
        return rtr;
    }

    std::optional<std::string> &getAnswerCandidate() {
        return m_candidate;
    }

    std::optional<std::string> &getAnswerSdp() {
        return m_sdp;
    }

private:
    mqtt::async_client m_client;
    mqtt::connect_options m_connOpts;
    std::optional<std::string> m_sdp, m_candidate;
};


#endif //MQTT_H
