#pragma once
#define AST
#include <map>
#include <vector>
#include <set>
#include <iostream>
#include <string>
#include <memory>

typedef int _type_id;
typedef int value_type;
typedef int err_type;


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
	virtual ValT eval(Context &CX) = 0;
};

template <typename Op>
class BinOp : public INode {
	Op m_op;
	std::unique_ptr<INode> m_lhs;
	std::unique_ptr<INode> m_rhs;
public:
	ValT eval(Context &CX) override {
		return m_op(m_lhs->eval(CX), m_rhs->eval(CX));
	}
	BinOp(Op&& op, INode *lhs, INode *rhs) :
		m_op(std::forward<Op>(op)),
		m_lhs(lhs),
		m_rhs(rhs) {}
};

class Scope : public INode {
private:
	INode* _expr;
public:
	Scope(INode* expr) : _expr(expr) {}

	virtual value_type eval(Context& CX) override {
		CX.addScope();
		_expr->eval(CX);
		value_type res = CX.getRes();
		CX.leaveScope();
		return res;
	}
};

class NameGive : public INode {
	std::string _name;
public:
	NameGive(std::string name) : _name(name) {}

	virtual value_type eval(Context &CX) override {
		return 0; 
	}	
	
	std::string getName() {
		return _name;
	}
};

class Function : public INode {
	INode* _expr;
	INode* _name;
	INode* _arguments;
public:
	Function(INode* expr, INode* name, INode* args) : _expr(expr), _name(name), _arguments(args) {}

	virtual value_type eval(Context &CX) override {
		CX.addFunc(((NameGive*)_name)->getName(), _expr, _arguments);
		return 0;
	}
};


class Return : public INode {
	INode* _expr;
public:
	Return(INode* expr) : _expr(expr) {}

	virtual value_type eval(Context &CX) {
		value_type res = _expr->eval(CX);
		CX.setRes(res);
		return res;
	}
};


class Split : public INode {
	INode* _left_child, *_right_child;
public:
	Split(INode* lc, INode* rc) : _left_child(lc), _right_child(rc) {
	
	}

	virtual value_type eval(Context &CX) override {
		value_type res = _left_child->eval(CX);
		_right_child->eval(CX);
		return res;
	}
};

class Variable : public INode {
private:
	INode* _name;
public:
	Variable(INode* name) : _name(name) {}
	value_type& get(Context &CX) {
		return CX.get(((NameGive*)_name)->getName());
	}
	value_type eval(Context &CX) {
		return CX.get(((NameGive*)_name)->getName());
	}
};
class Assign : public INode {
private:
	INode* _right_child;
	Variable* _left_child;
public:
	Assign(Variable* lc, INode* rc) : _left_child(lc), _right_child(rc) {
	}

	virtual value_type eval(Context &CX) override {
		return (_left_child->get(CX) = _right_child->eval(CX));
	}
};

class ImVal : public INode {
	ValT m_val;
public:
	ValT eval(Context &CX) const override {
		return m_val;
	}
	template <typename T>
	ImVal(T val) : m_val(val) {}
};

}


