#include <iostream>
#include "mcts_core.hpp"
#include "simulator.hpp"
#include "fms_types.hpp"

int main() {
    fms::State state;
    
    state.create_robot("Robot1", fms::Position{0, 0, 0}, 2);
    state.create_machine("Machine1", fms::Position{5, 0, 0}, 2, 5);
    state.create_rack("StorageRack", fms::Position{10, 0, 0}, 5, 5);
    state.create_tool_rack("ToolRack", fms::Position{15, 0, 0}, 3, 5);
    
    fms::IdType input_port_id = state.create_input_port("InputPort", fms::Position{-5, 0, 0}, 3);
    fms::IdType output_port_id = state.create_output_port("OutputPort", fms::Position{20, 0, 0}, 3);
    
    fms::IdType wp1 = state.create_workpiece("WP001", "drilling", 1);
    fms::IdType wp2 = state.create_workpiece("WP002", "milling", 1);
    
    fms::Port* input_port = state.get_input_port(input_port_id);
    if (input_port) {
        fms::IdType slot_id = input_port->get_empty_slot_id();
        fms::PortSlot* slot = input_port->get_slot(slot_id);
        if (slot) {
            slot->place_material(wp1);
        }
        slot_id = input_port->get_empty_slot_id();
        slot = input_port->get_slot(slot_id);
        if (slot) {
            slot->place_material(wp2);
        }
    }
    
    fms::Simulator simulator;
    fms::FMSSimulatorAdapter adapter(simulator);
    
    std::cout << "Initial state time: " << state.current_time << std::endl;
    std::cout << "Number of workpieces: " << state.materials.size() << std::endl;
    std::cout << "Input port occupied slots: " << input_port->occupied_slots() << std::endl;
    
    std::vector<fms::Action> actions = simulator.legal_actions(state);
    std::cout << "Legal actions available: " << actions.size() << std::endl;
    
    if (!actions.empty()) {
        fms::Action best_action = mcts(state, adapter, 100);
        std::cout << "Best action type: " << fms::to_string(best_action.type) << std::endl;
        std::cout << "Best action robot: " << best_action.robot_id << std::endl;
    }
    
    return 0;
}