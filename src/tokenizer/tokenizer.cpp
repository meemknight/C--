#include "tokenizer.h"


std::vector<Token> tokenize(const std::string_view &input)
{
	
	std::vector<Token> ret;
	ret.reserve(1000);
	
	TextPosition currentTextPosition;
	TextPosition startTextPosition;
	Token currentToken(Token::Types::none);
	int currentParsingType = 0;
	
	enum ParsingTypes
	{
		none = 0,
		word,
		number,
		op,
		stringLiteral,
	};
	
	auto parsingNumber = [&]() { return currentParsingType == number; };
	auto parsingWord = [&]() { return currentParsingType == word; };
	auto parsingOperator = [&]() { return currentParsingType == op; };
	auto parsingStringLiteral = [&]() { return currentParsingType == stringLiteral; };
	auto parsingNone = [&]() { return currentParsingType == none; };
	
	auto endCurrentToken = [&]()
	{
		currentToken.begin = startTextPosition;
		currentToken.end = currentTextPosition;
		startTextPosition = currentTextPosition;
	
		if (currentToken.isEmpty()) { return; }
	
		if (currentToken.type == Token::Types::error)
		{
			ret.push_back(currentToken);
		}
		else if (currentToken.type == Token::Types::stringLiteral)
		{
			if (!currentToken.stringLiteralClosed)
			{
				currentToken.type = Token::Types::error; //error parsing string literal, not closed
				currentToken.text = "Error parsing string iteral not closed.";
				ret.push_back(currentToken);
			}
			else
			{
				ret.push_back(currentToken);
			}
		}
		else if (currentToken.type == Token::Types::number)
		{
			currentToken.parseAsNumber();
			ret.push_back(currentToken);
		}
		else if (currentToken.type == Token::Types::op)
		{
			currentToken.parseAsOpperator();
			ret.push_back(currentToken);
		}
		else if (currentToken.type == Token::Types::keyWord) //token or user defined
		{
			currentToken.parseAsWord();
			ret.push_back(currentToken);
		}
		else
		{
			currentToken.text = "Compiler internal error";
			currentToken.type = Token::Types::error;
			ret.push_back(currentToken);
		}
	
		currentToken = {};
		currentParsingType = 0;
	
	};
	
	for (long int index = 0; index < input.size(); index++)
	{
		auto performIncrementation = [&]()
		{
			if (
				input[index] == '\n'
				)
			{
				currentTextPosition.newLine();
			}
			else if (input[index] != 0 && input[index] > 8 && input[index] != '\r')
			{
				currentTextPosition.increment();
			}
		};
	
		if (input == "\v")
		{
			ret.push_back(Token(Token::Types::error, "Vertical tab not supported", {}));
			performIncrementation();
			endCurrentToken();
			continue;
		}
		else
			if (parsingStringLiteral())
			{
				if (input[index] == '"')
				{
					currentToken.stringLiteralClosed = true;
					currentToken.type = Token::Types::stringLiteral;
					performIncrementation();
					endCurrentToken();
					continue;
				}
				else if (
					input[index] == '\n'
					|| input[index] == '\v'
					)
				{
					currentToken.type = Token::Types::error; //error parsing string literal, not closed
					currentToken.text = "Error parsing string iteral not closed.";
					performIncrementation();
					endCurrentToken();
					continue;
				}
				else
				{
					currentToken.text += input[index];
				}
	
			}
			else
				if (isspace(input[index]))
				{
					if (input[index] == '\t')
					{
						int a = 0;
					}
	
					performIncrementation();
					endCurrentToken();
					continue;
	
				}
				else //no more new line possible
					if (isParanthesis(input[index]))
					{
						auto prevPos = currentTextPosition;
						performIncrementation();
						endCurrentToken();
						ret.push_back(Token(Token::Types::parenthesis, input[index], prevPos));
						continue;
					}
					else if (input[index] == ';')
					{
						auto prevPos = currentTextPosition;
						performIncrementation();
						endCurrentToken();
						ret.push_back(Token(Token::Types::semicolin, 0, prevPos));
						continue;
					}
					else if (isalpha(input[index]) || input[index] == '_')
					{
						if (parsingNumber())
						{
							//signal error
							currentToken.type = Token::Types::error;
							currentToken.text += input[index];
							currentToken.text = "Invalid number: " + currentToken.text;
						}
						else if (parsingOperator())
						{
							currentToken.type = Token::Types::error;
							currentToken.text += input[index];
							currentToken.text = "Invalid number: " + currentToken.text;
						}
						else
						{
							currentParsingType = ParsingTypes::word;
							currentToken.type = Token::Types::keyWord;
							currentToken.text += input[index];
						}
					}
					else if (isdigit(input[index]))
					{
						if (parsingOperator())
						{
							endCurrentToken();
							currentParsingType = ParsingTypes::number;
							currentToken.type = Token::Types::number;
							currentToken.text += input[index];
						}
						else if (parsingNone())
						{
							currentParsingType = ParsingTypes::number;
							currentToken.type = Token::Types::number;
							currentToken.text += input[index];
						}
					}
					else if (isOperatorSymbol(input[index]))
					{
						if (parsingOperator())
						{
							currentToken.text += input[index];
						}
						else if (parsingNumber())
						{
							if (input[index] == '.')
							{
								currentToken.text += input[index];
							}
							else
							{
								endCurrentToken();
								currentParsingType = ParsingTypes::op;
								currentToken.type = Token::Types::op;
								currentToken.text += input[index];
							}
						}
						else
						{
							endCurrentToken();
							currentParsingType = ParsingTypes::op;
							currentToken.type = Token::Types::op;
							currentToken.text += input[index];
						}
	
					}
					else if (input[index] == '"')
					{
						endCurrentToken();
						currentParsingType = ParsingTypes::stringLiteral;
						currentToken.type = Token::Types::stringLiteral;
						currentToken.stringLiteralClosed = false;
					}
	
		performIncrementation();
	
	}
	
	endCurrentToken();
	
	
	return ret;
}