#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"

Player::Player() : Entity(EntityType::PLAYER)
{
	name = "Player";
}

Player::~Player() {

}

bool Player::Awake() {

	position = Vector2D(0, 0);
	return true;
}

bool Player::Start() {

	godMode = false;

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

	//set checkpoint (starting point)
	checkpoint.setX(position.getX());
	checkpoint.setY(position.getY());

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	walking.LoadAnimations(parameters.child("animations").child("walking"));
	jumping.LoadAnimations(parameters.child("animations").child("jumping"));
	death.LoadAnimations(parameters.child("animations").child("death"));
	damage.LoadAnimations(parameters.child("animations").child("damage"));
	attack.LoadAnimations(parameters.child("animations").child("attack"));
	currentAnimation = &idle;

	//set life
	life = parameters.attribute("life").as_int();
	lifeValue = life;
	power = life;

	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texW/2, bodyType::DYNAMIC);
	//pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() - texW / 2, (int)position.getY() + texH / 2, texH/2, bodyType::DYNAMIC);
	sensor = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX(), (int)(position.getY() + texH), texH, 5, bodyType::KINEMATIC);

	pbody->listener = this;
	sensor->listener = this;

	pbody->ctype = ColliderType::PLAYER;
	sensor->ctype = ColliderType::SENSOR;

	Mix_Volume(1, 90);
	runFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/GRASS - Run 1.wav");
	jumpStartFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/GRASS - Pre Jump 1.wav");
	jumpEndFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/GRASS - Post Jump 1.wav");
	gameOverFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Game Over.wav");
	healthItemFxId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/items/healthFX.wav");
	abilityItemFxId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/items/abilityFX.wav");
	pointsItemFxId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/items/pointsFX.wav");

	return true;
}

bool Player::Update(float dt)
{
	//earn power points if not full
	if (power < lifeValue) {
		--attackCooldown;
		if (attackCooldown <= 0) {
			++power;
			attackCooldown = (ATTACKCOOLDOWN/timerVar);
		}
	}
	else {
		if(attackCooldown != (ATTACKCOOLDOWN / timerVar)) attackCooldown = (ATTACKCOOLDOWN / timerVar);
	}

	//turn off power up when timer runs out
	if (PowerUpActive) {
		++powerupTime;
		if (powerupTime >= POWERUPTIMER/timerVar) {
			PowerUpActive = false;
			powerupTime = 0;
		}
	}
	
	respawn = false;
	b2Vec2 velocity = b2Vec2(0, -GRAVITY_Y);
	if(isDying) velocity.y = 0;
	isMoving = false;

	// turn god mode on/off
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F10) == KEY_DOWN) {
		if (godMode) {
			godMode = false;
			pbody->body->SetType(b2_dynamicBody);
			pbody->body->SetAwake(true);
		}
		else {
			godMode = true;
			pbody->body->SetType(b2_kinematicBody);
		}
	}

	if(life <= 0) {
		playerDeath = true;
		isDamaged = false;
	}

	if (isDamaged and state != Player_State::DAMAGE and playerDeath != true){
		pbody->body->SetType(b2_staticBody);
		state = Player_State::DAMAGE;
		currentAnimation = &damage;
	}

	if (state == Player_State::DAMAGE) {
		if (damage.HasFinished()) {
			isDamaged = false;
			state = Player_State::IDLE;
			currentAnimation = &idle;
			pbody->body->SetType(b2_dynamicBody);
			pbody->body->SetAwake(true);
			damage.Reset();
		}
	}

	//check if player is attacking and set animation
	if (isAttacking and state != Player_State::ATTACK) {
		pbody->body->SetType(b2_staticBody);
		state = Player_State::ATTACK;
		currentAnimation = &attack;
	}

	// reset attack
	if (state == Player_State::ATTACK) {
		if (attack.HasFinished()) {
			isAttacking = false;
			if (isJumping) {
				state = Player_State::JUMP;
				currentAnimation = &jumping;
			}
			else {
				state = Player_State::IDLE;
				currentAnimation = &idle;
			}
			pbody->body->SetType(b2_dynamicBody);
			pbody->body->SetAwake(true);
			attack.Reset();
		}
	}

	if (state != Player_State::DIE and state != Player_State::ATTACK and state != Player_State::DAMAGE) velocity = PlayerMovement(dt, velocity);

	//reset death
	if (state == Player_State::DIE) {
		pbody->body->SetLinearVelocity(velocity);
		if (isDying) {
			if (death.HasFinished()) {
				if (!finishedDeathAnim) finishedDeathAnim = true;

				if (doRespawn) {
					pbody->body->SetType(b2_dynamicBody);
					pbody->body->SetAwake(true);
					b2Transform pbodyPos = pbody->body->GetTransform();
					SetPosition(checkpoint);
					respawn = true;
					doRespawn = false;
					finishedDeathAnim = false;
					life = lifeValue;
					isDying = false;
					playerDeath = false;
					state = Player_State::IDLE;
					look = Player_Look::RIGHT;
					currentAnimation = &idle;
					death.Reset();
				}
			}
		}
	}

	//run soundfx
	if (state == Player_State::WALK and Mix_Playing(1) == 0 and !godMode) Engine::GetInstance().audio->PlayFx(runFX, 1);

	//render
	if(state == Player_State::ATTACK) Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 10, (int)position.getY() - texH / 6, &currentAnimation->GetCurrentFrame(), flip);
	else Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 4, (int)position.getY() - texH / 6, &currentAnimation->GetCurrentFrame(), flip);
	currentAnimation->Update();

	b2Vec2 playerPos = pbody->body->GetPosition();
	sensor->body->SetTransform({ playerPos.x, playerPos.y - 0.4f}, sensor->body->GetAngle());
	return true;
}

