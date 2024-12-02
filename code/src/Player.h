#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include <SDL2/SDL_mixer.h>
#include "Box2D/Box2D.h"
#include "Animation.h"
#include "Attack.h"

enum class Player_State{
IDLE, WALK, JUMP, FALL, ATTACK, DAMAGE, DIE};

enum class Player_Look {
	LEFT, RIGHT
};

struct SDL_Texture;

class Player : public Entity
{
public:

	Player();
	
	virtual ~Player();

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool CleanUp();

	// L08 TODO 6: Define OnCollision function for the player. 
	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	int GetXPos() {
		return position.getX();
	}

	Vector2D GetPosition();

	void SetCheckpoint(Vector2D pos) {
		pos.setY(pos.getY() - 2);
		checkpoint = pos;
	}

	void SetPosition(Vector2D pos);

public:

	int lifeValue;

	bool isAttacking = false;
	bool isDamaged = false;
	bool respawn;

	//Declare player parameters
	float speed = 5.0f;
	SDL_Texture* texture = NULL;
	int texW, texH;

	//Audio fx
	int pickCoinFxId;

	// L08 TODO 5: Add physics to the player - declare a Physics body
	PhysBody* pbody;
	PhysBody* sensor;

	float jumpForce = 1.8f; // The force to apply when jumping
	bool isJumping = false; // Flag to check if the player is currently jumping
	bool isMoving = false;
	bool isDying = false;
	Player_State state = Player_State::IDLE;
	Player_Look look = Player_Look::RIGHT;

	SDL_RendererFlip flip = SDL_FLIP_NONE;
	pugi::xml_node parameters;
	Animation* currentAnimation = nullptr;
	Animation idle;
	Animation walking;
	Animation jumping;
	Animation attack;
	Animation damage;
	Animation death;

	Vector2D checkpoint;

	bool godMode;
	bool playerDeath = false;
	bool checkpointDone = false;
	bool damageRight = false;

	Mix_Chunk* runFX;
	Mix_Chunk* jumpStartFX;
	Mix_Chunk* jumpEndFX;
	Mix_Chunk* gameOverFX;
};