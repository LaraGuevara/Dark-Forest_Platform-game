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
#include "Enemy.h"

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

	for (pugi::xml_node enemyNode = configParameters.child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
	{
		Enemy* enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ENEMY);
		enemy->SetParameters(enemyNode);
		enemyList.push_back(enemy);
	}
	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
	Engine::GetInstance().map->Load("Assets/Maps/", "newnocandymap.tmx");
	helptex = Engine::GetInstance().textures.get()->Load("Assets/Textures/help.png");

	Mix_VolumeMusic(60);
	saveFX = Mix_LoadWAV("Assets/Audio/Fx/Fantasy_UI (30).wav");
	loadFX = Mix_LoadWAV("Assets/Audio/Fx/Success 1 (subtle).wav");
	attackFX = Mix_LoadWAV("Assets/Audio/Fx/Ice Throw 1.wav");
	backgroundMusic = Mix_LoadMUS("Assets/Audio/Fx/Ambient Music.wav");
	Mix_PlayMusic(backgroundMusic, -1);
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

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F5) == KEY_DOWN) {
		SaveState();
		Mix_PlayChannel(2, saveFX, 0);
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F6) == KEY_DOWN) {
		LoadState();
		Mix_PlayChannel(2, loadFX, 0);
	}

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_Q) == KEY_DOWN) {
		Mix_PlayChannel(2, attackFX, 0);
		player->isAttacking = true;
		Attack* attack = (Attack*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ATTACK);
		Vector2D playerPos = GetPlayerPosition();
		if (player->look == Player_Look::RIGHT) attack->position = Vector2D(playerPos.getX() + 30, playerPos.getY() - 10);
		else attack->position = Vector2D(playerPos.getX() - 30, playerPos.getY() - 10);
		attack->SetFlip(player->flip);
		attack->Start();
		attackList.push_back(attack);
	}

	for (int i = 0; i < attackList.size(); i++) {
		if (attackList[i]->collision) {
			Engine::GetInstance().physics.get()->DeleteBody(attackList[i]->pbody->body);
			Engine::GetInstance().entityManager->DestroyEntity(attackList[i]);
			attackList.erase(attackList.begin() + i);
			i--;
		}
	}

	for (int i = 0; i < enemyList.size(); i++) {
		if (enemyList[i]->state == EnemyState::DEAD) {
			Engine::GetInstance().physics.get()->DeleteBody(enemyList[i]->pbody->body);
			Engine::GetInstance().entityManager->DestroyEntity(enemyList[i]);
			attackList.erase(attackList.begin() + i);
			i--;
		}
	}

	if (help) Engine::GetInstance().render.get()->DrawTexture(helptex, 150, -45);
	
	Engine::GetInstance().render.get()->camera.x = ((player->GetXPos() * -1) + 200) *2;

	if (Engine::GetInstance().render.get()->camera.x >= 0)
		Engine::GetInstance().render.get()->camera.x = 0;

	int max = (315 * Engine::GetInstance().map.get()->GetTileSize());
	if (Engine::GetInstance().render.get()->camera.x <= -max)
		Engine::GetInstance().render.get()->camera.x = -max;

	if (player->state == Player_State::DIE) {
		Mix_PauseMusic();
		respawn = true;
	}

	if (respawn and player->state != Player_State::DIE) {
		respawn = false;
		Mix_ResumeMusic();
	}

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
	SDL_DestroyTexture(helptex);

	return true;
}

Vector2D Scene::GetPlayerPosition()
{
	return player->GetPosition();
}

void Scene::LoadState() {
	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");

	if (result == NULL) LOG("Error loading config.xml: %s", result.description());

	Vector2D playerPos;
	playerPos.setX(loadFile.child("config").child("save").child("player").attribute("x").as_int());
	playerPos.setY(loadFile.child("config").child("save").child("player").attribute("y").as_int());
	player->SetPosition(playerPos);
}

void Scene::SaveState() {
	pugi::xml_document saveFile;
	pugi::xml_parse_result result = saveFile.load_file("config.xml");

	if (result == NULL) LOG("Error loading config.xml: %s", result.description());
	
	Vector2D playerPos = player->GetPosition();
	saveFile.child("config").child("save").child("player").attribute("x").set_value(playerPos.getX());
	saveFile.child("config").child("save").child("player").attribute("y").set_value(playerPos.getY());

	saveFile.save_file("config.xml");
}


