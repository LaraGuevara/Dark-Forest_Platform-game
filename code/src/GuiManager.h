#pragma once

#include "Module.h"
#include "GuiControl.h"

#include <list>

class GuiManager : public Module
{
public:

	// Constructor
	GuiManager();

	// Destructor
	virtual ~GuiManager();

	// Called before the first frame
	 bool Start();

	 // Called each loop iteration
	 bool Update(float dt);

	 //delete control
	 void DeleteGUIControl(int id);

	// Called before quitting
	bool CleanUp();

	// Additional methods
	GuiControl* CreateGuiControl(GuiControlType type, int id, const char* text, SDL_Rect bounds, Module* observer, SDL_Rect sliderBounds = { 0,0,0,0 });

public:

	std::list<GuiControl*> guiControlsList;
	SDL_Texture* texture;

	//on screen GUI menus
	SDL_Texture* credits;
	SDL_Texture* helptex;
	SDL_Texture* pausedMenu;
	SDL_Texture* deathScreen;
	SDL_Texture* finishedLevel;
	SDL_Texture* emptyMenu;

	//buffer to draw time on screen
	char buffer[10];

	bool drawBorders = false; 
};

