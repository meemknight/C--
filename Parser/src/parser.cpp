#include <parser.h>


std::string Expression::format()
{

	if (left || right)
	{
		if (token.type == Token::Types::parenthesis)
		{
			std::string rez;
			if (token.secondaryType == '(')
			{
				rez = "( ";
				std::string r = left->format();
				rez += r;
				rez += ")";
			}
			else
				if (token.secondaryType == '{')
				{
					rez = "{ ";
					rez += left->format();
					rez += "}";
				}
				else
					if (token.secondaryType == '[')
					{
						rez = "[ ";
						rez += left->format();
						rez += "]";
					}
					else
					{
						return "internal error in expression::format at paranthase";
					}

			return rez;
		}
		else
		{
			std::string rez = "";
			if (left)
			{
				rez += left->format() + " ";
			}

			Token t = token.toToken(tokenString);
			rez += t.formatSimple();

			if (right)
			{
				rez += " " + right->format();
			}

			return rez;
		}

	}
	else
	{
		//simple expression
		Token t = token.toToken(tokenString);
		return t.formatSimple();
	}

};


void parse(std::vector<Token> &tokens)
{

	FreeListAllocator allocator;
	allocator.init(new unsigned char[MB(100)], MB(100));


	// ( 1 + 2 )
	//std::string errors = "";
	//Expression expression = createExpressionFromSingleToken(tokenize("(")[0], allocator, errors);
	//
	//
	//*expression.left = createExpressionFromSingleToken(tokenize("+")[0], allocator, errors);
	//*expression.left->left = createExpressionFromSingleToken(tokenize("1")[0], allocator, errors);
	//*expression.left->right = createExpressionFromSingleToken(tokenize("2")[0], allocator, errors);
	////
	//std::cout << expression.format() << "\n";


	Parser parser;
	parser.allocator = &allocator;
	parser.tokens = &tokens;


	auto rez = parser.expression();

	if (!parser.err.empty())
	{
		std::cout << parser.err << "\n";
	}
	else
	{
		std::cout << rez.format() << "\n";
	}


	std::cin.get();

	delete[] allocator.initialBaseMemory;
}

void testEvaluate(std::string language)
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

		Parser parser;
		parser.allocator = &allocator;
		parser.tokens = &tokens;

		auto ast = parser.expression();
		if (!parser.err.empty()) { std::cout << "parser error: " << parser.err << "\n"; }
		else
		{
			std::string err;

			auto finalRez = evaluateExpression(&ast, err);

			if (!err.empty()) { std::cout << "evaluator error: " << err << "\n"; }
			else
			{
				std::cout << finalRez.format();
			}

		}

	}

	std::cin.get();
	delete[] allocator.initialBaseMemory;

}

bool Value::equals(Value other, std::string &err)
{
	if (this->isString() || this->isNone() || other.isString() || other.isNone()) { err = "String or none not allowed."; return 0; }
	
	auto copy = *this;
	copy.matchOpperatorsUnsafe(other);
	
	if (copy.isBool()) { return (bool)copy.reprezentation.i == (bool)other.reprezentation.i; }else
	if (copy.isInt32()) { return copy.reprezentation.i == other.reprezentation.i; }else
	if (copy.isReal32()) { return copy.reprezentation.f == other.reprezentation.f; }else
	{ assert(0); }
}

bool Value::notEquals(Value other, std::string &err)
{
	if (this->isString() || this->isNone() || other.isString() || other.isNone()) { err = "String or none not allowed."; return 0; }
	return !equals(other, err);
}

bool Value::greater(Value other, std::string &err)
{
	if (this->isString() || this->isNone() || other.isString() || other.isNone()) { err = "String or none not allowed."; return 0; }
	
	auto copy = *this;
	copy.matchOpperatorsUnsafe(other);
	
	if (copy.isBool()) { return (bool)copy.reprezentation.i > (bool)other.reprezentation.i; }else
	if (copy.isInt32()) { return copy.reprezentation.i > other.reprezentation.i; }else
	if (copy.isReal32()) { return copy.reprezentation.f > other.reprezentation.f; }else
	{ assert(0); }
}


bool Value::greaterEqual(Value other, std::string &err)
{
	if (this->isString() || this->isNone() || other.isString() || other.isNone()) { err = "String or none not allowed."; return 0; }
	
	auto copy = *this;
	copy.matchOpperatorsUnsafe(other);
	
	if (copy.isBool()) { return (bool)copy.reprezentation.i >= (bool)other.reprezentation.i; }else
	if (copy.isInt32()) { return copy.reprezentation.i >= other.reprezentation.i; }else
	if (copy.isReal32()) { return copy.reprezentation.f >= other.reprezentation.f; }else
	{ assert(0); }
}


