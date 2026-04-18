#ifndef CALL_MANAGER_HEART_BEAT_H
#define CALL_MANAGER_HEART_BEAT_H

#include "node.h"
#include <atomic>
#include <thread>
#include <functional>

class HeartBeat {
public:
    // callback: Primary çökünce ne yapılacağını dışarıdan alıyoruz
    HeartBeat(Node& backup_node, std::function<void()> on_failure);
    ~HeartBeat();

    void start();   // Heartbeat dinlemeye başla
    void stop();    // Durdur
    void pulse();   // Primary'den "Ben hayattayım" sinyali

private:
    void monitor(); // Arka planda çalışan kontrol döngüsü

    Node&                  backup_node_;
    std::function<void()>  on_failure_;  // Failover callback'i
    std::atomic<bool>      running_;     // Thread güvenli bool
    std::atomic<int>       miss_count_;  // Kaç sinyal kaçırıldı
    std::thread            monitor_thread_;

    static constexpr int MAX_MISS = 3;  // 3 sinyal kaçırılırsa failover
};
#endif //CALL_MANAGER_HEART_BEAT_H
