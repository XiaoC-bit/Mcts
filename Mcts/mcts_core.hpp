#pragma once
#include <random>
#include <memory>
#include <vector>
#include <iostream>
#include <iomanip>
#include "mcts_node.hpp"
#include "mcts_interface.hpp"
#include "id_constants.hpp"

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
Action mcts(const State& root_state, const MCTSInterface<State, Action>& game, int iterations = fms::id_constants::DEFAULT_MCTS_ITERATIONS) {

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
PlanResult<Action> mcts_plan(const State& root_state, const MCTSInterface<State, Action>& game, int iterations = fms::id_constants::DEFAULT_MCTS_ITERATIONS) {
	auto root = std::make_unique<Node<State, Action>>(root_state, Action{}, nullptr, game);
	root->n = 1;

	for (int i = 0; i < iterations; i++) {
		//找到最值得探索的节点
		Node<State, Action>* node = selection(root.get(), game);
		//从该节点扩展一个子节点
		//扩展的意思是从该节点的子节点中，找到一个没有探索的子节点，对其进行探索，也就是罗列出该节点的所有合法动作，
		//罗列后，将这个子节点加入到该节点的子节点列表中，并返回这个子节点
		Node<State, Action>* child = expansion(node, game);

		//对这个子节点进行模拟，得到一个分数
		//模拟的意思是从这个子节点开始，随机选择合法动作，不断往下探索，直到达到终止状态，然后计算这个终止状态的分数
		double score = simulation(child->state, game);

		//将这个分数反向传播回父节点，更新父节点的访问次数和总分数
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
				[](const auto& a, const auto& b) { 
					double avg_a = a->n > 0 ? a->v / a->n : 0.0;
					double avg_b = b->n > 0 ? b->v / b->n : 0.0;
					return avg_a < avg_b; 
				}
			)->get();
		} else {
			break;
		}
	}

	PlanResult<Action> result;
	result.next_action = best_child->action;
	result.predicted_sequence = predicted_sequence;
	result.estimated_score = best_child->v / best_child->n;

	print_tree(root.get(), 0);

	return result;
}

template <typename State, typename Action>
void print_tree(const Node<State, Action>* node, int depth, bool is_last = true, const std::string& prefix = "") {
	if (!node) return;

	double avg_score = node->n > 0 ? node->v / node->n : 0.0;
	
	if (depth == 0) {
		std::cout << "┌─────────────────────────────────────────────────────────────┐" << std::endl;
		std::cout << "│                   MCTS Search Tree                         │" << std::endl;
		std::cout << "└─────────────────────────────────────────────────────────────┘" << std::endl;
		std::cout << "Root Node [n=" << node->n << ", v=" << std::fixed << std::setprecision(3) << node->v 
				  << ", avg=" << avg_score << "]" << std::endl;
	} else {
		std::string branch = is_last ? "└── " : "├── ";
		std::cout << prefix << branch;
		
		std::cout << "[n=" << node->n << ", v=" << std::fixed << std::setprecision(3) << node->v 
				  << ", avg=" << avg_score << "] ";
		
		std::cout << node->action.to_string() << std::endl;
	}

	if (!node->children.empty()) {
		std::string new_prefix = prefix + (is_last ? "    " : "│   ");
		size_t last_idx = node->children.size() - 1;
		
		for (size_t i = 0; i < node->children.size(); ++i) {
			print_tree(node->children[i].get(), depth + 1, (i == last_idx), new_prefix);
		}
	}
	
	if (depth == 0) {
		std::cout << "\n┌─────────────────────────────────────────────────────────────┐" << std::endl;
		std::cout << "│                    Legend:                                 │" << std::endl;
		std::cout << "│  n: 访问次数   v: 累计得分   avg: 平均得分              │" << std::endl;
		std::cout << "└─────────────────────────────────────────────────────────────┘" << std::endl;
	}
}