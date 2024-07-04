#include "HaffmanTree.h"
#include <algorithm>

using namespace std;
using HT = HaffmanTree;

HT::Node::Node() {}

HT::Node::Node(char symbol_, int weight_) {
	symbol = symbol_;
	weight = weight_;
	right = nullptr;
	left = nullptr;
};

void HT::sortNodes() {
	sort(nodes.begin(), nodes.end(),
		[](HT::Node left, HT::Node right) {
			return left.weight < right.weight; // straight
		}
	);
}

unique_ptr<HT::Node> HT::createCombinationNode(Node* left, Node* right) {
	auto comb = make_unique<Node>(left->symbol + right->symbol, left->weight + right->weight);
	comb->left = left;
	comb->right = right;
	return comb;
}

void HT::recursiveCodeAssembly(Node* knot, string code, map<char, string>* output) {
	if (knot->left == nullptr && knot->right == nullptr) { // Check if it is a leaf node
		output->insert({ knot->symbol, code });
	}
	if (knot->left != nullptr) {
		recursiveCodeAssembly(knot->left, code + "1", output);
	}
	if (knot->right != nullptr) {
		recursiveCodeAssembly(knot->right, code + "0", output);
	}
}

void HT::deleteNodes(Node* knot) {
	if (knot != nullptr) {
		deleteNodes(knot->left);
		deleteNodes(knot->right);
		knot->left = nullptr;
		knot->right = nullptr;
		//delete knot; FIXME: unknown object, возможно это даже не нужно
	}
}

HT::HaffmanTree() {}

bool HT::Empty() {
	return nodes.empty();
}

void HT::CreateHaffmanTree(map<char, int> symbols) {
	Clear();
	if (symbols.size() == 0) return; // map needs to be checked
	for (const auto& symbol : symbols) {
		nodes.push_back(Node(symbol.first, symbol.second));
	}
	sortNodes();
	vector<Node*> tree;
	for (int i = 0; i < nodes.size(); i++) {
		tree.push_back(&nodes[i]);
	}
	while (true) {
		if (tree.size() == 1) break;
		Node* left = tree[0], * right = tree[1];
		tree.erase(tree.begin(), tree.begin() + 2);
		auto combinationNode = createCombinationNode(left, right); // combine two nodes
		auto pos = lower_bound(tree.begin(), tree.end(), combinationNode.get(),
			[](const Node* a, const Node* b) {
				return a->weight < b->weight;
			}); // define position of combNode
		tree.insert(pos, combinationNode.release()); // insert combNode in position
	}
	root = *tree[0]; // saving the root Node
}

map<char, string> HT::ReturnHaffmanTable() {
	map<char, string> haffmanCodes;
	recursiveCodeAssembly(&root, "", &haffmanCodes);
	return haffmanCodes;
}

void HT::Clear() {
	if (!Empty()) { // check
		deleteNodes(&root);
		nodes.clear();
	}
}