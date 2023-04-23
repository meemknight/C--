#pragma once
#include <string>
#include <parser.h>



bool execute(Statement program, std::string &err, Variables &variables);
void exectueFromLanguageString(std::string language);
