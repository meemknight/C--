#pragma once


#include <tokenizer.h>
#include <iostream>
#include <freeListAllocator.h>
#include <string_view>
#include <unordered_map>
#include <vector>



//we consider paranthases as unary expressions, and we'll keep the token set to paranthases and secondary type == '(' or '[' or '{',
//paranthases expressions have a single left child
struct Expression
{
	EmptyToken token = {};
	Expression *left=0;
	Expression *right=0;
	char *tokenString=0;

	std::string format();
};


void allocateMemoryForTheExpression(Expression &e, FreeListAllocator &allocator, bool allocateLeftExpression, bool allocateRightExpresiion,
	std::string_view string);


struct Statement
{
	EmptyToken token = {};
	
	int statementsCount = 0;
	int expressionsCount = 0;

	Statement *statements = 0; //the last expression is the next
	Expression *expressions = 0;

	char *statementText;
};

void allocateMemoryForTheStatement(Statement &e, FreeListAllocator &allocator,
	int statementsCount, int expressionsCount, const char *statementText = nullptr);


//
/*
Statement grammar

	program		-> block? EOF 

	block		-> "{" declaration* "}"

	declaration	-> varDeclaration | statement

	statement	-> printStatement | exprStmt | block | ifStmt | whileStmt


	----------------

	whileStmt		-> "while" "(" expression ")" statement ( "else" statement )?

	ifStmt			-> "if" "(" expression ")" statement ( "else" statement )?

	varDeclaration	-> TYPE_KEYWORD USER_IDENTIFIER ( "=" expression )? ";"

	exprStmt		-> expression ";"

	printStatement	-> "print" "(" expression ")" ";"


*/





//an expression is something like a + 10, string literals, etc

/*
Expression grammar:


//the gramar with no ambiguity

	expression		-> assignment ;
	assignment		-> (USER_DEFINED_IDENTIRIER "=" assignment) | equality
																	//todo add && here
																	//todo add & here ?
	equality		-> comparison ( ( "!=" | "==" ) comparison )* ;
	comparison		-> term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
	term			-> factor ( ( "-" | "+" ) factor )* ;
	factor			-> unary ( ( "/" | "*" | "%" ) unary )* ;
	unary			-> ( "!" | "-" | "~" ) unary | primary ;
	primary			-> NUMBER | STRING | "true" | "false" | USER_DEFINED_IDENTIRIER
				   | "(" expression ")" ;


*/


Expression createExpressionFromSingleToken(Token &token, FreeListAllocator &allocator, std::string &error);


struct Parser
{

	std::vector<Token> *tokens = 0;
	int position = 0;
	FreeListAllocator *allocator = 0;
	std::string err = "";

	void sincronize()
	{

		while (!isAtEnd())
		{
			if (previous().type == Token::Types::semicolin)return;

			auto t = peek();

			if(t.type == Token::Types::keyWord)
			{ return; }
		}
	}

	bool peek(Token t)
	{
		if (isAtEnd()) { return 0; }
		bool rez = t.type == tokens->at(position).type && t.secondaryType == tokens->at(position).secondaryType;
		return rez;
	}

	bool peek2(Token t)
	{
		if (position+1 >= tokens->size()) { return 0; }
		bool rez = t.type == tokens->at(position+1).type && t.secondaryType == tokens->at(position+1).secondaryType;
		return rez;
	}

	bool match(Token t)
	{
		bool rez = peek(t);
		if (rez)position++;
		return rez;
	}

	bool consume(Token t)
	{
		bool rez = match(t);
		if (!rez) err = "Expected: " + t.formatSimple();
		return rez;
	}

	Token peek()
	{
		if (tokens->size() <= position + 1) { return {}; }
	
		return tokens->at(position + 1);
	}

	bool isAtEnd() { return position >= tokens->size(); }

	Token previous()
	{
		return tokens->at(position - 1);
	}

