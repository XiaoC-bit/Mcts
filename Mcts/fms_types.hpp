#pragma once
#include <cstdint>
#include <string>

namespace fms {

using IdType = uint32_t;
using TimeType = double;
using CapacityType = uint32_t;

enum class MaterialType {
    WORKPIECE,
    TOOL
};

enum class LocationType {
    MACHINE_TABLE,
    RACK_SLOT,
    TOOL_SLOT,
    ROBOT_GRIPPER,
    INPUT_PORT,
    OUTPUT_PORT
};

enum class RobotState {
    IDLE,
    BUSY
};

enum class MachineState {
    IDLE,
    PROCESSING
};

enum class ActionType {
    PICK,
    PLACE,
    LOAD_MACHINE,
    UNLOAD_MACHINE,
    CHANGE_TOOL,
    WAIT
};

enum class EventType {
    MACHINE_FINISHED,
    ROBOT_FINISHED,
    MATERIAL_ARRIVED
};

constexpr IdType INVALID_ID = static_cast<IdType>(-1);
constexpr TimeType INVALID_TIME = -1.0;

struct Position {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

std::string to_string(MaterialType type);
std::string to_string(LocationType type);
std::string to_string(RobotState state);
std::string to_string(MachineState state);
std::string to_string(ActionType type);
std::string to_string(EventType type);

} // namespace fms