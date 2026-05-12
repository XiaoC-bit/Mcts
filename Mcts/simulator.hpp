#pragma once
#include "state.hpp"
#include "action.hpp"
#include "mcts_interface.hpp"
#include <vector>

namespace fms {

class Simulator {
public:
    std::vector<Action> legal_actions(const State& state) const;
    State apply(const State& state, const Action& action) const;
    bool is_terminal(const State& state) const;
    double evaluate(const State& state) const;
    void process_events(State& state) const;
    
private:
    bool is_action_valid(const State& state, const Action& action) const;
    bool can_pick(const State& state, IdType robot_id, IdType location_id) const;
    bool can_place(const State& state, IdType robot_id, IdType location_id, IdType material_id) const;
    bool can_load_machine(const State& state, IdType robot_id, IdType machine_id, IdType workpiece_id) const;
    bool can_unload_machine(const State& state, IdType robot_id, IdType machine_id) const;
    bool can_change_tool(const State& state, IdType robot_id, IdType machine_id, IdType tool_id) const;
    
    void execute_pick(State& state, const Action& action) const;
    void execute_place(State& state, const Action& action) const;
    void execute_load_machine(State& state, const Action& action) const;
    void execute_unload_machine(State& state, const Action& action) const;
    void execute_change_tool(State& state, const Action& action) const;
    void execute_wait(State& state, const Action& action) const;
    
    void move_material(State& state, IdType material_id, IdType from_location, IdType to_location) const;
    IdType find_material_location(const State& state, IdType material_id) const;
    double calculate_distance(const Position& a, const Position& b) const;
};

class FMSSimulatorAdapter : public MCTSInterface<State, Action> {
public:
    FMSSimulatorAdapter(Simulator& sim) : simulator(sim) {}
    
    std::vector<Action> legal_actions(const State& s) const override {
        return simulator.legal_actions(s);
    }
    
    State apply(const State& s, const Action& a) const override {
        return simulator.apply(s, a);
    }
    
    bool is_terminal(const State& s) const override {
        return simulator.is_terminal(s);
    }
    
    double terminal_score(const State& s) const override {
        return simulator.evaluate(s);
    }
    
private:
    Simulator& simulator;
};

} // namespace fms