bool Value::smaller(Value other, std::string &err)
{
	if (this->isString() || this->isNone() || other.isString() || other.isNone()) { err = "String or none not allowed."; return 0; }
	
	auto copy = *this;
	copy.matchOpperatorsUnsafe(other);
	
	if (copy.isBool()) { return (bool)copy.reprezentation.i < (bool)other.reprezentation.i; }else
	if (copy.isInt32()) { return copy.reprezentation.i < other.reprezentation.i; }else
	if (copy.isReal32()) { return copy.reprezentation.f < other.reprezentation.f; }else
	{ assert(0); }
}

bool Value::smallerEqual(Value other, std::string &err)
{
	if (this->isString() || this->isNone() || other.isString() || other.isNone()) { err = "String or none not allowed."; return 0; }
	
	auto copy = *this;
	copy.matchOpperatorsUnsafe(other);
	
	if (copy.isBool()) { return (bool)copy.reprezentation.i <= (bool)other.reprezentation.i; }else
	if (copy.isInt32()) { return copy.reprezentation.i <= other.reprezentation.i; }else
	if (copy.isReal32()) { return copy.reprezentation.f <= other.reprezentation.f; }else
	{ assert(0); }
}

//a type b,  for example a + b
Value performComputation(int type, Value a, Value b, std::string &err)
{
	if (a.isNone() || b.isNone())
	{
		err = "Interpretor internal error: Recieved a none value in perform computation";
	}

	auto mathOpperation = [&](int type, Value &out) -> bool
	{
		if (a.isNone() || a.isString() || b.isNone() || b.isString())
		{
			err = "mathOpperation works only on int bool real, recieved: " + a.formatType() + " and " + b.formatType();
			return 0;
		}

		if (a.isReal32() || b.isReal32()) 
		{
			a.toReal32(); b.toReal32(); 

			switch (type)
			{
			case Token::TypeOpperators::plus:
			a.reprezentation.f += b.reprezentation.f;
			break;
			case Token::TypeOpperators::minus:
			a.reprezentation.f -= b.reprezentation.f;
			break;
			case Token::TypeOpperators::multiplication:
			a.reprezentation.f *= b.reprezentation.f;
			break;
			case Token::TypeOpperators::division:
			a.reprezentation.f /= b.reprezentation.f;
			break;
			case Token::TypeOpperators::modulo:
			err = "Module doesn't work on floats.";
			return 0;
			break;
			};
		
		}else
		if (a.isInt32() || b.isInt32()) 
		{
			a.toInt32(); b.toInt32(); 

			switch (type)
			{
			case Token::TypeOpperators::plus:
			a.reprezentation.i += b.reprezentation.i;
			break;
			case Token::TypeOpperators::minus:
			a.reprezentation.i -= b.reprezentation.i;
			break;
			case Token::TypeOpperators::multiplication:
			a.reprezentation.i *= b.reprezentation.i;
			break;
			case Token::TypeOpperators::division:
			a.reprezentation.i /= b.reprezentation.i;
			break;
			case Token::TypeOpperators::modulo:
			a.reprezentation.i %= b.reprezentation.i;
			break;
			};
		}
		else //both bool
		{
			assert(a.isBool() && b.isBool()); //"both should be bool at this point"

			if (type == Token::TypeOpperators::division)
			{
				err = "Division doesn't work with bools";
				return 0;
			}

			switch (type)
			{
			case Token::TypeOpperators::plus:
			a.reprezentation.i += b.reprezentation.i;
			break;
			case Token::TypeOpperators::minus:
			a.reprezentation.i -= b.reprezentation.i;
			break;
			case Token::TypeOpperators::multiplication:
			a.reprezentation.i *= b.reprezentation.i;
			break;
			case Token::TypeOpperators::modulo:
			err = "Module doesn't work on bools.";
			return 0;
			break;
			};

			a.toBool();
		}

		out = a;
				
		return 1;
	};

	switch (type)
	{
	case Token::TypeOpperators::plus:
	case Token::TypeOpperators::minus:
	case Token::TypeOpperators::multiplication:
	case Token::TypeOpperators::division:
	case Token::TypeOpperators::modulo:
	{
		Value ret = {};
		if (!mathOpperation(type, ret))
		{
			return {};
		}
		else
		{
			return ret;
		}
		break;
	}

	case Token::TypeOpperators::asignment:
	{

		if (a.isBool())
		{
			if (!b.toBool()) { err = "Could not convert " + b.formatType() + " to bool."; return {}; }
			a.reprezentation.i = b.reprezentation.i;
		}else
		if (a.isInt32())
		{
			if (!b.toInt32()) { err = "Could not convert " + b.formatType() + " to int32."; return {}; }
			a.reprezentation.i = b.reprezentation.i;
		}else
		if (a.isReal32())
		{
			if (!b.toReal32()) { err = "Could not convert " + b.formatType() + " to real32."; return {}; }
			a.reprezentation.f = b.reprezentation.f;
		}
		else if (a.isString())
		{
			err = "Assignment doesn't work on strings"; return {};
		}
		else 
		{ assert(0, "unreachable in parse.cpp assignment"); }

		break;
	}

	case Token::TypeOpperators::equals:
	{
		Value rez;
		rez.type = Value::boolean;
		rez.reprezentation.i = a.equals(b, err);
		return rez;
		break;
	}

	case Token::TypeOpperators::and:
	{
		if (!a.toBool() && b.toBool()) { err = "Boolena opperators don't work on strings or none"; return {}; }

		Value rez;
		rez.type = Value::boolean;
		rez.reprezentation.i = (bool)a.reprezentation.i && (bool)b.reprezentation.i;
		return rez;
		break;
	}
	case Token::TypeOpperators::or:
	{
		if (!a.toBool() && b.toBool()) { err = "Boolena opperators don't work on strings or none"; return {}; }
		Value rez;
		rez.type = Value::boolean;
		rez.reprezentation.i = (bool)a.reprezentation.i || (bool)b.reprezentation.i;
		return rez;
		break;
	}

	case Token::TypeOpperators::logicAnd: //not for floats, just bool to bool or int to int
	{
		if(a.isReal32() || b.isReal32()) { err = "Logic opperators don't work on floats"; return {}; }
		
		if (a.isBool() || b.isBool())
		{
			if (!a.toBool() && b.toBool()) { err = "Logic opperators don't work on strings or none"; return {}; }

			Value rez;
			rez.type = Value::boolean;
			rez.reprezentation.i = (bool)a.reprezentation.i & (bool)b.reprezentation.i;
			return rez;
			break;
		}
		else if (a.isInt32() && b.isInt32())
		{
			Value rez;
			rez.type = Value::int32;
			rez.reprezentation.i = a.reprezentation.i & b.reprezentation.i;
			return rez;
			break;
		}
		else
		{
			err = "Logic opperators work only on ints and bool"; return{};
		}
	}


	case Token::TypeOpperators::logicor:
	{
		if (a.isReal32() || b.isReal32()) { err = "Logic opperators don't work on floats"; return {}; }

		if (a.isBool() || b.isBool())
		{
			if (!a.toBool() && b.toBool()) { err = "Logic opperators don't work on strings or none"; return {}; }

			Value rez;
			rez.type = Value::boolean;
			rez.reprezentation.i = (bool)a.reprezentation.i | (bool)b.reprezentation.i;
			return rez;
			break;
		}
		else if (a.isInt32() && b.isInt32())
		{
			Value rez;
			rez.type = Value::int32;
			rez.reprezentation.i = a.reprezentation.i | b.reprezentation.i;
			return rez;
			break;
		}
		else
		{
			err = "Logic opperators work only on ints and bool"; return{};
		}

	}

	case Token::TypeOpperators::logicxor:
	{
		if (a.isReal32() || b.isReal32()) { err = "Logic opperators don't work on floats"; return {}; }

		if (a.isBool() || b.isBool())
		{
			if (!a.toBool() && b.toBool()) { err = "Logic opperators don't work on strings or none"; return {}; }

			Value rez;
			rez.type = Value::boolean;
			rez.reprezentation.i = (bool)a.reprezentation.i ^ (bool)b.reprezentation.i;
			return rez;
			break;
		}
		else if (a.isInt32() && b.isInt32())
		{
			Value rez;
			rez.type = Value::int32;
			rez.reprezentation.i = a.reprezentation.i ^ b.reprezentation.i;
			return rez;
			break;
		}
		else
		{
			err = "Logic opperators work only on ints and bool"; return{};
		}

	}

	case Token::TypeOpperators::less:
	{
		Value rez;
		rez.toBool();
		rez.reprezentation.i = a.smaller(b, err);
		return rez;
		break;
	}

	case Token::TypeOpperators::leesEqual:
	{
		Value rez;
		rez.toBool();
		rez.reprezentation.i = a.smallerEqual(b, err);
		return rez;
		break;
	}

	case Token::TypeOpperators::greater:
	{
		Value rez;
		rez.toBool();
		rez.reprezentation.i = a.greater(b, err);
		return rez;
		break;
	}

	case Token::TypeOpperators::greaterEqual:
	{
		Value rez;
		rez.toBool();
		rez.reprezentation.i = a.greaterEqual(b, err);
		return rez;
		break;
	}

	default:
		err = "Internal evaluator error, not a double opperand"; return {};
	break;
	}
}

