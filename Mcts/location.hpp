#pragma once
#include "fms_types.hpp"
#include <string>

namespace fms {

struct Location {
    IdType id;
    LocationType type;
    std::string name;
    Position position;
    IdType parent_id;
    IdType material_id;
    CapacityType capacity;
    CapacityType current_load;
    
    Location() 
        : id(INVALID_ID), type(LocationType::RACK_SLOT), parent_id(INVALID_ID),
          material_id(INVALID_ID), capacity(1), current_load(0) {}
    
    Location(IdType loc_id, LocationType loc_type, const std::string& loc_name,
             const Position& pos, IdType parent = INVALID_ID, 
             CapacityType cap = 1)
        : id(loc_id), type(loc_type), name(loc_name), position(pos),
          parent_id(parent), material_id(INVALID_ID), capacity(cap), current_load(0) {}
    
    bool is_empty() const {
        return material_id == INVALID_ID && current_load == 0;
    }
    
    bool can_receive() const {
        return current_load < capacity;
    }
    
    bool has_material() const {
        return material_id != INVALID_ID;
    }
    
    void place_material(IdType mat_id) {
        material_id = mat_id;
        current_load = std::min(current_load + 1, capacity);
    }
    
    IdType remove_material() {
        IdType mat = material_id;
        material_id = INVALID_ID;
        current_load = std::max(current_load - 1, static_cast<CapacityType>(0));
        return mat;
    }
    
    bool operator==(const Location& other) const {
        return id == other.id;
    }
};

struct LocationKey {
    LocationType type;
    IdType parent_id;
    IdType index;
    
    LocationKey(LocationType t, IdType p, IdType i) 
        : type(t), parent_id(p), index(i) {}
    
    bool operator<(const LocationKey& other) const {
        if (type != other.type) return type < other.type;
        if (parent_id != other.parent_id) return parent_id < other.parent_id;
        return index < other.index;
    }
};

} // namespace fms