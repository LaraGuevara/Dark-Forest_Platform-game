#pragma once

#include "Module.h"
#include "Player.h"
#include "Enemy.h"
#include "Attack.h"
#include "Sensor.h"
#include <SDL2/SDL_mixer.h>


struct SDL_Texture;

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

private:
	//textures
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
	Mix_Chunk* loadFX;
	Mix_Chunk* saveFX;
	Mix_Chunk* attackFX;
	Mix_Music* backgroundMusic;

	//lists
	std::vector<Enemy*> enemyList;
	std::vector<Attack*> attackList;
	std::vector<Sensor*> checkpointList;
};