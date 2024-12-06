#include "Enemy.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Map.h"

Enemy::Enemy() : Entity(EntityType::ENEMY)
{

}

Enemy::~Enemy() {
	delete pathfinding;
}

bool Enemy::Awake() {
	return true;
}

bool Enemy::Start() {

	//initilize textures
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	moving.LoadAnimations(parameters.child("animations").child("moving"));
	attack.LoadAnimations(parameters.child("animations").child("attack"));
	damage.LoadAnimations(parameters.child("animations").child("damage"));
	death.LoadAnimations(parameters.child("animations").child("death"));

	//load damage soundfx
	Mix_Volume(4, 90);
	DamageFX = Mix_LoadWAV("Assets/Audio/Fx/Spell Impact 1.wav");

	// Set the enemy type
	if (parameters.attribute("Flyingtype").as_bool() == true) {
		AttackFX = Mix_LoadWAV("Assets/Audio/Fx/Spell Impact 3.wav");
		type = EnemyType::FLYING;
		AnimState = EnemyAnimationState::SLEEP;
		//load flying animations
		sleep.LoadAnimations(parameters.child("animations").child("sleep"));
		wake.LoadAnimations(parameters.child("animations").child("wake"));
		//Add a physics to an item - initialize the physics body
		pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texW / 2, (int)position.getY() + texH/2, texH / 4, bodyType::DYNAMIC);
		pbody->body->SetGravityScale(0);
		currentAnimation = &sleep;
	}
	else {
		AttackFX = Mix_LoadWAV("Assets/Audio/Fx/Rock Meteor Throw 2.wav");
		type = EnemyType::WALKING;
		AnimState = EnemyAnimationState::IDLE;
		look = EnemyLook::RIGHT;
		//Add a physics to an item - initialize the physics body
		pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 3, bodyType::DYNAMIC);
		currentAnimation = &idle;
	}

	//create sensor for radious around enemy (to activate pathfinding)
	sensor = Engine::GetInstance().physics.get()->CreateCircleSensor((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH * 4, bodyType::KINEMATIC);

	//Assign collider type and damage 
	pbody->ctype = ColliderType::ENEMY;
	sensor->ctype = ColliderType::SENSOR;

	pbody->damageDone = parameters.attribute("damage").as_int();

	pbody->listener = this;
	sensor->listener = this;
	
	// Set name and life values
	name = parameters.attribute("name").as_string();
	life = parameters.attribute("life").as_int();

	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	// Initialize pathfinding
	pathfinding = new Pathfinding();
	ResetPath();

	return true;
}

bool Enemy::Update(float dt)
{
	UpdateChecks();

	if (!isDying and !isDamaged) {
		if (type == EnemyType::WALKING) WalkingEnemyUpdate(dt);
		else FlyingEnemyUpdate(dt);
	}

	b2Transform pbodyPos = pbody->body->GetTransform();

	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	if (look == EnemyLook::RIGHT) {
		if (type == EnemyType::WALKING) flip = SDL_FLIP_NONE;
		else flip = SDL_FLIP_HORIZONTAL;
		pbody->lookRight = false;
	}
	else {
		if (type == EnemyType::WALKING) flip = SDL_FLIP_HORIZONTAL;
		else flip = SDL_FLIP_NONE;
		pbody->lookRight = true;
	}

	currentAnimation->Update();

	if(type == EnemyType::WALKING) Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 10, (int)position.getY() - texH / 6, &currentAnimation->GetCurrentFrame(), flip);
	else Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY() - texH / 10, &currentAnimation->GetCurrentFrame(), flip);

	b2Vec2 enemyPos = pbody->body->GetPosition();
	sensor->body->SetTransform({ enemyPos.x, enemyPos.y }, 0);

	return true;
}

void Enemy::UpdateChecks() {
	//check life to check if enemy should be dead
	if (life <= 0) isDead = true;

	//start death animation
	if (isDead and isDying == false) {
		pbody->body->SetType(b2_staticBody);
		isDying = true;
		death.Reset();
		currentAnimation = &death;
		LOG("ENEMY DEATH");
	}

	//set enemy state to dead to delete enemy
	if (isDying) {
		if (death.HasFinished()) state = EnemyState::DEAD;
	}

	//check if damaged
	if (isDamaged and AnimState != EnemyAnimationState::DAMAGE and isDead != true) {
		pbody->body->SetType(b2_staticBody);
		AnimState = EnemyAnimationState::DAMAGE;
		currentAnimation = &damage;
	}

	//reset damage animation
	if (AnimState == EnemyAnimationState::DAMAGE) {
		if (damage.HasFinished()) {
			isDamaged = false;
			AnimState = EnemyAnimationState::IDLE;
			currentAnimation = &idle;
			pbody->body->SetType(b2_dynamicBody);
			damage.Reset();
		}
	}


	//check if attacking
	if (isAttacking and AnimState != EnemyAnimationState::ATTACK) {
		AnimState = EnemyAnimationState::ATTACK;
		currentAnimation = &attack;
	}

	// reset attack
	if (AnimState == EnemyAnimationState::ATTACK) {
		if (attack.HasFinished()) {
			isAttacking = false;
			AnimState = EnemyAnimationState::IDLE;
			currentAnimation = &idle;
			attack.Reset();
		}
	}
}

