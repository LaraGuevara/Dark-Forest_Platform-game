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
	position.setX(parameters.attribute("x").as_int());
	position.setY(parameters.attribute("y").as_int());
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
	attack.LoadAnimations(parameters.child("animations").child("attack"));
	currentAnimation = &idle;

	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texW/2, bodyType::DYNAMIC);
	//pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() - texW / 2, (int)position.getY() + texH / 2, texH/2, bodyType::DYNAMIC);
	sensor = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX(), (int)(position.getY() + texH), texH, 5, bodyType::KINEMATIC);

	pbody->listener = this;
	sensor->listener = this;

	pbody->ctype = ColliderType::PLAYER;
	sensor->ctype = ColliderType::SENSOR;

	Mix_Volume(1, 90);
	runFX = Mix_LoadWAV("Assets/Audio/Fx/GRASS - Run 1.wav");
	jumpStartFX = Mix_LoadWAV("Assets/Audio/Fx/GRASS - Pre Jump 1.wav");
	jumpEndFX = Mix_LoadWAV("Assets/Audio/Fx/GRASS - Post Jump 1.wav");
	gameOverFX = Mix_LoadWAV("Assets/Audio/Fx/Game Over.wav");

	return true;
}

bool Player::Update(float dt)
{
	respawn = false;
	b2Vec2 velocity = b2Vec2(0, -GRAVITY_Y);
	if(isDying) velocity.y = 0;
	isMoving = false;

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

	if (isAttacking and state != Player_State::ATTACK) {
		state = Player_State::ATTACK;
		currentAnimation = &attack;
	}

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
			attack.Reset();
		}
	}

	if (state != Player_State::DIE and state != Player_State::ATTACK) {
		// Move left
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
			velocity.x = -0.2 * dt;
			isMoving = true;
			look = Player_Look::LEFT;
		}

		// Move right
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
			velocity.x = 0.2 * dt;
			isMoving = true;
			look = Player_Look::RIGHT;
		}

		//Jump
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && isJumping == false) {
			// Apply an initial upward force
			Mix_PlayChannel(1, jumpStartFX, 0);
			pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -jumpForce), true);
			isJumping = true;
		}

		if (godMode) {
			bool moving = false;
			if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
				velocity.y = -0.2 * dt;
				moving = true;
			}

			if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
				velocity.y = 0.2 * dt;
				moving = true;
			}

			if (!moving) {
				velocity.y = 0;
			}
		}

		// If the player is jumpling, we don't want to apply gravity, we use the current velocity prduced by the jump
		if (isJumping == true)
		{
			velocity = pbody->body->GetLinearVelocity();
		}

		if (isMoving && state != Player_State::DIE and state != Player_State::ATTACK) {
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

		if (!godMode) {
			if (position.getY() >= 282) {
				Mix_PlayChannel(2, gameOverFX, 0);
				pbody->body->SetType(b2_kinematicBody);
				isJumping = false;
				state = Player_State::DIE;
				death.Reset();
				currentAnimation = &death;
				isDying = true;
			}
		}
	}

	if (state == Player_State::DIE) {
		pbody->body->SetLinearVelocity(velocity);
		if (isDying) {
			if (death.HasFinished()) {
				pbody->body->SetType(b2_dynamicBody);
				pbody->body->SetAwake(true);
				b2Transform pbodyPos = pbody->body->GetTransform();
				pbody->body->SetTransform({ checkpoint.getX() / (checkpoint.getX() / 2), checkpoint.getY() }, pbody->body->GetAngle());
				respawn = true;
				isDying = false;
				state = Player_State::IDLE;
				look = Player_Look::RIGHT;
				currentAnimation = &idle;
				death.Reset();
			}
		}
	}

	if (state == Player_State::WALK and Mix_Playing(1) == 0) Mix_PlayChannel(1, runFX, 0);

	if(state == Player_State::ATTACK) Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 10, (int)position.getY() - texH / 6, &currentAnimation->GetCurrentFrame(), flip);
	else Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 4, (int)position.getY() - texH / 6, &currentAnimation->GetCurrentFrame(), flip);
	currentAnimation->Update();

	b2Vec2 playerPos = pbody->body->GetPosition();
	sensor->body->SetTransform({ playerPos.x, playerPos.y - 0.4f}, sensor->body->GetAngle());
	return true;
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
		isJumping = false;
		Mix_PlayChannel(1, jumpEndFX, 0);
		jumping.Reset();
		//if(state == Player_State::FALL) isJumping = false;
		break;
	case ColliderType::ITEM:
		LOG("Collision ITEM");
		break;
	case ColliderType::UNKNOWN:
		LOG("Collision UNKNOWN");
		break;
	case ColliderType::SENSOR:
		LOG("Collision SENSOR");
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