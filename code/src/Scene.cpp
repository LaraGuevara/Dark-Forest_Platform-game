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
#include "GuiControl.h"
#include "GuiManager.h"
#include "tracy/Tracy.hpp"


Scene::Scene() : Module()
{
	name = "scene";
	img = nullptr;
}

// Destructor
Scene::~Scene()
{}

bool Scene::CanContinueGame() {
	pugi::xml_document saveFile;
	pugi::xml_parse_result result = saveFile.load_file("config.xml");

	if (result == NULL) LOG("Error loading config.xml: %s", result.description());

	bool canSave = saveFile.child("config").child("save").attribute("load").as_bool();
	if (canSave) return true;
	else return false;
}

float Scene::GetBestTime() {
	pugi::xml_document saveFile;
	pugi::xml_parse_result result = saveFile.load_file("config.xml");

	if (result == NULL) LOG("Error loading config.xml: %s", result.description());

	pugi::xml_node levelSaveNode;
	for (pugi::xml_node saveNode = saveFile.child("config").child("save").child("levels").child("level"); saveNode; saveNode = saveNode.next_sibling("level")) {
		if (saveNode.attribute("level").as_int() == level) levelSaveNode = saveNode;
	}

	return levelSaveNode.attribute("time").as_float();
}

// Called before render is available
bool Scene::Awake()
{
	LOG("Loading Scene");
	bool ret = true;

	switch (state) {
	case SceneState::INTRO:
		break;
	case SceneState::MENU:
		startBT = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, GUI_ID::ID_PLAY, "Play", { 40, 230, 200,70 }, this);
		continueBT = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, GUI_ID::ID_CONTINUE, "Continue", { 40, 310, 200,70 }, this);
		settingBT = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, GUI_ID::ID_SETTINGS, "Settings", { 40, 390, 200,70 }, this);
		creditsBT = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, GUI_ID::ID_CREDITS, "Credits", { 40, 470, 200,70 }, this);
		exitBT = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, GUI_ID::ID_EXIT, "Exit", { 40, 550, 200,70 }, this);

		musicSlider= (GuiControlSlidebox*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::SLIDER, GUI_ID::ID_SLIDEM, "Music", { 200, 50, 200,70 }, this);
		musicSlider->state = GuiControlState::DISABLED;

		/*musicSliderbox = (GuiControlSlidebox*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::SLIDERBAR, GUI_ID::ID_SLIDEM, "Music", { 200, 50, 200,70 }, this);
		musicSliderbox->state = GuiControlState::DISABLED;*/
		
		

		fxSlider= (GuiControlSlidebox*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::SLIDER, GUI_ID::ID_SLIDEF, "Fx", { 200, 150, 200,70 }, this);
		fxSlider->state = GuiControlState::DISABLED;

		fullscreenCheckBox= (GuiControlCheckbox*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::CHECKBOX, GUI_ID::ID_CHECKBOX, "Fullscreen", { 540, 525, 200,70 }, this);
		fullscreenCheckBox->state = GuiControlState::DISABLED;


		break;
	case SceneState::GAME:
		if (!gameAwake) {
			resumeBT = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, GUI_ID::ID_RESUME, "Resume", { 520, 100, 200,70 }, this);
			PAUSEDsettingsBT = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, GUI_ID::ID_PAUSED_SETTINGS, "Settings", { 520, 180, 200,70 }, this);
			titleBT = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, GUI_ID::ID_TITLE, "Back to Title", { 520, 260, 200,70 }, this);
			PAUSEDexitBT = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, GUI_ID::ID_PAUSED_EXIT, "Exit", { 520, 340, 200,70 }, this);

			resumeBT->state = GuiControlState::DISABLED;
			PAUSEDsettingsBT->state = GuiControlState::DISABLED;
			titleBT->state = GuiControlState::DISABLED;
			PAUSEDexitBT->state = GuiControlState::DISABLED;

			respawnBT = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, GUI_ID::ID_RESPAWN, "Respawn", { 540, 525, 200,70 }, this);
			respawnBT->state = GuiControlState::DISABLED;


			nextBT = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, GUI_ID::ID_NEXT, "Next", { 555, 445, 200,70 }, this);
			nextBT->state = GuiControlState::DISABLED;
		}

		if (continueGame) ContinueGameAwake();
		else NewGameAwake();

		break;
	case SceneState::SETTINGS:
		musicSlider->state = GuiControlState::NORMAL;
		musicSlider->checkID = 1;
		fxSlider->state = GuiControlState::NORMAL;
		fxSlider->checkID = 2;
		fullscreenCheckBox->state = GuiControlState::NORMAL;


		break;
	case SceneState::CREDITS:
		break;
	default:
		return false;
		break;
	}
	return ret;
}

