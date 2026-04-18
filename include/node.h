#ifndef CALL_MANAGER_NODE_H
#define CALL_MANAGER_NODE_H

#include "call_state.h"
#include <unordered_map>
#include <string>
#include <functional>

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

    // State sync için callback
    // Primary'de değişiklik olunca Backup'a haber ver
    void set_sync_callback(std::function<void(const CallState&, bool)> cb);

    // Tüm call state'i hedef node'a kopyalar
    void sync_all_to(Node& target) const;

private:
    std::string name_;
    NodeRole    role_;
    NodeState   state_;

    // Aktif çağrıların tutulduğu tablo
    // key: call_id, value: CallState
    std::unordered_map<int, CallState> calls_;
    std::function<void(const CallState&, bool)> sync_callback_;
};


#endif //CALL_MANAGER_NODE_H
