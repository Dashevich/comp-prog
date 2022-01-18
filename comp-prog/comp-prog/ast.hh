#ifndef AST
#define AST
#include <map>
#include <vector>
#include <set>
#include <iostream>
#include <string>

typedef int _type_id;
typedef int value_type;
typedef int err_type;

#if DEBUG_MODE
	#define debug(x) std::cout << (x) << std::endl;
#else
	#define debug(x) ;
#endif

#pragma once
#include "inode.hh"

namespace AST {

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
		debug("Splitted initialized");
	}

	virtual value_type eval(Context &CX) override {
		debug("Splitted called");
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
		debug("Assign initialized");
	}

	virtual value_type eval(Context &CX) override {
		debug("Assign called");
		return (_left_child->get(CX) = _right_child->eval(CX));
	}
};

class Value : public INode {
private:
	value_type _value;
public:
	Value(value_type val) : _value(val) {}

	virtual value_type eval(Context &CX) override {
		debug("Value called " << _value);
		return _value;
	}
};
}
#endif