void Enemy::WalkingEnemyUpdate(float dt){
	b2Vec2 velocity = b2Vec2(0, -GRAVITY_Y);
	Vector2D pos = GetPosition();
	Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());

	if (resetDirection) {
		if (look == EnemyLook::LEFT) look = EnemyLook::RIGHT;
		else look = EnemyLook::LEFT;
		resetDirection = false;
	}

	//if player is within radious, activate pathfinding
	if (playerActivate and playerFound == false) {

		//reset pathfinding
		pathfinding->ResetPath(tilePos);

		//propagate pathfinding until player is found
		bool found = false;
		while (found == false) {
			found = pathfinding->PropagateAStar(MANHATTAN);
			if (Engine::GetInstance().physics.get()->getDebug()) {
				pathfinding->DrawPath();
			}
		}

		//get the pos to travel to
		std::list<Vector2D>::iterator it = pathfinding->pathTiles.end();
		--it;
		--it;
		Vector2D nextPos = *it;

		//check if tile is jumpable
		bool jumpable = Engine::GetInstance().map.get()->IsTileJumpable(pos.getX(), pos.getY());

		//Check if next tile is to the right or left and add movement 
		if (nextPos.getX() > tilePos.getX()) {
			LOG("RIGHT");
			velocity.x = 0.15 * dt;
			look = EnemyLook::RIGHT;
		}
		else if (nextPos.getX() < tilePos.getX()) {
			LOG("LEFT: %d, %d", tilePos.getX(), nextPos.getX());
			velocity.x = -0.15 * dt;
			look = EnemyLook::LEFT;
		}

		//jumping
		if (isJumping == false and jumpable == true) {
			pbody->body->ApplyLinearImpulseToCenter(b2Vec2((velocity.x/8), -(jumpForce / 2)), true);
			isJumping = true;
			if (AnimState != EnemyAnimationState::JUMPING) {
				AnimState = EnemyAnimationState::JUMPING;
			}
		}

		if (AnimState != EnemyAnimationState::JUMPING) {
			AnimState = EnemyAnimationState::MOVING;
			currentAnimation = &moving;
		} else currentAnimation = &idle;

		if (isJumping == true) velocity = pbody->body->GetLinearVelocity();

		pbody->body->SetLinearVelocity(velocity);
	}
	else if (playerFound) {
		b2Vec2 velocity = b2Vec2(0, 0);
		pbody->body->SetLinearVelocity(velocity);
		--idleCount;
		if (idleCount <= 0) {
			playerFound = false;
			idleCount = IDLECOUNT;
		}
	}
	else {
		pathfinding->ResetPath(tilePos);
		//if player isn't within range: do idle pattern
		if (idleCount == 0) {
			if(idleMove) idleMove = false;
			else {
				idleMove = true;
				if (look == EnemyLook::LEFT) look = EnemyLook::RIGHT;
				else look = EnemyLook::LEFT;
			}
			idleCount = IDLECOUNT;
		}

		if (!idleMove) {
			--idleCount;
			if (AnimState != EnemyAnimationState::IDLE) {
				AnimState = EnemyAnimationState::IDLE;
				currentAnimation = &idle;
			}
		}
		else if (look == EnemyLook::RIGHT) {
			bool jumpable = Engine::GetInstance().map.get()->IsTileJumpable(pos.getX() + 16, pos.getY() + 16);
			if(!jumpable){
				velocity.x = 0.1 * dt;
				if (AnimState != EnemyAnimationState::MOVING) {
					AnimState = EnemyAnimationState::MOVING;
					currentAnimation = &moving;
				}
			}
			else {
				if (AnimState != EnemyAnimationState::IDLE) {
					AnimState = EnemyAnimationState::IDLE;
					currentAnimation = &idle;
				}
			}
			--idleCount;
		} else if (look == EnemyLook::LEFT) {
			bool jumpable = Engine::GetInstance().map.get()->IsTileJumpable(pos.getX() + 16, pos.getY() + 16);
			if (!jumpable) {
				velocity.x = -0.1 * dt;
				if (AnimState != EnemyAnimationState::MOVING) {
					AnimState = EnemyAnimationState::MOVING;
					currentAnimation = &moving;
				}
			}
			else {
				if (AnimState != EnemyAnimationState::IDLE) {
					AnimState = EnemyAnimationState::IDLE;
					currentAnimation = &idle;
				}
			}
			--idleCount;
		} 
		pbody->body->SetLinearVelocity(velocity);
	}
}