	Expression primary()
	{
		if (!err.empty()) { return {}; }

		if (
			match(Token(Token::Types::number, Token::TypeNumber::int32)) ||
			match(Token(Token::Types::number, Token::TypeNumber::real32)) ||
			match(Token(Token::Types::number, Token::TypeNumber::boolean)) ||
			match(Token(Token::Types::userDefinedWord)) ||
			match(Token(Token::Types::stringLiteral, 0))
			)
		{
			return createExpressionFromSingleToken(previous(), *allocator, err);
		}
		
		if (match(Token(Token::Types::parenthesis, '(')))
		{
			Expression root = createExpressionFromSingleToken(previous(), *allocator, err);

			Expression exp = expression();
			if (!err.empty()) { return {}; }
			
			if(exp.token.type == 0)
			{
				err = "Parser Error: Empty ( ) ";
				return {};
			}

			if (match(Token(Token::Types::parenthesis, ')'))) //todo consume
			{
				*root.left = exp; 
				return root;
			}
			else
			{
				err = "Parser Error: Expected ')'";
				return {};
			}

		}

		err = "Parser Error: Expected an expresiion.";
		return {};
	}

	Expression unary()
	{
		if (!err.empty()) { return {}; }

		if (
			match(Token(Token::Types::op, Token::TypeOpperators::negation)) ||
			match(Token(Token::Types::op, Token::TypeOpperators::minus)) ||
			match(Token(Token::Types::op, Token::TypeOpperators::logicNot)))
		{
			Expression op = createExpressionFromSingleToken(previous(), *allocator, err);
			op.left = nullptr; //we don't want to anything here
			if (!err.empty()) { return {}; }

			Expression right = unary();
			if (!err.empty()) { return {}; }
			if (right.token.type == 0) { err = "Parser error: expected an unary token"; }

			*op.right = right;
			return op;
		}
		else
		{
			return primary();
		}
	}

	Expression factor()
	{
		if (!err.empty()) { return {}; }
		Expression left = unary();
		if (!err.empty()) { return {}; }
		if (left.token.type == 0) { err = "Expected a unary"; return{}; }

		while (match(Token(Token::Types::op, Token::TypeOpperators::multiplication))
			|| match(Token(Token::Types::op, Token::TypeOpperators::division))
			|| match(Token(Token::Types::op, Token::TypeOpperators::modulo))
			)
		{
			Expression op = createExpressionFromSingleToken(previous(), *allocator, err);
			if (!err.empty()) { return {}; }

			Expression right = unary();
			if (!err.empty()) { return {}; }
			if (right.token.type == 0) { err = "Parser error: expected an unary token"; return {}; }

			*op.left = left;
			*op.right = right;
			left = op;
		}

		return left;

	}

	Expression term()
	{
		if (!err.empty()) { return {}; }
		Expression left = factor();
		if (!err.empty()) { return {}; }
		if (left.token.type == 0) { err = "Expected a factor"; return{}; }


		while (match(Token(Token::Types::op, Token::TypeOpperators::plus))
			|| match(Token(Token::Types::op, Token::TypeOpperators::minus))
			)
		{
			Expression op = createExpressionFromSingleToken(previous(), *allocator, err);
			if (!err.empty()) { return {}; }

			Expression right = factor();
			if (!err.empty()) { return {}; }
			if (right.token.type == 0) { err = "Parser error: expected a factor token"; }


			*op.left = left;
			*op.right = right;
			left = op;
		}

		return left;

	}

	Expression comparison()
	{
		if (!err.empty()) { return {}; }
		Expression left = term(); //todo error out api

		while (match(Token(Token::Types::op, Token::TypeOpperators::greater))
			|| match(Token(Token::Types::op, Token::TypeOpperators::greaterEqual))
			|| match(Token(Token::Types::op, Token::TypeOpperators::less))
			|| match(Token(Token::Types::op, Token::TypeOpperators::leesEqual))
			)
		{
			Expression op = createExpressionFromSingleToken(previous(), *allocator, err);
			if (!err.empty()) { return {}; }
			Expression right = term();
			if (!err.empty()) { return {}; }
			if (right.token.type == 0) { err = "Parser error: expected a termen token"; }

			*op.left = left;
			*op.right = right;

			left = op;
		}

		return left;
	}

