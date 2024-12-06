#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"

#define IDLECOUNT 64

struct SDL_Texture;

enum EnemyState {
	ALIVE,
	DEAD
};

enum EnemyAnimationState {
	SLEEP,
	IDLE,
	ATTACK,
	MOVING,
	DAMAGE,
	JUMPING
};

enum EnemyLook {
	LEFT,
	RIGHT
};

enum EnemyType {
	FLYING,
	WALKING
};

class Enemy : public Entity
{
public:

	Enemy();
	virtual ~Enemy();

	bool Awake();

	bool Start();

	bool Update(float dt);

	void UpdateChecks();

	void WalkingEnemyUpdate(float dt);

	void FlyingEnemyUpdate(float dt);

	bool Waking();

	bool CleanUp();

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

	void ResetPath();

	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

public:
	EnemyState state = EnemyState::ALIVE;
	bool playerActivate = false;
	PhysBody* pbody;
	PhysBody* sensor;

private:

	EnemyAnimationState AnimState;

	float jumpForce = 2.0f;
	int idleCount = IDLECOUNT;
	bool idleMove = true;

	SDL_Texture* texture;
	EnemyType type;
	const char* texturePath;
	int texW, texH;
	pugi::xml_node parameters;

	Animation* currentAnimation = nullptr;
	Animation sleep;
	Animation wake;
	Animation idle;
	Animation moving;
	Animation attack;
	Animation damage;
	Animation death;

	bool isSleeping = true;
	bool startPathfinding = false;
	bool isDying = false;
	bool isDead = false;
	bool isJumping = false;
	bool isFalling = true;
	bool isDamaged = false;
	bool isAttacking = false;
	bool resetDirection = false;

	Pathfinding* pathfinding;
	bool playerFound = false;

	EnemyLook look = EnemyLook::LEFT;
	SDL_RendererFlip flip;
};
