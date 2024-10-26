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

	//Load animations
	idle.LoadAnimations(parameters.child("animations").child("idle"));
	walking.LoadAnimations(parameters.child("animations").child("walking"));
	jumping.LoadAnimations(parameters.child("animations").child("jumping"));
	death.LoadAnimations(parameters.child("animations").child("death"));
	currentAnimation = &idle;

	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX(), (int)position.getY(), texH / 2, bodyType::DYNAMIC);
	sensor = Engine::GetInstance().physics.get()->CreateRectangleSensor((int)position.getX(), (int)(position.getY() + texH), texH, 5, bodyType::KINEMATIC);

	pbody->listener = this;
	sensor->listener = this;

	pbody->ctype = ColliderType::PLAYER;
	sensor->ctype = ColliderType::SENSOR;
	gravityScale = pbody->body->GetGravityScale();

	return true;
}

bool Player::Update(float dt)
{
	b2Vec2 velocity = b2Vec2(0, -GRAVITY_Y);
	isMoving = false;

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F10) == KEY_DOWN) {
		if (godMode) {
			godMode = false;
			pbody->body->SetType(b2_dynamicBody);
			//pbody->body->SetGravityScale(gravityScale);
		}
		else {
			godMode = true;
			pbody->body->SetType(b2_kinematicBody);
			//pbody->body->SetGravityScale(0.0f);
		}
	}

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
	if(isJumping == true)
	{
		velocity = pbody->body->GetLinearVelocity();
	}

	if(isMoving){
		if (state != Player_State::WALK) {
			state = Player_State::WALK;
			currentAnimation = &walking;
		}
	}
	else if(isJumping){
		if (state != Player_State::JUMP) {
			state = Player_State::JUMP;
			currentAnimation = &jumping;
		}
	}
	else {
		if (state != Player_State::IDLE) {
			state = Player_State::IDLE;
			currentAnimation = &idle;
		}
	}

	if (look == Player_Look::LEFT) flip = SDL_FLIP_HORIZONTAL;
	else flip = SDL_FLIP_NONE;

	// Apply the velocity to the player
	pbody->body->SetLinearVelocity(velocity);

	b2Transform pbodyPos = pbody->body->GetTransform();
	if (isJumping) {
		if ((int)position.getY() > METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2) {
			state = Player_State::JUMP;
		}
		else state = Player_State::FALL;
	}

	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() - texW/6, (int)position.getY(), &currentAnimation->GetCurrentFrame(), flip);
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
		if(state == Player_State::FALL) isJumping = false;
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