void Enemy::FlyingEnemyUpdate(float dt) {

	
	b2Vec2 velocity = b2Vec2(0, 0);

	Vector2D pos = GetPosition();
	Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());

	//if player is within radious, activate pathfinding
	if (playerActivate and playerFound == false) {

		if (AnimState == EnemyAnimationState::SLEEP) startPathfinding = Waking();
		
		if (startPathfinding) {
			//reset pathfinding
			pathfinding->ResetPath(tilePos);

			//propagate pathfinding until player is found
			bool found = false;
			while (found == false) {
				found = pathfinding->PropagateAStar(MANHATTAN);
				if (Engine::GetInstance().physics.get()->getDebug()) {
					pathfinding->DrawPath();
				}
			}

			//get the pos to travel to
			std::list<Vector2D>::iterator it = pathfinding->pathTiles.end();
			--it;
			--it;
			Vector2D nextPos = *it;

			//Check if next tile is to the right, left, up or down and add movement 
			bool moved = false;
			if (nextPos.getX() > tilePos.getX()) {
				velocity.x = 0.15 * dt;
				look = EnemyLook::RIGHT;
				moved = true;
			}
			else if (nextPos.getX() < tilePos.getX()) {
				velocity.x = -0.15 * dt;
				look = EnemyLook::LEFT;
				moved = true;
			}

			if (!moved) {
				if (nextPos.getY() < tilePos.getY()) {
					velocity.y = -0.15 * dt;
				}
				else if (nextPos.getY() > tilePos.getY()) {
					velocity.y = 0.15 * dt;
				}
			}

			if (AnimState != EnemyAnimationState::MOVING) {
				AnimState = EnemyAnimationState::MOVING;
				currentAnimation = &moving;
			}

			pbody->body->SetLinearVelocity(velocity);
		}
		
	}
	else if (playerFound and !isSleeping) {
		b2Vec2 velocity = b2Vec2(0, 0);
		pbody->body->SetLinearVelocity(velocity);
		--idleCount;
		if (idleCount <= 0) {
			playerFound = false;
			idleCount = IDLECOUNT;
		}
	}
	else if (!isSleeping){
		pathfinding->ResetPath(tilePos);
		//if player isn't within range: do idle pattern
		if (idleCount == IDLECOUNT/2) {
			if (idleMove) idleMove = false;
			else {
				idleMove = true;
				if (look == EnemyLook::LEFT) look = EnemyLook::RIGHT;
				else look = EnemyLook::LEFT;
			}
			idleCount = IDLECOUNT;
		}

		if (!idleMove) {
			--idleCount;
			if (AnimState != EnemyAnimationState::IDLE) {
				AnimState = EnemyAnimationState::IDLE;
				currentAnimation = &idle;
			}
		}
		else if (look == EnemyLook::RIGHT) {
			velocity.x = 0.1 * dt;
			if (AnimState != EnemyAnimationState::MOVING) {
				AnimState = EnemyAnimationState::MOVING;
				currentAnimation = &idle;
			}
			--idleCount;
		}
		else if (look == EnemyLook::LEFT) {
			velocity.x = -0.1 * dt;
			if (AnimState != EnemyAnimationState::MOVING) {
				AnimState = EnemyAnimationState::MOVING;
				currentAnimation = &moving;
			}
			--idleCount;
		}
		pbody->body->SetLinearVelocity(velocity);
	}
}

bool Enemy::Waking() {
	if (isSleeping) {
		currentAnimation = &wake;
		isSleeping = false;
	}
	else {
		if (wake.HasFinished()) {
			AnimState == EnemyAnimationState::IDLE;
			currentAnimation = &idle;
			return true;
		}
	}
	return false;
}

bool Enemy::CleanUp()
{
	return true;
}

void Enemy::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() );
	pos.setY(pos.getY() );
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Enemy::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}

void Enemy::ResetPath() {
	Vector2D pos = GetPosition();
	Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
	pathfinding->ResetPath(tilePos);
}

void Enemy::OnCollision(PhysBody* physA, PhysBody* physB) {

	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		if (isJumping) {
			isJumping = false;
		}
		break;
	case ColliderType::PLAYER:
		if (physA->ctype == ColliderType::SENSOR and playerActivate == false) {
			LOG("ENEMY ACTIVE");
			playerActivate = true;
			resetDirection = true;
		}
		else {
			playerFound = true;
			isAttacking = true;
			Mix_PlayChannel(4, AttackFX, 0);
		}
		break;
	case ColliderType::ATTACK:
		Mix_PlayChannel(4, DamageFX, 0);
		life = life - physB->damageDone;
		isDamaged = true;
		LOG("DAMAGE %d", life);
		break;
	case ColliderType::DEATH:
		if(type == EnemyType::WALKING) state = EnemyState::DEAD;
		break;
	default:
		break;
	}
}

void Enemy::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::ATTACK:
		break;
	case ColliderType::PLAYER:
		if (physA->ctype == ColliderType::SENSOR and playerActivate == true) {
			LOG("ENEMY DISABLED");
			playerActivate = false;
			resetDirection = true;
			playerFound = false;
		}
		break;
	default:
		break;
	}
}