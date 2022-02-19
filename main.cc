#include "grammar.tab.hh"
#include <iostream>

int main() {
	INode* res;
	yy::parser parse(res);
	parse();
	AST::Context CX;
	value_type code = res->eval(CX);
	std::cout << std::endl << "Program finished with exit code " << code << std::endl;
}
