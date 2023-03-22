#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <types.h>
#include <fstream>

struct TextPosition
{
	long int pos=0;
	long int line=0;
	long int totalPos=0;
	void newLine()
	{
		pos = 0;
		totalPos++;
		line++;
	}

	void increment()
	{
		pos++;
		totalPos++;
	}
};

struct Token
{
	TextPosition begin = {};
	TextPosition end = {};

	Token() {};
	Token(int type):type(type) {};

	Token(int type, char c, TextPosition pos):type(type) { text = c; begin = pos; end = pos; };
	Token(int type, std::string_view v, TextPosition begin):type(type) { text = v; this->begin = begin;
		end = begin;
		end.pos += v.size();
		end.totalPos += v.size();
	};

	struct Types
	{
		enum
		{
			none = 0,
			error,
			parenthesis,
			semicolin,
			stringLiteral,
			op,
			number,
			keyWord,
			userDefinedWord,
		};
	};

	struct TypeNumber
	{
		enum
		{
			none = 0,
			int32,
			real32,
		};
	};

	struct TypeOpperators
	{
		enum
		{
			none = 0,
			plus,
			minus,
			multiplication,
			division,
			modulo,
			asignment,
			equals,
			negation,
			and,
			or ,
			logicAnd,
			logicor,
			logicxor,
			logicNot,
			OpperatorsCount
		};
	};

	constexpr static char *opperators[TypeOpperators::OpperatorsCount] = {
		"",
		"+",
		"-",
		"*",
		"/",
		"%",
		"=",
		"==",
		"!",
		"&&",
		"||",
		"&",
		"|",
		"^",
		"~",
	};

	struct KeyWords
	{
		enum
		{
			none = 0,
			if_,
			while_,
			int32_,
			float_,
			void_,
			return_,
			keyWordsCount
		};
	};

	constexpr static char *keyWords[KeyWords::keyWordsCount] = {
		"",
		"if",
		"while",
		"int32",
		"float",
		"void",
		"return",
	};


	int type = 0;
	int secondaryType = 0;
	std::string text = "";

	bool isEmpty() { return type == Types::none && text.empty(); };

	bool stringLiteralClosed = 0;

	union
	{
		int i = 0;
		float f;
	}reprezentation;

	//can't be empty
	void parseAsWord()
	{
		//todo trim?

		type = Types::keyWord;
		for (int i = 0; i < Token::KeyWords::keyWordsCount; i++)
		{
			if (text == Token::keyWords[i])
			{
				secondaryType = i;
				return;
			}
		}

		type = Types::userDefinedWord;

	}

	//can't be empty
	void parseAsOpperator()
	{
		type = Types::op;

		for (int i = 0; i < Token::TypeOpperators::OpperatorsCount; i++)
		{
			if (text == Token::opperators[i])
			{
				secondaryType = i;
				return;
			}
		}

		type = Types::error;

		text = "Unknown opperator: " + text;

	}

	//can't be empty
	void parseAsNumber()
	{

		int number = 0;
		float floatNumber = 0;
		int floatCounter = 1;
		secondaryType = TypeNumber::int32;
		type = Types::number;

		for (int i = 0; i < text.size(); i++)
		{
			if (isdigit(text[i]))
			{
				if (secondaryType == TypeNumber::real32)
				{
					floatNumber *= 10;
					floatNumber += text[i] - '0';
					floatCounter *= 10;
				}
				else
				{
					number *= 10;
					number += text[i] - '0';
				}
			}
			else if (text[i] == '.')
			{
				if (secondaryType == TypeNumber::real32)
				{
					secondaryType = 0;
					type = Types::error;
					text = "Invalid character when parsing number: \'.\'";
					return;
				}
				else
				{
					secondaryType = TypeNumber::real32;
					floatNumber = number;
				}
			}
			else
			{
				secondaryType = 0;
				type = Types::error;
				text = "Invalid character when parsing number: \'" + std::to_string(text[i]) + "\'";
				return;
			}
		}

		if (secondaryType == TypeNumber::real32)
		{
			floatNumber /= floatCounter;
			reprezentation.f = floatNumber;
		}
		else
		{
			reprezentation.i = number;
		}

	}
	
	std::string formatTextPos()
	{
		return "(" + std::to_string(begin.line) + ", " + std::to_string(begin.pos) + ")";
	}

	//for printing
	std::string format()
	{
		std::string rez = {};
		if (type == Types::none)
		{
			rez = {};
		}
		else if(type == Types::error)
		{
			rez = "Error: " + text;
		}
		else if (type == Types::parenthesis) { rez = "Parenthesis: " + text; }
		else if (type == Types::semicolin) { rez = "Semicolin [;]"; }
		else if (type == Types::stringLiteral) { rez = "String Literal: \"" + text + "\""; }
		else if (type == Types::op) { rez = "Opperator: " + text + " -> " + opperators[secondaryType]; }
		else if (type == Types::number) 
		{
			if (secondaryType != TypeNumber::int32 && secondaryType != TypeNumber::real32) 
				{ rez =  "internal error" + std::to_string(__LINE__); }
			rez = (secondaryType == TypeNumber::int32 ? "Int32: " : "Float: ") +
				(secondaryType == TypeNumber::int32 ? std::to_string(reprezentation.i) : std::to_string(reprezentation.f));
		}
		else if (type == Types::keyWord)
		{ rez =  "keyword: " + text + " -> " + keyWords[secondaryType]; }
		else if (type == Types::userDefinedWord) { rez = "User defined keyword: " + text; }
		
		return rez + " " + formatTextPos();
	}
};

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
		}else
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
			else if(
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

		}else
		if (isspace(input[index])) 
		{
			if (input[index] == '\t') 
			{
				int a = 0; 
			}

			performIncrementation();
			endCurrentToken();
			continue;

		}else //no more new line possible
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


int main()
{
	std::ifstream f("main.cmm");
	if (f.is_open())
	{
		std::string str((std::istreambuf_iterator<char>(f)),
			std::istreambuf_iterator<char>());

		auto rez = tokenize(str);

		for (auto &i : rez)
		{
			std::cout << i.format() << "\n";
		}
	}
	else
	{
		std::cout << "Err oppening file\n";
	}


	std::cin.get();
	return 0;
}