bool Player::RenderUpdate() {
	if (state == Player_State::ATTACK) Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 10, (int)position.getY() - texH / 6, &currentAnimation->GetCurrentFrame(), flip);
	else Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 4, (int)position.getY() - texH / 6, &currentAnimation->GetCurrentFrame(), flip);
	
	return true;
}

b2Vec2 Player::PlayerMovement(float dt, b2Vec2 velocity) {
	// Move left
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
		velocity.x = -0.2 * 16;
		isMoving = true;
		look = Player_Look::LEFT;
	}

	// Move right
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
		velocity.x = 0.2 * 16;
		isMoving = true;
		look = Player_Look::RIGHT;
	}

	//Jump
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && isJumping == false && godMode == false) {
		// Apply an initial upward force
		Engine::GetInstance().audio->PlayFx(jumpStartFX, 1);
		pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);
		isJumping = true;
	}

	//god mode movement
	if (godMode) {
		bool moving = false;
		//move up
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
			velocity.y = -0.2 * 16;
			moving = true;
		}

		//move down
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
			velocity.y = 0.2 * 16;
			moving = true;
		}

		if (!moving) {
			velocity.y = 0;
		}
	}

	// If the player is jumpling, we don't want to apply gravity, we use the current velocity prduced by the jump
	if (isJumping == true) velocity = pbody->body->GetLinearVelocity();


	//change animation and state
	if (isMoving and state != Player_State::DIE and state != Player_State::ATTACK) {
		if (state != Player_State::WALK) {
			state = Player_State::WALK;
			currentAnimation = &walking;
		}
	}
	else if (isJumping) {
		if (state != Player_State::JUMP and state != Player_State::ATTACK) {
			state = Player_State::JUMP;
			currentAnimation = &jumping;
		}
	}
	else {
		if (state != Player_State::IDLE and state != Player_State::ATTACK) {
			state = Player_State::IDLE;
			currentAnimation = &idle;
		}
	}

	if (look == Player_Look::LEFT) flip = SDL_FLIP_HORIZONTAL;
	else flip = SDL_FLIP_NONE;


	// Apply the velocity to the player
	pbody->body->SetLinearVelocity(velocity);

	b2Transform pbodyPos = pbody->body->GetTransform();
	if (isJumping && state != Player_State::DIE) {
		currentAnimation = &jumping;
		if ((int)position.getY() > METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2) {
			state = Player_State::JUMP;
		}
		else state = Player_State::FALL;
	}

	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	//check if player has died
	if (!godMode) {
		if (playerDeath == true) {
			Engine::GetInstance().audio->PlayFx(gameOverFX);
			pbody->body->SetType(b2_kinematicBody);
			isJumping = false;
			state = Player_State::DIE;
			death.Reset();
			currentAnimation = &death;
			isDying = true;
		}
	}

	return velocity;
}

