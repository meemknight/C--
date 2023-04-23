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
R"#(
{	
	int32 a = 2;
	int32 b = 6;
	{
		print("test");
	}
	a = b - 2;
	print(a + b);
}
)#"
);


	std::cout << "\n";
	std::cout << "done";
	std::cin.get();


	return 0;
}