#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Box2D/Box2D.h"

struct SDL_Texture;

class Attack : public Entity
{
public:

	Attack();
	virtual ~Attack();

	void SetToPowerUp() {
		isPowerUp = true;
	}

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool RenderUpdate();

	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void SetFlip(SDL_RendererFlip f) {
		flip = f;
	}

	bool CleanUp();

public:

	bool collision = false;
	PhysBody* pbody;

private:

	bool isPowerUp = false;

	SDL_Texture* texture;
	int texH = 32;
	int texW = 32;

	//L08 TODO 4: Add a physics to an item

	b2Vec2 velocity;
	float speed = 5.8f;
	Animation animation;
	Animation* currentAnimation = nullptr;
	SDL_RendererFlip flip;
};
