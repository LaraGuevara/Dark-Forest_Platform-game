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
	death.LoadAnimations(parameters.child("animations").child("death"));
	currentAnimation = &idle;
	
	//Add a physics to an item - initialize the physics body
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::DYNAMIC);

	//Assign collider type and damage 
	pbody->ctype = ColliderType::ENEMY;
	pbody->damageDone = parameters.attribute("damage").as_int();
	pbody->listener = this;

	// Set the enemy type
	if (parameters.attribute("type").as_string() == "flying") type = EnemyType::FLYING;
	else type = EnemyType::WALKING;
	
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
	// Pathfinding testing inputs
	if (life <= 0) isDead = true;
	
	if (isDead and isDying == false){
		isDying = true;
		//death.Reset();
		currentAnimation = &death;
	}
	
	if (isDying){
		if (death.HasFinished()) state = EnemyState::DEAD;
	}


	if(!isDying) {
		b2Vec2 velocity = b2Vec2(0, -GRAVITY_Y);
		isFalling = true;

		Vector2D pos = GetPosition();
		Vector2D tilePos = Engine::GetInstance().map.get()->WorldToMap(pos.getX(), pos.getY());
		pathfinding->ResetPath(tilePos);

		bool found = false;
		while(found == false) {
			found = pathfinding->PropagateAStar(MANHATTAN);
			if (Engine::GetInstance().physics.get()->getDebug()) {
				pathfinding->DrawPath();
			}
		}
		int length = pathfinding->breadcrumbs.size();
		Vector2D nextPos = Engine::GetInstance().map.get()->WorldToMap(pathfinding->breadcrumbs[length-2].getX(), pathfinding->breadcrumbs[length - 2].getY());

		bool jumpable = Engine::GetInstance().map.get()->IsTileJumpable(pos.getX(), pos.getY());

		bool moved = false;
		if(nextPos.getX() > pos.getX()){
			velocity.x = 0.2 * dt;
			look = EnemyLook::LEFT;
			moved = true;
		}
		else {
			velocity.x = -0.2 * dt;
			look = EnemyLook::RIGHT;
			moved = true;
		}

		if (isJumping == false and jumpable == true) {
			LOG("JUMP");
			pbody->body->ApplyLinearImpulseToCenter(b2Vec2(0, -(jumpForce/2)), true);
			isJumping = true;
		}

		if (isJumping == true) velocity = pbody->body->GetLinearVelocity();


		pbody->body->SetLinearVelocity(velocity);
		b2Transform pbodyPos = pbody->body->GetTransform();

		position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
		position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

		if (look == EnemyLook::LEFT) flip = SDL_FLIP_HORIZONTAL;
		else flip = SDL_FLIP_NONE;

		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 10, (int)position.getY() - texH / 6, &currentAnimation->GetCurrentFrame(), flip);
	}

	return true;
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
		isFalling = false;
		break;
	case ColliderType::ATTACK:
		life = life - physB->damageDone;
		LOG("DAMAGE %d", life);
		break;
	case ColliderType::DEATH:
		state = EnemyState::DEAD;
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
	default:
		break;
	}
}