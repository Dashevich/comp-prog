#pragma once
#include <memory>

#include <vector>
#include <map>
#include <string>

namespace AST {

using ValT = int;

class INode;

class Context {
	std::vector<value_type> nums;
	std::map<std::string, std::pair<INode*, INode*>> funcs;
	using MapT = std::map<std::string, ValT>;
	std::vector<MapT> ScopeStack;
	std::vector<std::map<std::string, value_type>> variables;

public:
	ValT& get(const std::string &name) {
		for (auto &Scope : ScopeStack) {
			auto It = Scope.find(name);
			if (It != Scope.end())
				return It->second;
		}
		return ScopeStack.back()[name];
	}
	void enterScope() {
		ScopeStack.emplace_back();
	}
	void leaveScope() {
		ScopeStack.pop_back();
	}
	void addFunc(std::string name, INode* func, INode* args) {
		funcs[name] = std::make_pair(func, args);
	}
	void setRes(value_type v) {
		nums[nums.size() - 1] = v;
	}
	void addScope() {
		variables.push_back({});
		nums.push_back(0);
	}
	value_type getRes() {
		return nums.back();
	}
};

struct INode {
	virtual ValT eval(Context &) = 0;
};

}// namespace AST
