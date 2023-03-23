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
	
		if (input[index] == '\v')
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

Token tokenize(const char *begin, const char *end, const char *&outBegin, const char *&outEnd)
{

	assert(begin <= end);

	while ((begin < end) && (*begin == ' ' || *begin == '\n' || *begin == '\t' || *begin == '\r'))
	{
		begin++;
	}

	outBegin = begin;
	outEnd = end;

	if (begin == end)
	{
		return Token();
	}

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
			return; //finished
		}
		else if (currentToken.type == Token::Types::stringLiteral)
		{
			if (!currentToken.stringLiteralClosed)
			{
				currentToken.type = Token::Types::error; //error parsing string literal, not closed
				currentToken.text = "Error parsing string iteral not closed.";
				return; //finished
			}
			else
			{
				return; //finished
			}
		}
		else if (currentToken.type == Token::Types::number)
		{
			currentToken.parseAsNumber();
			return; //finished

		}
		else if (currentToken.type == Token::Types::op)
		{
			currentToken.parseAsOpperator();
			return; //finished
		}
		else if (currentToken.type == Token::Types::keyWord) //token or user defined
		{
			currentToken.parseAsWord();
			return; //finished
		}
		else
		{
			currentToken.text = "Compiler internal error";
			currentToken.type = Token::Types::error;
			return; //finished
		}

		currentToken = {};
		currentParsingType = 0;

	};

	auto setOut = [&]()
	{
		outBegin = begin;
	};

	for (begin; begin < end; begin++)
	{
		auto performIncrementation = [&]()
		{
			if (
				*begin == '\n'
				)
			{
				currentTextPosition.newLine();
			}
			else if (*begin != 0 && *begin > 8 && *begin != '\r')
			{
				currentTextPosition.increment();
			}
		};
	
		if (*begin == '\v')
		{
			currentToken = Token(Token::Types::error, "Vertical tab not supported", {});
			performIncrementation();
			//endCurrentToken();
			setOut();
			return currentToken;
		}
		else
		if (parsingStringLiteral())
		{
			if (*begin == '"')
			{
				currentToken.stringLiteralClosed = true;
				currentToken.type = Token::Types::stringLiteral;
				performIncrementation();
				endCurrentToken();
				setOut();
				return currentToken;
			}
			else if (
				*begin == '\n'
				|| *begin == '\v'
				)
			{
				currentToken.type = Token::Types::error; //error parsing string literal, not closed
				currentToken.text = "Error parsing string iteral not closed.";
				performIncrementation();
				//endCurrentToken();
				setOut();
				return currentToken;
			}
			else
			{
				currentToken.text += *begin;
			}
	
		}
		else
		if (isspace(*begin))
		{
	
			performIncrementation();
			endCurrentToken();
			setOut();
			return currentToken;
	
		}
		else //no more new line possible
		if (isParanthesis(*begin))
		{
			if (currentToken.isEmpty() && parsingNone())
			{
				currentToken = Token(Token::Types::parenthesis, *begin, currentTextPosition);
				performIncrementation();
				begin++; //we have parsed this item
				setOut();
				return currentToken;
			}
			else
			{
				performIncrementation();
				endCurrentToken();
				setOut();
				return currentToken;
			}
		}
		else if (*begin == ';')
		{
			if (currentToken.isEmpty() && parsingNone())
			{
				currentToken = Token(Token::Types::semicolin, 0, currentTextPosition);
				performIncrementation();
				begin++; //we have parsed this item
				setOut();
				return currentToken;
			}
			else
			{
				performIncrementation();
				endCurrentToken();
				setOut();
				return currentToken;
			}

		}
		else if (isalpha(*begin) || *begin == '_')
		{
			if (parsingNumber())
			{
				//signal error
				currentToken.type = Token::Types::error;
				currentToken.text += *begin;
				currentToken.text = "Invalid number: " + currentToken.text;
				setOut();
				return currentToken;
			}
			else if (parsingOperator())
			{
				currentToken.type = Token::Types::error;
				currentToken.text += *begin;
				currentToken.text = "Invalid number: " + currentToken.text;
				setOut();
				return currentToken;
			}
			else
			{
				currentParsingType = ParsingTypes::word;
				currentToken.type = Token::Types::keyWord;
				currentToken.text += *begin;
			}
		}
		else if (isdigit(*begin))
		{
			if (parsingOperator())
			{
				endCurrentToken();
				setOut();
				return currentToken;
				//currentParsingType = ParsingTypes::number;
				//currentToken.type = Token::Types::number;
				//currentToken.text += input[index];
			}
			else if (parsingNone())
			{
				currentParsingType = ParsingTypes::number;
				currentToken.type = Token::Types::number;
				currentToken.text += *begin;
			}
		}
		else if (isOperatorSymbol(*begin))
		{
			if (parsingOperator())
			{
				currentToken.text += *begin;
			}
			else if (parsingNumber())
			{
				if (*begin == '.')
				{
					currentToken.text += *begin;
				}
				else
				{
					endCurrentToken();
					setOut();
					return currentToken;
					//currentParsingType = ParsingTypes::op;
					//currentToken.type = Token::Types::op;
					//currentToken.text += input[index];
				}
			}
			else
			{
				if (parsingNone() && currentToken.isEmpty())
				{
					currentParsingType = ParsingTypes::op;
					currentToken.type = Token::Types::op;
					currentToken.text += *begin;
				}
				else
				{
					endCurrentToken();
					setOut();
					return currentToken;
				}
			}
	
		}
		else if (*begin == '"')
		{
			if (parsingNone() && currentToken.isEmpty())
			{
				currentParsingType = ParsingTypes::stringLiteral;
				currentToken.type = Token::Types::stringLiteral;
				currentToken.stringLiteralClosed = false;
			}
			else
			{
				endCurrentToken();
				setOut();
				return currentToken;
			}
		}
	
		performIncrementation();
	
	}
	
	endCurrentToken();
	outBegin = end;

	return currentToken;

}

std::vector<Token> tokenize2(const std::string_view &input)
{
	const char *begin = input.data();
	const char *end = begin + input.size();

	std::vector<Token> ret;

	while (begin < end)
	{
		const char *nb = 0;
		const char *ne = 0;

		auto t = tokenize(begin, end, nb, ne);
		begin = nb;
		end = ne;

		if(!t.isEmpty())
			ret.push_back(t);
	}

	return ret;
}
