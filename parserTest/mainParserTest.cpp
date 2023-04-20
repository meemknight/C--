#include <iostream>

#include <parser.h>
#include <evaluator.h>

int main()
{



	//parse(tokenize("-2 + 3)"));

	//testEvaluate("(((true + 2.0) * -2) + 6) + 1.0");
	//testEvaluate("~10");
	//std::cout << ~10;
	//std::cin.get();

	exectueFromLanguageString(
		"print(10 + 2);"
		"print(2);"
		"print(\"test\"); "
	);


	return 0;
}