void Scene::NewGameAwake() {
	Engine::GetInstance().physics.get()->CleanUp();
	Engine::GetInstance().physics.get()->Start();

	player = (Player*)Engine::GetInstance().entityManager->CreateEntity(EntityType::PLAYER);
	player->SetParameters(configParameters.child("entities").child("player"));
	if (playerPoints != 0) player->SetPoints(playerPoints);

	if (!ActiveBossFight) {
		for (pugi::xml_node enemyNode = configParameters.child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
		{
			if (enemyNode.attribute("level").as_int() == level) {
				Enemy* enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ENEMY);
				enemy->SetParameters(enemyNode);
				enemyList.push_back(enemy);
			}
		}

		for (pugi::xml_node itemNode = configParameters.child("entities").child("items").child("item"); itemNode; itemNode = itemNode.next_sibling("item")) {
			if (itemNode.attribute("level").as_int() == level) {
				Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
				item->SetParameters(itemNode);
				switch (itemNode.attribute("type").as_int()) {
				case (int)ItemType::ITEM_ABILITY:
					item->SetType(ItemType::ITEM_ABILITY);
					break;
				case (int)ItemType::ITEM_HEALTH:
					item->SetType(ItemType::ITEM_HEALTH);
					break;
				case (int)ItemType::ITEM_POINTS:
					item->SetType(ItemType::ITEM_POINTS);
					break;
				}
				itemList.push_back(item);
			}
		}
	}
	else {
		boss = (Boss*)Engine::GetInstance().entityManager->CreateEntity(EntityType::BOSS);
		boss->SetParameters(configParameters.child("entities").child("boss"));
	}
}

void Scene::ContinueGameAwake() {
	Engine::GetInstance().physics.get()->CleanUp();
	Engine::GetInstance().physics.get()->Start();

	pugi::xml_document loadFile;
	pugi::xml_parse_result result = loadFile.load_file("config.xml");

	player = (Player*)Engine::GetInstance().entityManager->CreateEntity(EntityType::PLAYER);
	player->SetParameters(configParameters.child("entities").child("player"));

	//load player position and points
	Vector2D playerPos;
	playerPos.setX(loadFile.child("config").child("save").child("player").attribute("x").as_int());
	playerPos.setY(loadFile.child("config").child("save").child("player").attribute("y").as_int());
	player->SaveStartingPos(playerPos);
	player->SetPoints(loadFile.child("config").child("save").child("player").attribute("points").as_int());

	//load current level
	level = loadFile.child("config").child("save").child("player").attribute("level").as_int();

	//find the proper level to load
	pugi::xml_node levelSaveNode;
	for (pugi::xml_node saveNode = loadFile.child("config").child("save").child("levels").child("level"); saveNode; saveNode = saveNode.next_sibling("level")) {
		if (saveNode.attribute("level").as_int() == level) levelSaveNode = saveNode;
	}

	//load alive enemies
	Vector2D enemyPos;
	for (pugi::xml_node enemyNode = configParameters.child("entities").child("enemies").child("enemy"); enemyNode; enemyNode = enemyNode.next_sibling("enemy"))
	{
		bool alive = false;
		std::string enemyName = enemyNode.attribute("name").as_string();
		
		for (pugi::xml_node saveNode = levelSaveNode.child("enemies").child("enemy"); saveNode; saveNode = saveNode.next_sibling("enemy")) {
			std::string saveName = saveNode.attribute("name").as_string();
			if (saveName == enemyName) {
				enemyPos.setX(saveNode.attribute("x").as_int());
				enemyPos.setY(saveNode.attribute("y").as_int());
				alive = true;
			}
		}
		
		if (alive) {
			Enemy* enemy = (Enemy*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ENEMY);
			enemy->SetParameters(enemyNode);
			enemy->SaveStartingPos(enemyPos);
			enemy->SetWake();
			enemyList.push_back(enemy);
		}
	}

	for (pugi::xml_node itemNode = configParameters.child("entities").child("items").child("item"); itemNode; itemNode = itemNode.next_sibling("item")) {
		bool collected = true;
		int itemID = itemNode.attribute("id").as_int();
		for (pugi::xml_node saveNode = levelSaveNode.child("items").child("item"); saveNode; saveNode = saveNode.next_sibling("item")) {
			int saveID = saveNode.attribute("id").as_int();
			if (saveID == itemID) collected = false;
		}
		
		if (!collected) {
			Item* item = (Item*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ITEM);
			item->SetParameters(itemNode);
			switch (itemNode.attribute("type").as_int()) {
			case (int)ItemType::ITEM_ABILITY:
				item->SetType(ItemType::ITEM_ABILITY);
				break;
			case (int)ItemType::ITEM_HEALTH:
				item->SetType(ItemType::ITEM_HEALTH);
				break;
			case (int)ItemType::ITEM_POINTS:
				item->SetType(ItemType::ITEM_POINTS);
				break;
			}
			itemList.push_back(item);
		}
	}
}

void Scene::LoadLevel(int lvl) {
	if (lvl > LEVELS) {
		Mix_PauseMusic();
		pausedGame = false;
		CleanUp();
		Engine::GetInstance().entityManager->CleanUp();
		state = SceneState::MENU;
		Start();
	}
	else if (ActiveBossFight) {
		Engine::GetInstance().map.get()->CleanUp();
		Engine::GetInstance().map->Load("Assets/Maps/", "bossmap.tmx");
	}
	else {
		Engine::GetInstance().map.get()->CleanUp();
		switch (lvl) {
		case 1:
			Engine::GetInstance().map->Load("Assets/Maps/", "newnocandymap.tmx");
			break;
		
		case 2:
			Engine::GetInstance().map->Load("Assets/Maps/", "newnocandymap2.tmx");
			break;
		}
	}
}

