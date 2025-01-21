#pragma once

#include "Entity.h"
#include "SDL2/SDL.h"
#include "Animation.h"
#include "Pathfinding.h"
#include <SDL2/SDL_mixer.h>

#define IDLECOUNT 64

struct SDL_Texture;

enum BossState {
	BOSS_ACTIVE,
	BOSS_DEFEATED
};

enum BossLook {
	BOSS_LEFT,
	BOSS_RIGHT
};

enum BossAnimationState {
	BOSS_IDLE,
	BOSS_RUN,
	BOSS_DASH,
	BOSS_DAMAGE,
	BOSS_DEATH,
	BOSS_ATTACK
};

class Boss : public Entity
{
public:

	Boss();
	virtual ~Boss();

	void SaveStartingPos(Vector2D pos) {
		isStartingPos = true;
		startingPos = pos;
	}

	bool Awake();

	bool Start();

	bool Update(float dt);

	bool RenderUpdate();

	bool CleanUp();

	void SetWake() {
		isSleeping = false;
		startPathfinding = true;
	}

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

	void ResetPath();

	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);

public:
	BossState state = BossState::BOSS_ACTIVE;
	int timerVar = 1;
	bool playerActivate = false;
	PhysBody* pbody;
	PhysBody* sensor;

private:
	bool isStartingPos = false;
	Vector2D startingPos;

	BossAnimationState AnimState;
	BossLook look = BossLook::BOSS_RIGHT;

	float jumpForce = 2.0f;
	int idleCount = IDLECOUNT;
	bool idleMove = true;

	SDL_Texture* texture;
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

	SDL_RendererFlip flip;

	int AttackFX;
	int DamageFX;
};