	Expression equality()
	{
		if (!err.empty()) { return {}; }
		
		Expression left = comparison();

		while (match(Token(Token::Types::op, Token::TypeOpperators::equals))
			|| match(Token(Token::Types::op, Token::TypeOpperators::notEquals))
			)
		{
			
			Expression op = createExpressionFromSingleToken(previous(), *allocator, err);
			if (!err.empty()) { return {}; }
			Expression right = comparison();
			if (!err.empty()) { return {}; }
			
			if (right.token.type == 0) { err = "Parser error: expected a comparison token"; }

			*op.left = left;
			*op.right = right;

			left = op;
		}

		return left;
	}

	Expression assignment()
	{
		//	assignment		-> (USER_DEFINED_IDENTIRIER "=" assignment) | equality
		if (!err.empty()) { return {}; }

		if (peek(Token(Token::Types::userDefinedWord))
			&& peek2(Token(Token::Types::op, Token::TypeOpperators::asignment))
			)
		{
			if (!consume(Token(Token::Types::userDefinedWord))) { return {}; }

			Expression var = createExpressionFromSingleToken(previous(), *allocator, err);
			if (!err.empty()) { return {}; }

			if (!consume(Token(Token::Types::op, Token::TypeOpperators::asignment))) { return {}; }

			Expression equals = createExpressionFromSingleToken(previous(), *allocator, err);
			if (!err.empty()) { return {}; }

			*equals.left = var;

			auto right = equality();
			if (!err.empty()) { return {}; }
			
			*equals.right = right;

			return equals;
		}
		else
		{
			return equality();
		}
	}

	Expression expression()
	{
		return assignment();
	}

	//
	Statement program()
	{
		if (!err.empty()) { return {}; }

		if (match(Token(Token::Types::eof))) { return {}; }

		Statement rez = block();
		if (!err.empty()) { return {}; }

		if (!consume(Token(Token::Types::eof))) { return {}; }

		return rez;
	}
	
	Statement block()
	{
		if (!err.empty()) { return {}; }

		if (!consume(Token(Token::Types::parenthesis, '{'))) { return {}; }

		Statement rez;
		rez.token.type = Token::Types::parenthesis;
		rez.token.secondaryType = '{';

		allocateMemoryForTheStatement(rez, *allocator, 2, 0);

		if (match(Token(Token::Types::parenthesis, '}')))
		{
			return rez;
		}
		
		Statement inside = declaration();
		rez.statements[0] = inside;
		Statement *currentStatement = &rez.statements[0];

		while (!match(Token(Token::Types::parenthesis, '}')))
		{
			inside = declaration();
			if (!err.empty()) { return {}; }
			
			currentStatement->statements[currentStatement->statementsCount - 1] = inside;
			currentStatement = &currentStatement->statements[currentStatement->statementsCount - 1];
		}

		return rez;

		//Statement open;
		//allocateMemoryForTheStatement(open, *allocator, 1, 0);
		//open.token.type = Token::Types::parenthesis;
		//open.token.secondaryType = '{';
		//
		//Statement *currentStatement = &open;
		//
		//while (!match(Token(Token::Types::parenthesis, '}')))
		//{
		//	Statement rezult = declaration();
		//	if (!err.empty()) { return {}; }
		//
		//	currentStatement->statements[currentStatement->statementsCount - 1] = rezult;
		//	currentStatement = &currentStatement->statements[currentStatement->statementsCount - 1];
		//	//currentStatement->statements[currentStatement->statementsCount - 1] = Statement{};
		//}
		//
		//Statement close;
		//allocateMemoryForTheStatement(close, *allocator, 1, 0);
		//close.token.type = Token::Types::parenthesis;
		//close.token.secondaryType = '}';
		//currentStatement->statements[currentStatement->statementsCount - 1] = close;
		//
		//return open;
	}

