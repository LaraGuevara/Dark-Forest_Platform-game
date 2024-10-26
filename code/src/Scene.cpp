#include "Engine.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Log.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Player.h"
#include "Map.h"
#include "Item.h"

Scene::Scene() : Module()
{
	name = "scene";
	img = nullptr;
}

// Destructor
Scene::~Scene()
{}

// Called before render is available
bool Scene::Awake()
{
	LOG("Loading Scene");
	bool ret = true;

	player = (Player*)Engine::GetInstance().entityManager->CreateEntity(EntityType::PLAYER);
	player->SetParameters(configParameters.child("entities").child("player"));
	
	/*Item* item = (Item*) Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
	item->position = Vector2D(100, 0);*/
	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	Engine::GetInstance().map->Load("Assets/Maps/", "CandyMap.tmx");
	helptex = Engine::GetInstance().textures.get()->Load("Assets/Textures/help.png");

	return true;
}

// Called each loop iteration
bool Scene::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool Scene::Update(float dt)
{
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_H) == KEY_DOWN) {
		if (help) help = false;
		else help = true;
	}

	if (help) {
		Engine::GetInstance().render.get()->DrawTexture(helptex, 150, -45);
	}
	
	Engine::GetInstance().render.get()->camera.x = (player->GetXPos() * -1) + 200;

	if (Engine::GetInstance().render.get()->camera.x >= 0)
		Engine::GetInstance().render.get()->camera.x = 0;

	int max = (Engine::GetInstance().map.get()->GetTileColumns() * Engine::GetInstance().map.get()->GetTileSize());
	if (Engine::GetInstance().render.get()->camera.x <= -max)
		Engine::GetInstance().render.get()->camera.x = -max;

	return true;
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;

	if(Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");

	SDL_DestroyTexture(img);

	return true;
}
