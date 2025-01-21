#include "Boss.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Map.h"
#include "tracy/Tracy.hpp"

Boss::Boss() : Entity(EntityType::BOSS)
{

}

Boss::~Boss() {
	delete pathfinding;
}

bool Boss::Awake() {
	return true;
}

bool Boss::Start() {

	//initilize textures
	texture = Engine::GetInstance().textures.get()->Load(parameters.attribute("texture").as_string());
	if (isStartingPos) {
		position.setX(startingPos.getX());
		position.setY(startingPos.getY());
	}
	else {
		position.setX(parameters.attribute("x").as_int());
		position.setY(parameters.attribute("y").as_int());
	}
	texW = parameters.attribute("w").as_int();
	texH = parameters.attribute("h").as_int();

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	/*moving.LoadAnimations(parameters.child("animations").child("moving"));
	attack.LoadAnimations(parameters.child("animations").child("attack"));
	damage.LoadAnimations(parameters.child("animations").child("damage"));
	death.LoadAnimations(parameters.child("animations").child("death"));*/

	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 3, bodyType::DYNAMIC);
	currentAnimation = &idle;
	AnimState = BossAnimationState::BOSS_IDLE;

	//load damage soundfx
	Mix_Volume(4, 90);
	DamageFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Spell Impact 1.wav");

	//create sensor for radious around enemy (to activate pathfinding)
	sensor = Engine::GetInstance().physics.get()->CreateCircleSensor((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH * 4, bodyType::KINEMATIC);

	//Assign collider type and damage 
	pbody->ctype = ColliderType::BOSS;
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

bool Boss::Update(float dt)
{
	ZoneScoped;

	b2Transform pbodyPos = pbody->body->GetTransform();

	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	if (look == BossLook::BOSS_RIGHT) {
		flip = SDL_FLIP_NONE;
		pbody->lookRight = false;
	}
	else {
		flip = SDL_FLIP_HORIZONTAL;
		pbody->lookRight = true;
	}

	currentAnimation->Update();

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 10, (int)position.getY() - texH / 6, &currentAnimation->GetCurrentFrame(), flip);

	b2Vec2 enemyPos = pbody->body->GetPosition();
	sensor->body->SetTransform({ enemyPos.x, enemyPos.y }, 0);

	return true;
}

bool Boss::RenderUpdate() {

	ZoneScoped;
	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 10, (int)position.getY() - texH / 6, &currentAnimation->GetCurrentFrame(), flip);

	return true;
}

bool Boss::CleanUp()
{
	return true;
}

void Boss::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() );
	pos.setY(pos.getY() );
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}

Vector2D Boss::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}

void Boss::ResetPath() {
	Vector2D pos = GetPosition();
	Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
	pathfinding->ResetPath(tilePos);
}

void Boss::OnCollision(PhysBody* physA, PhysBody* physB) {
	
		switch (physB->ctype)
		{
		case ColliderType::PLATFORM:
			if (isJumping) {
				isJumping = false;
			}
			break;
		case ColliderType::PLAYER:
			if (!isDead) {
				if (physA->ctype == ColliderType::SENSOR and playerActivate == false) {
					LOG("ENEMY ACTIVE");
					playerActivate = true;
					resetDirection = true;
				}
				else {
					playerFound = true;
					isAttacking = true;
					Engine::GetInstance().audio->PlayFx(AttackFX, 4);
				}
			}
			break;
		case ColliderType::ATTACK:
			if (!isDead) {
				if (physA->ctype == ColliderType::ENEMY) {
					Engine::GetInstance().audio->PlayFx(DamageFX, 4);
					life = life - physB->damageDone;
					isDamaged = true;
					if (isSleeping) {
						isSleeping = false;
						startPathfinding = true;
					}
					LOG("DAMAGE %d", life);
				}
			}
			break;
		case ColliderType::DEATH:
			state = BossState::BOSS_DEFEATED;
			break;
		default:
			break;
		}
	}


void Boss::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
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



//boss data

void Enemy::BossPattern(float dt) {
	velocity = b2Vec2(0, -GRAVITY_Y);

	if (currentAnimation == &crouch) texW = 48;
	else if (currentAnimation == &attack) texW = 80;
	else if (currentAnimation == &die) texW = 64;
	else texW = parameters.attribute("w").as_int();

	if ((currentAnimation == &attack || currentAnimation == &dmg) && currentAnimation->HasFinished()) currentAnimation = &idle;

	if (bossActive && !Engine::GetInstance().scene.get()->IsPause()) {

		if (bossCooldown >= 0)
			bossCooldown--;

		if (randomAttack == 2 && currentAnimation == &attack && bossCooldown == 100) {
			fireball = true;
			Engine::GetInstance().audio.get()->PlayFx(bossSword);
		}
		if (randomAttack == 3 && currentAnimation == &attack && (bossCooldown == 100 || bossCooldown == 80)) {
			fireball = true;
			Engine::GetInstance().audio.get()->PlayFx(bossSword);
		}


		if (bossCooldown <= 0) {

			if (currentAnimation == &walk) {
				currentAnimation = &idle;
				bossCooldown = 60;
			}
			else {
				randomAttack = rand() % 3 + 1;

				switch (randomAttack) {
				case 2:
					currentAnimation = &attack;

					currentAnimation->Reset();
					bossCooldown = 120;
					break;
				case 3:
					currentAnimation = &attack;

					currentAnimation->Reset();
					bossCooldown = 120;
					break;
				default:
					currentAnimation = &walk;
					bossCooldown = 160;
					de = directionLeft ? DirectionEnemy::LEFT : DirectionEnemy::RIGHT;
					break;
				}
			}
		}
		else if (bossCooldown <= 1 && currentAnimation == &walk) {
			directionLeft = !directionLeft;
			de = directionLeft ? DirectionEnemy::LEFT : DirectionEnemy::RIGHT;
		}

		if (currentAnimation == &walk) {
			if (directionLeft) {
				velocity.x = -speed * 2;
				flipType = SDL_FLIP_HORIZONTAL;
			}
			else {
				velocity.x = +speed * 2;
				flipType = SDL_FLIP_NONE;
			}
		}

	}

	if (isJumping)velocity = pbody->body->GetLinearVelocity();
	pbody->body->SetLinearVelocity(velocity);

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	if (currentAnimation != &attack)
		Engine::GetInstance().render.get()->DrawTexture(texture, flipType, (int)position.getX() + texW / 3, (int)position.getY() - texH / 4, &currentAnimation->GetCurrentFrame());
	else
		Engine::GetInstance().render.get()->DrawTexture(texture, flipType, (int)position.getX() - 4, (int)position.getY() - texH / 4, &currentAnimation->GetCurrentFrame());

	currentAnimation->Update();

	b2Vec2 enemyPos = pbody->body->GetPosition();
	sensor->body->SetTransform({ enemyPos.x, enemyPos.y }, 0);

	if (currentAnimation == &die) {
		if (!audioDie) {
			Engine::GetInstance().audio.get()->PlayFx(bossDie);
			audioDie = true;
		}
		if (currentAnimation->HasFinished())dead = true;
	}

}


//-----------


if (type == EnemyType::BOSS) flipType = SDL_FLIP_HORIZONTAL;

// Initialize pathfinding
pathfinding = new Pathfinding();
ResetPath();