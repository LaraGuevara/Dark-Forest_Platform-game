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

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	bool Update(float dt);

	bool RenderUpdate();

	bool CleanUp();

	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

public:

	int id;
	bool isPicked = false;
	ItemType type;
	PhysBody* pbody;

private:

	pugi::xml_node parameters;
	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;
};
