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
		zoomLevel += zoomIncrement;
	else if (input->KeyPressed(SDL_SCANCODE_MINUS))
		zoomLevel -= zoomIncrement;

	SetZoom(zoomLevel);

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
// FIXME: currently utilizes debug target image to center the camera
//***************
void eCamera::SetZoom(const float level) {
	if (zoomLevel < minZoom)
		zoomLevel = minZoom;
	else if (zoomLevel > maxZoom)
		zoomLevel = maxZoom;

	zoomLevel = level;
	eVec2 targetBottomRight = eVec2((float)target->GetWidth(), (float)target->GetHeight());	// FIXME: dont use target?
	targetBottomRight *= level;
	localBounds = eBounds(-targetBottomRight / 2.0f, targetBottomRight / 2.0f);	// variable rectangle with (0, 0) at its center)
}

