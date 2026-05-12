#pragma once
#include <random>
#include <memory>
#include "mcts_node.hpp"
#include "mcts_interface.hpp"

template <typename State, typename Action>
Node<State, Action>* selection(Node<State, Action>* root, const MCTSInterface<State, Action>& game) {
    Node<State, Action>* node = root;
    while (!game.is_terminal(node->state) && node->is_fully_expanded()) {
        node = node->best_ucb_child();
    }
    return node;
}

template <typename State, typename Action>
Node<State, Action>* expansion(Node<State, Action>* node, const MCTSInterface<State, Action>& game) {
    if (game.is_terminal(node->state))
        return node;

    Action action = node->untried_actions.back();
    node->untried_actions.pop_back();

    State new_state = game.apply(node->state, action);
    auto child = std::make_unique<Node<State, Action>>(new_state, action, node, game);
    node->children.push_back(std::move(child));
    return node->children.back().get();
}

template <typename State, typename Action>
double simulation(State state, const MCTSInterface<State, Action>& game) {
    static std::mt19937 rng{std::random_device{}()};

    while (!game.is_terminal(state)) {
        auto actions = game.legal_actions(state);
        std::uniform_int_distribution<int> dist(0, (int)actions.size() - 1);
        state = game.apply(state, actions[dist(rng)]);
    }
    return game.terminal_score(state);
}

template <typename State, typename Action>
void backpropagation(Node<State, Action>* node, double score) {
    while (node != nullptr) {
        node->n += 1;
        node->v += score;
        node = node->parent;
    }
}

template <typename State, typename Action>
Action mcts(const State& root_state, const MCTSInterface<State, Action>& game, int iterations = 1000) {
    auto root = std::make_unique<Node<State, Action>>(root_state, Action{}, nullptr, game);
    root->n = 1;

    for (int i = 0; i < iterations; i++) {
        Node<State, Action>* node = selection(root.get(), game);
        Node<State, Action>* child = expansion(node, game);
        double score = simulation(child->state, game);
        backpropagation(child, score);
    }

    auto& best = *std::max_element(
        root->children.begin(), root->children.end(),
        [](const auto& a, const auto& b) { return a->n < b->n; }
    );
    return best->action;
}