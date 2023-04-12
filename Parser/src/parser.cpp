#include <parser.h>

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

	delete[] allocator.baseMemory;
}


//a type b,  for example a + b
Value performComputation(int type, Value a, Value b, std::string &err)
{

	(10 + 1.f + true);


	switch (type)
	{
	case Token::TypeOpperators::plus:
	case Token::TypeOpperators::minus:
	case Token::TypeOpperators::multiplication:
	case Token::TypeOpperators::division:
	case Token::TypeOpperators::modulo:
	case Token::TypeOpperators::asignment:
	case Token::TypeOpperators::equals:
	case Token::TypeOpperators::and:
	case Token::TypeOpperators::or:
	case Token::TypeOpperators::logicAnd: //not for floats, just bool to bool or int to int
	case Token::TypeOpperators::logicor:
	case Token::TypeOpperators::logicxor:
	case Token::TypeOpperators::less:
	case Token::TypeOpperators::leesEqual:
	case Token::TypeOpperators::greater:
	case Token::TypeOpperators::greaterEqual:

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

	break;
	case Token::TypeOpperators::logicNot: //biti


	break;
	case Token::TypeOpperators::negation: //!(bool)
		if (!a.toBool()) { err = "Couldn't convert opperator to bool."; return {}; };
		a.reprezentation.i = !(bool)a.reprezentation.i;
		return a;
	break;
	default:
	err = "Internal evaluator error, not a single opperand"; return {};
	break;
	}
}

Value evaluate(Expression *e, std::string &err)
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
		return evaluate(e->left, err);
	}
	else if (e->token.type == Token::Types::op)
	{

		if (e->token.secondaryType != Token::TypeOpperators::minus)
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

			auto rightVal = evaluate(e->right, err);
			if (!err.empty()) { return {}; }

			auto rez = performComputation(e->token.secondaryType, rightVal, err);
			if (!err.empty()) { return {}; }

			return rez;
		}
		else
		{
			auto leftVal = evaluate(e->left, err);
			if (!err.empty()) { return {}; }

			auto rightVal = evaluate(e->right, err);
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
