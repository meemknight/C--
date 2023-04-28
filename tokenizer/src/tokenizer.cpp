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

	Token returnVal;

	auto peek = [&]()
	{
		if (begin + 1 < end)
		{
			return begin[1];
		}
		else
		{
			return '\0';
		}
	};

	auto increment = [&]() { begin++; return *begin; };

	auto match =[&](char c)
	{
		if (peek() == c)
		{
			increment();
			return true;
		}
		else
		{
			return false;
		}
	};

	switch (*begin)
	{
	case ';':
		returnVal.type = Token::Types::semicolin;
		break;
	case '+':
		if (match('+'))
		{
			returnVal.type = Token::Types::op;
			returnVal.secondaryType = Token::TypeOpperators::plusplus;
		}
		else
		{
			returnVal.type = Token::Types::op;
			returnVal.secondaryType = Token::TypeOpperators::plus;
		}
		break;
	case '-':
		if (match('-'))
		{
			returnVal.type = Token::Types::op;
			returnVal.secondaryType = Token::TypeOpperators::minusminus;
		}
		else
		{
			returnVal.type = Token::Types::op;
			returnVal.secondaryType = Token::TypeOpperators::minus;
		}
		break;

	case '*':
		returnVal.type = Token::Types::op;
		returnVal.secondaryType = Token::TypeOpperators::multiplication;
		break;

	case '/':
		{
			if (match('/'))
			{
				//parse comment 
				while (true)
				{
					char c = peek();
					if (c == '\0' || c == '\v' || c == '\n')
					{
						break;
					}
					increment();
				}
			}
			else
			{
				returnVal.type = Token::Types::op;
				returnVal.secondaryType = Token::TypeOpperators::division;
			}
		break;
		}

	case '%':
		returnVal.type = Token::Types::op;
		returnVal.secondaryType = Token::TypeOpperators::modulo;
		break;

	case '=':
		{
			if (match('='))
			{
				returnVal.type = Token::Types::op;
				returnVal.secondaryType = Token::TypeOpperators::equals;
			}
			else
			{
				returnVal.type = Token::Types::op;
				returnVal.secondaryType = Token::TypeOpperators::asignment;
			}
		break;
		}

	case '!':
		if (match('='))
		{
			returnVal.type = Token::Types::op;
			returnVal.secondaryType = Token::TypeOpperators::notEquals;
		}
		else
		{
			returnVal.type = Token::Types::op;
			returnVal.secondaryType = Token::TypeOpperators::negation;
		}
		break;

	case '&':
		{
			if (match('&'))
			{
				returnVal.type = Token::Types::op;
				returnVal.secondaryType = Token::TypeOpperators::and;
			}
			else
			{
				returnVal.type = Token::Types::op;
				returnVal.secondaryType = Token::TypeOpperators::logicAnd;
			}
			break;
		}

	case '|':
		{
			if (match('|'))
			{
				returnVal.type = Token::Types::op;
				returnVal.secondaryType = Token::TypeOpperators::or;
			}
			else
			{
				returnVal.type = Token::Types::op;
				returnVal.secondaryType = Token::TypeOpperators::logicor;
			}
			break;
		}

	case '^':
		returnVal.type = Token::Types::op;
		returnVal.secondaryType = Token::TypeOpperators::logicxor;
		break;

	case '~':
		returnVal.type = Token::Types::op;
		returnVal.secondaryType = Token::TypeOpperators::logicNot;
		break;

	case '<':
		{
			if (match('='))
			{
				returnVal.type = Token::Types::op;
				returnVal.secondaryType = Token::TypeOpperators::leesEqual;
			}
			else
			{
				returnVal.type = Token::Types::op;
				returnVal.secondaryType = Token::TypeOpperators::less;
			}
			break;
		}

	case '>':
		{
			if (match('='))
			{
				returnVal.type = Token::Types::op;
				returnVal.secondaryType = Token::TypeOpperators::greaterEqual;
			}
			else
			{
				returnVal.type = Token::Types::op;
				returnVal.secondaryType = Token::TypeOpperators::greater;
			}
			break;
		}

	case '"':
		{
			//parse tring literal
			returnVal.type = Token::Types::stringLiteral;

			while (true)
			{
				char c = peek();
				if (c == '\0' || c == '\v' || c == '\n')
				{
					returnVal.type = Token::Types::error;
					returnVal.text = "Error: string literal not closed";
					break;
				}else
				if (c == '"')
				{
					increment();
					//found the string literal
					break;
				}
				else
				{
					increment();
					returnVal.text += c;
				}
			}

			break;
		}

	default:
		{
			
			if (isParanthesis(*begin))
			{
				returnVal.type = Token::Types::parenthesis;
				returnVal.secondaryType = *begin;
				returnVal.text = *begin;
			}else if (isdigit(*begin))
			{
				returnVal.type = Token::Types::number;
				char c = *begin;
				returnVal.text += c;
				c = peek();
				while (isdigit(c) || c == '.')
				{
					increment();
					returnVal.text += c;
					c = peek();
				}

				returnVal.parseAsNumber();
			}
			else if (isalpha(*begin) || *begin == '_')
			{
				returnVal.type = Token::Types::keyWord;
				char c = *begin;
				returnVal.text += c;
				c = peek();

				while (isalnum(c) || c == '_')
				{
					increment();
					returnVal.text += c;
					c = peek();
				}

				returnVal.parseAsWord();
			}
			else
			{
				returnVal.type = Token::Types::error;
				returnVal.text = "Error: unexpected character: " + *begin;
			}
			
			break;
		}

	}

	outEnd = begin+1;
	return returnVal;

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
