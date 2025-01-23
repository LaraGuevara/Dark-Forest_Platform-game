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
	texH = parameters.attribute("h").as_int();
	texW = parameters.attribute("w").as_int();

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	moving.LoadAnimations(parameters.child("animations").child("walking"));
	dash.LoadAnimations(parameters.child("animations").child("dash"));
	attack.LoadAnimations(parameters.child("animations").child("attack"));
	damage.LoadAnimations(parameters.child("animations").child("damage"));
	death.LoadAnimations(parameters.child("animations").child("death"));

	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 3, bodyType::DYNAMIC);
	currentAnimation = &idle;
	AnimState = BossAnimationState::BOSS_IDLE;

	//load damage soundfx
	DamageFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Spell Impact 1.wav");
	AttackFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Sword Attack 1.wav");
	SuccessFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Success 3.wav");

	//Assign collider type and damage 
	pbody->ctype = ColliderType::BOSS;

	pbody->damageDone = parameters.attribute("damage").as_int();

	pbody->listener = this;
	
	// Set name and life values
	name = parameters.attribute("name").as_string();
	life = parameters.attribute("life").as_int();

	// Set the gravity of the body
	if (!parameters.attribute("gravity").as_bool()) pbody->body->SetGravityScale(0);

	return true;
}

bool Boss::Update(float dt)
{
	ZoneScoped;

	b2Transform pbodyPos = pbody->body->GetTransform();

	BossPattern();

	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

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

void Boss::OnCollision(PhysBody* physA, PhysBody* physB) {
	
		switch (physB->ctype)
		{
		case ColliderType::PLATFORM:
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
				Engine::GetInstance().audio->PlayFx(DamageFX, 4);
				life = life - physB->damageDone;
				isDamaged = true;
				LOG("DAMAGE %d", life);
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

void Boss::BossPattern() {
	velocity = b2Vec2(0, -GRAVITY_Y);

	
	if (life <= 0) {
		isDying = true;
		currentAnimation = &death;
	}

	if ((currentAnimation == &attack || currentAnimation == &damage) && currentAnimation->HasFinished()) currentAnimation = &idle;

	if (!isSleeping and !isDying) {
		

		if (bossCooldown >= 0)
			bossCooldown--;

		if (randomAttack == 2 && currentAnimation == &attack && bossCooldown == 100) {
			
			Engine::GetInstance().audio.get()->PlayFx(AttackFX);
		}
		if (randomAttack == 3 && currentAnimation == &attack && (bossCooldown == 100 || bossCooldown == 80)) {
			
			Engine::GetInstance().audio.get()->PlayFx(AttackFX);
		}


		if (bossCooldown <= 0) {

			if (currentAnimation == &moving) {
				AnimState = BossAnimationState::BOSS_IDLE;
				currentAnimation = &idle;
				bossCooldown = 60;
			}
			else {
				randomAttack = rand() % 3 + 1;

				switch (randomAttack) {
				case 2:
					currentAnimation = &attack;
					AnimState = BossAnimationState::BOSS_ATTACK;
					flip = SDL_FLIP_HORIZONTAL;

					currentAnimation->Reset();
					bossCooldown = 120;
					break;
				case 3:
					currentAnimation = &attack;
					AnimState = BossAnimationState::BOSS_ATTACK;
					flip = SDL_FLIP_HORIZONTAL;

					currentAnimation->Reset();
					bossCooldown = 120;
					break;
				default:
					currentAnimation = &moving;
					AnimState = BossAnimationState::BOSS_RUN;
					bossCooldown = 160;
					de = directionLeft ? DirectionEnemy::LEFT : DirectionEnemy::RIGHT;
					break;
				}
			}
		}
		else if (bossCooldown <= 1 && currentAnimation == &moving) {
			AnimState = BossAnimationState::BOSS_RUN;
			directionLeft = !directionLeft;
			de = directionLeft ? DirectionEnemy::LEFT : DirectionEnemy::RIGHT;
		}

		if (currentAnimation == &moving or currentAnimation == &dash) {
			AnimState = BossAnimationState::BOSS_RUN;
			if (directionLeft) {
				AnimState = BossAnimationState::BOSS_DASH;
				currentAnimation = &dash;
				velocity.x = -speed * 3;
				flip = SDL_FLIP_HORIZONTAL;
			}
			else {
				velocity.x = +speed * 2;
				flip = SDL_FLIP_NONE;
			}
		}

	}

	pbody->body->SetLinearVelocity(velocity);

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	switch (AnimState) {
	case BossAnimationState::BOSS_ATTACK:
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() - texW/2, (int)position.getY() - 30, &currentAnimation->GetCurrentFrame(), flip);
		break;
	case BossAnimationState::BOSS_DAMAGE:
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 2, (int)position.getY() - texH / 6, &currentAnimation->GetCurrentFrame(), flip);
		break;
	case BossAnimationState::BOSS_DASH:
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 10, (int)position.getY() - texW/3, &currentAnimation->GetCurrentFrame(), flip);
		break;
	case BossAnimationState::BOSS_DEATH:
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 10, (int)position.getY(), &currentAnimation->GetCurrentFrame(), flip);
		break;
	case BossAnimationState::BOSS_IDLE:
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 2, (int)position.getY() - texH/6, &currentAnimation->GetCurrentFrame(), flip);
		break;
	case BossAnimationState::BOSS_RUN:
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 10, (int)position.getY() - texW/3, &currentAnimation->GetCurrentFrame(), flip);
		break;
	}

	currentAnimation->Update();

	if (currentAnimation == &death) {
		
		if (currentAnimation->HasFinished()) {
			isDead = true;
			Engine::GetInstance().audio->PlayFx(SuccessFX, 4);
			state = BossState::BOSS_DEFEATED;
		}

	}

}


//-----------




