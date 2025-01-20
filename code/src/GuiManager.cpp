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
	credits = Engine::GetInstance().textures.get()->Load("Assets/Textures/screens/credits.png");
	helptex = Engine::GetInstance().textures.get()->Load("Assets/Textures/menu.png");
	pausedMenu = Engine::GetInstance().textures.get()->Load("Assets/Textures/screens/halfBookBase.png");
	deathScreen = Engine::GetInstance().textures.get()->Load("Assets/Textures/screens/deathScreen.png");
	finishedLevel = Engine::GetInstance().textures.get()->Load("Assets/Textures/screens/finishedLevel.png");
	emptyMenu= Engine::GetInstance().textures.get()->Load("Assets/Textures/menu_empty.png");
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
	case GuiControlType::SLIDER:
		guiControl = new GuiControlButton(id, bounds, text);
		break;
	case GuiControlType::SLIDERBAR:
		guiControl = new GuiControlButton(id, bounds, text);
		break;
	}
	

	//Set the observer
	guiControl->observer = observer;

	// Created GuiControls are add it to the list of controls
	guiControlsList.push_back(guiControl);

	return guiControl;
}

void GuiManager::DeleteGUIControl(int id) {

	GuiControl* target = nullptr;
	for (const auto& control : guiControlsList) {
		if (control->id == id) target = control;
	}

	if (target != nullptr) {
		std::list<GuiControl*>::iterator i = guiControlsList.begin();
		while (i != guiControlsList.end()) {
			if (*i == target) i = guiControlsList.erase(i);
			else i++;
		}
	}
}

bool GuiManager::Update(float dt)
{	
	if (Engine::GetInstance().scene.get()->pausedGame) {
		Engine::GetInstance().render.get()->DrawTexture(pausedMenu, 400, 50, NULL, SDL_FLIP_NONE, false);
	}
	else if (Engine::GetInstance().scene.get()->deathScreen) {
		Engine::GetInstance().render.get()->DrawTexture(deathScreen, 0, 0, NULL, SDL_FLIP_NONE, false);
	}
	else if (Engine::GetInstance().scene.get()->levelFinishedScreen) {
		Engine::GetInstance().render.get()->DrawTexture(finishedLevel, 400, 70, NULL, SDL_FLIP_NONE, false);
		float timeCount = (float)(Engine::GetInstance().scene.get()->finalTime);
		timeCount = std::round(timeCount * 100.0f) / 100.0f;
		std::snprintf(buffer, sizeof(buffer), "%.2f", timeCount);
		std::string time = buffer;
		std::string text = "Time: ";
		std::string sec = "s";
		Engine::GetInstance().render.get()->DrawText((text + time + sec).c_str(), 615, 335, 100, 32);
	}
	else if (Engine::GetInstance().scene.get()->config) {
		Engine::GetInstance().render.get()->DrawTexture(finishedLevel, 400, 70, NULL, SDL_FLIP_NONE, false);
	}
	
	for (const auto& control : guiControlsList)
	{
		control->Update(dt);
	}

	if (Engine::GetInstance().scene.get()->state == SceneState::CREDITS) Engine::GetInstance().render.get()->DrawTexture(credits, 90, 50, NULL, SDL_FLIP_NONE, false);
	if (Engine::GetInstance().scene.get()->help) Engine::GetInstance().render.get()->DrawTexture(helptex, 750, 0, NULL, SDL_FLIP_NONE, false);

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
	SDL_DestroyTexture(emptyMenu);

	return true;
}



