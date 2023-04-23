#define GLM_ENABLE_EXPERIMENTAL
#include "gameLayer.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "platformInput.h"
#include "imgui.h"
#include <iostream>
#include <sstream>
#include "imfilebrowser.h"
#include "tokenizer.h"
#include <TextEditor.h>
#include <fstream>
#include <evaluator.h>

TextEditor textEditor;

bool initGame()
{
	std::ifstream f(RESOURCES_PATH "main.cmm");
	if (f.is_open())
	{
		std::string str((std::istreambuf_iterator<char>(f)),
			std::istreambuf_iterator<char>());
		
		textEditor.SetText(str);
		textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::CMinusMinus());
	}
	else
	{
		std::cout << "Err oppening file\n";
		return 0;
	}

	return true;
}

std::vector<std::string> tokens;
static void callback(std::string s)
{
	tokens.push_back(s);
}

bool gameLogic(float deltaTime)
{
#pragma region init stuff
	int w = 0; int h = 0;
	w = platform::getWindowSizeX();
	h = platform::getWindowSizeY();

	glViewport(0, 0, w, h);
	glClear(GL_COLOR_BUFFER_BIT);


#pragma endregion


	if (ImGui::Begin("Text editor"))
	{

		textEditor.Render("TextEditor");

	}
	ImGui::End();


	if (ImGui::Begin("Console"))
	{
		if (ImGui::Button("test tokens"))
		{
			tokens.clear();
			auto rez = textEditor.GetText();
			auto rez2 = tokenize(rez);
			for (auto &i : rez2)
			{
				tokens.push_back(i.format());
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("execute"))
		{
			tokens.clear();
			auto rez = textEditor.GetText();
			exectueFromLanguageString(rez, callback);
		}

		for (auto &t : tokens)
		{
			ImGui::Text(t.c_str());
		}

	}
	ImGui::End();




	return true;
#pragma endregion

}

void closeGame()
{


}
