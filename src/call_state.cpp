//
// Created by User on 18.04.2026.
//

#include "call_state.h"
#include <sstream>

// CallState'i okunabilir string'e çevirir
// Loglama ve debug için kullanacağız
std::string call_state_to_string(const CallState& cs) {
    std::ostringstream oss;

    // Status'u string'e çevir
    std::string status_str;
    switch (cs.status) {
        case CallStatus::RINGING:  status_str = "RINGING";  break;
        case CallStatus::ACTIVE:   status_str = "ACTIVE";   break;
        case CallStatus::ON_HOLD:  status_str = "ON_HOLD";  break;
        case CallStatus::ENDED:    status_str = "ENDED";    break;
    }

    oss << "[Call #" << cs.call_id << "] "
        << cs.caller << " -> " << cs.callee
        << " | " << status_str
        << " | started: " << std::ctime(&cs.start_time);

    return oss.str();
}
