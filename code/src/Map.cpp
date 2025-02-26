
#include "Engine.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"
#include "Sensor.h"
#include "tracy/Tracy.hpp"

#include <math.h>

Map::Map() : Module(), mapLoaded(false)
{
    name = "map";
}

// Destructor
Map::~Map()
{}

// Called before render is available
bool Map::Awake()
{
    name = "map";
    LOG("Loading Map Parser");

    return true;
}

bool Map::Start() {

    return true;
}

bool Map::Update(float dt)
{
    ZoneScoped;
    bool ret = true;

    if (mapLoaded) {
        // iterate all tiles in a layer
        for (const auto& mapLayer : mapData.layers) {
            //Check if the property Draw exist get the value, if it's true draw the lawyer
            if (mapLayer->properties.GetProperty("Draw") != NULL && mapLayer->properties.GetProperty("Draw")->value == true) {
                for (int i = 0; i < mapData.width; i++) {
                    for (int j = 0; j < mapData.height; j++) {

                        //Get the gid from tile
                        int gid = mapLayer->Get(i, j);
                        //Check if the gid is different from 0 - some tiles are empty
                        if (gid != 0) {
                            TileSet* tileSet = GetTilesetFromTileId(gid);
                            if (tileSet != nullptr) {
                                //Get the Rect from the tileSetTexture;
                                SDL_Rect tileRect = tileSet->GetRect(gid);
                                //Get the screen coordinates from the tile coordinates
                                Vector2D mapCoord = MapToWorld(i, j);
                                //Draw the texture
                                int cameraX = Engine::GetInstance().render.get()->camera.x;
                                if(mapCoord.getX() >= -cameraX/2-50 and mapCoord.getX() <= -cameraX / 2 +650) Engine::GetInstance().render->DrawTexture(tileSet->texture, mapCoord.getX(), mapCoord.getY(), &tileRect);
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

TileSet* Map::GetTilesetFromTileId(int gid) const
{
	TileSet* set = nullptr;

    for (const auto& tileset : mapData.tilesets) {
    	if (gid >= tileset->firstGid && gid < (tileset->firstGid + tileset->tileCount)) {
			set = tileset;
			break;
		}
    }

    return set;
}

// Called before quitting
bool Map::CleanUp()
{
    LOG("Unloading map");

    for (const auto& tileset : mapData.tilesets) {
        delete tileset;
    }
    mapData.tilesets.clear();

    for (const auto& layer : mapData.layers)
    {
        delete layer;
    }
    mapData.layers.clear();

    return true;
}

// Load new map
bool Map::Load(std::string path, std::string fileName)
{

    mapData.layers.clear();
    
    bool ret = false;

    // Assigns the name of the map file and the path
    mapFileName = fileName;
    mapPath = path;
    std::string mapPathName = mapPath + mapFileName;

    pugi::xml_document mapFileXML;
    pugi::xml_parse_result result = mapFileXML.load_file(mapPathName.c_str());

    if(result == NULL)
	{
		LOG("Could not load map xml file %s. pugi error: %s", mapPathName.c_str(), result.description());
		ret = false;
    }
    else {
        // retrieve the paremeters of the <map> node and store the into the mapData struct
        mapData.width = mapFileXML.child("map").attribute("width").as_int();
        mapData.height = mapFileXML.child("map").attribute("height").as_int();
        mapData.tileWidth = mapFileXML.child("map").attribute("tilewidth").as_int();
        mapData.tileHeight = mapFileXML.child("map").attribute("tileheight").as_int();
       
        //Iterate the Tileset
        for(pugi::xml_node tilesetNode = mapFileXML.child("map").child("tileset"); tilesetNode!=NULL; tilesetNode = tilesetNode.next_sibling("tileset"))
		{
            //Load Tileset attributes
			TileSet* tileSet = new TileSet();
            tileSet->firstGid = tilesetNode.attribute("firstgid").as_int();
            tileSet->name = tilesetNode.attribute("name").as_string();
            tileSet->tileWidth = tilesetNode.attribute("tilewidth").as_int();
            tileSet->tileHeight = tilesetNode.attribute("tileheight").as_int();
            tileSet->spacing = tilesetNode.attribute("spacing").as_int();
            tileSet->margin = tilesetNode.attribute("margin").as_int();
            tileSet->tileCount = tilesetNode.attribute("tilecount").as_int();
            tileSet->columns = tilesetNode.attribute("columns").as_int();

			//Load the tileset image
			std::string imgName = tilesetNode.child("image").attribute("source").as_string();
            tileSet->texture = Engine::GetInstance().textures->Load((mapPath+imgName).c_str());

			mapData.tilesets.push_back(tileSet);
		}
        for (pugi::xml_node layerNode = mapFileXML.child("map").child("layer"); layerNode != NULL; layerNode = layerNode.next_sibling("layer")) {

            //Load the attributes and saved in a new MapLayer
            MapLayer* mapLayer = new MapLayer();
            mapLayer->id = layerNode.attribute("id").as_int();
            mapLayer->name = layerNode.attribute("name").as_string();
            mapLayer->width = layerNode.attribute("width").as_int();
            mapLayer->height = layerNode.attribute("height").as_int();

            LoadProperties(layerNode, mapLayer->properties);

            //Iterate over all the tiles and assign the values in the data array
            for (pugi::xml_node tileNode = layerNode.child("data").child("tile"); tileNode != NULL; tileNode = tileNode.next_sibling("tile")) {
                mapLayer->tiles.push_back(tileNode.attribute("gid").as_int());
            }

            //add the layer to the map
            mapData.layers.push_back(mapLayer);
        }

        for (pugi::xml_node objectNode = mapFileXML.child("map").child("objectgroup"); objectNode != NULL; objectNode = objectNode.next_sibling("objectgroup")) {
            std::string objectGroupName = objectNode.attribute("name").as_string();

            for (pugi::xml_node tileNode = objectNode.child("object"); tileNode != NULL; tileNode = tileNode.next_sibling("object")) {
                if (objectGroupName == "Finish Level") {
                    LOG("%d, %d", tileNode.attribute("x").as_int(), tileNode.attribute("y").as_int());
                    int width = tileNode.attribute("width").as_int();
                    int height = tileNode.attribute("height").as_int();
                    PhysBody* c1 = Engine::GetInstance().physics.get()->CreateRectangleSensor(tileNode.attribute("x").as_int() + width / 2, tileNode.attribute("y").as_int() + height / 2, width, height, STATIC);
                    c1->ctype = ColliderType::LEVELEND;
                    rectangles.push_back(c1);
                }
                else {
                    LOG("%d, %d", tileNode.attribute("x").as_int(), tileNode.attribute("y").as_int());
                    int width = tileNode.attribute("width").as_int();
                    int height = tileNode.attribute("height").as_int();
                    PhysBody* c1 = Engine::GetInstance().physics.get()->CreateRectangle(tileNode.attribute("x").as_int() + width / 2, tileNode.attribute("y").as_int() + height / 2, width, height, STATIC);
                    if (tileNode.child("properties") != NULL and tileNode.child("properties").child("property").attribute("value").as_bool() == true) {
                        c1->ctype = ColliderType::DEATH;
                    }
                    else c1->ctype = ColliderType::PLATFORM;
                    rectangles.push_back(c1);
                }
            }
        }

        ret = true;

        if (ret == true)
        {
            LOG("Successfully parsed map XML file :%s", fileName.c_str());
            LOG("width : %d height : %d", mapData.width, mapData.height);
            LOG("tile_width : %d tile_height : %d", mapData.tileWidth, mapData.tileHeight);

            LOG("Tilesets----");

            //iterate the tilesets
            for (const auto& tileset : mapData.tilesets) {
                LOG("name : %s firstgid : %d", tileset->name.c_str(), tileset->firstGid);
                LOG("tile width : %d tile height : %d", tileset->tileWidth, tileset->tileHeight);
                LOG("spacing : %d margin : %d", tileset->spacing, tileset->margin);
            }
            			
            LOG("Layers----");

            for (const auto& layer : mapData.layers) {
                LOG("id : %d name : %s", layer->id, layer->name.c_str());
				LOG("Layer width : %d Layer height : %d", layer->width, layer->height);
            }   
        }
        else {
            LOG("Error while parsing map file: %s", mapPathName.c_str());
        }

        if (mapFileXML) mapFileXML.reset();

    }

    mapLoaded = ret;
    return ret;
}

Vector2D Map::MapToWorld(int x, int y) const
{
    Vector2D ret;

    ret.setX(x * mapData.tileWidth);
    ret.setY(y * mapData.tileHeight);

    return ret;
}

Vector2D Map::WorldToMap(int x, int y) {

    Vector2D ret(0, 0);

    ret.setX(x / mapData.tileWidth);
    ret.setY(y / mapData.tileHeight);
    
    return ret;
}

bool Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
    bool ret = false;

    for (pugi::xml_node propertieNode = node.child("properties").child("property"); propertieNode; propertieNode = propertieNode.next_sibling("property"))
    {
        Properties::Property* p = new Properties::Property();
        p->name = propertieNode.attribute("name").as_string();
        p->value = propertieNode.attribute("value").as_bool(); // (!!) I'm assuming that all values are bool !!

        properties.propertyList.push_back(p);
    }

    return ret;
}

MapLayer* Map::GetNavigationLayer() {
    for (const auto& layer : mapData.layers) {
        if (layer->properties.GetProperty("Navigation") != NULL &&
            layer->properties.GetProperty("Navigation")->value) {
            return layer;
        }
    }

    return nullptr;
}

MapLayer* Map::GetCheckpointLayer() {
    for (const auto& layer : mapData.layers) {
        if (layer->properties.GetProperty("Checkpoint") != NULL &&
            layer->properties.GetProperty("Checkpoint")->value) {
            return layer;
        }
    }

    return nullptr;
}

MapLayer* Map::GetJumpingLayer() {
    for (const auto& layer : mapData.layers) {
        if (layer->properties.GetProperty("Jumping") != NULL &&
            layer->properties.GetProperty("Jumping")->value) {
            return layer;
        }
    }
    return nullptr;
}

std::vector<Sensor*> Map::LoadCheckpoints(int level) {
    MapLayer* checkLayer = GetCheckpointLayer();
    std::vector<Sensor*> checkpoints;

    int makeID = 1 + (10*(level-1));
    for (int i = 0; i < checkLayer->width; i++) {
        for (int j = 0; j < checkLayer->height; j++) {
            //Get the gid from tile
            int gid = checkLayer->Get(i, j);
            //Check if the gid is a checkpoint
            if (gid == checkpointGid) {
                Sensor* cp = (Sensor*)Engine::GetInstance().entityManager->CreateEntity(EntityType::SENSOR);
                cp->id = makeID;
                makeID++;
                cp->type = SensorType::CHECKPOINT;
                Vector2D pos;
                pos.setX(i);
                pos.setY(j);
                pos = MapToWorld(pos.getX(), pos.getY());
                cp->position = pos;
                checkpoints.push_back(cp);
            }
        }
    }
    return checkpoints;
}


Properties::Property* Properties::GetProperty(const char* name)
{
    for (const auto& property : propertyList) {
        if (property->name == name) {
			return property;
		}
    }

    return nullptr;
}

bool Map::IsTileJumpable(int x, int y) {
    MapLayer* layerJump = GetJumpingLayer();
    Vector2D pos = WorldToMap(x, y);
    x = pos.getX();
    y = pos.getY();
    if (layerJump != nullptr) {
        if (x >= 0 && y >= 0 && x < mapData.width && y < mapData.height) {
            int gid = layerJump->Get(x, y);
            if (gid == jumpingGid) return true;
        }
    }

    return false;
}

