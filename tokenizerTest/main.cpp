#include <iostream>
#include <fstream>
#include "tokenizer.h"


int main()
{
	std::ifstream f(RESOURCES_PATH "main.cmm");
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
