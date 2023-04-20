#include <evaluator.h>
#include <string>
#include <freeListAllocator.h>
#include <tokenizer.h>
#include <parser.h>
#include <iostream>




void exectueFromLanguageString(std::string language)
{

	FreeListAllocator allocator;
	allocator.init(new unsigned char[MB(100)], MB(100));

	auto tokens = tokenize(language);

	bool goodFlag = 1;
	for (auto &t : tokens)
	{
		if (t.type == Token::Types::error)
		{
			std::cout << "Tokenizer error: " + t.text << "\n";
			goodFlag = 0;
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
			std::string err = "";
			execute(ast, err);

			if (!err.empty()) { std::cout << "Runtime error: " << err << "\n"; }

		}
	}

	std::cin.get();
	delete[] allocator.initialBaseMemory;

}

bool execute(Statement program, std::string &err)
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

					Value v = evaluateExpression(&currentStatement->expressions[0], err);
					if (!err.empty()) { return 0; }

					std::cout << v.format() << "\n";
					break;
				}

			default:
			err = "Internal evaluator error: unexpected token secondary type for the statement execution: " +
				currentStatement->token.format();
			return 0;
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