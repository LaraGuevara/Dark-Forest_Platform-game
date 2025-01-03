#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Box2D/Box2D.h"

struct SDL_Texture;

enum SensorType {
	DEFAULT,
	CHECKPOINT
};

class Sensor : public Entity
{
public:

	Sensor();
	virtual ~Sensor();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool RenderUpdate();

	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	bool CleanUp();

public:
	int id;
	bool isAdded = false;
	bool isActive = false;
	bool deactivate = false;
	SensorType type = SensorType::DEFAULT;
	PhysBody* pbody;
	Vector2D playerPos;

private:

	SDL_Texture* texture;
	int texH = 32;
	int texW = 32;

	Animation idle;
	Animation active;
	Animation* currentAnimation = nullptr;
};
