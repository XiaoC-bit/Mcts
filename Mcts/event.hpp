#pragma once
#include "fms_types.hpp"
#include <queue>
#include <memory>

namespace fms {

struct Event {
    TimeType time;
    EventType type;
    IdType source_id;
    IdType target_id;
    IdType material_id;
    std::string details;
    
    Event() 
        : time(INVALID_TIME), type(EventType::MACHINE_FINISHED),
          source_id(INVALID_ID), target_id(INVALID_ID), 
          material_id(INVALID_ID) {}
    
    Event(TimeType t, EventType et, IdType src, IdType tgt = INVALID_ID, 
          IdType mat = INVALID_ID, const std::string& det = "")
        : time(t), type(et), source_id(src), target_id(tgt), 
          material_id(mat), details(det) {}
    
    bool operator<(const Event& other) const {
        return time > other.time;
    }
};

class EventQueue {
public:
    void push(const Event& event) {
        queue.push(event);
    }
    
    void push(TimeType time, EventType type, IdType source_id, 
              IdType target_id = INVALID_ID, IdType material_id = INVALID_ID,
              const std::string& details = "") {
        queue.emplace(time, type, source_id, target_id, material_id, details);
    }
    
    bool empty() const {
        return queue.empty();
    }
    
    size_t size() const {
        return queue.size();
    }
    
    const Event& top() const {
        return queue.top();
    }
    
    Event pop() {
        Event event = queue.top();
        queue.pop();
        return event;
    }
    
    TimeType get_next_time() const {
        if (empty()) {
            return INVALID_TIME;
        }
        return queue.top().time;
    }
    
    void clear() {
        while (!queue.empty()) {
            queue.pop();
        }
    }
    
    bool has_events_before(TimeType time) const {
        if (empty()) {
            return false;
        }
        return queue.top().time <= time;
    }
    
private:
    std::priority_queue<Event> queue;
};

} // namespace fms