// Called before the first frame
bool Scene::Start()
{
	switch (state) {
	case SceneState::INTRO:
		//Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
		introImg = Engine::GetInstance().textures.get()->Load("Assets/Textures/screens/intro.png");
		break;
	case SceneState::MENU:
		menuBackground = Engine::GetInstance().textures.get()->Load("Assets/Textures/screens/mainmenu.png");
		sliderBackground = Engine::GetInstance().textures.get()->Load("Assets/Textures/sliderBackground.png");
		slider = Engine::GetInstance().textures.get()->Load("Assets/Textures/slider.png");
		startBT->state = GuiControlState::NORMAL;
		if(CanContinueGame()) continueBT->state = GuiControlState::NORMAL;
		else continueBT->state = GuiControlState::UNCLICKABLE;
		settingBT->state = GuiControlState::NORMAL;
		creditsBT->state = GuiControlState::NORMAL;
		exitBT->state = GuiControlState::NORMAL;
		Engine::GetInstance().audio->PlayMusic("Assets/Audio/Music/mainMenu.ogg", 0);
		break;
	case SceneState::GAME:
		//Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
		//Engine::GetInstance().map->Load("Assets/Maps/", "newnocandymap.tmx");

		LoadLevel(level);
		if (state != SceneState::MENU) {
			healthbar = Engine::GetInstance().textures.get()->Load("Assets/Textures/healthbar.png");
			gemIcon = Engine::GetInstance().textures.get()->Load("Assets/Textures/gemIcon.png");
			powerUpIcon = Engine::GetInstance().textures.get()->Load("Assets/Textures/items/abilityItem.png");

			if (!ActiveBossFight) {
				//create checkpoints
				checkpointList = Engine::GetInstance().map->LoadCheckpoints(level);
				if (newGame or continueGame) {
					checkpointTPList.clear();
					newGame = false;
				}

				if (continueGame) {
					pugi::xml_document loadFile;
					pugi::xml_parse_result result = loadFile.load_file("config.xml");

					//find the proper level to load
					pugi::xml_node levelSaveNode;
					for (pugi::xml_node saveNode = loadFile.child("config").child("save").child("levels").child("level"); saveNode; saveNode = saveNode.next_sibling("level")) {
						for (pugi::xml_node cpNode = saveNode.child("checkpoints").child("checkpoint"); cpNode; cpNode = cpNode.next_sibling("checkpoint")) {
							Teleport tp;
							tp.id = cpNode.attribute("id").as_int();
							tp.level = saveNode.attribute("level").as_int();
							tp.UI_ID = checkpointTPList.size() + 1 + GUI_ID::ID_TELEPORT;
							tp.playerPos = { cpNode.attribute("x").as_float(), cpNode.attribute("y").as_float() };
							checkpointTPList.push_back(tp);
						}

						if (saveNode.attribute("level").as_int() == level) levelSaveNode = saveNode;
					}

					for (int i = 0; i < checkpointList.size(); i++) {
						int checkpointID = checkpointList[i]->id;

						for (pugi::xml_node cpNode = levelSaveNode.child("checkpoints").child("checkpoint"); cpNode; cpNode = cpNode.next_sibling("checkpoint")) {
							int saveID = cpNode.attribute("id").as_int();
							if (saveID == checkpointID) {
								checkpointList[i]->isActive = true;
								checkpointList[i]->isAdded = true;
								checkpointList[i]->playerPos = { cpNode.attribute("x").as_float(), cpNode.attribute("y").as_float() };
							}
						}
					}
				}
			}

			continueGame = false;

			//Mix_VolumeMusic(60);
			saveFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Fantasy_UI (30).wav");
			loadFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Success 1 (subtle).wav");
			attackFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/Fireball 2.wav");

			if (!ActiveBossFight) {
				switch (level) {
				case 1:
					Engine::GetInstance().audio->PlayMusic("Assets/Audio/Music/level1.ogg", 0);
					break;
				case 2:
					Engine::GetInstance().audio->PlayMusic("Assets/Audio/Music/level2.ogg", 0);
					break;
				}
			}
			else Engine::GetInstance().audio->PlayMusic("Assets/Audio/Music/boss.ogg", 0);

			if (!ActiveBossFight) bestTime = GetBestTime();

			pausedTime = 0.f;
			timeCount = 0.f;
			FadeInActive = true;
		}

		break;
	case SceneState::SETTINGS:
		
		//musicSlider->color();
		
		
		break;
	case SceneState::CREDITS:
		break;
	default:
		return false;
		break;
	}
	return true;
	
	return true;
}

void Scene::TeleportUI() {

	for (auto b : teleportBTs) Engine::GetInstance().guiManager->DeleteGUIControl(b->id);
	teleportBTs.clear();

	Vector2D playerPos = GetPlayerPosition();
	int y = 100;
	for (int i = 0; i < checkpointTPList.size(); ++i) {
		std::string text1 = std::to_string(checkpointTPList[i].id);
		std::string text2 = " / lvl: ";
		std::string text3 = std::to_string(checkpointTPList[i].level);
		std::string fullText = text1 + text2 + text3;

		GuiControlButton* button = (GuiControlButton*)Engine::GetInstance().guiManager->CreateGuiControl(GuiControlType::BUTTON, checkpointTPList[i].UI_ID, fullText.c_str(), { 50, y, 150,30}, this);
		teleportBTs.push_back(button);
		y += 40;
	}
}

void Scene::EndTeleportUI() {
	checkpointTeleportView = false;
	for (auto b : teleportBTs) {
		b->state = GuiControlState::DISABLED;
	}
}

// Called each loop iteration
bool Scene::PreUpdate()
{
	ZoneScoped;
	return true;
}

