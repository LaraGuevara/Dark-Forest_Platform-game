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
	switch (type) {
	case ItemType::ITEM_ABILITY:
		texture = Engine::GetInstance().textures.get()->Load("Assets/Textures/items/abilityItem.png");
		break;
	case ItemType::ITEM_HEALTH:
		texture = Engine::GetInstance().textures.get()->Load("Assets/Textures/items/healthItem.png");
		break;
	case ItemType::ITEM_POINTS:
		texture = Engine::GetInstance().textures.get()->Load("Assets/Textures/items/pointItem.png");
		break;
	}
	
	// L08 TODO 4: Add a physics to an item - initialize the physics body
	Engine::GetInstance().textures.get()->GetSize(texture, texW, texH);
	pbody = Engine::GetInstance().physics.get()->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::DYNAMIC);

	// L08 TODO 7: Assign collider type
	switch (type) {
	case ItemType::ITEM_ABILITY:
		pbody->ctype = ColliderType::ITEM_ABILITY;
		break;
	case ItemType::ITEM_HEALTH:
		pbody->ctype = ColliderType::ITEM_HEALTH;
		break;
	case ItemType::ITEM_POINTS:
		pbody->ctype = ColliderType::ITEM_POINTS;
		break;
	}

	return true;
}

bool Item::Update(float dt)
{

	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY());

	return true;
}

bool Item::RenderUpdate() {
	b2Transform pbodyPos = pbody->body->GetTransform();
	position.setX(METERS_TO_PIXELS(pbodyPos.p.x) - texH / 2);
	position.setY(METERS_TO_PIXELS(pbodyPos.p.y) - texH / 2);

	Engine::GetInstance().render.get()->DrawTexture(texture, (int)position.getX(), (int)position.getY());

	return true;
}

bool Item::CleanUp()
{
	SDL_DestroyTexture(texture);
	return true;
}

void Item::OnCollision(PhysBody* physA, PhysBody* physB) {

	switch (physB->ctype)
	{
	case ColliderType::PLAYER:
		isPicked = true;
		break;
	case ColliderType::DEATH:
		isPicked = true;
		break;
	default:
		break;
	}
}

void Item::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	
}