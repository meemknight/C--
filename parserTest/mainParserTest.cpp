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
	int32 counter = 10;
	
	while(counter > 0)
	{
		print(counter);
		counter = counter - 1;
	}else
	{
		print("executed 0 times");
	}

	print("done");
}
)#"
);


	std::cout << "\n";
	std::cout << "done";
	std::cin.get();


	return 0;
}