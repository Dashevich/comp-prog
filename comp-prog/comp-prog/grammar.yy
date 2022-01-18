%language "c++"
%require "3.2"

%define api.token.raw

%code requires {
	#include "ast.hh"
	namespace AST
}	

%define api.token.raw
%define api.value.type { INode* }
%parse-param {INode*& ast}
%parse-param {yy::parser::semantic_type &ast}


%code provides {
	#define YYSTYPE yy::parser::semantic_type
}

%code {
	// Give Flex the prototype of yylex we want ...
	#define YY_DECL \
		yy::parser::token::yytokentype yylex(yy::parser::semantic_type *yylval)
	// ... and declare it for the parser's sake.
	YY_DECL;
}

%define parse.error verbose
%define parse.lac full


%define api.token.prefix {TOK_}
%token
	RETURN
	SPLIT
	VAR
	ASSIGN
	PLUS
	MINUS
	MULT
	DIV
	MOD
	LPAR
	RPAR
	NUM
	FUNCTION
%nterm expr

%left ASSIGN
%left SPLIT
%left PLUS
%left MINUS
%left MULT
%left DIV
%left MOD

%start program
%%
program: expr SPLIT { ast = new Scope($1); }
;

expr:	NUM { $$ = $1; }
    |   FUNCTION VAR LPAR expr RPAR expr {$$ = new Function($6, $2, $4); }
    |   FUNCTION VAR LPAR RPAR expr {$$ = new Function($5, $2, nullptr); }
    |	RETURN LPAR expr RPAR {$$ = new Return($3); }
    |	VAR { $$ = new Variable($1); }
    |	LPAR expr RPAR { $$ = $2; }
    |	expr SPLIT expr { $$ = new Splitted($1, $3); }
    |	expr PLUS expr { $$ = new Plus($1, $3); }
    |	expr ASSIGN expr { $$ = new Assign((Variable*)$1, $3); }
;

%%

void yy::parser::error(const std::string &err_message) {
	std::cerr << "Error: " << err_message << std::endl;
}