	Statement declaration()
	{
		if (!err.empty()) { return {}; }

		if (
			peek(Token(Token::Types::keyWord, Token::KeyWords::int32_)) ||
			peek(Token(Token::Types::keyWord, Token::KeyWords::float_)) ||
			peek(Token(Token::Types::keyWord, Token::KeyWords::bool_))
			)
		{
			return varDeclaration();
		}
		else
		{
			return statement();
		}


	}

	Statement statement()
	{
		if (!err.empty()) { return {}; }

		if (peek(Token(Token::Types::keyWord, Token::KeyWords::print)))
		{
			return printStatement();
		}else if(peek(Token(Token::Types::parenthesis, '{')))
		{
			return block();
		}
		else if (peek(Token(Token::Types::keyWord, Token::KeyWords::if_)))
		{
			return ifStmt();
		}
		else if (peek(Token(Token::Types::keyWord, Token::KeyWords::while_)))
		{
			return whileStmt();
		}else
		{
			return exprStmt();
		}
	}

	Statement whileStmt()
	{
		if (!err.empty()) { return {}; }
		if (!consume(Token(Token::Types::keyWord, Token::KeyWords::while_))) { return {}; }
		if (!consume(Token(Token::Types::parenthesis, '('))) { return {}; }

		Expression expr = expression();
		if (!err.empty()) { return {}; }

		if (!consume(Token(Token::Types::parenthesis, ')'))) { return {}; }

		Statement block = statement();
		if (!err.empty()) { return {}; }

		if (match(Token(Token::Types::keyWord, Token::KeyWords::else_)))
		{
			Statement ret;
			allocateMemoryForTheStatement(ret, *allocator, 3, 1);
			ret.token.type = Token::Types::keyWord;
			ret.token.secondaryType = Token::KeyWords::while_;
			ret.expressions[0] = expr;
			ret.statements[0] = block;

			Statement elseStatement = statement();
			if (!err.empty()) { return {}; }

			ret.statements[1] = elseStatement;

			return ret;
		}
		else
		{
			Statement ret;
			allocateMemoryForTheStatement(ret, *allocator, 2, 1);
			ret.token.type = Token::Types::keyWord;
			ret.token.secondaryType = Token::KeyWords::while_;
			ret.expressions[0] = expr;
			ret.statements[0] = block;

			return ret;
		}
	}

	Statement ifStmt()
	{
		if (!err.empty()) { return {}; }
		if (!consume(Token(Token::Types::keyWord, Token::KeyWords::if_))) { return {}; }
		if (!consume(Token(Token::Types::parenthesis, '('))) { return {}; }

		Expression expr = expression();
		if (!err.empty()) { return {}; }

		if (!consume(Token(Token::Types::parenthesis, ')'))) { return {}; }

		Statement firstBlock = statement();
		if (!err.empty()) { return {}; }

		if (match(Token(Token::Types::keyWord, Token::KeyWords::else_)))
		{
			Statement ret;
			allocateMemoryForTheStatement(ret, *allocator, 3, 1);
			ret.token.type = Token::Types::keyWord;
			ret.token.secondaryType = Token::KeyWords::if_;
			ret.expressions[0] = expr;
			ret.statements[0] = firstBlock;

			Statement elseStatement = statement();
			if (!err.empty()) { return {}; }

			ret.statements[1] = elseStatement;

			return ret;
		}
		else
		{
			Statement ret;
			allocateMemoryForTheStatement(ret, *allocator, 2, 1);
			ret.token.type = Token::Types::keyWord;
			ret.token.secondaryType = Token::KeyWords::if_;
			ret.expressions[0] = expr;
			ret.statements[0] = firstBlock;

			return ret;
		}
	}

