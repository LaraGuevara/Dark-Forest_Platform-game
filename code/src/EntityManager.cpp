#include "EntityManager.h"
#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Scene.h"
#include "Log.h"
#include "Item.h"
#include "Attack.h"
#include "tracy/Tracy.hpp"

EntityManager::EntityManager() : Module()
{
	name = "entitymanager";
}

// Destructor
EntityManager::~EntityManager()
{}

// Called before render is available
bool EntityManager::Awake()
{
	LOG("Loading Entity Manager");
	bool ret = true;

	//Iterates over the entities and calls the Awake
	for(const auto entity : entities)
	{
		if (entity->active == false) continue;
		ret = entity->Awake();
	}

	return ret;

}

bool EntityManager::Start() {

	bool ret = true; 

	//Iterates over the entities and calls Start
	for(const auto entity : entities)
	{
		if (entity->active == false) continue;
		ret = entity->Start();
	}

	return ret;
}

// Called before quitting
bool EntityManager::CleanUp()
{
	bool ret = true;

	for(const auto entity : entities)
	{
		if (entity->active == false) continue;
		ret = entity->CleanUp();
	}

	entities.clear();

	return ret;
}

Entity* EntityManager::CreateEntity(EntityType type)
{
	Entity* entity = nullptr; 

	//L04: TODO 3a: Instantiate entity according to the type and add the new entity to the list of Entities
	switch (type)
	{
	case EntityType::PLAYER:
		entity = new Player();
		break;
	case EntityType::ITEM:
		entity = new Item();
		break;
	case EntityType::ENEMY:
		entity = new Enemy();
		break;
	case EntityType::ATTACK:
		entity = new Attack();
		break;
	case EntityType::SENSOR:
		entity = new Sensor();
		break;
	case EntityType::BOSS:
		entity = new Boss();
		break;
	default:
		break;
	}

	entities.push_back(entity);

	return entity;
}

void EntityManager::DestroyEntity(Entity* entity)
{
	for (auto it = entities.begin(); it != entities.end(); ++it)
	{
		if (*it == entity) {
			(*it)->CleanUp();
			delete* it; // Free the allocated memory
			entities.erase(it); // Remove the entity from the list
			break; // Exit the loop after removing the entity
		}
	}
}

void EntityManager::AddEntity(Entity* entity)
{
	if ( entity != nullptr) entities.push_back(entity);
}

bool EntityManager::Update(float dt)
{
	ZoneScoped;
	bool ret = true;
	if (Engine::GetInstance().scene.get()->pausedGame or Engine::GetInstance().scene.get()->checkpointTeleportView or Engine::GetInstance().scene.get()->levelFinishedScreen) {
		for (const auto entity : entities)
		{
			if (entity->active == false) continue;
			ret = entity->RenderUpdate();
		}
	}
	else {
		int cameraX = Engine::GetInstance().render.get()->camera.x;
		for (const auto entity : entities)
		{
			if (entity->position.getX() >= -cameraX / 2 - 50 and entity->position.getX() <= -cameraX / 2 + 650) {
				if (entity->type != EntityType::PLAYER and entity->type != EntityType::ENEMY) {
					if (entity->active == false) continue;
					ret = entity->Update(dt);
				}
			}
		}
		for (const auto entity : entities)
		{
			if (entity->type == EntityType::PLAYER or entity->type == EntityType::ENEMY) {
				if (entity->position.getX() >= -cameraX / 2 - 50 and entity->position.getX() <= -cameraX / 2 + 650) {
					if (entity->active == false) continue;
					ret = entity->Update(dt);
				}
			}
		}
	}
	return ret;
}