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

TextEditor textEditor;

bool initGame()
{
	std::ifstream f(RESOURCES_PATH "main.cmm");
	if (f.is_open())
	{
		std::string str((std::istreambuf_iterator<char>(f)),
			std::istreambuf_iterator<char>());
		
		textEditor.SetText(str);
	}
	else
	{
		std::cout << "Err oppening file\n";
		return 0;
	}

	return true;
}



bool gameLogic(float deltaTime)
{
#pragma region init stuff
	int w = 0; int h = 0;
	w= platform::getWindowSizeX();
	h = platform::getWindowSizeY();
	
	glViewport(0, 0, w, h);
	glClear(GL_COLOR_BUFFER_BIT);


#pragma endregion


	if (ImGui::Begin("Text editor"))
	{

		textEditor.Render("TextEditor");

	}

	ImGui::End();


	return true;
#pragma endregion

}

void closeGame()
{


}