	Statement varDeclaration()
	{
		if (!err.empty()) { return {}; }

		int type = 0;
		if (match(Token(Token::Types::keyWord, Token::KeyWords::int32_)))
		{
			type = Token::TypeNumber::int32;
		}
		else if (match(Token(Token::Types::keyWord, Token::KeyWords::float_)))
		{
			type = Token::TypeNumber::real32;
		}
		else if (match(Token(Token::Types::keyWord, Token::KeyWords::bool_)))
		{
			type = Token::TypeNumber::boolean;
		}
		else
		{
			err = "Internal parser error: expected a var type.";
			return {};
		}

		if (!consume(Token(Token::Types::userDefinedWord))) { return {}; }

		std::string varName = previous().text; if (varName.empty()) { err = "Internal parser error: varName is empty"; return {}; }
		
		if (match(Token(Token::Types::op, Token::TypeOpperators::asignment)))
		{
			Expression expr = expression();

			Statement rezult;
			allocateMemoryForTheStatement(rezult, *allocator, 1, 1, varName.c_str());
			rezult.token.type = Token::Types::varDeclaration;
			rezult.token.secondaryType = type;
			rezult.expressions[0] = expr;
			if (!consume(Token(Token::Types::semicolin))) { return {}; }
			return rezult;
		}
		else
		{

			Statement rezult;
			allocateMemoryForTheStatement(rezult, *allocator, 1, 0, varName.c_str());
			rezult.token.type = Token::Types::varDeclaration;
			rezult.token.secondaryType = type;
			if (!consume(Token(Token::Types::semicolin))) { return {}; }
			return rezult;
		}

	}

	Statement exprStmt()
	{
		auto expressionRez = expression(); if (!err.empty()) { return {}; }

		Statement rezult;
		allocateMemoryForTheStatement(rezult, *allocator, 1, 1);
		rezult.token.type = Token::Types::expressionStatement;
		rezult.expressions[0] = expressionRez;

		if (!consume(Token(Token::Types::semicolin))) { return {}; }
		return rezult;


	}

	Statement printStatement()
	{
		if (!err.empty()) { return {}; }

		if (!consume(Token(Token::Types::keyWord, Token::KeyWords::print))) { return {}; }
		if (!consume(Token(Token::Types::parenthesis, '('))) { return {}; }

		auto expressionRez = expression(); if (!err.empty()) { return {}; }
		
		Statement rezult;
		allocateMemoryForTheStatement(rezult, *allocator, 1, 1);

		rezult.token.type = Token::Types::keyWord;
		rezult.token.secondaryType = Token::KeyWords::print;
		rezult.expressions[0] = expressionRez;

		if (!consume(Token(Token::Types::parenthesis, ')'))) { return {}; }
		if (!consume(Token(Token::Types::semicolin))) { return {}; }

		return rezult;
	}



};

struct Value
{
	union
	{
		int i = 0;
		float f;
		char* string;
	}reprezentation;

	enum
	{
		none = 0,
		int32,
		real32,
		boolean,
		string,
	};

	std::string formatType()
	{
		const char *names[5] = {
			"none",
			"int32",
			"real32",
			"boolean",
			"string"
		};
		return names[type];
	}

	std::string formatValue()
	{
		std::string rez;
		if (isNone()) {}else
		if (isInt32()) {rez += std::to_string(reprezentation.i);}else
		if (isReal32()) { rez += std::to_string(reprezentation.f); }else
		if (isString()) { rez += reprezentation.string; }else
		if (isBool()) { rez += reprezentation.i != 0 ? "True" : "False"; }
		else { assert(0); }
		return rez;
	}

	std::string format()
	{
		std::string rez = formatType() + ": ";
		
		rez += formatValue();

		return rez;
	}

