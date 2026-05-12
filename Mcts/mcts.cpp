#include <iostream>
#include "mcts_core.hpp"

using State = int;
using Action = int;

class ExampleGame : public MCTSInterface<State, Action> {
public:
    std::vector<Action> legal_actions(const State& s) const override {
        if (s >= 10) return {};
        return {0, 1};
    }

    State apply(const State& s, const Action& a) const override {
        return s + 1 + a;
    }

    bool is_terminal(const State& s) const override {
        return s >= 10;
    }

    double terminal_score(const State& s) const override {
        return s / 10.0;
    }
};

int main() {
    ExampleGame game;
    State initial_state = 0;
    
    std::cout << "Initial state: " << initial_state << std::endl;
    
    Action best_action = mcts(initial_state, game, 1000);
    
    std::cout << "Best action: " << best_action << std::endl;
    
    return 0;
}