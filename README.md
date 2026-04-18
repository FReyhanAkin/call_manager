# call_manager

A High Availability (HA) call state manager written in C++17, simulating real-world cloud telephony infrastructure concepts including Primary/Backup node architecture, heartbeat monitoring, state synchronization, and automatic failover.

---

## Motivation

In production VoIP and cloud telephony systems, active call state must survive server failures. If the node managing call sessions crashes, all active calls drop — unacceptable in carrier-grade systems. This project demonstrates the core HA patterns used to prevent that: real-time state replication between nodes and automatic failover when the primary becomes unresponsive.

---

## Architecture

```
┌─────────────────┐        heartbeat (1s interval)       ┌─────────────────┐
│   PRIMARY NODE  │ ──────────────────────────────────►  │   BACKUP NODE   │
│                 │ ◄────────────────────────────────── │                 │
│  - manages      │                                       │  - mirrors      │
│    active calls │ ──── state sync on every change ───► │    call state   │
│  - sends pulse  │                                       │  - monitors     │
└─────────────────┘                                       │    heartbeat    │
         │                                                └─────────────────┘
    [Primary crashes]                                              │
         │                                                   [Failover]
         │                                                         │
         │                                                         ▼
         │                                            ┌─────────────────────┐
         │                                            │  BACKUP (new Primary)│
         │                                            │  - full call state  │
         │                                            │  - takes over       │
         │                                            └─────────────────────┘
         │                                                         │
    [Primary recovers]                                    [Re-sync on recovery]
         │                                                         │
         └──────────────── re-sync (full state copy) ◄────────────┘
```

### Key Concepts

**Heartbeat:** Primary sends a pulse signal every second. If Backup misses 3 consecutive pulses (3 seconds), it concludes the Primary has failed and triggers failover.

**State Sync:** Every time a call is added or removed on Primary, the change is immediately propagated to Backup via a sync callback. Backup always holds a current mirror of Primary's call table.

**Failover:** When the heartbeat timeout is reached, Backup promotes itself to Primary role. Because it already has the full call state, no calls are lost.

**Recovery Re-sync:** When the old Primary comes back online, the new Primary (former Backup) copies its full call state back to it via `sync_all_to()`. During this operation the sync callback is temporarily disabled to prevent a feedback loop where the recovered Primary would re-propagate the same calls back to the new Primary.

**Tradeoff — Why 3 seconds?**
- Too short (e.g. 1s): network jitter causes false positives → unnecessary failovers
- Too long (e.g. 10s): real failures are detected too late → SLA violations
- 3 seconds is a pragmatic middle ground balancing false positive rate vs. detection latency

---

## Project Structure

```
call_manager/
├── CMakeLists.txt
├── include/
│   ├── call_state.h      # CallState struct and CallStatus enum
│   ├── node.h            # Node class (Primary / Backup)
│   └── heart_beat.h      # HeartBeat monitor class
└── src/
    ├── main.cpp          # Demo: simulates Primary failure, Backup takeover, and recovery re-sync
    ├── call_state.cpp    # call_state_to_string() helper
    ├── node.cpp          # Node method implementations
    └── heart_beat.cpp    # Heartbeat thread, pulse, and failover logic
```

---

## Key Implementation Details

### `CallState` — Data Model

```cpp
struct CallState {
    int         call_id;
    std::string caller;
    std::string callee;
    CallStatus  status;    // RINGING | ACTIVE | ON_HOLD | ENDED
    std::time_t start_time;
};
```

Calls are stored in `std::unordered_map<int, CallState>` for O(1) lookup by call ID — important when managing thousands of concurrent sessions.

### `Node` — Call State Manager

Each node (Primary or Backup) maintains its own call table. When a sync callback is registered, every mutation (`add_call`, `remove_call`) is automatically propagated:

```cpp
primary.set_sync_callback([&backup](const CallState& cs, bool is_add) {
    if (is_add) backup.add_call(cs);
    else        backup.remove_call(cs.call_id);
});
```

### `HeartBeat` — Monitor Thread

Runs a dedicated background thread (`std::thread`) that increments a miss counter every second. The counter is reset to zero whenever `pulse()` is called. If the counter reaches `MAX_MISS` (3), the `on_failure` callback fires.

```cpp
HeartBeat hb(backup, [&backup]() {
    backup.takeover();
});
hb.start();
```

`std::atomic<bool>` and `std::atomic<int>` are used for `running_` and `miss_count_` to ensure thread-safe access without explicit mutex locking.

### `sync_all_to()` — Recovery Re-sync