//type b,   for example - b
Value performComputation(int type, Value a, std::string &err)
{

	switch (type)
	{
	case Token::TypeOpperators::minus:
		if(a.isInt32() || a.isBool())
		{
			a.toInt32();
			a.reprezentation.i = -a.reprezentation.i;
			return a;
		}
		else if (a.isReal32())
		{
			a.reprezentation.f = -a.reprezentation.f;
			return a;
		}
		else
		{
			err = "Negation works only on int bool real, recieved: " + a.formatType(); return {};
		}

	break;
	case Token::TypeOpperators::logicNot: //biti
		if (a.isInt32())
		{
			a.reprezentation.i = ~a.reprezentation.i;
			return a;
		}
		else if (a.isBool())
		{
			a.reprezentation.i = !(bool)a.reprezentation.i;
			return a;
		}
		else
		{
			err = "Logic not works only on int or bool, recieved: " + a.formatType(); return {};
		}

	break;
	case Token::TypeOpperators::negation: //!(bool)
		if (!a.toBool()) { err = "Couldn't convert opperator to bool. Recieved: " + a.formatType(); return {}; };
		a.reprezentation.i = !(bool)a.reprezentation.i;
		return a;
	break;
	default:
	err = "Internal evaluator error, not a single opperand"; return {};
	break;
	}
}

