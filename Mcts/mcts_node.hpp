#pragma once
#include <vector>
#include <memory>
#include <limits>
#include <algorithm>
#include "mcts_interface.hpp"

template <typename State, typename Action>
struct Node {
    State state;
    Action action;
    Node* parent = nullptr;

    std::vector<std::unique_ptr<Node>> children;
    std::vector<Action> untried_actions;

    int n = 0;
    double v = 0.0;

    Node(const State& s, const Action& a, Node* p, const MCTSInterface<State, Action>& game)
        : state(s), action(a), parent(p)
    {
        untried_actions = game.legal_actions(s);
    }

    double ucb(double C = 1.41) const {
        if (n == 0)
            return std::numeric_limits<double>::max();
        return v / n + C * std::sqrt(std::log(parent->n) / n);
    }

    bool is_fully_expanded() const {
        return untried_actions.empty();
    }

    Node* best_ucb_child(double C = 1.41) {
        return std::max_element(
            children.begin(), children.end(),
            [C](const auto& a, const auto& b) {
                return a->ucb(C) < b->ucb(C);
            }
        )->get();
    }
};