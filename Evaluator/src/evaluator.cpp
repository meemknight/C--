#include <evaluator.h>
#include <string>
#include <freeListAllocator.h>
#include <tokenizer.h>
#include <parser.h>
#include <iostream>
#include <unordered_map>


void exectueFromLanguageString(std::string language)
{

	FreeListAllocator allocator;
	allocator.init(new unsigned char[MB(100)], MB(100));

	auto tokens = tokenize(language);

	bool goodFlag = 1;
	for (int i = 0; i < tokens.size(); i++)
	{
		if (tokens[i].type == Token::Types::error)
		{
			std::cout << "Tokenizer error: " + tokens[i].text << "\n";
			goodFlag = 0;
		}
		else if (tokens[i].type == Token::Types::comment)
		{
			tokens.erase(tokens.begin() + i);
			i--;
		}
	}

	if (goodFlag)
	{

		tokens.push_back(Token(Token::Types::eof));

		Parser parser;
		parser.allocator = &allocator;
		parser.tokens = &tokens;

		auto ast = parser.program();
		if (!parser.err.empty()) { std::cout << "Parser error: " << parser.err << "\n"; }
		else
		{
			Variables variables;
			std::string err = "";
			execute(ast, err, variables);

			if (!err.empty()) { std::cout << "Runtime error: " << err << "\n"; }

		}
	}

	delete[] allocator.initialBaseMemory;

}

bool execute(Statement program, std::string &err, Variables &variables)
{

	Statement *currentStatement = &program;


	while (currentStatement->token.type != 0)
	{

		switch (currentStatement->token.type)
		{

		case Token::Types::keyWord:

			switch(currentStatement->token.secondaryType)
			{
				case Token::KeyWords::print:
				{
					if (currentStatement->expressionsCount != 1)
					{
						err = "Internal evaluator error: print expects 1 expression";
						return 0;
					}

					Value v = evaluateExpression(&currentStatement->expressions[0], err, variables);
					if (!err.empty()) { return 0; }

					std::cout << v.formatValue() << "\n";
					break;
				}

			default:
			err = "Internal evaluator error: unexpected token secondary type for the statement execution: " +
				currentStatement->token.format();
			return 0;
			}
		break;

		case Token::Types::expressionStatement:
		{
			
			evaluateExpression(&currentStatement->expressions[0], err, variables);
			if (!err.empty()) { return 0; }
			break;
		}
		break;

		case Token::Types::varDeclaration:
		{
			
			Value rez;

			std::string varName = currentStatement->statementText;

			if (currentStatement->expressionsCount == 1)
			{
				rez = evaluateExpression(&currentStatement->expressions[0], err, variables);
				if (!err.empty()) { return 0; }

				if (currentStatement->token.secondaryType == Token::TypeNumber::int32)
				{
					rez.toInt32();
				}else
				if (currentStatement->token.secondaryType == Token::TypeNumber::real32)
				{
					rez.toReal32();
				}else
				if (currentStatement->token.secondaryType == Token::TypeNumber::boolean)
				{
					rez.toBool();
				}else
				{
					err = "Internal evaluator err: unexpected type in var declaration.";
				}
			}
			else if (currentStatement->expressionsCount == 0)
			{
				if (currentStatement->token.secondaryType == Token::TypeNumber::int32)
				{
					rez.type = Value::int32;
				}else
				if (currentStatement->token.secondaryType == Token::TypeNumber::real32)
				{
					rez.type = Value::real32;
				}if (currentStatement->token.secondaryType == Token::TypeNumber::boolean)
				{
					rez.type = Value::boolean;
				}
				else
				{
					err = "Internal evaluator err: unexpected type in var declaration2.";
				}
			}
			else { assert(0); }


			if (!variables.addVariable(varName, rez))
			{
				err = "Variable already declared: " + varName;
				return 0;
			}
			//std::cout << "declared variable: " << currentStatement->statementText << " = " << v.format() << "\n";

		}
		break;

		case Token::Types::parenthesis:
		{
			if (currentStatement->token.secondaryType == '{')
			{
				if (currentStatement->statements[0].token.type == 0)
				{
					//empty paranthases
				}
				else
				{
					variables.push();

					auto rez = execute(currentStatement->statements[0], err, variables);

					if (!rez)
					{
						return 0;
					}

					if (!variables.pop())
					{
						err = "Internal compiler error: stack underflow.";
						return 0;
					}
				}
			}
			else
			{
				err = "Internal compiler error: parenthesis type for the statement execution: " +
					currentStatement->token.format();
				return 0;
			}


		}
		break;

		default:
		err = "Internal compiler error: unexpected token type for the statement execution: " +
			currentStatement->token.format();
		return 0;
		}

		currentStatement = &currentStatement->statements[currentStatement->statementsCount - 1];


	}

	return true;

}