bool Scene::Update(float dt) {
	ZoneScoped;

	switch (state) {
	case SceneState::INTRO:
		introTime++;
		if (introTime >= 180 / timeVar) {
			CleanUp();
			state = SceneState::MENU;
			Awake();
			Start();
		} else Engine::GetInstance().render.get()->DrawTexture(introImg, 0, 0, NULL, SDL_FLIP_NONE, false);
		break;
	case SceneState::MENU:
		if (!inMenu) inMenu = true;
		Engine::GetInstance().render.get()->DrawTexture(menuBackground, 0, 0, NULL, SDL_FLIP_NONE, false);
		break;
	case SceneState::GAME:
		if (inMenu) inMenu = false;
		GameUpdate(dt);
		break;
	case SceneState::SETTINGS:
		if(inMenu) Engine::GetInstance().render.get()->DrawTexture(menuBackground, 0, 0, NULL, SDL_FLIP_NONE, false);
		break;
	case SceneState::CREDITS:
		Engine::GetInstance().render.get()->DrawTexture(menuBackground, 0, 0, NULL, SDL_FLIP_NONE, false);
		break;
	default:
		return false;
		break;
	}
	return true;
}

void Scene::FadeIn() {
	Engine::GetInstance().render.get()->DrawRectangle(SDL_Rect{ 0, 0, Engine::GetInstance().window.get()->width, Engine::GetInstance().window.get()->height }, 0, 0, 0, fadeValue);
	fadeValue -= 5 * player->timerVar;

	if (fadeValue <= 0) {
		FadeInActive = false;
		fadeValue = 255;
		timer = Timer();
		startTime = timer.ReadMSec();
	}
}

