#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <types.h>
#include <assert.h>

struct TextPosition
{
	long int pos = 0;
	long int line = 0;
	long int totalPos = 0;
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

	Token() {};
	Token(int type):type(type) {};

	Token(int type, char c, TextPosition pos):type(type) { text = c; begin = pos; };
	Token(int type, std::string_view v, TextPosition begin):type(type)
	{
		text = v; this->begin = begin;
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
			comment,
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
		else if (type == Types::error)
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
			{
				rez = "internal error" + std::to_string(__LINE__);
			}
			rez = (secondaryType == TypeNumber::int32 ? "Int32: " : "Float: ") +
				(secondaryType == TypeNumber::int32 ? std::to_string(reprezentation.i) : std::to_string(reprezentation.f));
		}
		else if (type == Types::keyWord)
		{
			rez = "keyword: " + text + " -> " + keyWords[secondaryType];
		}
		else if (type == Types::userDefinedWord) { rez = "User defined keyword: " + text; }
		else if (type == Types::comment)
		{
			rez = "comment: " + text;

		}
		return rez + " " + formatTextPos();
	}
};

Token tokenize(const char *begin, const char *end, const char *&outBegin, const char *&outEnd, TextPosition &t);

std::vector<Token> tokenize(const std::string_view &input);
