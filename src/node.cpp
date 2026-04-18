//
// Created by User on 18.04.2026.
//

#include "node.h"
#include "call_state.h"
#include <iostream>

Node::Node(NodeRole role, const std::string& name)
        : role_(role)
        , name_(name)
        , state_(NodeState::RUNNING)
{
}

void Node::add_call(const CallState& cs) {
    calls_[cs.call_id] = cs;
    std::cout << "[" << name_ << "] Çağrı eklendi: "
              << call_state_to_string(cs) << "\n";
}

void Node::remove_call(int call_id) {
    if (calls_.erase(call_id)) {
        std::cout << "[" << name_ << "] Çağrı silindi: #" << call_id << "\n";
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