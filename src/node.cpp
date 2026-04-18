//
// Created by User on 18.04.2026.
//

#include "node.h"
#include "call_state.h"
#include <iostream>

Node::Node(NodeRole role, const std::string& name)
        : name_(name)
        , role_(role)
        , state_(NodeState::RUNNING)
{
}

void Node::add_call(const CallState& cs) {
    calls_[cs.call_id] = cs;
    std::cout << "[" << name_ << "] Çağrı eklendi: "
              << call_state_to_string(cs) << "\n";

    // Backup'a bildir
    if (sync_callback_) {
        sync_callback_(cs, true);
    }
}

void Node::remove_call(int call_id) {
    auto it = calls_.find(call_id);
    if (it != calls_.end()) {
        CallState cs = it->second;
        calls_.erase(it);
        std::cout << "[" << name_ << "] Çağrı silindi: #" << call_id << "\n";

        // Backup'a bildir
        if (sync_callback_) {
            sync_callback_(cs, false);
        }
    } else {
        std::cout << "[" << name_ << "] Çağrı bulunamadı: #" << call_id << "\n";
    }
}

void Node::print_calls() const {
    std::cout << "[" << name_ << "] Aktif çağrılar ("
              << calls_.size() << "):\n";
    for (const auto& [id, cs] : calls_) {
        std::cout << "  " << call_state_to_string(cs);
    }
}

NodeRole Node::get_role() const {
    return role_;
}

NodeState Node::get_state() const {
    return state_;
}

void Node::set_state(NodeState state) {
    state_ = state;
}

void Node::takeover() {
    role_  = NodeRole::PRIMARY;
    state_ = NodeState::TAKEOVER;
    std::cout << "[" << name_ << "] FAILOVER! Backup Primary rolünü devraldı.\n";
}

void Node::set_sync_callback(std::function<void(const CallState&, bool)> cb) {
    sync_callback_ = cb;
}

void Node::sync_all_to(Node& target) const {
    std::cout << "[" << name_ << "] Tüm state hedef node'a kopyalanıyor ("
              << calls_.size() << " çağrı)...\n";

    // Hedefin callback'ini geçici olarak devre dışı bırak
    target.set_sync_callback(nullptr);

    for (const auto& [id, cs] : calls_) {
        target.add_call(cs);
    }
}