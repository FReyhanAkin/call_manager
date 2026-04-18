#include "heart_beat.h"
#include <iostream>
#include <chrono>

HeartBeat::HeartBeat(Node& backup_node, std::function<void()> on_failure)
        : backup_node_(backup_node)
        , on_failure_(on_failure)
        , running_(false)
        , miss_count_(0)
{
}

HeartBeat::~HeartBeat() {
    stop();
}

void HeartBeat::start() {
    running_ = true;
    miss_count_ = 0;
    monitor_thread_ = std::thread(&HeartBeat::monitor, this);
    std::cout << "[HeartBeat] Dinleme başladı.\n";
}

void HeartBeat::stop() {
    running_ = false;
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }
}

void HeartBeat::pulse() {
    miss_count_ = 0; // Primary hayatta, sayacı sıfırla
    std::cout << "[HeartBeat] Sinyal alındı.\n";
}

void HeartBeat::monitor() {
    while (running_) {
        // Her saniye bir kontrol et
        std::this_thread::sleep_for(std::chrono::seconds(1));

        miss_count_++;
        std::cout << "[HeartBeat] Sinyal bekleniyor... ("
                  << miss_count_ << "/" << MAX_MISS << ")\n";

        if (miss_count_ >= MAX_MISS) {
            std::cout << "[HeartBeat] Primary cevap vermiyor! Failover başlıyor...\n";
            on_failure_(); // Callback'i çağır
            running_ = false;
        }
    }
}