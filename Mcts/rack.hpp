#pragma once
#include "fms_types.hpp"
#include "location.hpp"
#include <string>
#include <vector>
#include <algorithm>

namespace fms {

struct RackSlot {
    IdType id;
    IdType rack_id;
    size_t row;
    size_t col;
    IdType material_id;
    bool is_reserved;
    
    RackSlot() 
        : id(INVALID_ID), rack_id(INVALID_ID), row(0), col(0), 
          material_id(INVALID_ID), is_reserved(false) {}
    
    RackSlot(IdType slot_id, IdType r_id, size_t r, size_t c)
        : id(slot_id), rack_id(r_id), row(r), col(c), 
          material_id(INVALID_ID), is_reserved(false) {}
    
    bool is_empty() const {
        return material_id == INVALID_ID && !is_reserved;
    }
    
    void place_material(IdType mat_id) {
        material_id = mat_id;
    }
    
    IdType remove_material() {
        IdType mat = material_id;
        material_id = INVALID_ID;
        return mat;
    }
};

struct Rack {
    IdType id;
    std::string name;
    Position position;
    size_t rows;
    size_t cols;
    std::vector<std::vector<RackSlot>> slots;
    
    Rack() : id(INVALID_ID), rows(0), cols(0) {}
    
    Rack(IdType r_id, const std::string& r_name, const Position& pos,
         size_t num_rows = 5, size_t num_cols = 5)
        : id(r_id), name(r_name), position(pos), 
          rows(num_rows), cols(num_cols) {
        slots.resize(rows, std::vector<RackSlot>(cols));
        for (size_t r = 0; r < rows; ++r) {
            for (size_t c = 0; c < cols; ++c) {
                slots[r][c] = RackSlot(
                    static_cast<IdType>(r_id * id_constants::RACK_SLOT_OFFSET + r * cols + c + 1),
                    r_id, r, c
                );
            }
        }
    }
    
    size_t total_slots() const {
        return rows * cols;
    }
    
    size_t empty_slots() const {
        size_t count = 0;
        for (const auto& row : slots) {
            for (const auto& slot : row) {
                if (slot.is_empty()) {
                    count++;
                }
            }
        }
        return count;
    }
    
    bool has_empty_slot() const {
        return empty_slots() > 0;
    }
    
    RackSlot* get_slot(size_t row, size_t col) {
        if (row < rows && col < cols) {
            return &slots[row][col];
        }
        return nullptr;
    }
    
    const RackSlot* get_slot(size_t row, size_t col) const {
        if (row < rows && col < cols) {
            return &slots[row][col];
        }
        return nullptr;
    }
    
    RackSlot* get_slot_by_id(IdType slot_id) {
        for (auto& row : slots) {
            for (auto& slot : row) {
                if (slot.id == slot_id) {
                    return &slot;
                }
            }
        }
        return nullptr;
    }
    
    const RackSlot* get_slot_by_id(IdType slot_id) const {
        for (const auto& row : slots) {
            for (const auto& slot : row) {
                if (slot.id == slot_id) {
                    return &slot;
                }
            }
        }
        return nullptr;
    }
    
    IdType get_empty_slot_id() const {
        for (const auto& row : slots) {
            for (const auto& slot : row) {
                if (slot.is_empty()) {
                    return slot.id;
                }
            }
        }
        return INVALID_ID;
    }
    
    void expand(size_t new_rows, size_t new_cols) {
        if (new_rows > rows) {
            slots.resize(new_rows);
            for (size_t r = rows; r < new_rows; ++r) {
                slots[r].resize(cols);
                for (size_t c = 0; c < cols; ++c) {
                    slots[r][c] = RackSlot(
                        static_cast<IdType>(id * 1000 + r * cols + c + 1),
                        id, r, c
                    );
                }
            }
            rows = new_rows;
        }
        if (new_cols > cols) {
            for (size_t r = 0; r < rows; ++r) {
                slots[r].resize(new_cols);
                for (size_t c = cols; c < new_cols; ++c) {
                    slots[r][c] = RackSlot(
                        static_cast<IdType>(id * 1000 + r * new_cols + c + 1),
                        id, r, c
                    );
                }
            }
            cols = new_cols;
        }
    }
};

struct ToolRack : public Rack {
    std::vector<std::string> tool_categories;
    
    ToolRack() : Rack() {}
    
    ToolRack(IdType r_id, const std::string& r_name, const Position& pos,
             size_t num_rows = 3, size_t num_cols = 5)
        : Rack(r_id, r_name, pos, num_rows, num_cols) {}
    
    void add_tool_category(const std::string& category) {
        if (std::find(tool_categories.begin(), tool_categories.end(), category) 
            == tool_categories.end()) {
            tool_categories.push_back(category);
        }
    }
    
    bool has_tool_category(const std::string& category) const {
        return std::find(tool_categories.begin(), tool_categories.end(), category) 
            != tool_categories.end();
    }
};

} // namespace fms