When the old Primary recovers, the new Primary pushes its entire call table to it. The sync callback on the target is disabled during the operation to avoid a feedback loop:

```cpp
void Node::sync_all_to(Node& target) const {
    target.set_sync_callback(nullptr); // disable feedback loop
    for (const auto& [id, cs] : calls_) {
        target.add_call(cs);
    }
}
```

Usage in recovery scenario:

```cpp
primary.set_state(NodeState::RUNNING);
backup.sync_all_to(primary); // new Primary pushes full state to recovered node
```

### Callback Pattern

`HeartBeat` accepts an `std::function<void()>` as its failure handler. This decouples the monitoring logic from the failover action — the caller decides what happens on failure, not the monitor.

---

## Building

### Requirements

- Linux or WSL2 (Ubuntu 20.04+)
- CMake 3.16+
- g++ with C++17 support
- POSIX threads (included with g++)

### Install dependencies (Ubuntu / WSL2)

```bash
sudo apt update && sudo apt install -y cmake g++ gdb
```

### Build

```bash
mkdir cmake-build-debug && cd cmake-build-debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

### Run

```bash
./call_manager
```

---

## Sample Output

```
=== Call Manager - HA Sistemi Başlatılıyor ===

[HeartBeat] Dinleme başladı.
[Primary] Çağrı eklendi: [Call #1] +90 555 111 22 33 -> +90 212 444 55 66 | ACTIVE
[Backup]  Çağrı eklendi: [Call #1] +90 555 111 22 33 -> +90 212 444 55 66 | ACTIVE
[Primary] Çağrı eklendi: [Call #2] +90 533 999 88 77 -> +90 216 333 44 55 | RINGING
[Backup]  Çağrı eklendi: [Call #2] +90 533 999 88 77 -> +90 216 333 44 55 | RINGING

--- Primary aktif, sinyal gönderiliyor ---
[HeartBeat] Sinyal bekleniyor... (1/3)
[HeartBeat] Sinyal alındı.
[HeartBeat] Sinyal alındı.
[HeartBeat] Sinyal alındı.

--- Primary çöktü! Sinyal kesildi ---
[HeartBeat] Sinyal bekleniyor... (1/3)
[HeartBeat] Sinyal bekleniyor... (2/3)
[HeartBeat] Sinyal bekleniyor... (3/3)
[HeartBeat] Primary cevap vermiyor! Failover başlıyor...
[Backup] FAILOVER! Backup Primary rolünü devraldı.

--- Son durum ---
[Primary] Aktif çağrılar (2): [Call #1] ... [Call #2] ...
[Backup]  Aktif çağrılar (2): [Call #1] ... [Call #2] ...

--- Eski Primary geri döndü! Re-sync başlıyor ---
[Backup] Tüm state hedef node'a kopyalanıyor (2 çağrı)...
[Primary] Çağrı eklendi: [Call #2] ...
[Primary] Çağrı eklendi: [Call #1] ...

--- Re-sync sonrası durum ---
[Primary] Aktif çağrılar (2): [Call #1] ... [Call #2] ...
[Backup]  Aktif çağrılar (2): [Call #1] ... [Call #2] ...
```

---

## Concepts Demonstrated

| Concept | Implementation |
|---|---|
| High Availability | Primary/Backup node architecture |
| Heartbeat & failure detection | Background thread with configurable timeout |
| Real-time state replication | Sync callback on every call mutation |
| Recovery re-sync | `sync_all_to()` with feedback loop prevention |
| Thread safety | `std::atomic` for shared variables |
| Callback / observer pattern | `std::function` for decoupled failover logic |
| Efficient call lookup | `std::unordered_map` for O(1) access |
| POSIX threading | `std::thread`, `std::this_thread::sleep_for` |
| Modern C++17 | Structured bindings, `std::atomic`, `std::function` |

---

## Potential Improvements

- **Network layer:** Replace in-process callbacks with actual TCP/Unix socket communication between nodes running as separate processes
- **Persistence:** Write call state to disk or Redis for durability across full system restarts
- **SIP/RTP integration:** Replace simulated calls with actual SIP session tracking
- **Metrics:** Expose failover count, sync latency, and missed heartbeat rate
- **Role negotiation:** After recovery, nodes decide Primary/Backup roles dynamically instead of hardcoded assignment

---

## Why This Matters for VoIP

Real telephony platforms (Asterisk, Kamailio, FreeSWITCH) implement these same patterns at production scale. This project demonstrates the foundational principles: state replication, failure detection with tunable sensitivity, and zero-downtime failover — all core requirements for carrier-grade systems.