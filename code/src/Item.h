#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"

struct SDL_Texture;

enum ItemType {
	ITEM_HEALTH,
	ITEM_POINTS,
	ITEM_ABILITY
};

class Item : public Entity
{
public:

	Item();
	virtual ~Item();

	bool Awake();

	bool Start();

	void SetType(ItemType t) {
		type = t;
	}

	bool Update(float dt);

	bool RenderUpdate();

	bool CleanUp();

	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

public:

	bool isPicked = false;
	ItemType type;

private:

	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;

	//L08 TODO 4: Add a physics to an item
	PhysBody* pbody;
};
