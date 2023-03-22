#include <types.h>

static const char *paranthesis = "{}[]()";

bool isParanthesis(char c)
{
	return findChar(paranthesis, c);
}

static const char *operatorsSymbols = ".,+-*&|^%=!/~";

bool isOperatorSymbol(char c)
{
	return findChar(operatorsSymbols, c);
}