#pragma once
#include "fms_types.hpp"
#include "location.hpp"
#include "material.hpp"
#include <string>
#include <vector>

namespace fms {

struct PortSlot {
    IdType id;
    IdType port_id;
    IdType material_id;
    bool is_occupied;
    
    PortSlot() 
        : id(INVALID_ID), port_id(INVALID_ID), material_id(INVALID_ID), 
          is_occupied(false) {}
    
    PortSlot(IdType slot_id, IdType p_id)
        : id(slot_id), port_id(p_id), material_id(INVALID_ID), 
          is_occupied(false) {}
    
    bool is_empty() const {
        return !is_occupied && material_id == INVALID_ID;
    }
    
    void place_material(IdType mat_id) {
        material_id = mat_id;
        is_occupied = true;
    }
    
    IdType remove_material() {
        IdType mat = material_id;
        material_id = INVALID_ID;
        is_occupied = false;
        return mat;
    }
};

struct Port {
    IdType id;
    std::string name;
    Position position;
    LocationType type;
    CapacityType capacity;
    std::vector<PortSlot> slots;
    
    Port() 
        : id(INVALID_ID), type(LocationType::INPUT_PORT), capacity(1) {}
    
    Port(IdType p_id, const std::string& p_name, const Position& pos,
         LocationType port_type, CapacityType cap = 5)
        : id(p_id), name(p_name), position(pos), 
          type(port_type), capacity(cap) {
        for (CapacityType i = 0; i < capacity; ++i) {
            slots.emplace_back(
                static_cast<IdType>(p_id * id_constants::PORT_SLOT_OFFSET + i + 1), 
                p_id
            );
        }
    }
    
    bool is_input_port() const {
        return type == LocationType::INPUT_PORT;
    }
    
    bool is_output_port() const {
        return type == LocationType::OUTPUT_PORT;
    }
    
    size_t occupied_slots() const {
        size_t count = 0;
        for (const auto& slot : slots) {
            if (slot.is_occupied) {
                count++;
            }
        }
        return count;
    }
    
    size_t empty_slots() const {
        return capacity - occupied_slots();
    }
    
    bool has_empty_slot() const {
        return empty_slots() > 0;
    }
    
    bool is_full() const {
        return occupied_slots() >= capacity;
    }
    
    bool is_empty() const {
        return occupied_slots() == 0;
    }
    
    PortSlot* get_slot(IdType slot_id) {
        for (auto& slot : slots) {
            if (slot.id == slot_id) {
                return &slot;
            }
        }
        return nullptr;
    }
    
    const PortSlot* get_slot(IdType slot_id) const {
        for (const auto& slot : slots) {
            if (slot.id == slot_id) {
                return &slot;
            }
        }
        return nullptr;
    }
    
    IdType get_empty_slot_id() const {
        for (const auto& slot : slots) {
            if (slot.is_empty()) {
                return slot.id;
            }
        }
        return INVALID_ID;
    }
    
    IdType get_occupied_slot_id() const {
        for (const auto& slot : slots) {
            if (slot.is_occupied) {
                return slot.id;
            }
        }
        return INVALID_ID;
    }
    
    void expand(CapacityType new_capacity) {
        if (new_capacity > capacity) {
            slots.reserve(new_capacity);
            for (CapacityType i = capacity; i < new_capacity; ++i) {
                slots.emplace_back(
                    static_cast<IdType>(id * 100 + i + 1), 
                    id
                );
            }
            capacity = new_capacity;
        }
    }
};

} // namespace fms