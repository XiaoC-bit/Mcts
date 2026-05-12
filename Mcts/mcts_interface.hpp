#pragma once
#include <vector>

template <typename State, typename Action>
class MCTSInterface {
public:
    virtual ~MCTSInterface() = default;
    
    virtual std::vector<Action> legal_actions(const State& s) const = 0;
    virtual State apply(const State& s, const Action& a) const = 0;
    virtual bool is_terminal(const State& s) const = 0;
    virtual double terminal_score(const State& s) const = 0;
};