// Called each loop iteration
bool Scene::GameUpdate(float dt)
{
	if (FadeInActive) FadeIn();

	if (pausedGame) {
		resumeBT->state = GuiControlState::NORMAL;
		PAUSEDsettingsBT->state = GuiControlState::NORMAL;
		titleBT->state = GuiControlState::NORMAL;
		PAUSEDexitBT->state = GuiControlState::NORMAL;
		disabledButtons = false;
	}
	else {
		if (!disabledButtons) {
			resumeBT->state = GuiControlState::DISABLED;
			PAUSEDsettingsBT->state = GuiControlState::DISABLED;
			titleBT->state = GuiControlState::DISABLED;
			PAUSEDexitBT->state = GuiControlState::DISABLED;
			disabledButtons = true;
		}
	}

	//trigger/hide checkpoint teleport
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F7) == KEY_DOWN) {
		if (checkpointTeleportView) {
			EndTeleportUI();
			checkpointTeleportView = false;
		}
		else if (!checkpointTeleportView) {
			TeleportUI();
			checkpointTeleportView = true;
		}
	}
	
	//draw healthbar
	Engine::GetInstance().render.get()->DrawTexture(healthbar, 10, 20, &healthRect, SDL_FLIP_NONE, false);
	SDL_Rect pointsRect = { 0,32,32 + (GetPlayerLife()*4), 3};
	//trigger flash if player takes damage
	if (player->isDamaged and !tookDamage) {
		tookDamage = true;
		flashCount = 0;
		flashDurationCount = 0;
	} else if(!tookDamage) previousPlayerLife = GetPlayerLife();

	//flash effect for taking damage
	if (tookDamage) {
		++flashDurationCount;
		if (flashDurationCount >= 12 / player->timerVar) {
			flashDurationCount = 0;
			if (iconFlash) {
				iconFlash = false;
				++flashCount;
			}
			else iconFlash = true;
		}

		if(iconFlash) pointsRect = { 0,38,32 + (previousPlayerLife * 4), 3 };

		if (flashCount >= 4) tookDamage = false;
	}

	Engine::GetInstance().render.get()->DrawTexture(healthbar, 10, 42, &pointsRect, SDL_FLIP_NONE, false);

	pointsRect = { 0,35,32 + (GetPlayerPower() * 4), 3 };
	Engine::GetInstance().render.get()->DrawTexture(healthbar, 10, 56, &pointsRect, SDL_FLIP_NONE, false);

	//draw gem/point counter
	Engine::GetInstance().render.get()->DrawTexture(gemIcon, 155, 35, &gemRect, SDL_FLIP_NONE, false);
	std::string points = std::to_string(player->GemPoints);
	Engine::GetInstance().render.get()->DrawText(points.c_str(), 190, 35, 16, 32);

	if (!ActiveBossFight) {
		//write time
		if (!pausedGame and !levelFinishedScreen and player->state != Player_State::DIE and !FadeInActive) {
			timeCount = (float)((timer.ReadMSec() - (startTime + pausedTime)) / 1000);
			timeCount = std::round(timeCount * 100.0f) / 100.0f;
		}
		std::snprintf(buffer, sizeof(buffer), "%.2f", timeCount);
		std::string time = buffer;
		Engine::GetInstance().render.get()->DrawText(time.c_str(), 1200, 20, 50, 32);

		//write best time
		std::snprintf(buffer, sizeof(buffer), "%.2f", bestTime);
		std::string BTime = buffer;
		std::string highScore = "Best Time: " + BTime;
		Engine::GetInstance().render.get()->DrawText(highScore.c_str(), 1130, 55, 120, 40);
	}

	//draw power-up icon if required
	if (player->PowerUpActive) {
		//disappear and appear for when power up is about to run out
		if (player->almostEndPower) {
			if (iconFade) Engine::GetInstance().render.get()->DrawTexture(powerUpIcon, 155, 75, &gemRect, SDL_FLIP_NONE, false);

			++iconFadeCount;
			if (iconFadeCount >= (30 / player->timerVar)) {
				if (iconFade) iconFade = false;
				else iconFade = true;

				iconFadeCount = 0;
			}
		}
		else Engine::GetInstance().render.get()->DrawTexture(powerUpIcon, 155, 75, &gemRect, SDL_FLIP_NONE, false);
	}
	else {
		if(iconFadeCount != 0) iconFadeCount = 0;
	}

	//help menu
	if (!pausedGame and Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_H) == KEY_DOWN) {
		if (help) help = false;
		else help = true;
	}

	//f1/f2 triggers
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F1) == KEY_DOWN) SetAtLevelStart(1);
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F2) == KEY_DOWN) SetAtLevelStart(2);

	//load and save
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F5) == KEY_DOWN) {
		canLoad = true;
		SaveState();
		Engine::GetInstance().audio->PlayFx(saveFX);
	}
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F6) == KEY_DOWN) {
		LoadState();
		Engine::GetInstance().audio->PlayFx(loadFX);
	}

	//checkpoint
	for (auto c : checkpointList) {
		if (!c->deactivate and c->isActive == true and player->state != Player_State::DIE) {
			Vector2D playerPos = player->GetPosition();
			Vector2D playerPosTile = Engine::GetInstance().map.get()->WorldToMap((int)playerPos.getX(), (int)playerPos.getY());
			Engine::GetInstance().audio->PlayFx(saveFX);
			player->SetCheckpoint(playerPos);
			canLoad = true;
			SaveState();
			c->deactivate = true;

			//add to teleport list
			if (!c->isAdded) {
				Teleport tp;
				tp.id = c->id;
				tp.level = level;
				tp.UI_ID = checkpointTPList.size() + 1 + GUI_ID::ID_TELEPORT;
				tp.playerPos = c->playerPos;
				checkpointTPList.push_back(tp);
				c->isAdded = true;
			}

			LOG("CHECKPOINT (%d, %d)", (int)playerPos.getX(), (int)playerPos.getY() - 2);
		}
	}

	//attack creation
	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_Q) == KEY_DOWN and player->state != Player_State::DIE) {
		if (GetPlayerPower() >= 2) {
			Engine::GetInstance().audio->PlayFx(attackFX);
			player->isAttacking = true;
			Attack* attack = (Attack*)Engine::GetInstance().entityManager->CreateEntity(EntityType::ATTACK);

			Vector2D playerPos = GetPlayerPosition();
			if (player->look == Player_Look::RIGHT) attack->position = Vector2D(playerPos.getX() + 30, playerPos.getY() - 10);
			else attack->position = Vector2D(playerPos.getX() - 30, playerPos.getY() - 10);

			attack->SetFlip(player->flip);
			if (player->PowerUpActive) attack->SetToPowerUp();
			attack->Start();

			attackList.push_back(attack);
			player->power = player->power - 2;
		}
	}

	//check and delete finished attack effects
	for (int i = 0; i < attackList.size(); i++) {
		if (attackList[i]->collision) {
			Engine::GetInstance().physics.get()->DeleteBody(attackList[i]->pbody->body);
			Engine::GetInstance().entityManager->DestroyEntity(attackList[i]);
			attackList.erase(attackList.begin() + i);
			i--;
		}
	}

	//check and delete dead enemies
	for (int i = 0; i < enemyList.size(); i++) {
		if (enemyList[i]->state == EnemyState::DEAD) {
			Engine::GetInstance().physics.get()->DeleteBody(enemyList[i]->pbody->body);
			Engine::GetInstance().physics.get()->DeleteBody(enemyList[i]->sensor->body);
			Engine::GetInstance().entityManager->DestroyEntity(enemyList[i]);
			enemyList.erase(enemyList.begin() + i);
			i--;
		}
	}

	//check and delete collected items
	for (int i = 0; i < itemList.size(); i++) {
		if (itemList[i]->isPicked) {
			Engine::GetInstance().physics.get()->DeleteBody(itemList[i]->pbody->body);
			Engine::GetInstance().entityManager->DestroyEntity(itemList[i]);
			itemList.erase(itemList.begin() + i);
			i--;
		}
	}
	
	Engine::GetInstance().render.get()->camera.x = ((player->GetXPos() * -1) + 200) *2;

	if (Engine::GetInstance().render.get()->camera.x >= 0)
		Engine::GetInstance().render.get()->camera.x = 0;

	int max = (315 * Engine::GetInstance().map.get()->GetTileSize());
	if (Engine::GetInstance().render.get()->camera.x <= -max)
		Engine::GetInstance().render.get()->camera.x = -max;

	if (player->state == Player_State::DIE) {
		Mix_PauseMusic();
		if (!checkTime) {
			startPauseTime = timer.ReadMSec();
			checkTime = true;
		}
		respawn = true;
	}

	if (respawn and player->finishedDeathAnim) {
		deathScreen = true;
		respawnBT->state = GuiControlState::NORMAL;
		player->finishedDeathAnim = false;
	}

	if (respawn and player->state != Player_State::DIE) {
		deathScreen = false;
		respawnBT->state = GuiControlState::DISABLED;
		respawn = false;
		pausedTime += (timer.ReadMSec() - startPauseTime);
		checkTime = false;
		Mix_ResumeMusic();
	}

	if (player->finishedLevel or Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_I) == KEY_DOWN) {
		if (!levelFinishedScreen and level != 2) {
			continueGame = false;
			finalTime = (float)((timer.ReadMSec() - (startTime + pausedTime)) / 1000);
			if (finalTime < bestTime or bestTime == 0) bestTime = finalTime;
			playerPoints = player->GemPoints;
			canLoad = false;
			SaveState();
			level += 1;
			levelFinishedScreen = true;
			nextBT->state = GuiControlState::NORMAL;
		}
		else if (level == 2) {
			//nextBT->state = GuiControlState::DISABLED;
			finalTime = (float)((timer.ReadMSec() - (startTime + pausedTime)) / 1000);
			playerPoints = player->GemPoints;
			canLoad = false;
			SaveState();
			ActiveBossFight = true;
			player->finishedLevel = false;
			CleanUp();
			Engine::GetInstance().entityManager->CleanUp();
			Awake();
			Engine::GetInstance().entityManager->Awake();
			Start();
			Engine::GetInstance().entityManager->Start();
		}
	}

	//check if boss has been killed
	if (ActiveBossFight) {
		if (boss->state == BossState::BOSS_DEFEATED and !defeatedBoss) defeatedBoss = true;
	}

	if (defeatedBoss) {
		ActiveBossFight = false;
		defeatedBoss = false;
		canLoad = false;
		Mix_PauseMusic();
		CleanUp();
		Engine::GetInstance().entityManager->CleanUp();
		state = SceneState::MENU;
		Start();
	}

	return true;
}

