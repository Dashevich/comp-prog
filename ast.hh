#pragma once
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
	std::vector<ValT> nums;
	std::map<std::string, std::pair<INode*, INode*>> funcs;
	using MapT = std::map<std::string, ValT>;
	std::vector<MapT> ScopeStack;
	std::vector<std::map<std::string, ValT>> vars;
public:
	ValT& get(const std::string &name) {
		for (auto &Scope : ScopeStack) {
			auto It = Scope.find(name);
			if (It != Scope.end())
				return It->second;
		}
		return ScopeStack.back()[name];
	}
	void leaveScope() {
		ScopeStack.pop_back();
		nums.pop_back();
	}
	void addFunc(std::string name, INode* func, INode* args) {
		funcs[name] = std::make_pair(func, args);
	}
	std::pair<INode*, INode*> getFunc(std::string name) {
		if (funcs.find(name) == funcs.end()) {
			throw 1;
		}
		return funcs[name];
	}
	void setRes(ValT v) {
		nums[nums.size() - 1] = v;
	}
	void addScope() {
		ScopeStack.push_back({});
		nums.push_back(0);
	}
	ValT getRes() {
		return nums.back();
	}
};
class INode {
public:
	virtual ValT eval(Context &CX) = 0;
};

template <typename Op>
class BinOp : public INode {
	Op m_op;
	std::unique_ptr<INode> m_lhs;
	std::unique_ptr<INode> m_rhs;
public:
	ValT eval(Context &CX) override {
		auto lhs = m_lhs->eval(CX);
		auto rhs = m_rhs->eval(CX);
		return m_op(lhs, rhs);
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
	virtual ValT eval(Context& CX) override {
		CX.addScope();
		_expr->eval(CX);
		ValT res = CX.getRes();
		CX.leaveScope();
		return res;
	}
};

class Tuple : public INode {
	INode* _current; INode* _next;
	size_t size;
public:
	Tuple(INode* c, INode* next) : _current(c), _next(next) {
		size = 1 + ((Tuple*)_next)->size;
	}

	ValT eval(Context& CX) {
		return _current->eval(CX);
	}

	INode* getNext() {
		return _next;
	}

	INode* getCurrent() {
		return _current;
	}
};

class NameGive : public INode {
	std::string _name;
public:
	NameGive(std::string name) : _name(name) {}
	virtual ValT eval(Context &CX) override {
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
	virtual ValT eval(Context &CX) override {
		CX.addFunc(((NameGive*)_name)->getName(), _expr, _arguments);
		return 0;
	}
};

class FunctionCall : public INode { 
	INode* _arguments;
	INode* _name;
public:
	FunctionCall(INode* argsTuple, INode* name) : _arguments(argsTuple), _name(name) {}

	virtual ValT eval(Context& CX) override {
		std::pair<INode*, INode*> funcInfo = CX.getFunc(((NameGive*)_name)->getName());
		INode* expr = funcInfo.first;
		INode* argNames = funcInfo.second;
		
		CX.addScope();
		Tuple* current = (Tuple*)_arguments;
		Tuple* currentName = (Tuple*)argNames;
		while(current != nullptr && currentName != nullptr) {
			NameGive* name = (NameGive*)currentName->getCurrent();
			ValT val = current->eval(CX);
			CX.get(name->getName()) = val;

			current = (Tuple*)current->getNext();
			currentName = (Tuple*)currentName->getNext();
		}
		if (current != nullptr) throw 1;
		if (currentName != nullptr) throw 1;

		ValT res = expr->eval(CX);
		CX.leaveScope();
		return res;
	}
};

class Return : public INode {
	INode* _expr;
public:
	Return(INode* expr) : _expr(expr) {}
	virtual ValT eval(Context &CX) override {
		ValT res = _expr->eval(CX);
		CX.setRes(res);
		return res;
	}
};

class Variable : public INode {
private:
	INode* _name;
public:
	Variable(INode* name) : _name(name) {}
	ValT& get(Context &CX) {
		return CX.get(((NameGive*)_name)->getName());
	}
	ValT eval(Context &CX) override {
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
	virtual ValT eval(Context &CX) override {
		return (_left_child->get(CX) = _right_child->eval(CX));
	}
};

class Log : public INode {
private:
	INode* _expr;
public:
	Log(INode* lc) : _expr(lc) {
	}

	virtual ValT eval(Context& CX) override {
		ValT res = _expr->eval(CX);			
		std::cout << res << std::endl;
		return res;
	}
};

class Value : public INode {
private:
	ValT _value;
public:
	Value(ValT val) : _value(val) {}

	virtual ValT eval(Context& CX) override {
		return _value;
	}
};
}


