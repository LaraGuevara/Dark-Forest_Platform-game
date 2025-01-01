#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include <SDL2/SDL_mixer.h>
#include "Box2D/Box2D.h"
#include "Animation.h"
#include "Attack.h"

#define ATTACKCOOLDOWN 600
#define POWERUPTIMER 900

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

	bool RenderUpdate();

	b2Vec2 PlayerMovement(float dt, b2Vec2 velocity);

	bool CleanUp();

	// L08 TODO 6: Define OnCollision function for the player. 
	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SaveStartingPos(Vector2D pos) {
		isStartingPos = true;
		startingPos = pos;
	}

	void SetPoints(int p) {
		GemPoints = p;
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
	bool isStartingPos = false;
	Vector2D startingPos;

	int GemPoints = 0;
	bool PowerUpActive = false;

	int lifeValue;
	int power;
	int attackCooldown = ATTACKCOOLDOWN;
	int powerupTime = 0;
	int timerVar = 1;

	bool isAttacking = false;
	bool isDamaged = false;
	bool respawn;

	//Declare player parameters
	float speed = 5.0f;
	SDL_Texture* texture = NULL;
	int texW, texH;

	//Audio fx
	int runFX;
	int jumpStartFX;
	int jumpEndFX;
	int gameOverFX;
	int pickCoinFxId;
	int healthItemFxId, abilityItemFxId, pointsItemFxId;

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
};