void Scene::SetAtLevelStart(int lvl) {
	if (level != lvl) {
		SaveState();
		level = lvl;
		playerPoints = player->GemPoints;
		Mix_PauseMusic();
		CleanUp();
		Engine::GetInstance().entityManager->CleanUp();
		Awake();
		Engine::GetInstance().entityManager->Awake();
		Start();
		Engine::GetInstance().entityManager->Start();
	}
	else {
		player->SetPositionToStart();
	}
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	ZoneScoped;

	bool ret = true;

	if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN) {
		switch (state) {
		case SceneState::MENU:
			ret = false;
			break;
		case SceneState::CREDITS:
			startBT->state = GuiControlState::NORMAL;
			if (canLoad) continueBT->state = GuiControlState::NORMAL;
			else continueBT->state = GuiControlState::UNCLICKABLE;
			settingBT->state = GuiControlState::NORMAL;
			creditsBT->state = GuiControlState::NORMAL;
			exitBT->state = GuiControlState::NORMAL;
			state = SceneState::MENU;
			break;
		case SceneState::SETTINGS:
			if (!pausedGame) {
			startBT->state = GuiControlState::NORMAL;
			if(canLoad) continueBT->state = GuiControlState::NORMAL;
			else continueBT->state = GuiControlState::UNCLICKABLE;
			settingBT->state = GuiControlState::NORMAL;
			creditsBT->state = GuiControlState::NORMAL;
			exitBT->state = GuiControlState::NORMAL;
			state = SceneState::MENU;
			}

			musicSlider->state = GuiControlState::DISABLED;
			//musicSliderbox->state = GuiControlState::DISABLED;
			fxSlider->state = GuiControlState::DISABLED;
			fullscreenCheckBox->state = GuiControlState::DISABLED;

			if (!inMenu) {
				resumeBT->state = GuiControlState::NORMAL;
				PAUSEDsettingsBT->state = GuiControlState::NORMAL;
				titleBT->state = GuiControlState::NORMAL;
				PAUSEDexitBT->state = GuiControlState::NORMAL;
			}
			
			if (!inMenu) state = SceneState::GAME;
			else state = SceneState::MENU;
			break;
		case SceneState::GAME:
			if (pausedGame) {
				pausedGame = false;
				pausedTime += (timer.ReadMSec() - startPauseTime);
			}
			else {
				pausedGame = true;
				startPauseTime = timer.ReadMSec();
			}
			break;
		}
	}

	if (toExit) ret = false;

	return ret;
}

// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");

	switch (state) {
	case SceneState::INTRO:
		SDL_DestroyTexture(introImg);
		break;
	case SceneState::MENU:
		Mix_PauseMusic();
		SDL_DestroyTexture(menuBackground);
		startBT->state = GuiControlState::DISABLED;
		continueBT->state = GuiControlState::DISABLED;
		settingBT->state = GuiControlState::DISABLED;
		creditsBT->state = GuiControlState::DISABLED;
		exitBT->state = GuiControlState::DISABLED;
		break;
	case SceneState::GAME:
		if(img) SDL_DestroyTexture(img);

		enemyList.clear();
		itemList.clear();
		checkpointList.clear();
		//checkpointTPList.clear();
		break;
	case SceneState::SETTINGS:
		startBT->state = GuiControlState::DISABLED;
		continueBT->state = GuiControlState::DISABLED;
		settingBT->state = GuiControlState::DISABLED;
		creditsBT->state = GuiControlState::DISABLED;
		exitBT->state = GuiControlState::DISABLED;
		break;
	case SceneState::CREDITS:
		break;
	default:
		return false;
		break;
	}
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

	//load player position and points
	Vector2D playerPos;
	playerPos.setX(loadFile.child("config").child("save").child("player").attribute("x").as_int());
	playerPos.setY(loadFile.child("config").child("save").child("player").attribute("y").as_int());
	player->SetPosition(playerPos);
	player->SetPoints(loadFile.child("config").child("save").child("player").attribute("points").as_int());

	//load current level
	level = loadFile.child("config").child("save").child("player").attribute("level").as_int();

	//find the proper level to load
	pugi::xml_node levelSaveNode;
	checkpointTPList.clear();
	for (pugi::xml_node saveNode = loadFile.child("config").child("save").child("levels").child("level"); saveNode; saveNode = saveNode.next_sibling("level")) {
		for (pugi::xml_node cpNode = saveNode.child("checkpoints").child("checkpoint"); cpNode; cpNode = cpNode.next_sibling("checkpoint")) {
			Teleport tp;
			tp.id = cpNode.attribute("id").as_int();
			tp.level = saveNode.attribute("level").as_int();
			tp.UI_ID = checkpointTPList.size() + 1 + GUI_ID::ID_TELEPORT;
			tp.playerPos = { cpNode.attribute("x").as_float(), cpNode.attribute("y").as_float() };
			checkpointTPList.push_back(tp);
		}

		if (saveNode.attribute("level").as_int() == level) levelSaveNode = saveNode;
	}

	//load alive enemies (deleting dead ones)
	for (int i = 0; i < enemyList.size(); i++) {
		bool alive = false;
		std::string enemyName = enemyList[i]->name;
		for (pugi::xml_node saveNode = levelSaveNode.child("enemies").child("enemy"); saveNode; saveNode = saveNode.next_sibling("enemy")) {
			std::string saveName = saveNode.attribute("name").as_string();
			if (saveName == enemyName) {
				enemyList[i]->SetPosition(Vector2D(saveNode.attribute("x").as_int(), saveNode.attribute("y").as_int()));
				enemyList[i]->SetWake();
				alive = true;
			}
		}
		if (!alive) enemyList[i]->state = EnemyState::DEAD;
	}

	//delete items that have been collected in save
	for (int i = 0; i < itemList.size(); i++) {
		bool collected = true;
		int itemID = itemList[i]->id;
		for (pugi::xml_node saveNode = levelSaveNode.child("items").child("item"); saveNode; saveNode = saveNode.next_sibling("item")) {
			int saveID = saveNode.attribute("id").as_int();
			if (saveID == itemID) collected = false;
		}

		if (collected) itemList[i]->isPicked = true;
	}

	//set activated checkpoints to active
	for (int i = 0; i < checkpointList.size(); i++) {
		int checkpointID = checkpointList[i]->id;

		for (pugi::xml_node saveNode = levelSaveNode.child("checkpoints").child("checkpoint"); saveNode; saveNode = saveNode.next_sibling("checkpoint")) {
			int saveID = saveNode.attribute("id").as_int();
			if (saveID == checkpointID) {
				checkpointList[i]->isActive = true;
				checkpointList[i]->isAdded = true;
				checkpointList[i]->playerPos = { saveNode.attribute("x").as_float(), saveNode.attribute("y").as_float() };
			}
		}
	}
}

void Scene::SaveState() {
	pugi::xml_document saveFile;
	pugi::xml_parse_result result = saveFile.load_file("config.xml");

	if (result == NULL) LOG("Error loading config.xml: %s", result.description());

	//save whether game can be continued or not
	saveFile.child("config").child("save").attribute("load").set_value(canLoad);
	
	//save player postion and points
	Vector2D playerPos = player->GetPosition();
	saveFile.child("config").child("save").child("player").attribute("x").set_value(playerPos.getX());
	saveFile.child("config").child("save").child("player").attribute("y").set_value(playerPos.getY());
	saveFile.child("config").child("save").child("player").attribute("points").set_value(player->GemPoints);
	saveFile.child("config").child("save").child("player").attribute("level").set_value(level);

	//find the proper level to save to
	pugi::xml_node levelSaveNode;
	for (pugi::xml_node saveNode = saveFile.child("config").child("save").child("levels").child("level"); saveNode; saveNode = saveNode.next_sibling("level")) {
		if (saveNode.attribute("level").as_int() == level) levelSaveNode = saveNode;
	}

	//save best time
	levelSaveNode.attribute("time").set_value(bestTime);

	//save alive enemies and their postions
	pugi::xml_node enemiesNode = levelSaveNode.child("enemies");
	enemiesNode.remove_children();
	Vector2D enemyPos;

	for (auto e : enemyList) {
		pugi::xml_node nodeEnemy = enemiesNode.append_child("enemy");
		nodeEnemy.append_attribute("name") = e->name.c_str();
		enemyPos = e->GetPosition();
		nodeEnemy.append_attribute("x") = enemyPos.getX();
		nodeEnemy.append_attribute("y") = enemyPos.getY();
	}

	//save items that haven't been collected yet
	pugi::xml_node itemsNode = levelSaveNode.child("items");
	itemsNode.remove_children();

	for (auto i : itemList) {
		pugi::xml_node nodeItem = itemsNode.append_child("item");
		nodeItem.append_attribute("id") = i->id;
	}

	//save checkpoints that are active
	pugi::xml_node checkpointsNode = levelSaveNode.child("checkpoints");
	checkpointsNode.remove_children();

	for (auto c : checkpointList) {
		if (c->isActive) {
			pugi::xml_node nodeCheckpoint = checkpointsNode.append_child("checkpoint");
			nodeCheckpoint.append_attribute("id") = c->id;
			nodeCheckpoint.append_attribute("x") = c->playerPos.getX();
			nodeCheckpoint.append_attribute("y") = c->playerPos.getY();
		}
	}

	saveFile.save_file("config.xml");
}


