#pragma once
#include "fms_types.hpp"
#include "location.hpp"
#include <string>
#include <vector>

namespace fms {

struct Gripper {
    IdType id;
    IdType robot_id;
    IdType material_id;
    CapacityType capacity;
    double max_weight;
    double current_weight;
    
    Gripper() 
        : id(INVALID_ID), robot_id(INVALID_ID), material_id(INVALID_ID),
          capacity(1), max_weight(5.0), current_weight(0.0) {}
    
    Gripper(IdType grip_id, IdType rob_id, CapacityType cap = 1, double max_w = 5.0)
        : id(grip_id), robot_id(rob_id), material_id(INVALID_ID),
          capacity(cap), max_weight(max_w), current_weight(0.0) {}
    
    bool is_empty() const {
        return material_id == INVALID_ID;
    }
    
    bool can_grasp(double weight) const {
        return is_empty() && (current_weight + weight) <= max_weight;
    }
    
    void grasp(IdType mat_id, double weight) {
        material_id = mat_id;
        current_weight += weight;
    }
    
    IdType release() {
        IdType mat = material_id;
        material_id = INVALID_ID;
        current_weight = 0.0;
        return mat;
    }
};

struct Robot {
    IdType id;
    std::string name;
    Position position;
    RobotState state;
    TimeType busy_until;
    bool has_seventh_axis;
    std::vector<Gripper> grippers;
    double total_load_capacity;
    double current_load;
    
    Robot() 
        : id(INVALID_ID), state(RobotState::IDLE), busy_until(0.0),
          has_seventh_axis(false), total_load_capacity(10.0), current_load(0.0) {}
    
    Robot(IdType rob_id, const std::string& rob_name, const Position& pos,
          size_t num_grippers = 1, bool seventh_axis = false)
        : id(rob_id), name(rob_name), position(pos), 
          state(RobotState::IDLE), busy_until(0.0),
          has_seventh_axis(seventh_axis), 
          total_load_capacity(10.0 * num_grippers), current_load(0.0) {
        for (size_t i = 0; i < num_grippers; ++i) {
            grippers.emplace_back(static_cast<IdType>(rob_id * 100 + i + 1), rob_id);
        }
    }
    
    bool is_idle(TimeType current_time) const {
        return state == RobotState::IDLE && busy_until <= current_time;
    }
    
    bool is_busy(TimeType current_time) const {
        return !is_idle(current_time);
    }
    
    void set_busy(TimeType duration, TimeType current_time) {
        state = RobotState::BUSY;
        busy_until = current_time + duration;
    }
    
    void set_idle() {
        state = RobotState::IDLE;
        busy_until = 0.0;
    }
    
    size_t get_num_grippers() const {
        return grippers.size();
    }
    
    Gripper* get_gripper(IdType gripper_id) {
        for (auto& grip : grippers) {
            if (grip.id == gripper_id) {
                return &grip;
            }
        }
        return nullptr;
    }
    
    const Gripper* get_gripper(IdType gripper_id) const {
        for (const auto& grip : grippers) {
            if (grip.id == gripper_id) {
                return &grip;
            }
        }
        return nullptr;
    }
    
    IdType get_empty_gripper_id() const {
        for (const auto& grip : grippers) {
            if (grip.is_empty()) {
                return grip.id;
            }
        }
        return INVALID_ID;
    }
    
    bool has_empty_gripper() const {
        return get_empty_gripper_id() != INVALID_ID;
    }
};

} // namespace fms