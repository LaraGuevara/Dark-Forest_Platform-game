#include "GuiManager.h"
#include "Engine.h"
#include "Textures.h"

#include "GuiControlButton.h"
#include "Audio.h"
#include "Scene.h"
#include "Window.h"

GuiManager::GuiManager() :Module()
{
	name = "guiManager";
}

GuiManager::~GuiManager() {}

bool GuiManager::Start()
{
	credits = Engine::GetInstance().textures.get()->Load("Assets/Textures/screens/bookBase.png");
	helptex = Engine::GetInstance().textures.get()->Load("Assets/Textures/menu.png");
	pausedMenu = Engine::GetInstance().textures.get()->Load("Assets/Textures/screens/halfBookBase.png");
	deathScreen = Engine::GetInstance().textures.get()->Load("Assets/Textures/screens/deathScreen.png");
	return true;
}

// L16: TODO 1: Implement CreateGuiControl function that instantiates a new GUI control and add it to the list of controls
GuiControl* GuiManager::CreateGuiControl(GuiControlType type, int id, const char* text, SDL_Rect bounds, Module* observer, SDL_Rect sliderBounds)
{
	GuiControl* guiControl = nullptr;

	//Call the constructor according to the GuiControlType
	switch (type)
	{
	case GuiControlType::BUTTON:
		guiControl = new GuiControlButton(id, bounds, text);
		break;
	}

	//Set the observer
	guiControl->observer = observer;

	// Created GuiControls are add it to the list of controls
	guiControlsList.push_back(guiControl);

	return guiControl;
}

bool GuiManager::Update(float dt)
{	
	if (Engine::GetInstance().scene.get()->pausedGame) {
		Engine::GetInstance().render.get()->DrawTexture(pausedMenu, 400, 50, NULL, SDL_FLIP_NONE, false);

		for (const auto& control : guiControlsList)
		{
			control->Update(dt);
		}
	}
	else if (Engine::GetInstance().scene.get()->deathScreen) {
		Engine::GetInstance().render.get()->DrawTexture(deathScreen, 0, 0, NULL, SDL_FLIP_NONE, false);

		for (const auto& control : guiControlsList)
		{
			control->Update(dt);
		}
	}
	else {
		for (const auto& control : guiControlsList)
		{
			control->Update(dt);
		}

		if (Engine::GetInstance().scene.get()->state == SceneState::CREDITS) {
			Engine::GetInstance().render.get()->DrawTexture(credits, 90, 50, NULL, SDL_FLIP_NONE, false);
		}
		if (Engine::GetInstance().scene.get()->help) Engine::GetInstance().render.get()->DrawTexture(helptex, 750, 0, NULL, SDL_FLIP_NONE, false);
	}

	return true;
}

bool GuiManager::CleanUp()
{
	for (const auto& control : guiControlsList)
	{
		delete control;
	}

	SDL_DestroyTexture(credits);
	SDL_DestroyTexture(helptex);
	SDL_DestroyTexture(pausedMenu);
	SDL_DestroyTexture(deathScreen);

	return true;
}



