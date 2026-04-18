//
// Created by User on 18.04.2026.
//

#ifndef CALL_MANAGER_NODE_H
#define CALL_MANAGER_NODE_H

#pragma once

#include "call_state.h"
#include <unordered_map>
#include <string>

// Node'un rolü
enum class NodeRole {
    PRIMARY,
    BACKUP
};

// Node'un durumu
enum class NodeState {
    RUNNING,    // Normal çalışıyor
    FAILED,     // Çöktü
    TAKEOVER    // Backup, Primary'nin görevini devraldı
};

class Node {
public:
    Node(NodeRole role, const std::string& name);

    // Çağrı yönetimi
    void add_call(const CallState& cs);
    void remove_call(int call_id);
    void print_calls() const;

    // Node bilgisi
    NodeRole  get_role()  const;
    NodeState get_state() const;
    void      set_state(NodeState state);

    // Failover
    void takeover(); // Backup → Primary rolünü üstlenir

private:
    std::string name_;
    NodeRole    role_;
    NodeState   state_;

    // Aktif çağrıların tutulduğu tablo
    // key: call_id, value: CallState
    std::unordered_map<int, CallState> calls_;
};


#endif //CALL_MANAGER_NODE_H