Value evaluateExpression(Expression *e, std::string &err)
{

	if (!err.empty()) { return {}; }
	if (!e) { err = "Internal evaluator error, expression is null."; return{}; }


	if (e->token.type == Token::Types::number
		|| e->token.type == Token::Types::stringLiteral
		)
	{
		Value ret;
		bool succeed = ret.createFromToken(*e);

		if (!succeed) { err = "Internal evaluator error at parsing values"; return {}; }

		return ret;
	}
	else if (e->token.type == Token::Types::parenthesis)
	{
		if(e->token.secondaryType != '(') { err = "Internal evaluator error expected '('"; return {}; }
		if (!e->left) { err = "Internal evaluator error parenthesis"; return {}; }
		return evaluateExpression(e->left, err);
	}
	else if (e->token.type == Token::Types::op)
	{
		
		//unary opperators here
		if (
			e->token.secondaryType != Token::TypeOpperators::minus &&
			e->token.secondaryType != Token::TypeOpperators::logicNot &&
			e->token.secondaryType != Token::TypeOpperators::negation 
			)
		{
			if (!e->left || !e->right)
			{
				err = "Internal evaluator error, at opperator, missing left and right pointers."; return {};
			}
		}

		if(
			(e->token.secondaryType == Token::TypeOpperators::minus ||
			e->token.secondaryType == Token::TypeOpperators::logicNot ||
			e->token.secondaryType == Token::TypeOpperators::negation)
			&& !e->left
			)
		{
			if (!e->right)
			{
				err = "Internal evaluator error, at opperator, missing left pointer."; return {};
			}

			auto rightVal = evaluateExpression(e->right, err);
			if (!err.empty()) { return {}; }

			auto rez = performComputation(e->token.secondaryType, rightVal, err);
			if (!err.empty()) { return {}; }

			return rez;
		}
		else
		{
			auto leftVal = evaluateExpression(e->left, err);
			if (!err.empty()) { return {}; }

			auto rightVal = evaluateExpression(e->right, err);
			if (!err.empty()) { return {}; }

			auto rez = performComputation(e->token.secondaryType,leftVal, rightVal, err);
			if (!err.empty()) { return {}; }

			return rez;
		}

		
	}
	else
	{
		err = "Internal evaluator error, not an expression.";
		return {};
	}

	//todo user defined word here


	return {};
}


