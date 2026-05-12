#pragma once
#include <random>
#include <memory>
#include <vector>
#include "mcts_node.hpp"
#include "mcts_interface.hpp"

template <typename Action>
struct PlanResult {
    Action next_action;
    std::vector<Action> predicted_sequence;
    double estimated_score;
};

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
        if (actions.empty()) {
            break;
        }
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

template <typename State, typename Action>
PlanResult<Action> mcts_plan(const State& root_state, const MCTSInterface<State, Action>& game, int iterations = 1000) {
    auto root = std::make_unique<Node<State, Action>>(root_state, Action{}, nullptr, game);
    root->n = 1;

    for (int i = 0; i < iterations; i++) {
        Node<State, Action>* node = selection(root.get(), game);
        Node<State, Action>* child = expansion(node, game);
        double score = simulation(child->state, game);
        backpropagation(child, score);
    }

    auto& best_child = *std::max_element(
        root->children.begin(), root->children.end(),
        [](const auto& a, const auto& b) { return a->n < b->n; }
    );

    std::vector<Action> predicted_sequence;
    Node<State, Action>* current = best_child.get();
    
    while (current != nullptr && current->parent != nullptr) {
        predicted_sequence.push_back(current->action);
        auto& children = current->children;
        if (!children.empty()) {
            current = std::max_element(
                children.begin(), children.end(),
                [](const auto& a, const auto& b) { return a->n < b->n; }
            )->get();
        } else {
            break;
        }
    }

    PlanResult<Action> result;
    result.next_action = best_child->action;
    result.predicted_sequence = predicted_sequence;
    result.estimated_score = best_child->v / best_child->n;

    return result;
}