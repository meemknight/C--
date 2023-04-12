#pragma once


#include <tokenizer.h>
#include <iostream>
#include <freeListAllocator.h>
#include <string_view>

//we consider paranthases as unary expressions, and we'll keep the token set to paranthases and secondary type == '(' or '[' or '{',
//paranthases expressions have a single left child
struct Expression
{
	EmptyToken token;
	Expression *left;
	Expression *right;
	char *tokenString;

	std::string format();
};

std::string Expression::format()
{
	
	if (left || right)
	{
		if (token.type == Token::Types::parenthesis)
		{
			std::string rez;
			if (token.secondaryType == '(')
			{
				rez = "( ";
				std::string r = left->format();
				rez += r;
				rez += ")";
			}else
			if (token.secondaryType == '{')
			{
				rez = "{ ";
				rez += left->format();
				rez += "}";
			}
			else
			if (token.secondaryType == '[')
			{
				rez = "[ ";
				rez += left->format();
				rez += "]";
			}
			else
			{
				return "internal error in expression::format at paranthase";
			}

			return rez;
		}
		else
		{
			std::string rez = "";
			if (left)
			{
				rez += left->format() + " ";
			}

			Token t = token.toToken(tokenString);
			rez += t.formatSimple();

			if (right)
			{
				rez += " " + right->format();
			}			

			return rez;
		}

	}
	else
	{
		//simple expression
		Token t = token.toToken(tokenString);
		return t.formatSimple();
	}

};


void allocateMemoryForTheExpression(Expression &e, FreeListAllocator &allocator, bool allocateLeftExpression, bool allocateRightExpresiion,
	std::string_view string);


//an expression is something like a + 10, string literals, etc

/*
Expression grammar:


	EXPRESSION = 
					intLiteral | string literal | booleanLiteral | 

					identifier |

					( + EXPRESSION + ) |

					identifier + ( + FUNCTION_PARAMETERS + ) |   //function call expression, for now you can only pass identifiers, might promote to statement?

					EXPRESSION + BINARY_OPPERATOR + EXPRESSION | 

					UNARY_OPPERATOR + EXPRESSION




	FUNCTION_PARAMETERS = EXPRESSION + (, + EXPRESSION)* | LAMBDA

	BINARY_OPPERATOR =	 = + - * / % == && & || | ^ < <= > >=

	UNARY_OPPERATOR = ! ~ -


//the gramar with no ambiguity

	expression     -> equality ;
	equality       -> comparison ( ( "!=" | "==" ) comparison )* ;
																	//todo add && here and the family
	comparison     -> term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
	term           -> factor ( ( "-" | "+" ) factor )* ;
	factor         -> unary ( ( "/" | "*" | "%" ) unary )* ;
	unary          -> ( "!" | "-" | "~" ) unary | primary ;
	primary        -> NUMBER | STRING | "true" | "false"
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

	bool match(Token t)
	{
		if (isAtEnd()) { return 0; }
		
		bool rez = t.type == tokens->at(position).type && t.secondaryType == tokens->at(position).secondaryType;
		
		if (rez)position++;
		
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

	Expression expression()
	{
		return equality();
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

	}

	bool toReal32()
	{

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
};


Value evaluate(Expression *e, std::string &err);


void parse(std::vector<Token> &tokens);
