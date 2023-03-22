#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <types.h>
#include <fstream>

struct Token
{
	Token() {};
	Token(int type):type(type) {};

	Token(int type, char c):type(type) { text = c; };

	struct Types
	{
		enum
		{
			none = 0,
			error,
			paranthases,
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

	//for printing
	std::string format()
	{
		
		if (type == Types::none)
		{
			return {};
		}
		else if(type == Types::error)
		{
			return "Error: " + text;
		}
		else if (type == Types::paranthases) { return "Paranthase: " + text; }
		else if (type == Types::semicolin) { return "Semicolin [;]"; }
		else if (type == Types::stringLiteral) { return "String Literal: \"" + text + "\""; }
		else if (type == Types::op) { return "Opperator: " + text + " -> " + opperators[secondaryType]; }
		else if (type == Types::number) 
		{
			if (secondaryType != TypeNumber::int32 && secondaryType != TypeNumber::real32) 
				{ return "internal error" + std::to_string(__LINE__); }
			return (secondaryType == TypeNumber::int32 ? "Int32: " : "Float: ") +
				(secondaryType == TypeNumber::int32 ? std::to_string(reprezentation.i) : std::to_string(reprezentation.f));
		}
		else if (type == Types::keyWord)
		{ return "keyword: " + text + " -> " + keyWords[secondaryType]; }
		else if (type == Types::userDefinedWord) { return "User defined keyword: " + text; }
	}
};

std::vector<Token> tokenize(const std::string_view &input)
{

	std::vector<Token> ret;
	ret.reserve(1000);


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

		if (parsingStringLiteral())
		{
			if (input[index] == '"')
			{
				currentToken.stringLiteralClosed = true;
				currentToken.type = Token::Types::stringLiteral;
				endCurrentToken();
			}
			else if(
				input[index] == '\n'
				||input[index] == '\v'
				)
			{
				currentToken.type = Token::Types::error; //error parsing string literal, not closed
				currentToken.text = "Error parsing string iteral not closed.";
				endCurrentToken();
			}
			else
			{
				currentToken.text += input[index];
			}

		}else
		if (isspace(input[index]))
		{
			endCurrentToken();
		}else
		if (isParanthesis(input[index]))
		{
			endCurrentToken();
			
			ret.push_back(Token(Token::Types::paranthases, input[index]));
		}
		else if (input[index] == ';')
		{
			endCurrentToken();
			ret.push_back(Token(Token::Types::semicolin));
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
			}
			else if (parsingNone())
			{
				currentParsingType = ParsingTypes::number;
				currentToken.type = Token::Types::number;
			}

			currentToken.text += input[index];

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
