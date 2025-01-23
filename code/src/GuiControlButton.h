#pragma once

#include "GuiControl.h"
#include "Vector2D.h"

class Window;

class GuiControlButton : public GuiControl
{

public:

	GuiControlButton(int id, SDL_Rect bounds, const char* text);
	virtual ~GuiControlButton();

	// Called each loop iteration
	bool Update(float dt);

private:

	bool canClick = true;
	bool drawBasic = false;
	int selectFX, hoverFX;
	bool focused = false;
};

class GuiControlSlidebox : public GuiControl
{

public:

	GuiControlSlidebox(int id, SDL_Rect bounds, const char* text);
	virtual ~GuiControlSlidebox();

	// Called each loop iteration
	bool Update(float dt);

	void SetTexture(SDL_Texture* background, SDL_Texture* movement) {
		sliderBackground = background;
		slider = movement;
	}

	int checkID;

private:
	SDL_Texture* sliderBackground = nullptr;
	SDL_Texture* slider = nullptr;
	int posButton;
	Vector2D pos;
	Vector2D posHitbox;
	Vector2D posTexture;
	bool drawBasic = false;
	int selectFX, hoverFX;
};


class GuiControlCheckbox : public GuiControl
{

public:

	GuiControlCheckbox(int id, SDL_Rect bounds, const char* text);
	virtual ~GuiControlCheckbox();

	// Called each loop iteration
	bool Update(float dt);

	void SetTexture(SDL_Texture* off, SDL_Texture* on) {
		checkboxOff = off;
		checkboxOn = on;
		Check = checkboxOff;
	}

	bool IsActiveFullScreen() {
		return fullScreen;
	}

private:

	bool fullScreen = false;

	SDL_Texture* checkboxOff;
	SDL_Texture* checkboxOn;
	SDL_Texture* Check;
	Vector2D posHitbox;
	Vector2D posTexture;
	bool drawBasic = false;
	bool activated;
	int selectFX, hoverFX;
};

#pragma once