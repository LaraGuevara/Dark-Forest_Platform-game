#include "GuiControlButton.h"
#include "Render.h"
#include "Engine.h"
#include "Audio.h"

GuiControlButton::GuiControlButton(int id, SDL_Rect bounds, const char* text) : GuiControl(GuiControlType::BUTTON, id)
{
	this->bounds = bounds;
	this->text = text;

	canClick = true;
	drawBasic = false;

	hoverFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/UI/Fantasy_UI (3).wav");
	selectFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/UI/Fantasy_UI (18).wav");
}

GuiControlButton::~GuiControlButton()
{

}

GuiControlSlidebox::GuiControlSlidebox(int id, SDL_Rect bounds, const char* text) : GuiControl(GuiControlType::SLIDERBAR, id)
{
	this->bounds = bounds;
	posButton = bounds.x + 90;
	this->text = text;
	hoverFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/UI/Fantasy_UI (3).wav");
	selectFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/UI/Fantasy_UI (18).wav");
	//sliderBackground = Engine::GetInstance().textures.get()->Load("Assets/Textures/sliderBackground.png");
	//slider = Engine::GetInstance().textures.get()->Load("Assets/Textures/slider.png");
}

GuiControlSlidebox::~GuiControlSlidebox()
{
}


GuiControlCheckbox::GuiControlCheckbox(int id, SDL_Rect bounds, const char* text) : GuiControl(GuiControlType::CHECKBOX, id)
{
	this->bounds = bounds;
	this->text = text;
	hoverFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/UI/Fantasy_UI (3).wav");
	selectFX = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/UI/Fantasy_UI (18).wav");
}

	

GuiControlCheckbox::~GuiControlCheckbox()
{

}

bool GuiControlButton::Update(float dt)
{
	if (state != GuiControlState::DISABLED and state != GuiControlState::VISIBLE and state != GuiControlState::UNCLICKABLE)
	{
		// L16: TODO 3: Update the state of the GUiButton according to the mouse position
		Vector2D mousePos = Engine::GetInstance().input->GetMousePosition();

		//If the position of the mouse if inside the bounds of the button 
		if (mousePos.getX()*2 > bounds.x && mousePos.getX()*2 < bounds.x + bounds.w && mousePos.getY()*2 > bounds.y && mousePos.getY()*2 < bounds.y + bounds.h) {
		
			if (state != GuiControlState::FOCUSED and !focused) {
				Engine::GetInstance().audio->PlayFx(hoverFX);
				focused = true;
			}

			state = GuiControlState::FOCUSED;

			if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT) {
				state = GuiControlState::PRESSED;
				Engine::GetInstance().audio->PlayFx(selectFX);
			}
			
			if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP) {
				NotifyObserver();
			}
		}
		else {
			state = GuiControlState::NORMAL;
			focused = false;
		}

		//L16: TODO 4: Draw the button according the GuiControl State
		switch (state)
		{
		case GuiControlState::DISABLED:
			Engine::GetInstance().render->DrawRectangle(bounds, 200, 200, 200, 255, true, false);
			break;
		case GuiControlState::NORMAL:
			Engine::GetInstance().render->DrawRectangle(bounds, 238, 231, 215, 200, true, false);
			break;
		case GuiControlState::FOCUSED:
			Engine::GetInstance().render->DrawRectangle(bounds, 172, 229, 238, 200, true, false);
			break;
		case GuiControlState::PRESSED:
			Engine::GetInstance().render->DrawRectangle(bounds, 0, 255, 255, 255, true, false);
			break;
		}

		Engine::GetInstance().render->DrawText(text.c_str(), bounds.x, bounds.y, bounds.w, bounds.h);

	}

	if (state == GuiControlState::VISIBLE) {
		Engine::GetInstance().render->DrawRectangle(bounds, 246, 238, 227, 200, true, false);
		Engine::GetInstance().render->DrawText(text.c_str(), bounds.x, bounds.y, bounds.w, bounds.h);
	}

	if (state == GuiControlState::UNCLICKABLE) {
		Engine::GetInstance().render->DrawRectangle(bounds, 93, 93, 93, 200, true, false);
		Engine::GetInstance().render->DrawText(text.c_str(), bounds.x, bounds.y, bounds.w, bounds.h);
	}

	if (debugDraw and state != GuiControlState::DISABLED) {
		switch (state) {
		case GuiControlState::NORMAL:
			Engine::GetInstance().render->DrawRectangle(bounds, 255, 255, 255, 255, false, false);
			break;
		case GuiControlState::FOCUSED:
			Engine::GetInstance().render->DrawRectangle(bounds, 0, 0, 255, 255, false, false);
			break;
		case GuiControlState::PRESSED:
			Engine::GetInstance().render->DrawRectangle(bounds, 0, 128, 0, 255, false, false);
			break;
		case GuiControlState::VISIBLE:
			Engine::GetInstance().render->DrawRectangle(bounds, 93, 93, 93, 255, false, false);
			break;
		case GuiControlState::UNCLICKABLE:
			Engine::GetInstance().render->DrawRectangle(bounds, 0, 0, 0, 255, false, false);
			break;
		}
	}

	return false;
}

