#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Attack.h"

Attack::Attack() : Entity(EntityType::ATTACK)
{
	name = "attack";
	//Start();
}

Attack::~Attack() {}

bool Attack::Awake() {
	position = Vector2D(0, 0);
	return true;
}

bool Attack::Start() {

	//initilize textures
	texture = Engine::GetInstance().textures.get()->Load("Assets/Textures/attack.png");

	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");

	if (result == NULL) LOG("Error loading config.xml: %s", result.description());
	
	animation.LoadAnimations(loadFile.child("config").child("scene").child("entities").child("attack").child("animation").child("idle"));
	currentAnimation = &animation;
	
	// L08 TODO 4: Add a physics to an item - initialize the physics body
	//Engine::GetInstance().textures.get()->GetSize(texture, texW, texH);
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 4, bodyType::KINEMATIC);
	pbody->body->SetGravityScale(0);

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::ATTACK;

	pbody->listener = this;

	if (flip == SDL_FLIP_NONE) velocity = b2Vec2(speed, 0);
	else velocity = b2Vec2(-speed, 0);

	return true;
}

bool Attack::Update(float dt)
{
	// L08 TODO 4: Add a physics to an item - update the position of the object from the physics.  
	if (!collision) {
		pbody->body->SetLinearVelocity(velocity);
		b2Transform pbodyPos = pbody->body->GetTransform();
		position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
		position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() + texW / 4, (int)position.getY() - texH / 6, &currentAnimation->GetCurrentFrame(), flip);
		currentAnimation->Update();
	}
	
	return true;
}

bool Attack::CleanUp()
{
	Engine::GetInstance().textures.get()->UnLoad(texture);
	return true;
}

void Attack::OnCollision(PhysBody* physA, PhysBody* physB) {
	
}

void Attack::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		break;
	default:
		collision = true;
		break;
	}
}