Expression createExpressionFromSingleToken(Token &token, FreeListAllocator &allocator, std::string &error)
{
	Expression returnVal = {};
	error = "";

	returnVal.token.type = token.type;
	returnVal.token.secondaryType = token.secondaryType;
	returnVal.token.begin = token.begin;
	memcpy(&returnVal.token.reprezentation, &token.reprezentation, sizeof(token.reprezentation));

	static_assert(sizeof(returnVal.token.reprezentation) == sizeof(token.reprezentation), "");

	switch (token.type)
	{
	case Token::Types::none:
	assert(0); //we should not get here
	break;
	case Token::Types::error:
	assert(0); //we should not get here
	break;
	case Token::Types::comment:
	assert(0); //we should not get here
	break;
	case Token::Types::parenthesis:
	if (
		returnVal.token.secondaryType != '(' &&
		returnVal.token.secondaryType != '{' &&
		returnVal.token.secondaryType != '['
		)
	{
		assert(0);
	}
	else
	{
		allocateMemoryForTheExpression(returnVal, allocator, 1, 0, "");
	}
	break;
	case Token::Types::semicolin:
	assert(0); //we should not get here
	break;
	case Token::Types::stringLiteral:
	allocateMemoryForTheExpression(returnVal, allocator, 0, 0, token.text);
	break;
	case Token::Types::op:
	//lets see what kind of opperator we have

	switch (token.secondaryType)
	{
	case Token::TypeOpperators::plus:
	case Token::TypeOpperators::minus: //the minus opperator can also be unary
	case Token::TypeOpperators::multiplication:
	case Token::TypeOpperators::division:
	case Token::TypeOpperators::modulo:
	case Token::TypeOpperators::asignment:
	case Token::TypeOpperators::equals:
	case Token::TypeOpperators:: and:
	case Token::TypeOpperators:: or :
	case Token::TypeOpperators::logicAnd:
	case Token::TypeOpperators::logicor:
	case Token::TypeOpperators::logicxor:
	case Token::TypeOpperators::less:
	case Token::TypeOpperators::leesEqual:
	case Token::TypeOpperators::greater:
	case Token::TypeOpperators::greaterEqual:
	allocateMemoryForTheExpression(returnVal, allocator, 1, 1, token.text);

	break;


	case Token::TypeOpperators::negation:
	case Token::TypeOpperators::logicNot:
	allocateMemoryForTheExpression(returnVal, allocator, 0, 1, token.text); //we have something on the right

	break;


	default:
	assert(0); //we should not get here
	break;

	}

	break;
	case Token::Types::number:
	//
	break;
	case Token::Types::keyWord:
	assert(0); //we should not get here
	break;
	case Token::Types::userDefinedWord:
	assert(0); //we should not get here
	break;
	assert(0); //we should not get here
	default:
	break;
	}

	return returnVal;
}



void allocateMemoryForTheStatement(Statement &e, FreeListAllocator &allocator, 
	int statementsCount, int expressionsCount)
{
	

	if (!statementsCount && !expressionsCount)
	{
		//don't allocate
	}
	else
	{

		e.statementsCount = statementsCount;
		e.statements = (Statement *)allocator.allocate(statementsCount * sizeof(Statement));

		e.expressionsCount = expressionsCount;
		e.expressions = (Expression *)allocator.allocate(statementsCount * sizeof(Expression));
		
	}

}

void allocateMemoryForTheExpression(Expression &e, FreeListAllocator &allocator, bool allocateLeftExpression, bool allocateRightExpresiion,
	std::string_view string)
{
	e.left = 0;
	e.right = 0;
	e.tokenString = 0;
	if (!allocateLeftExpression && !allocateRightExpresiion && string.size() == 0)
	{
		//don't allocate
	}
	else
	{
		if (!allocateLeftExpression && !allocateRightExpresiion)
		{
			//just allocate memory for the string
			e.tokenString = (char *)allocator.allocate(string.size() + 1);
			strcpy(e.tokenString, string.data());
		}
		else
		{
			//alco allocate memory for data
			static_assert(sizeof(Expression) < 64u, "");

			size_t totalSize = 64 * 2 + string.size() + 1;
			void *allocatedData = allocator.allocate(totalSize);

			if (allocateLeftExpression)e.left = (Expression *)allocatedData;
			if (allocateRightExpresiion)e.right = (Expression *)((unsigned char *)allocatedData + 64);
			e.tokenString = (char *)((unsigned char *)allocatedData + 64 * 2);
			strcpy(e.tokenString, string.data());
		}
	}

}
