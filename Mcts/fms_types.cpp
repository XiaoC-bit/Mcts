#include "fms_types.hpp"

namespace fms {

std::string to_string(MaterialType type) {
    switch (type) {
        case MaterialType::WORKPIECE: return "WORKPIECE";
        case MaterialType::TOOL: return "TOOL";
        default: return "UNKNOWN";
    }
}

std::string to_string(LocationType type) {
    switch (type) {
        case LocationType::MACHINE_TABLE: return "MACHINE_TABLE";
        case LocationType::RACK_SLOT: return "RACK_SLOT";
        case LocationType::TOOL_SLOT: return "TOOL_SLOT";
        case LocationType::ROBOT_GRIPPER: return "ROBOT_GRIPPER";
        case LocationType::INPUT_PORT: return "INPUT_PORT";
        case LocationType::OUTPUT_PORT: return "OUTPUT_PORT";
        default: return "UNKNOWN";
    }
}

std::string to_string(RobotState state) {
    switch (state) {
        case RobotState::IDLE: return "IDLE";
        case RobotState::BUSY: return "BUSY";
        default: return "UNKNOWN";
    }
}

std::string to_string(MachineState state) {
    switch (state) {
        case MachineState::IDLE: return "IDLE";
        case MachineState::PROCESSING: return "PROCESSING";
        default: return "UNKNOWN";
    }
}

std::string to_string(ActionType type) {
    switch (type) {
        case ActionType::PICK: return "PICK";
        case ActionType::PLACE: return "PLACE";
        case ActionType::LOAD_MACHINE: return "LOAD_MACHINE";
        case ActionType::UNLOAD_MACHINE: return "UNLOAD_MACHINE";
        case ActionType::CHANGE_TOOL: return "CHANGE_TOOL";
        case ActionType::WAIT: return "WAIT";
        default: return "UNKNOWN";
    }
}

std::string to_string(EventType type) {
    switch (type) {
        case EventType::MACHINE_FINISHED: return "MACHINE_FINISHED";
        case EventType::ROBOT_FINISHED: return "ROBOT_FINISHED";
        case EventType::MATERIAL_ARRIVED: return "MATERIAL_ARRIVED";
        default: return "UNKNOWN";
    }
}

} // namespace fms