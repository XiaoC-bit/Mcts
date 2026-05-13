#include "simulator.hpp"
#include <algorithm>
#include <cmath>

namespace fms {

std::vector<Action> Simulator::legal_actions(const State& state) const {
    std::vector<Action> actions;
    
    for (const auto& robot : state.robots) {
        if (!robot.is_idle(state.current_time)) continue;
        
        actions.push_back(make_wait_action(robot.id, 1.0));
        
        for (const auto& input_port : state.input_ports) {
            for (const auto& slot : input_port.slots) {
                if (slot.is_occupied && can_pick(state, robot.id, slot.id)) {
                    actions.push_back(make_pick_action(robot.id, slot.id));
                }
            }
        }
        
        for (const auto& rack : state.racks) {
            for (const auto& row : rack.slots) {
                for (const auto& slot : row) {
                    if (!slot.is_empty() && can_pick(state, robot.id, slot.id)) {
                        actions.push_back(make_pick_action(robot.id, slot.id));
                    }
                }
            }
        }
        
        for (const auto& machine : state.machines) {
            for (const auto& table : machine.tables) {
                if (table.is_finished(state.current_time)) {
                    if (can_unload_machine(state, robot.id, machine.id)) {
                        actions.push_back(make_unload_machine_action(robot.id, machine.id));
                    }
                } else if (table.is_idle()) {
                    for (const auto& grip : robot.grippers) {
                        if (!grip.is_empty()) {
                            const Material* mat = state.get_material(grip.material_id);
                            if (mat && mat->type == MaterialType::WORKPIECE) {
                                if (can_load_machine(state, robot.id, machine.id, mat->id)) {
                                    actions.push_back(make_load_machine_action(robot.id, machine.id, mat->id));
                                }
                            }
                        }
                    }
                }
            }
        }
        
        for (const auto& grip : robot.grippers) {
            if (!grip.is_empty()) {
                const Material* mat = state.get_material(grip.material_id);
                if (mat) {
                    for (const auto& rack : state.racks) {
                        IdType empty_slot_id = rack.get_empty_slot_id();
                        if (empty_slot_id != INVALID_ID) {
                            if (can_place(state, robot.id, empty_slot_id, mat->id)) {
                                actions.push_back(make_place_action(robot.id, empty_slot_id, mat->id));
                            }
                        }
                    }
                    
                    for (const auto& output_port : state.output_ports) {
                        IdType empty_slot_id = output_port.get_empty_slot_id();
                        if (empty_slot_id != INVALID_ID) {
                            const Workpiece* wp = dynamic_cast<const Workpiece*>(mat);
                            if (wp && wp->is_processed) {
                                if (can_place(state, robot.id, empty_slot_id, mat->id)) {
                                    actions.push_back(make_place_action(robot.id, empty_slot_id, mat->id));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    return actions;
}

State Simulator::apply(const State& state, const Action& action) const {
    State new_state = state;
    
    process_events(new_state);
    
    if (!is_action_valid(new_state, action)) {
        return new_state;
    }
    
    switch (action.type) {
        case ActionType::PICK:
            execute_pick(new_state, action);
            break;
        case ActionType::PLACE:
            execute_place(new_state, action);
            break;
        case ActionType::LOAD_MACHINE:
            execute_load_machine(new_state, action);
            break;
        case ActionType::UNLOAD_MACHINE:
            execute_unload_machine(new_state, action);
            break;
        case ActionType::CHANGE_TOOL:
            execute_change_tool(new_state, action);
            break;
        case ActionType::WAIT:
            execute_wait(new_state, action);
            break;
    }
    
    process_events(new_state);
    
    return new_state;
}

bool Simulator::is_terminal(const State& state) const {
    return !state.has_pending_workpieces() && state.event_queue.empty();
}

double Simulator::evaluate(const State& state) const {
    State temp_state = state;
    process_events(temp_state);
    
    size_t processed = temp_state.count_processed_workpieces();
    size_t total_workpieces = 0;
    for (const auto& pair : temp_state.materials) {
        if (pair.second->type == MaterialType::WORKPIECE) {
            total_workpieces++;
        }
    }
    
    if (total_workpieces == 0) return 0.0;
    
    double throughput = static_cast<double>(processed) / static_cast<double>(total_workpieces);
    double time_penalty = temp_state.current_time / id_constants::TIME_PENALTY_FACTOR;
    
    return throughput - time_penalty;
}

void Simulator::process_events(State& state) const {
    while (!state.event_queue.empty() && state.event_queue.top().time <= state.current_time) {
        Event event = state.event_queue.pop();
        
        switch (event.type) {
            case EventType::MACHINE_FINISHED: {
                Machine* machine = state.get_machine(event.source_id);
                if (machine) {
                    for (auto& table : machine->tables) {
                        if (table.id == event.target_id) {
                            table.finish_processing();
                            Workpiece* wp = state.get_workpiece(event.material_id);
                            if (wp) {
                                wp->complete_step();
                            }
                        }
                    }
                }
                break;
            }
            case EventType::ROBOT_FINISHED: {
                Robot* robot = state.get_robot(event.source_id);
                if (robot) {
                    robot->set_idle();
                }
                break;
            }
            case EventType::MATERIAL_ARRIVED: {
                Port* port = state.get_input_port(event.source_id);
                if (port) {
                    PortSlot* slot = port->get_slot(event.target_id);
                    if (slot && !slot->is_occupied) {
                        slot->place_material(event.material_id);
                    }
                }
                break;
            }
        }
    }
}

bool Simulator::is_action_valid(const State& state, const Action& action) const {
    if (!action.is_valid()) return false;
    
    const Robot* robot = state.get_robot(action.robot_id);
    if (!robot || !robot->is_idle(state.current_time)) return false;
    
    switch (action.type) {
        case ActionType::PICK:
            return can_pick(state, action.robot_id, action.source_location);
        case ActionType::PLACE:
            return can_place(state, action.robot_id, action.target_location, action.material_id);
        case ActionType::LOAD_MACHINE:
            return can_load_machine(state, action.robot_id, action.machine_id, action.material_id);
        case ActionType::UNLOAD_MACHINE:
            return can_unload_machine(state, action.robot_id, action.machine_id);
        case ActionType::CHANGE_TOOL:
            return can_change_tool(state, action.robot_id, action.machine_id, action.material_id);
        case ActionType::WAIT:
            return true;
        default:
            return false;
    }
}

bool Simulator::can_pick(const State& state, IdType robot_id, IdType location_id) const {
    const Robot* robot = state.get_robot(robot_id);
    if (!robot || !robot->has_empty_gripper()) return false;
    
    for (const auto& input_port : state.input_ports) {
        const PortSlot* slot = input_port.get_slot(location_id);
        if (slot && slot->is_occupied) {
            return true;
        }
    }
    
    for (const auto& rack : state.racks) {
        const RackSlot* slot = rack.get_slot_by_id(location_id);
        if (slot && !slot->is_empty()) {
            return true;
        }
    }
    
    for (const auto& machine : state.machines) {
        for (const auto& table : machine.tables) {
            if (table.id == location_id && table.is_finished(state.current_time)) {
                return true;
            }
        }
    }
    
    return false;
}

bool Simulator::can_place(const State& state, IdType robot_id, IdType location_id, IdType material_id) const {
    const Robot* robot = state.get_robot(robot_id);
    if (!robot) return false;
    
    bool has_material = false;
    for (const auto& grip : robot->grippers) {
        if (grip.material_id == material_id) {
            has_material = true;
            break;
        }
    }
    if (!has_material) return false;
    
    for (const auto& rack : state.racks) {
        const RackSlot* slot = rack.get_slot_by_id(location_id);
        if (slot && slot->is_empty()) {
            return true;
        }
    }
    
    for (const auto& output_port : state.output_ports) {
        const PortSlot* slot = output_port.get_slot(location_id);
        if (slot && slot->is_empty()) {
            return true;
        }
    }
    
    return false;
}

bool Simulator::can_load_machine(const State& state, IdType robot_id, IdType machine_id, IdType workpiece_id) const {
    const Robot* robot = state.get_robot(robot_id);
    const Machine* machine = state.get_machine(machine_id);
    
    if (!robot || !machine) return false;
    
    if (!machine->has_idle_table()) return false;
    
    bool has_workpiece = false;
    for (const auto& grip : robot->grippers) {
        if (grip.material_id == workpiece_id) {
            has_workpiece = true;
            break;
        }
    }
    return has_workpiece;
}

bool Simulator::can_unload_machine(const State& state, IdType robot_id, IdType machine_id) const {
    const Robot* robot = state.get_robot(robot_id);
    const Machine* machine = state.get_machine(machine_id);
    
    if (!robot || !machine) return false;
    if (!robot->has_empty_gripper()) return false;
    
    for (const auto& table : machine->tables) {
        if (table.is_finished(state.current_time)) {
            return true;
        }
    }
    return false;
}

bool Simulator::can_change_tool(const State& state, IdType robot_id, IdType machine_id, IdType tool_id) const {
    const Robot* robot = state.get_robot(robot_id);
    const Machine* machine = state.get_machine(machine_id);
    
    if (!robot || !machine) return false;
    if (!robot->has_empty_gripper()) return false;
    if (!machine->has_empty_tool_slot()) return false;
    
    const Tool* tool = state.get_tool(tool_id);
    return tool != nullptr;
}

void Simulator::execute_pick(State& state, const Action& action) const {
    Robot* robot = state.get_robot(action.robot_id);
    if (!robot) return;
    
    IdType gripper_id = robot->get_empty_gripper_id();
    Gripper* gripper = robot->get_gripper(gripper_id);
    if (!gripper) return;
    
    IdType material_id = INVALID_ID;
    
    for (auto& input_port : state.input_ports) {
        PortSlot* slot = input_port.get_slot(action.source_location);
        if (slot && slot->is_occupied) {
            material_id = slot->remove_material();
            break;
        }
    }
    
    if (material_id == INVALID_ID) {
        for (auto& rack : state.racks) {
            RackSlot* slot = rack.get_slot_by_id(action.source_location);
            if (slot && !slot->is_empty()) {
                material_id = slot->remove_material();
                break;
            }
        }
    }
    
    if (material_id != INVALID_ID) {
        Material* mat = state.get_material(material_id);
        if (mat) {
            gripper->grasp(material_id, mat->weight);
            mat->move_to(gripper_id);
            robot->set_busy(action.duration, state.current_time);
            state.event_queue.push(state.current_time + action.duration, 
                                   EventType::ROBOT_FINISHED, action.robot_id);
        }
    }
}

void Simulator::execute_place(State& state, const Action& action) const {
    Robot* robot = state.get_robot(action.robot_id);
    if (!robot) return;
    
    Gripper* gripper = nullptr;
    for (auto& grip : robot->grippers) {
        if (grip.material_id == action.material_id) {
            gripper = &grip;
            break;
        }
    }
    if (!gripper) return;
    
    bool placed = false;
    
    for (auto& rack : state.racks) {
        RackSlot* slot = rack.get_slot_by_id(action.target_location);
        if (slot && slot->is_empty()) {
            slot->place_material(action.material_id);
            placed = true;
            break;
        }
    }
    
    if (!placed) {
        for (auto& output_port : state.output_ports) {
            PortSlot* slot = output_port.get_slot(action.target_location);
            if (slot && slot->is_empty()) {
                slot->place_material(action.material_id);
                placed = true;
                break;
            }
        }
    }
    
    if (placed) {
        gripper->release();
        Material* mat = state.get_material(action.material_id);
        if (mat) {
            mat->move_to(action.target_location);
        }
        robot->set_busy(action.duration, state.current_time);
        state.event_queue.push(state.current_time + action.duration, 
                               EventType::ROBOT_FINISHED, action.robot_id);
    }
}

void Simulator::execute_load_machine(State& state, const Action& action) const {
    Robot* robot = state.get_robot(action.robot_id);
    Machine* machine = state.get_machine(action.machine_id);
    
    if (!robot || !machine) return;
    
    Gripper* gripper = nullptr;
    for (auto& grip : robot->grippers) {
        if (grip.material_id == action.material_id) {
            gripper = &grip;
            break;
        }
    }
    if (!gripper) return;
    
    IdType table_id = machine->get_idle_table_id();
    MachineTable* table = machine->get_table(table_id);
    if (!table) return;
    
    double process_time = 5.0;
    table->start_processing(action.material_id, process_time, state.current_time);
    
    gripper->release();
    Material* mat = state.get_material(action.material_id);
    if (mat) {
        mat->move_to(table_id);
    }
    
    robot->set_busy(action.duration, state.current_time);
    state.event_queue.push(state.current_time + action.duration, 
                           EventType::ROBOT_FINISHED, action.robot_id);
    state.event_queue.push(state.current_time + process_time, 
                           EventType::MACHINE_FINISHED, machine->id, table_id, action.material_id);
}

void Simulator::execute_unload_machine(State& state, const Action& action) const {
    Robot* robot = state.get_robot(action.robot_id);
    Machine* machine = state.get_machine(action.machine_id);
    
    if (!robot || !machine) return;
    
    MachineTable* table = nullptr;
    for (auto& tbl : machine->tables) {
        if (tbl.is_finished(state.current_time)) {
            table = &tbl;
            break;
        }
    }
    if (!table) return;
    
    IdType workpiece_id = table->workpiece_id;
    IdType gripper_id = robot->get_empty_gripper_id();
    Gripper* gripper = robot->get_gripper(gripper_id);
    
    if (!gripper || workpiece_id == INVALID_ID) return;
    
    Material* mat = state.get_material(workpiece_id);
    if (mat) {
        gripper->grasp(workpiece_id, mat->weight);
        mat->move_to(gripper_id);
    }
    
    table->finish_processing();
    
    robot->set_busy(action.duration, state.current_time);
    state.event_queue.push(state.current_time + action.duration, 
                           EventType::ROBOT_FINISHED, action.robot_id);
}

void Simulator::execute_change_tool(State& state, const Action& action) const {
    Robot* robot = state.get_robot(action.robot_id);
    Machine* machine = state.get_machine(action.machine_id);
    
    if (!robot || !machine) return;
    
    IdType gripper_id = robot->get_empty_gripper_id();
    Gripper* gripper = robot->get_gripper(gripper_id);
    if (!gripper) return;
    
    IdType tool_slot_id = machine->get_empty_tool_slot_id();
    ToolSlot* tool_slot = machine->get_tool_slot(tool_slot_id);
    if (!tool_slot) return;
    
    Tool* tool = state.get_tool(action.material_id);
    if (!tool) return;
    
    tool_slot->load_tool(action.material_id, tool->category);
    tool->move_to(tool_slot_id);
    
    robot->set_busy(action.duration, state.current_time);
    state.event_queue.push(state.current_time + action.duration, 
                           EventType::ROBOT_FINISHED, action.robot_id);
}

void Simulator::execute_wait(State& state, const Action& action) const {
    Robot* robot = state.get_robot(action.robot_id);
    if (!robot) return;
    
    state.current_time += action.duration;
    process_events(state);
}

void Simulator::move_material(State& state, IdType material_id, IdType from_location, IdType to_location) const {
    Material* mat = state.get_material(material_id);
    if (!mat) return;
    mat->move_to(to_location);
}

IdType Simulator::find_material_location(const State& state, IdType material_id) const {
    const Material* mat = state.get_material(material_id);
    return mat ? mat->location_id : INVALID_ID;
}

double Simulator::calculate_distance(const Position& a, const Position& b) const {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    double dz = a.z - b.z;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

} // namespace fms