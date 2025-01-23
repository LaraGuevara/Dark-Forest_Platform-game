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

enum class DirectionEnemy {
	LEFT, RIGHT
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

	DirectionEnemy GetDirection() {
		return de;
	}

	void SetParameters(pugi::xml_node parameters) {
		this->parameters = parameters;
	}

	void SetPosition(Vector2D pos);

	Vector2D GetPosition();

	void OnCollision(PhysBody* physA, PhysBody* physB);

	void OnCollisionEnd(PhysBody* physA, PhysBody* physB);
	void BossPattern();

public:
	BossState state = BossState::BOSS_ACTIVE;
	int timerVar = 1;
	bool playerActivate = false;
	PhysBody* pbody;

private:
	bool isStartingPos = false;
	Vector2D startingPos;
	DirectionEnemy de;

	BossAnimationState AnimState;
	BossLook look = BossLook::BOSS_RIGHT;

	int idleCount = IDLECOUNT;
	bool idleMove = true;
	b2Vec2 velocity;

	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;
	pugi::xml_node parameters;

	Animation* currentAnimation = nullptr;
	Animation sleep;
	Animation wake;
	Animation idle;
	Animation dash;
	Animation moving;
	Animation attack;
	Animation damage;
	Animation death;

	bool isSleeping = false;
	bool startPathfinding = false;
	bool isDying = false;
	bool isDead = false;
	bool isDamaged = false;
	bool isAttacking = false;
	bool resetDirection = false;

	bool playerFound = false;

	SDL_RendererFlip flip;

	int AttackFX;
	int DamageFX;
	int SuccessFX;

	
	int bossCooldown = 120;
	int lifes;
	bool showing = true;
	int randomAttack = 0;
	bool audioDie = false;
	bool directionLeft;
	float speed = 1.2;
	

};
