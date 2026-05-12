#pragma once
#include "fms_types.hpp"
#include <string>

namespace fms {

struct Action {
    ActionType type;
    IdType robot_id;
    IdType source_location;
    IdType target_location;
    IdType material_id;
    IdType machine_id;
    IdType gripper_id;
    TimeType duration;
    std::string parameters;
    
    Action() 
        : type(ActionType::WAIT), robot_id(INVALID_ID), 
          source_location(INVALID_ID), target_location(INVALID_ID),
          material_id(INVALID_ID), machine_id(INVALID_ID),
          gripper_id(INVALID_ID), duration(0.0) {}
    
    Action(ActionType t, IdType robot, IdType src_loc = INVALID_ID, 
           IdType tgt_loc = INVALID_ID, IdType mat = INVALID_ID,
           IdType mach = INVALID_ID, IdType grip = INVALID_ID,
           TimeType dur = 0.0, const std::string& params = "")
        : type(t), robot_id(robot), source_location(src_loc), 
          target_location(tgt_loc), material_id(mat), machine_id(mach),
          gripper_id(grip), duration(dur), parameters(params) {}
    
    bool is_valid() const {
        if (robot_id == INVALID_ID) return false;
        
        switch (type) {
            case ActionType::PICK:
                return source_location != INVALID_ID;
            case ActionType::PLACE:
                return target_location != INVALID_ID && material_id != INVALID_ID;
            case ActionType::LOAD_MACHINE:
                return machine_id != INVALID_ID && material_id != INVALID_ID;
            case ActionType::UNLOAD_MACHINE:
                return machine_id != INVALID_ID;
            case ActionType::CHANGE_TOOL:
                return machine_id != INVALID_ID && material_id != INVALID_ID;
            case ActionType::WAIT:
                return duration >= 0.0;
            default:
                return false;
        }
    }
    
    bool operator==(const Action& other) const {
        return type == other.type &&
               robot_id == other.robot_id &&
               source_location == other.source_location &&
               target_location == other.target_location &&
               material_id == other.material_id &&
               machine_id == other.machine_id &&
               gripper_id == other.gripper_id;
    }
};

static inline Action make_pick_action(IdType robot_id, IdType source_loc, IdType gripper_id = INVALID_ID) {
    return Action(ActionType::PICK, robot_id, source_loc, INVALID_ID, INVALID_ID, 
                  INVALID_ID, gripper_id, 1.0);
}

static inline Action make_place_action(IdType robot_id, IdType target_loc, IdType material_id, IdType gripper_id = INVALID_ID) {
    return Action(ActionType::PLACE, robot_id, INVALID_ID, target_loc, material_id, 
                  INVALID_ID, gripper_id, 1.0);
}

static inline Action make_load_machine_action(IdType robot_id, IdType machine_id, IdType material_id) {
    return Action(ActionType::LOAD_MACHINE, robot_id, INVALID_ID, INVALID_ID, material_id, 
                  machine_id, INVALID_ID, 2.0);
}

static inline Action make_unload_machine_action(IdType robot_id, IdType machine_id) {
    return Action(ActionType::UNLOAD_MACHINE, robot_id, INVALID_ID, INVALID_ID, INVALID_ID, 
                  machine_id, INVALID_ID, 2.0);
}

static inline Action make_change_tool_action(IdType robot_id, IdType machine_id, IdType tool_id) {
    return Action(ActionType::CHANGE_TOOL, robot_id, INVALID_ID, INVALID_ID, tool_id, 
                  machine_id, INVALID_ID, 3.0);
}

static inline Action make_wait_action(IdType robot_id, TimeType duration) {
    return Action(ActionType::WAIT, robot_id, INVALID_ID, INVALID_ID, INVALID_ID, 
                  INVALID_ID, INVALID_ID, duration);
}

} // namespace fms