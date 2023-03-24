#include "tokenizer.h"


Token tokenize(const char *begin, const char *end, const char *&outBegin, const char *&outEnd, TextPosition &textPos)
{

	assert(begin <= end);


	while ((begin < end) && (*begin == ' ' || *begin == '\n' || *begin == '\t' || *begin == '\r'))
	{
		if (*begin == ' ' || *begin == '\t')
		{
			textPos.increment();
		}
		else if (*begin == '\n')
		{
			textPos.newLine();
		}

		begin++;
	}

	outBegin = begin;
	outEnd = end;

	if (begin == end)
	{
		return Token();
	}

	//TextPosition currentTextPosition;
	Token currentToken(Token::Types::none);
	currentToken.begin = textPos;
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
		outEnd = begin;
	};

	for (begin; begin < end; begin++)
	{
		auto performIncrementation = [&]()
		{
			if (
				*begin == '\n'
				)
			{
				textPos.newLine();
			}
			else if (*begin != 0 && *begin > 8 && *begin != '\r')
			{
				textPos.increment();
			}
		};
	
		if (*begin == '\v')
		{
			currentToken = Token(Token::Types::error, "Vertical tab not supported", {});
			//performIncrementation();
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
				//performIncrementation();

				begin++; //we have parsed this item
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
				//performIncrementation();
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
	
			//performIncrementation();
			endCurrentToken();
			setOut();
			return currentToken;
	
		}
		else //no more new line possible
		if (isParanthesis(*begin))
		{
			if (currentToken.isEmpty() && parsingNone())
			{
				currentToken = Token(Token::Types::parenthesis, *begin, textPos);
				//performIncrementation();
				begin++; //we have parsed this item
				performIncrementation();
				setOut();
				return currentToken;
			}
			else
			{
				//performIncrementation();
				endCurrentToken();
				setOut();
				return currentToken;
			}
		}
		else if (*begin == ';')
		{
			if (currentToken.isEmpty() && parsingNone())
			{
				currentToken = Token(Token::Types::semicolin, 0, textPos);
				//performIncrementation();
				begin++; //we have parsed this item
				performIncrementation();
				setOut();
				return currentToken;
			}
			else
			{
				//performIncrementation();
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
				endCurrentToken();
				setOut();
				return currentToken;

				//currentToken.type = Token::Types::error;
				//currentToken.text += *begin;
				//currentToken.text = "Invalid number: " + currentToken.text;
				//setOut();
				//return currentToken;
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
			else if (parsingWord())
			{
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

	return currentToken;

}

std::vector<Token> tokenize(const std::string_view &input)
{
	const char *begin = input.data();
	const char *end = begin + input.size();

	std::vector<Token> ret;

	TextPosition textPos;

	while (begin < end)
	{
		const char *nb = 0;
		const char *ne = 0;

		auto t = tokenize(begin, end, nb, ne, textPos);
		begin = ne;

		if(!t.isEmpty())
			ret.push_back(t);
	}

	return ret;
}
