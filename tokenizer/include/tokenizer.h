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

	Token(int type, int secondaryType):type(type), secondaryType(secondaryType) {};


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
			boolean,
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
			notEquals,
			negation,
			and,
			or ,
			logicAnd,
			logicor,
			logicxor,
			logicNot,
			less,
			leesEqual,
			greater,
			greaterEqual,
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
		"!=",
		"!",
		"&&",
		"||",
		"&",
		"|",
		"^",
		"~",
		"<",
		"<=",
		">",
		">=",
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
			true_,
			false_,
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
		"true",
		"false",
	};


	int type = 0;
	int secondaryType = 0;
	std::string text = "";

	bool isEmpty() { return type == Types::none && text.empty(); };

	union
	{
		int i = 0;
		float f;
		char *notUsed;
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

				if (i == Token::KeyWords::true_)
				{
					type = Types::number;
					secondaryType = TypeNumber::boolean;
					reprezentation.i = 1;
				}
				else if (i == Token::KeyWords::false_)
				{
					type = Types::number;
					secondaryType = TypeNumber::boolean;
					reprezentation.i = 0;
				}

				return;
			}
		}

		type = Types::userDefinedWord;

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

	std::string formatSimple()
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
		else if (type == Types::parenthesis) { rez = (char)secondaryType; }
		else if (type == Types::semicolin) { rez = ";"; }
		else if (type == Types::stringLiteral) { rez = "\"" + text + "\""; }
		else if (type == Types::op) { rez = opperators[secondaryType]; }
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
			rez = keyWords[secondaryType];
		}
		else if (type == Types::userDefinedWord) { rez = text; }
		else if (type == Types::comment)
		{
			rez = "comment: " + text;

		}
		return rez;
	}


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
		else if (type == Types::parenthesis) { rez = "Parenthesis: " + (char)secondaryType; }
		else if (type == Types::semicolin) { rez = "Semicolin [;]"; }
		else if (type == Types::stringLiteral) { rez = "String Literal: \"" + text + "\""; }
		else if (type == Types::op) { rez = "Opperator: " + text + " -> " + opperators[secondaryType]; }
		else if (type == Types::number)
		{
			if (secondaryType != TypeNumber::int32 && secondaryType != TypeNumber::real32 && secondaryType != TypeNumber::boolean)
			{
				rez = "internal error" + std::to_string(__LINE__);
			}

			if (secondaryType == TypeNumber::int32) { rez = "Int32 " + std::to_string(reprezentation.i); }
			if (secondaryType == TypeNumber::real32) { rez = "Float " + std::to_string(reprezentation.f); }
			if (secondaryType == TypeNumber::boolean) { rez = "Bool " + std::to_string(reprezentation.i); }

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

//a token without the string, just plain old data
struct EmptyToken
{
	TextPosition begin = {};
	
	int type = 0;
	int secondaryType = 0;

	union
	{
		int i = 0;
		float f;
		char *notUsed;
	}reprezentation;

	std::string format()
	{
		Token t = toToken(0);
		return t.format();
	}

	Token toToken(const char *data)
	{
		Token t;
		t.type = type;
		t.secondaryType = secondaryType;
		memcpy(&t.reprezentation, &reprezentation, sizeof(reprezentation));
		t.begin = begin;

		if (data)
		{
			t.text = data;
		}

		return t;
	}
};


Token tokenize(const char *begin, const char *end, const char *&outBegin, const char *&outEnd, TextPosition &t);

std::vector<Token> tokenize(const std::string_view &input);
