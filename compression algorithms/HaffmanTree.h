#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

using namespace std;

class HaffmanTree {
private:
	struct Node {
		char symbol;
		int weight;
		Node* right;
		Node* left;

		Node();
		Node(char symbol_, int weight_);
	};
	Node root;
	vector<Node> nodes;

	void sortNodes();

	unique_ptr<Node> createCombinationNode(Node* left, Node* right);
	
	void recursiveCodeAssembly(Node* knot, string code, map<char, string>* output);

	void deleteNodes(Node* knot);
public:
	HaffmanTree();

	bool Empty();

	void CreateHaffmanTree(map<char, int> symbols);

	map<char, string> ReturnHaffmanTable();

	void Clear();
};
