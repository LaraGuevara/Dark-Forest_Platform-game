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

bool GuiControlButton::Update(float dt)
{
	if (state != GuiControlState::DISABLED and state != GuiControlState::VISIBLE)
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

	return false;
}

