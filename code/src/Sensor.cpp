#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "Sensor.h"

Sensor::Sensor() : Entity(EntityType::ATTACK)
{
	name = "sensor";
}

Sensor::~Sensor() {}

bool Sensor::Awake() {
	position = Vector2D(0, 0);
	return true;
}

bool Sensor::Start() {

	if (type == SensorType::CHECKPOINT) {
		pugi::xml_document loadFile;
		pugi::xml_parse_result result = loadFile.load_file("config.xml");

		if (result == NULL) LOG("Error loading config.xml: %s", result.description());

		texture = Engine::GetInstance().textures.get()->Load(loadFile.child("config").child("scene").child("entities").child("checkpoint").attribute("texture").as_string());

		idle.LoadAnimations(loadFile.child("config").child("scene").child("entities").child("checkpoint").child("animations").child("idle"));
		active.LoadAnimations(loadFile.child("config").child("scene").child("entities").child("checkpoint").child("animations").child("active"));
		currentAnimation = &idle;

		pbody = Engine::GetInstance().physics.get()->CreateCircleSensor((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::STATIC);
		pbody->body->SetGravityScale(0);

		pbody->listener = this;
	}

	return true;
}

bool Sensor::Update(float dt)
{
	if (type == SensorType::CHECKPOINT) {
		if (isActive) currentAnimation = &active;
		Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX() - texW / 4, (int)position.getY() - texH / 2, &currentAnimation->GetCurrentFrame());
		currentAnimation->Update();
	}
	
	return true;
}

bool Sensor::CleanUp()
{
	Engine::GetInstance().textures.get()->UnLoad(texture);
	return true;
}

void Sensor::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		if(type == SensorType::CHECKPOINT) isActive = true;
		break;
	default:
		break;
	}
}

void Sensor::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		break;
	default:
		break;
	}
}