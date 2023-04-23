#pragma once
#include <string>
#include <parser.h>



bool execute(Statement program, std::string &err, Variables &variables, void(*printCallback)(std::string));
void exectueFromLanguageString(std::string language, void(*printCallback)(std::string));