bool GuiControlSlidebox::Update(float dt)
{
	if (state != GuiControlState::DISABLED and state != GuiControlState::VISIBLE and state != GuiControlState::UNCLICKABLE)
	{

		Vector2D mousePos = Engine::GetInstance().input->GetMousePosition();

		posHitbox.setX(bounds.w);
		posHitbox.setY(bounds.y);
		posTexture.setX(-(Engine::GetInstance().render.get()->camera.x / 2) + bounds.x);


		//If the position of the mouse if inside the bounds of the button 
		if (mousePos.getX() > posHitbox.getX() && mousePos.getX() < (posHitbox.getX() + (bounds.w / 2)) && mousePos.getY() > posHitbox.getY() && mousePos.getY() < (posHitbox.getY() + bounds.h)) {

			state = GuiControlState::FOCUSED;

			if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT) {
				state = GuiControlState::PRESSED;
				posButton = -(Engine::GetInstance().render.get()->camera.x / 2) + mousePos.getX() - 8;
				if (checkID == 1) {/*MUSIC*/
					Mix_VolumeMusic((posButton - 200)*2);
				}
				else {
					Engine::GetInstance().audio.get()->ChangeVolume(((posButton - 255) * 2) + Engine::GetInstance().render.get()->camera.x, 3);
					Engine::GetInstance().audio.get()->ChangeVolume(((posButton - 255) * 2) + Engine::GetInstance().render.get()->camera.x, 2);
					Engine::GetInstance().audio.get()->ChangeVolume(((posButton - 255) * 2) + Engine::GetInstance().render.get()->camera.x, 4);
				}
			}

			if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP) {
				NotifyObserver();
			}
		}
		else {
			state = GuiControlState::NORMAL;
		}

		SDL_Rect background = { 0,0,100,12 };
		SDL_Rect box = { 0,0,12,12 };
		switch (state)
		{
		case GuiControlState::DISABLED:
			Engine::GetInstance().render->DrawTexture(sliderBackground, bounds.x, bounds.y, &background);
			Engine::GetInstance().render->DrawTexture(slider, posButton, bounds.y, &box);
			break;
		case GuiControlState::NORMAL:
			Engine::GetInstance().render->DrawTexture(sliderBackground, bounds.x, bounds.y, &background);
			Engine::GetInstance().render->DrawTexture(slider, posButton, bounds.y, &box);
			break;
		case GuiControlState::FOCUSED:
			Engine::GetInstance().render->DrawTexture(sliderBackground, bounds.x, bounds.y, &background);
			Engine::GetInstance().render->DrawTexture(slider, posButton, bounds.y, &box);
			break;
		case GuiControlState::PRESSED:
			Engine::GetInstance().render->DrawTexture(sliderBackground, bounds.x, bounds.y, &background);
			Engine::GetInstance().render->DrawTexture(slider, posButton, bounds.y, &box);
			break;
		}

		Engine::GetInstance().render->DrawText(text.c_str(), bounds.x + (100), bounds.y + (30 * checkID), 100, 50);
	}

	return false;
}

bool GuiControlCheckbox::Update(float dt)
{
	if (state != GuiControlState::DISABLED and state != GuiControlState::VISIBLE and state != GuiControlState::UNCLICKABLE)
	{
		// L16: TODO 3: Update the state of the GUiButton according to the mouse position
		Vector2D mousePos = Engine::GetInstance().input->GetMousePosition();

		//If the position of the mouse if inside the bounds of the button 
		if (mousePos.getX() > bounds.x && mousePos.getX() < bounds.x + bounds.w && mousePos.getY() > bounds.y && mousePos.getY() < bounds.y + bounds.h) {
			state = GuiControlState::FOCUSED;

			if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT) {
				state = GuiControlState::PRESSED;
				if (fullScreen) fullScreen = false;
				else fullScreen = true;
				Engine::GetInstance().audio->PlayFx(selectFX);
			}

			if (Engine::GetInstance().input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP) {
				NotifyObserver();
			}
		}
		else {
			state = GuiControlState::NORMAL;
			
		}

		SDL_Rect rect = { 0,0,12,12 };

		//L16: TODO 4: Draw the button according the GuiControl State
		switch (state)
		{
		case GuiControlState::DISABLED:
			break;
		case GuiControlState::NORMAL:
			if (fullScreen) {
				Engine::GetInstance().render->DrawTexture(checkboxOn, bounds.x, bounds.y, &rect);
			}
			else {
				Engine::GetInstance().render->DrawTexture(checkboxOff, bounds.x, bounds.y, &rect);
			}
			break;
		case GuiControlState::FOCUSED:
			if (fullScreen) {
				Engine::GetInstance().render->DrawTexture(checkboxOn, bounds.x, bounds.y, &rect);
			}
			else {
				Engine::GetInstance().render->DrawTexture(checkboxOff, bounds.x, bounds.y, &rect);
			}
			break;
		case GuiControlState::PRESSED:
			if (fullScreen) {
				Engine::GetInstance().render->DrawTexture(checkboxOn, bounds.x, bounds.y, &rect);
			}
			else {
				Engine::GetInstance().render->DrawTexture(checkboxOff, bounds.x, bounds.y, &rect);
			}
			break;
		}

		Engine::GetInstance().render->DrawText(text.c_str(), bounds.x + 70, bounds.y+80, 100, 40);

	}

	return false;
}