	bool isBool() { return type == boolean; }
	bool isInt32() { return type == int32; }
	bool isReal32() { return type == real32; }
	bool isString() { return type == string; }
	bool isNone() { return type == 0 || type > string; }

	bool toBool()
	{
		if (isNone() || isString()) { return 0; }
		
		if (isBool()) { return 1; }
		else if(isReal32())
		{
			type = boolean;
			reprezentation.i = (bool)reprezentation.f;
			return 1;
		}
		else if (isInt32())
		{
			type = boolean;
			reprezentation.i = (bool)reprezentation.i;
			return 1;
		}
		else
		{
			assert(0);
			return 0;
		}
	}

	bool toInt32()
	{
		if (isNone() || isString()) { return 0; }
		if (isInt32()) { return true; }
		else if (isReal32())
		{
			type = int32;
			reprezentation.i = reprezentation.f;
			return 1;
		}
		else if (isBool())
		{
			type = int32;
			reprezentation.i = (bool)reprezentation.i;
			return 1;
		}
		else
		{
			assert(0);
			return 0;
		}
	}

	bool toReal32()
	{
		if (isNone() || isString()) { return 0; }
		if (isReal32()) { return true; }
		else if (isInt32())
		{
			type = real32;
			reprezentation.f = reprezentation.i;
			return 1;
		}
		else if (isBool())
		{
			type = real32;
			reprezentation.f = (bool)reprezentation.i;
			return 1;
		}
		else
		{
			assert(0);
			return 0;
		}
	}


	char type = 0;

	bool createFromToken(Expression &t)
	{
		*this = {};
		if (t.token.type == Token::Types::number)
		{
			if (t.token.secondaryType == Token::TypeNumber::int32) { type = int32; }else
			if (t.token.secondaryType == Token::TypeNumber::real32) { type = real32; }else
			if (t.token.secondaryType == Token::TypeNumber::boolean) { type = boolean; }
			else { return 0; }

			static_assert(sizeof(reprezentation) == sizeof(Token::reprezentation), "");

			memcpy(&reprezentation, &t.token.reprezentation, sizeof(reprezentation));

		}
		else if (t.token.type == Token::Types::stringLiteral)
		{
			type = string;
			reprezentation.string = t.tokenString;
		}
		else
		{
			return 0;
		}
	}


	bool equals(Value other, std::string &err);
	bool notEquals(Value other, std::string &err);
	bool greater(Value other, std::string &err);
	bool greaterEqual(Value other, std::string &err);
	bool smaller(Value other, std::string &err);
	bool smallerEqual(Value other, std::string &err);

	void matchOpperatorsUnsafe(Value &other)
	{
		if (this->isReal32() || other.isReal32())
		{
			this->toReal32();
			other.toReal32();
		}
		else if (this->isInt32() || other.isInt32())
		{
			this->toInt32();
			other.toInt32();
		}else if (this->isBool() && other.isBool())
		{
		}
		else
		{
			assert(0); //should not reach here
		}

	}

};


struct Variables
{
	std::vector<std::unordered_map<std::string, Value>> variabels;

	Value *getVariable(std::string name)
	{
		for (int i = variabels.size() - 1; i >= 0; i--)
		{
			auto it = variabels[i].find(name);
			if (it != variabels[i].end()) { return &it->second; }
		}
		return 0;
	}

	bool addVariable(std::string name, Value v)
	{
		if (variabels.empty()) { return 0; }

		auto it = variabels.back().find(name);
		if (it == variabels.back().end()) 
		{
			variabels.back()[name] = v;
			return 1; 
		}
		else
		{
			return 0;
		}
	}

	void push() 
	{
		variabels.push_back({});
	};

	bool pop() 
	{
		if (!variabels.empty())
		{
			variabels.pop_back();
			return 1;
		}
		else
		{
			return 0;
		}
	};
};



Value evaluateExpression(Expression *e, std::string &err, Variables &vars);
void testEvaluate(std::string language);


void parse(std::vector<Token> &tokens);
