#include <iostream>

#include <parser.h>
#include <evaluator.h>

void callback(std::string t)
{
	std::cout << t;
}

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
	int32 a = 0;
	int32 b = 1;

	int32 counter = 10;
	while(counter > 0)
	{
		int32 old = a;
		a = a + b;
		b = old;
		counter = counter -1;
	}else
	{
		
	}
	
	print(a);
	print("done");
}
)#", callback
);

	std::cout << "\n";
	std::cout << "done";
	std::cin.get();


	return 0;
}