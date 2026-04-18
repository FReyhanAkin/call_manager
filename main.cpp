#include "node.h"
#include "heart_beat.h"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    std::cout << "=== Call Manager - HA Sistemi Başlatılıyor ===\n\n";

    // Primary ve Backup node'ları oluştur
    Node primary(NodeRole::PRIMARY, "Primary");
    Node backup(NodeRole::BACKUP,  "Backup");

    // Heartbeat: Primary çökerse backup.takeover() çağır
    HeartBeat hb(backup, [&backup]() {
        backup.takeover();
    });

    // Heartbeat dinlemeye başlasın
    hb.start();

    // Primary'de birkaç çağrı simüle et
    CallState c1 = {1, "+90 555 111 22 33", "+90 212 444 55 66",
                    CallStatus::ACTIVE, std::time(nullptr)};
    CallState c2 = {2, "+90 533 999 88 77", "+90 216 333 44 55",
                    CallStatus::RINGING, std::time(nullptr)};

    primary.add_call(c1);
    primary.add_call(c2);

    std::cout << "\n--- Primary aktif, sinyal gönderiliyor ---\n";

    // 3 saniye boyunca Primary hayatta, sinyal gönder
    for (int i = 0; i < 3; i++) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        hb.pulse();
    }

    // Primary çöktü — artık sinyal gönderilmiyor
    std::cout << "\n--- Primary çöktü! Sinyal kesildi ---\n";

    // Backup'ın failover yapmasını bekle
    std::this_thread::sleep_for(std::chrono::seconds(4));

    std::cout << "\n--- Son durum ---\n";
    primary.print_calls();
    backup.print_calls();

    return 0;
}