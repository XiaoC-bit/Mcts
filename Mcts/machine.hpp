#pragma once
#include "fms_types.hpp"
#include "location.hpp"
#include <string>
#include <vector>

namespace fms {

struct MachineTable {
    IdType id;
    IdType machine_id;
    std::string name;
    MachineState state;
    IdType workpiece_id;
    TimeType finish_time;
    std::string current_operation;
    
    MachineTable() 
        : id(INVALID_ID), machine_id(INVALID_ID), state(MachineState::IDLE),
          workpiece_id(INVALID_ID), finish_time(INVALID_TIME) {}
    
    MachineTable(IdType table_id, IdType mach_id, const std::string& table_name)
        : id(table_id), machine_id(mach_id), name(table_name), 
          state(MachineState::IDLE), workpiece_id(INVALID_ID), 
          finish_time(INVALID_TIME) {}
    
    bool is_idle() const {
        return state == MachineState::IDLE;
    }
    
    bool is_processing() const {
        return state == MachineState::PROCESSING;
    }
    
    void start_processing(IdType wp_id, TimeType process_time, TimeType current_time) {
        workpiece_id = wp_id;
        state = MachineState::PROCESSING;
        finish_time = current_time + process_time;
    }
    
    void finish_processing() {
        state = MachineState::IDLE;
        workpiece_id = INVALID_ID;
        finish_time = INVALID_TIME;
        current_operation.clear();
    }
    
    bool is_finished(TimeType current_time) const {
        return is_processing() && finish_time <= current_time;
    }
};

struct ToolSlot {
    IdType id;
    IdType machine_id;
    IdType tool_id;
    std::string tool_type;
    bool is_reserved;
    
    ToolSlot() 
        : id(INVALID_ID), machine_id(INVALID_ID), tool_id(INVALID_ID), 
          is_reserved(false) {}
    
    ToolSlot(IdType slot_id, IdType mach_id)
        : id(slot_id), machine_id(mach_id), tool_id(INVALID_ID), 
          is_reserved(false) {}
    
    bool is_empty() const {
        return tool_id == INVALID_ID;
    }
    
    void load_tool(IdType tl_id, const std::string& type) {
        tool_id = tl_id;
        tool_type = type;
    }
    
    IdType unload_tool() {
        IdType tl = tool_id;
        tool_id = INVALID_ID;
        tool_type.clear();
        return tl;
    }
};

struct Machine {
    IdType id;
    std::string name;
    Position position;
    std::vector<MachineTable> tables;
    std::vector<ToolSlot> tool_slots;
    std::string machine_type;
    
    Machine() : id(INVALID_ID) {}
    
    Machine(IdType mach_id, const std::string& mach_name, const Position& pos,
            size_t num_tables = 1, size_t num_tool_slots = 10)
        : id(mach_id), name(mach_name), position(pos) {
        for (size_t i = 0; i < num_tables; ++i) {
            tables.emplace_back(
                static_cast<IdType>(mach_id * id_constants::MACHINE_TABLE_OFFSET + i + 1), 
                mach_id, 
                "Table_" + std::to_string(i + 1)
            );
        }
        for (size_t i = 0; i < num_tool_slots; ++i) {
            tool_slots.emplace_back(
                static_cast<IdType>(mach_id * id_constants::MACHINE_TOOL_SLOT_OFFSET + i + 1), 
                mach_id
            );
        }
    }
    
    size_t get_num_tables() const {
        return tables.size();
    }
    
    size_t get_num_tool_slots() const {
        return tool_slots.size();
    }
    
    MachineTable* get_table(IdType table_id) {
        for (auto& table : tables) {
            if (table.id == table_id) {
                return &table;
            }
        }
        return nullptr;
    }
    
    const MachineTable* get_table(IdType table_id) const {
        for (const auto& table : tables) {
            if (table.id == table_id) {
                return &table;
            }
        }
        return nullptr;
    }
    
    IdType get_idle_table_id() const {
        for (const auto& table : tables) {
            if (table.is_idle()) {
                return table.id;
            }
        }
        return INVALID_ID;
    }
    
    bool has_idle_table() const {
        return get_idle_table_id() != INVALID_ID;
    }
    
    ToolSlot* get_tool_slot(IdType slot_id) {
        for (auto& slot : tool_slots) {
            if (slot.id == slot_id) {
                return &slot;
            }
        }
        return nullptr;
    }
    
    const ToolSlot* get_tool_slot(IdType slot_id) const {
        for (const auto& slot : tool_slots) {
            if (slot.id == slot_id) {
                return &slot;
            }
        }
        return nullptr;
    }
    
    IdType get_empty_tool_slot_id() const {
        for (const auto& slot : tool_slots) {
            if (slot.is_empty()) {
                return slot.id;
            }
        }
        return INVALID_ID;
    }
    
    bool has_empty_tool_slot() const {
        return get_empty_tool_slot_id() != INVALID_ID;
    }
    
    bool is_fully_busy(TimeType current_time) const {
        for (const auto& table : tables) {
            if (table.is_idle() || table.is_finished(current_time)) {
                return false;
            }
        }
        return true;
    }
};

} // namespace fms