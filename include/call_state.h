#ifndef CALL_MANAGER_CALL_STATE_H
#define CALL_MANAGER_CALL_STATE_H

#include <string>
#include <ctime>

// Bir çağrının olası durumları
enum class CallStatus {
    RINGING,    // Çalıyor
    ACTIVE,     // Görüşme devam ediyor
    ON_HOLD,    // Beklemeye alındı
    ENDED       // Bitti
};

// Bir çağrıyı temsil eden veri yapısı
struct CallState {
    int         call_id;
    std::string caller;     // Arayan numara
    std::string callee;     // Aranan numara
    CallStatus  status;
    std::time_t start_time; // Çağrı başlangıç zamanı
};

std::string call_state_to_string(const CallState& cs);

#endif //CALL_MANAGER_CALL_STATE_H