bool Player::CleanUp()
{
	LOG("Cleanup player");
	Engine::GetInstance().textures.get()->UnLoad(texture);
	return true;
}

void Player::OnCollision(PhysBody* physA, PhysBody* physB) {

	if(physA->ctype == ColliderType::SENSOR) LOG("SENSOR");

	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("%s Collision PLATFORM ", physA->ctype);
		//reset the jump flag when touching the ground
		if (isJumping) {
			isJumping = false;
			Engine::GetInstance().audio->PlayFx(jumpEndFX, 1);
			jumping.Reset();
		}
		break;
	case ColliderType::ITEM_ABILITY:
		LOG("Collision ITEM ABILITY");
		Engine::GetInstance().audio->PlayFx(abilityItemFxId);
		PowerUpActive = true;
		break;
	case ColliderType::ITEM_HEALTH:
		LOG("Collision ITEM HEALTH");
		Engine::GetInstance().audio->PlayFx(healthItemFxId);
		life += 2;
		if (life > lifeValue) life = lifeValue;
		break;
	case ColliderType::ITEM_POINTS:
		LOG("Collision ITEM POINTS");
		Engine::GetInstance().audio->PlayFx(pointsItemFxId);
		GemPoints += 1;
		break;
	case ColliderType::ENEMY:
		LOG("Collision ENEMY");
		if(!godMode and physB->active == false) {
			life = life - physB->damageDone;
			isDamaged = true;
			physB->active = true;
			if (physB->lookRight) damageRight = true;
			else damageRight = false;
			LOG("PLAYER DAMAGE %d", life);
		}
		break;
	case ColliderType::UNKNOWN:
		LOG("Collision UNKNOWN");
		break;
	case ColliderType::SENSOR:
		LOG("Collision SENSOR");
		break;
	case ColliderType::DEATH:
		LOG("Collision DEATH");
		if (!godMode) playerDeath = true;
		break;
	default:
		break;
	}
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("End Collision PLATFORM");
		break;
	case ColliderType::ITEM:
		LOG("End Collision ITEM");
		break;
	case ColliderType::UNKNOWN:
		LOG("End Collision UNKNOWN");
		break;
	case ColliderType::SENSOR:
		LOG("End Collision SENSOR");
		break;
	case ColliderType::ENEMY:
		LOG("End Collision ENEMY");
		if (physB->active == true and physB->dead == false) physB->active = false;
		break;
	default:
		break;
	}
}

Vector2D Player::GetPosition() {
	b2Vec2 bodyPos = pbody->body->GetTransform().p;
	Vector2D pos = Vector2D(METERS_TO_PIXELS(bodyPos.x), METERS_TO_PIXELS(bodyPos.y));
	return pos;
}

void Player::SetPosition(Vector2D pos) {
	pos.setX(pos.getX() + texW / 2);
	pos.setY(pos.getY() + texH / 2);
	b2Vec2 bodyPos = b2Vec2(PIXEL_TO_METERS(pos.getX()), PIXEL_TO_METERS(pos.getY()));
	pbody->body->SetTransform(bodyPos, 0);
}