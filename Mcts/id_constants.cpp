#include "id_constants.hpp"

namespace fms {

std::string location_id_to_name(IdType id) {
    if (id == INVALID_ID) {
        return "INVALID";
    }
    
    IdType parent_id = id / id_constants::MACHINE_TOOL_SLOT_OFFSET;
    if (parent_id > 0) {
        return "Machine_" + std::to_string(parent_id) + "_ToolSlot_" + std::to_string(id % id_constants::MACHINE_TOOL_SLOT_OFFSET);
    }
    
    parent_id = id / id_constants::MACHINE_TABLE_OFFSET;
    if (parent_id > 0) {
        return "Machine_" + std::to_string(parent_id) + "_Table_" + std::to_string(id % id_constants::MACHINE_TABLE_OFFSET);
    }
    
    parent_id = id / id_constants::RACK_SLOT_OFFSET;
    if (parent_id > 0) {
        return "Rack_" + std::to_string(parent_id) + "_Slot_" + std::to_string(id % id_constants::RACK_SLOT_OFFSET);
    }
    
    parent_id = id / id_constants::PORT_SLOT_OFFSET;
    if (parent_id > 0) {
        return "Port_" + std::to_string(parent_id) + "_Slot_" + std::to_string(id % id_constants::PORT_SLOT_OFFSET);
    }
    
    parent_id = id / id_constants::ROBOT_GRIPPER_OFFSET;
    if (parent_id > 0) {
        return "Robot_" + std::to_string(parent_id) + "_Gripper_" + std::to_string(id % id_constants::ROBOT_GRIPPER_OFFSET);
    }
    
    return "Location_" + std::to_string(id);
}

} // namespace fms