#pragma once

#include "Module.h"
#include "Player.h"
#include "Enemy.h"
#include "Attack.h"
#include "Sensor.h"
#include <SDL2/SDL_mixer.h>
#include "GuiControlButton.h"


struct SDL_Texture;

enum GUI_ID {
	ID_PLAY,
	ID_CONTINUE,
	ID_SETTINGS,
	ID_CREDITS,
	ID_EXIT
};

enum SceneState {
	INTRO,
	MENU,
	SETTINGS,
	GAME,
	PAUSE,
	CREDITS,
	DIE,
	LEVELCOMPLETE
};

class Scene : public Module
{
public:

	Scene();

	// Destructor
	virtual ~Scene();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);
	//Updates for each screen
	bool GameUpdate(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// Return the player position
	Vector2D GetPlayerPosition();

	//Return player life points
	int GetPlayerLife() {
		return player->life;
	}

	//Return player power points
	int GetPlayerPower() {
		return player->power;
	}

	void LoadState();

	void SaveState();

	void ChangeTimerVar(bool sixty);

	bool OnGuiMouseClickEvent(GuiControl* control);

	SceneState state = SceneState::INTRO;

private:
	int timeVar = 1;
	int introTime = 0;
	bool toExit = false;

	//textures
	//intro
	SDL_Texture* introImg;
	//main menu
	SDL_Texture* menuBackground;
	//game
	SDL_Texture* img;
	SDL_Texture* helptex;
	SDL_Texture* healthbar;
	SDL_Rect healthRect = { 0,0,80,32 };

	//trigger help menu
	bool help = false;

	//player
	Player* player;
	bool respawn = false;

	//sounds
	int loadFX;
	int saveFX;
	int attackFX;
	int backgroundMusic;

	//lists
	std::vector<Enemy*> enemyList;
	std::vector<Attack*> attackList;
	std::vector<Sensor*> checkpointList;

	//GUI
	//main menu
	GuiControlButton* startBT;
	GuiControlButton* continueBT;
	GuiControlButton* settingBT;
	GuiControlButton* creditsBT;
	GuiControlButton* exitBT;
};