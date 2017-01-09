#include "Button.h"

//*************
// eButton::Init
// TODO: parse buttonDefFile
// read rectangles, and default state
//**************
bool eButton::Init(const SDL_Rect & screenRegion, const char * buttonDefFile) {

	std::shared_ptr<eImage> source;
	editor.GetImageManager().GetImage(buttonDefFile, source); // have the parse do this call, obviously
//	statesImage.Init(source, subFrameList);
	statesImage.SetFrame(0);
	return false;
}

//*************
// eButton::Think
//**************
void eButton::Think() {
	eInput & input = editor.GetInput();
	if (clickRegion.ContainsPoint(eVec2((float)input.GetMouseX(), (float)input.GetMouseY()))) {
		mouseOver = true;
		if (!pressed && input.MousePressed(SDL_BUTTON_LEFT)) {
			pressed = true;
		} else if (pressed && input.MouseReleased(SDL_BUTTON_LEFT)) {
			triggered = true;
			pressed = false;
		}
	} else { 
		mouseOver = false; 
	}
}