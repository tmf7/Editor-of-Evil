#include "Camera.h"
#include "Editor.h"

//***************
// eCamera::Init
// starting view of map
// TODO: allow the localBounds to be resized as the window resizes or goes fullscreen
//***************
void eCamera::Init() {
	SetZoom(1);
	SetOrigin(localBounds[1] / 2.0f);
}

//***************
// eCamera::Think
// FIXME/TODO: modify the movement limits (either stay inside the "diamond" or allow some minimal wander beyond it
// EG: an overall larger rectangle that the diamond is within [risky, given that some corner gaps would lead to total abyss])
//***************
void eCamera::Think() {
	eInput * input;
	float vx, vy;

	input = &editor.GetInput();
	if (input->KeyPressed(SDL_SCANCODE_EQUALS))
		SetZoom(zoomLevel + zoomIncrement);
	else if (input->KeyPressed(SDL_SCANCODE_MINUS))
		SetZoom(zoomLevel - zoomIncrement);

	if (input->KeyHeld(SDL_SCANCODE_SPACE)) {
		SetOrigin(vec2_zero);
	} else {
		vx = speed * (float)(input->KeyHeld(SDL_SCANCODE_D) - input->KeyHeld(SDL_SCANCODE_A));
		vy = speed * (float)(input->KeyHeld(SDL_SCANCODE_S) - input->KeyHeld(SDL_SCANCODE_W));
		velocity.Set(vx, vy);
		UpdateOrigin();
	}
}

//***************
// eCamera::SetZoom
//***************
void eCamera::SetZoom(float level) {
	if (level < minZoom)
		level = minZoom;
	else if (level > maxZoom)
		level = maxZoom;

	zoomLevel = level;

	eVec2 screenBottomRight = eVec2((float)editor.GetRenderer().ViewArea().w, (float)editor.GetRenderer().ViewArea().h);
	screenBottomRight *= level;

	// variable rectangle with (0, 0) at its center)
	localBounds = eBounds(-screenBottomRight / 2.0f, screenBottomRight / 2.0f);	
}

