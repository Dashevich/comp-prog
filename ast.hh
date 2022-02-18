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
	std::vector<value_type> nums;
	std::map<std::string, std::pair<INode*, INode*>> funcs;
	using MapT = std::map<std::string, ValT>;
	std::vector<MapT> ScopeStack;
	std::vector<std::map<std::string, value_type>> vars;
public:
	value_type& get(std::string var) {
		for(int i = vars.size() - 1; i >= 0; i--) {
			if (vars[i].find(var) != vars[i].end()) {
				return vars[i][var];
			}
		}
		
		return vars.back()[var];
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
	std::pair<INode*, INode*> getFunc(std::string name) {
		return funcs[name];
	}
	void setRes(value_type v) {
		nums[nums.size() - 1] = v;
	}
	void addScope() {
		vars.push_back({});
		nums.push_back(0);
	}
	value_type getRes() {
		return nums.back();
	}
};
class INode {
public:
	virtual ValT eval(Context &CX) const = 0;
};

template <typename Op>
class BinOp : public INode {
	Op m_op;
	std::unique_ptr<INode> m_lhs;
	std::unique_ptr<INode> m_rhs;
public:
	ValT eval(Context &CX) const override {
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
	virtual value_type eval(Context& CX) const override {
		CX.addScope();
		_expr->eval(CX);
		value_type res = CX.getRes();
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

	value_type eval(Context& CX) {
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
	virtual value_type eval(Context &CX) const override {
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
	virtual value_type eval(Context &CX) const override {
		CX.addFunc(((NameGive*)_name)->getName(), _expr, _arguments);
		return 0;
	}
};

class FunctionCall : public INode { 
	INode* _arguments;
	INode* _name;
public:
	FunctionCall(INode* argsTuple, INode* name) : _arguments(argsTuple), _name(name) {}

	virtual value_type eval(Context& CX) const override {
		std::pair<INode*, INode*> funcInfo = CX.getFunc(((NameGive*)_name)->getName());
		INode* expr = funcInfo.first;
		INode* argNames = funcInfo.second;
		
		CX.addScope();
		Tuple* current = (Tuple*)_arguments;
		Tuple* currentName = (Tuple*)argNames;
		while(current != nullptr && currentName != nullptr) {
			NameGive* name = (NameGive*)currentName->getCurrent();
			value_type val = current->eval(CX);
			CX.get(name->getName()) = val;

			current = (Tuple*)current->getNext();
			currentName = (Tuple*)currentName->getNext();
		}
		if (current != nullptr) throw 1;
		if (currentName != nullptr) throw 1;

		value_type res = expr->eval(CX);
		CX.leaveScope();
		return res;
	}
};

class Return : public INode {
	INode* _expr;
public:
	Return(INode* expr) : _expr(expr) {}
	virtual value_type eval(Context &CX) const override {
		value_type res = _expr->eval(CX);
		CX.setRes(res);
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
	value_type eval(Context &CX) const override {
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
	virtual value_type eval(Context &CX) const override {
		return (_left_child->get(CX) = _right_child->eval(CX));
	}
};

class Log : public INode {
private:
	INode* _expr;
public:
	Log(INode* lc) : _expr(lc) {
	}

	virtual value_type eval(Context& CX) const override {
		value_type res = _expr->eval(CX);			
		std::cout << res << std::endl;
		return res;
	}
};

class Value : public INode {
private:
	value_type _value;
public:
	Value(value_type val) : _value(val) {}

	virtual value_type eval(Context& CX) const override {
		return _value;
	}
};
}


