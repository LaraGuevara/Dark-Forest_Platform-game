#include "Item.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"

Item::Item() : Entity(EntityType::ITEM)
{
	name = "item";
}

Item::~Item() {}

bool Item::Awake() {
	return true;
}

bool Item::Start() {

	//initilize textures
	switch (type){
	case ItemType::ABILITY:
		texture = Engine::GetInstance().textures.get()->Load("Assets/Textures/goldCoin.png");
		break;
	case ItemType::HEALTH:
		texture = Engine::GetInstance().textures.get()->Load("Assets/Textures/goldCoin.png");
		break;
	case ItemType::POINTS:
		texture = Engine::GetInstance().textures.get()->Load("Assets/Textures/goldCoin.png");
		break;
	}
	
	// L08 TODO 4: Add a physics to an item - initialize the physics body
	Engine::GetInstance().textures.get()->GetSize(texture, texW, texH);
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::DYNAMIC);

	switch (type) {
	case ItemType::ABILITY:
		pbody->ctype = ColliderType::ITEM_ABILITY;
		break;
	case ItemType::HEALTH:
		pbody->ctype = ColliderType::ITEM_HEALTH;
		break;
	case ItemType::POINTS:
		pbody->ctype = ColliderType::ITEM_POINTS;
		break;
	}

	return true;
}

bool Item::Update(float dt)
{
	// L08 TODO 4: Add a physics to an item - update the position of the object from the physics.  

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY());

	return true;
}

bool Item::CleanUp()
{
	return true;
}