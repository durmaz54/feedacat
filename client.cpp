#include <iostream>
#include <memory>
#include <thread>
#include "rtc/rtc.hpp"
#include <chrono>
#include "mqtt/client.h"

const std::string SERVER_ADDRESS   = "ssl://tb187608.ala.us-east-1.emqxsl.com:8883";  // SSL bağlantısı için
const std::string CLIENT_ID        = "mqtt_ssl_client";
const std::string TOPIC            = "test/topic";
const std::string PAYLOAD          = "Hello, MQTT over SSL!";
const int QOS = 1;
const int TIMEOUT = 10000;

// Sertifika yolları
const std::string CA_CERT         = "/home/durmaz/Downloads/emqxsl-ca.crt";
const std::string CLIENT_CERT     = "/path/to/client.crt";
const std::string CLIENT_KEY      = "/path/to/client.key";

class callback : public virtual mqtt::callback {
public:
    void message_arrived(mqtt::const_message_ptr msg) override {
        std::cout << "Mesaj alındı: " << msg->get_topic() << " -> " << msg->to_string() << std::endl;
    }
};

int main() {
    mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID);
    callback cb;
    client.set_callback(cb);

    // SSL Seçenekleri
    mqtt::ssl_options sslOpts;
    sslOpts.set_trust_store(CA_CERT);


    // Bağlantı Seçenekleri
    mqtt::connect_options connOpts;
    connOpts.set_ssl(sslOpts);
    connOpts.set_user_name("samet");
    connOpts.set_password("samet");
    connOpts.set_clean_session(true);

    try {
        // Bağlan
        std::cout << "Broker'a bağlanılıyor..." << std::endl;
        client.connect(connOpts)->wait();

        // Mesaj Gönder
        std::cout << "Mesaj gönderiliyor..." << std::endl;
        mqtt::message_ptr pubmsg = mqtt::make_message(TOPIC, PAYLOAD);
        pubmsg->set_qos(QOS);
        client.publish(pubmsg)->wait_for(TIMEOUT);

        // Abone Ol
        std::cout << "Topic'e abone olunuyor: " << TOPIC << std::endl;
        client.subscribe(TOPIC, QOS)->wait();

        // 10 saniye dinle
        std::this_thread::sleep_for(std::chrono::seconds(10));

        // Bağlantıyı Kapat
        std::cout << "Bağlantı kapatılıyor..." << std::endl;
        client.disconnect()->wait();
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "MQTT Hatası: " << exc.what() << std::endl;
        return 1;
    }

    return 0;
}
