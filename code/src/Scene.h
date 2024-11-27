#pragma once

#include "Module.h"
#include "Player.h"
#include "Enemy.h"
#include "Attack.h"
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

	void LoadState();

	void SaveState();

private:
	SDL_Texture* img;
	SDL_Texture* helptex;

	//L03: TODO 3b: Declare a Player attribute
	Player* player;
	std::vector<Enemy*> enemyList;
	bool help = false;
	bool respawn = false;

	Mix_Chunk* loadFX;
	Mix_Chunk* saveFX;
	Mix_Chunk* attackFX;
	Mix_Music* backgroundMusic;

	std::vector<Attack*> attackList;
};