void Scene::ChangeTimerVar(bool sixty) {
	if (sixty) {
		timeVar = 1;
		player->timerVar = 1;
		for (auto e : enemyList) {
			e->timerVar = 1;
		}
	}
	else {
		timeVar = 2;
		player->timerVar = 2;
		for (auto e : enemyList) {
			e->timerVar = 2;
		}
	}
}

bool Scene::OnGuiMouseClickEvent(GuiControl* control)
{
	// L15: DONE 5: Implement the OnGuiMouseClickEvent method
	LOG("Press Gui Control: %d", control->id);
	if (control->id < GUI_ID::ID_TELEPORT) {
		switch (control->id) {
		case GUI_ID::ID_PLAY:
			level = 1;
			newGame = true;
			playerPoints = 0;
			CleanUp();
			state = SceneState::GAME;
			Awake();
			Engine::GetInstance().entityManager->Awake();
			Start();
			Engine::GetInstance().entityManager->Start();
			break;
		case GUI_ID::ID_CONTINUE:
			CleanUp();
			state = SceneState::GAME;
			continueGame = true;
			Awake();
			Engine::GetInstance().entityManager->Awake();
			Start();
			Engine::GetInstance().entityManager->Start();
			break;
		case GUI_ID::ID_EXIT:
			toExit = true;
			break;
		case GUI_ID::ID_CREDITS:
			startBT->state = GuiControlState::VISIBLE;
			continueBT->state = GuiControlState::VISIBLE;
			settingBT->state = GuiControlState::VISIBLE;
			creditsBT->state = GuiControlState::VISIBLE;
			exitBT->state = GuiControlState::VISIBLE;
			state = SceneState::CREDITS;
			break;
		case GUI_ID::ID_SETTINGS:
			startBT->state = GuiControlState::VISIBLE;
			continueBT->state = GuiControlState::VISIBLE;
			settingBT->state = GuiControlState::VISIBLE;
			creditsBT->state = GuiControlState::VISIBLE;
			exitBT->state = GuiControlState::VISIBLE;

			musicSlider->state = GuiControlState::NORMAL;
			fxSlider->state = GuiControlState::NORMAL;
			fullscreenCheckBox->state = GuiControlState::NORMAL;

			state = SceneState::SETTINGS;
			break;
		case GUI_ID::ID_PAUSED_EXIT:
			toExit = true;
			break;
		case GUI_ID::ID_PAUSED_SETTINGS:
			state = SceneState::SETTINGS;

			resumeBT->state = GuiControlState::DISABLED;
			PAUSEDsettingsBT->state = GuiControlState::DISABLED;
			titleBT->state = GuiControlState::DISABLED;
			PAUSEDexitBT->state = GuiControlState::DISABLED;

			musicSlider->state = GuiControlState::NORMAL;
			fxSlider->state = GuiControlState::NORMAL;
			fullscreenCheckBox->state = GuiControlState::NORMAL;
			break;
		case GUI_ID::ID_RESUME:
			pausedGame = false;
			pausedTime += (timer.ReadMSec() - startPauseTime);
			break;
		case GUI_ID::ID_TITLE:
			Mix_PauseMusic();
			pausedGame = false;
			CleanUp();
			resumeBT->state = GuiControlState::DISABLED;
			PAUSEDsettingsBT->state = GuiControlState::DISABLED;
			titleBT->state = GuiControlState::DISABLED;
			PAUSEDexitBT->state = GuiControlState::DISABLED;
			Engine::GetInstance().entityManager->CleanUp();
			state = SceneState::MENU;
			Start();
			break;
		case GUI_ID::ID_RESPAWN:
			deathScreen = false;
			player->doRespawn = true;
			break;
		case GUI_ID::ID_NEXT:
			Mix_PauseMusic();
			levelFinishedScreen = false;
			nextBT->state = GuiControlState::DISABLED;
			player->finishedLevel = false;
			CleanUp();
			Engine::GetInstance().entityManager->CleanUp();
			Awake();
			Engine::GetInstance().entityManager->Awake();
			Start();
			Engine::GetInstance().entityManager->Start();
			break;
		case GUI_ID::ID_SLIDEM:
			
			break;
		case GUI_ID::ID_SLIDEF:

			break;
		case GUI_ID::ID_CHECKBOX:
			
			break;
		}
	}
	else {
		bool IDfound = false;
		Vector2D teleportPos = GetPlayerPosition();
		int TPlevel = level;

		for (int i = 0; i < checkpointTPList.size(); ++i) {
			if (!IDfound) {
				if (checkpointTPList[i].UI_ID == control->id) {
					IDfound = true;
					teleportPos = checkpointTPList[i].playerPos;
					TPlevel = checkpointTPList[i].level;
				}
			}
		}

		if (TPlevel != level) {
			level = TPlevel;
			canLoad = true;
			SaveState();
			CleanUp();
			Engine::GetInstance().entityManager->CleanUp();
			continueGame = true;
			Awake();
			Engine::GetInstance().entityManager->Awake();
			Start();
			Engine::GetInstance().entityManager->Start();
		}

		checkpointTeleportView = false;
		player->SetPosition(teleportPos);
		EndTeleportUI();
	}

	return true;
}

