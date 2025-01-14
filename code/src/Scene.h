#pragma once

#include "Module.h"
#include "Player.h"
#include "Enemy.h"
#include "Attack.h"
#include "Item.h"
#include "Sensor.h"
#include <SDL2/SDL_mixer.h>
#include "GuiControlButton.h"

#define LEVELS 2


struct SDL_Texture;

enum GUI_ID {
	ID_PLAY,
	ID_CONTINUE,
	ID_SETTINGS,
	ID_CREDITS,
	ID_EXIT,
	ID_RESUME,
	ID_PAUSED_SETTINGS,
	ID_TITLE,
	ID_PAUSED_EXIT,
	ID_RESPAWN,
	ID_NEXT,
	ID_TELEPORT
};

enum SceneState {
	INTRO,
	MENU,
	SETTINGS,
	GAME,
	CREDITS
};

struct Teleport {
	int level;
	int id;
	int UI_ID;
	Vector2D playerPos;
};

class Scene : public Module
{
public:

	Scene();

	// Destructor
	virtual ~Scene();

	// Called before render is available
	bool Awake();

	void NewGameAwake();
	void ContinueGameAwake();

	// Called before the first frame
	bool Start();

	//load level
	void LoadLevel(int lvl);

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);
	
	bool GameUpdate(float dt);

	// Sets player at the start of the corresponding level
	void SetAtLevelStart(int lvl);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// Make the buttons for teleporting between checkpoints
	void TeleportUI();
	void EndTeleportUI();

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

	//trigger level finished screen
	bool levelFinishedScreen = false;

	//trigger help menu
	bool help = false;

	//trigger config menu
	bool config = false;

	//trigger pause menu
	bool pausedGame = false;
	bool disabledButtons = true;

	//trigger death screen
	bool deathScreen = false;

	bool gameAwake = false;
	bool continueGame = false;

	//trigger checkpoint teleport
	bool checkpointTeleportView = false;

	//what level player is in
	int level = 1;

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
	SDL_Texture* gemIcon;
	SDL_Rect gemRect = { 0,0,16,16 };
	SDL_Rect healthRect = { 0,0,80,32 };

	//player
	Player* player;
	bool respawn = false;
	int playerPoints = 0;

	//sounds (ids)
	int loadFX;
	int saveFX;
	int attackFX;
	int backgroundMusic;

	//lists
	std::vector<Enemy*> enemyList;
	std::vector<Item*> itemList;
	std::vector<Attack*> attackList;
	std::vector<Sensor*> checkpointList;
	std::vector<Teleport> checkpointTPList;
	std::vector<GuiControlButton*> teleportBTs;

	//GUI
	//main menu
	GuiControlButton* startBT;
	GuiControlButton* continueBT;
	GuiControlButton* settingBT;
	GuiControlButton* creditsBT;
	GuiControlButton* exitBT;
	//paused menu
	GuiControlButton* resumeBT;
	GuiControlButton* PAUSEDsettingsBT;
	GuiControlButton* titleBT;
	GuiControlButton* PAUSEDexitBT;
	//death screen 
	GuiControlButton* respawnBT;
	//finished level
	GuiControlButton* nextBT;

	//config
	GuiControlButton* musicSlider;
	GuiControlButton* fxSlider;
	GuiControlButton* fullscreenCheckBox;
};