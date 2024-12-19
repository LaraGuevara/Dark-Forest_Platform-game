#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"

struct SDL_Texture;

enum ItemType {
	HEALTH, POINTS, ABILITY
};

class Item : public Entity
{
public:

	Item();
	virtual ~Item();

	bool Awake();

	void SetType(ItemType t) {
		type = t;
	}

	bool Start();

	bool Update(float dt);

	bool CleanUp();

public:

	bool isPicked = false;

private:

	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;

	ItemType type;

	//L08 TODO 4: Add a physics to an item
	PhysBody* pbody;
};
