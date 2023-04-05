#pragma once


#include <tokenizer.h>
#include <iostream>
#include <freeListAllocator.h>


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
	std::string_view string)
{
	e.left = 0;
	e.right = 0;
	e.tokenString = 0;
	if (!allocateLeftExpression && !allocateRightExpresiion && string.size() == 0)
	{
		//don't allocate
	}
	else
	{
		if (!allocateLeftExpression && !allocateRightExpresiion)
		{
			//just allocate memory for the string
			e.tokenString = (char*)allocator.allocate(string.size() + 1);
			strcpy(e.tokenString, string.data());
		}
		else
		{
			//alco allocate memory for data
			static_assert(sizeof(Expression) < 64u);

			size_t totalSize = 64 * 2 + string.size() + 1;
			void *allocatedData = allocator.allocate(totalSize);
			
			if (allocateLeftExpression)e.left = (Expression*)allocatedData;
			if(allocateRightExpresiion)e.right = (Expression *)((unsigned char *)allocatedData + 64);
			e.tokenString = (char *)((unsigned char*)allocatedData+64*2);
			strcpy(e.tokenString, string.data());
		}
	}

}

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


Expression createExpressionFromSingleToken(Token &token, FreeListAllocator &allocator, std::string &error)
{
	Expression returnVal= {};
	error = "";

	returnVal.token.type = token.type;
	returnVal.token.secondaryType = token.secondaryType;
	returnVal.token.begin = token.begin;
	memcpy(&returnVal.token.reprezentation, &token.reprezentation, sizeof(token.reprezentation));

	static_assert(sizeof(returnVal.token.reprezentation) ==  sizeof(token.reprezentation));

	switch (token.type)
	{
	case Token::Types::none:
	assert(0); //we should not get here
	break;
	case Token::Types::error:
	assert(0); //we should not get here
	break;
	case Token::Types::comment:
	assert(0); //we should not get here
	break;
	case Token::Types::parenthesis:
		if (
			returnVal.token.secondaryType != '(' &&
			returnVal.token.secondaryType != '{' &&
			returnVal.token.secondaryType != '['
			)
		{
		assert(0);
		}
		else
		{
			allocateMemoryForTheExpression(returnVal, allocator, 1, 0, "");
		}
	break;
	case Token::Types::semicolin:
	assert(0); //we should not get here
	break;
	case Token::Types::stringLiteral:
	allocateMemoryForTheExpression(returnVal, allocator, 0, 0, token.text);
	break;
	case Token::Types::op:
	//lets see what kind of opperator we have

		switch (token.secondaryType)
		{
		case Token::TypeOpperators::plus: 
		case Token::TypeOpperators::minus: //the minus opperator can also be unary
		case Token::TypeOpperators::multiplication: 
		case Token::TypeOpperators::division: 
		case Token::TypeOpperators::modulo: 
		case Token::TypeOpperators::asignment: 
		case Token::TypeOpperators::equals: 
		case Token::TypeOpperators::and:
		case Token::TypeOpperators::or: 
		case Token::TypeOpperators::logicAnd:
		case Token::TypeOpperators::logicor: 
		case Token::TypeOpperators::logicxor:
		case Token::TypeOpperators::less:
		case Token::TypeOpperators::leesEqual:
		case Token::TypeOpperators::greater:
		case Token::TypeOpperators::greaterEqual:
			allocateMemoryForTheExpression(returnVal, allocator, 1, 1, token.text); 
			
		break;
		

		case Token::TypeOpperators::negation:
		case Token::TypeOpperators::logicNot: 
			allocateMemoryForTheExpression(returnVal, allocator, 0, 1, token.text); //we have something on the right
			
		break;


		default:
		assert(0); //we should not get here
		break;

		}

	break;
	case Token::Types::number:
	//
	break;
	case Token::Types::keyWord:
	assert(0); //we should not get here
	break;
	case Token::Types::userDefinedWord:
	assert(0); //we should not get here
	break;
	assert(0); //we should not get here
	default:
	break;
	}

	return returnVal;
}


struct Parser
{

	std::vector<Token> *tokens = 0;
	int position = 0;
	FreeListAllocator *allocator = 0;
	std::string err = "";


	bool match(Token t)
	{
		if (isAtEnd()) { return 0; }
		
		bool rez = t.type == tokens->at(position).type && t.secondaryType == tokens->at(position).secondaryType;
		
		if (rez)position++;
		
		return rez;
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

			if (match(Token(Token::Types::parenthesis, ')')))
			{
				*root.left = exp; 
			}
			else
			{
				err = "Parser Error: Expected ')'";
				return {};
			}

		}

		return {}; //?
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

		while (match(Token(Token::Types::op, Token::TypeOpperators::multiplication))
			|| match(Token(Token::Types::op, Token::TypeOpperators::division))
			|| match(Token(Token::Types::op, Token::TypeOpperators::modulo))
			)
		{
			Expression op = createExpressionFromSingleToken(previous(), *allocator, err);
			if (!err.empty()) { return {}; }

			Expression right = unary();

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

		while (match(Token(Token::Types::op, Token::TypeOpperators::plus))
			|| match(Token(Token::Types::op, Token::TypeOpperators::minus))
			)
		{
			Expression op = createExpressionFromSingleToken(previous(), *allocator, err);
			if (!err.empty()) { return {}; }

			Expression right = factor();

			*op.left = left;
			*op.right = right;
			left = op;
		}

		return left;

	}

	Expression comparison()
	{
		if (!err.empty()) { return {}; }
		Expression left = term();

		while (match(Token(Token::Types::op, Token::TypeOpperators::greater))
			|| match(Token(Token::Types::op, Token::TypeOpperators::greaterEqual))
			|| match(Token(Token::Types::op, Token::TypeOpperators::less))
			|| match(Token(Token::Types::op, Token::TypeOpperators::leesEqual))
			)
		{
			Expression op = createExpressionFromSingleToken(previous(), *allocator, err);
			if (!err.empty()) { return {}; }

			Expression right = term();

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




void parse(std::vector<Token> &tokens)
{

	FreeListAllocator allocator;
	allocator.init(new unsigned char[MB(100)], MB(100));
	

	// ( 1 + 2 )
	//std::string errors = "";
	//Expression expression = createExpressionFromSingleToken(tokenize("(")[0], allocator, errors);
	//
	//
	//*expression.left = createExpressionFromSingleToken(tokenize("+")[0], allocator, errors);
	//*expression.left->left = createExpressionFromSingleToken(tokenize("1")[0], allocator, errors);
	//*expression.left->right = createExpressionFromSingleToken(tokenize("2")[0], allocator, errors);
	////
	//std::cout << expression.format() << "\n";


	Parser parser;
	parser.allocator = &allocator;
	parser.tokens = &tokens;


	auto rez = parser.expression();

	std::cout << rez.format() << "\n";


	std::cin.get();
}