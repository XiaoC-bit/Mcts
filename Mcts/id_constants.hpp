#pragma once
#include "fms_types.hpp"

namespace fms {

namespace id_constants {
    constexpr IdType ROBOT_GRIPPER_OFFSET = 100;
    constexpr IdType PORT_SLOT_OFFSET = 200;
    constexpr IdType RACK_SLOT_OFFSET = 1000;
    constexpr IdType MACHINE_TABLE_OFFSET = 2000;
    constexpr IdType MACHINE_TOOL_SLOT_OFFSET = 10000;
    
    constexpr IdType DEFAULT_TOOL_LIFE = 1000;
    constexpr int DEFAULT_MCTS_ITERATIONS = 1000;
    constexpr double TIME_PENALTY_FACTOR = 1000.0;
}



std::string location_id_to_name(IdType id);

} // namespace fms