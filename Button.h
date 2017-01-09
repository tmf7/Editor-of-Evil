#ifndef EVIL_BUTTON_H
#define EVIL_BUTTON_H

#include "Editor.h"
//#include "Image.h"

//******************************
//		eButton
// interactive animated graphic
// that hold three states: 
// pressed, mouse over, and triggered
// TODO: add a eButtonManager that can update all buttons in one call
// TODO: user defines IsTriggered() behavior 
// TODO: user sets the button image's frame (to allow for extension to radio buttons, or unanimated buttons)
//******************************
class eButton {
public:

	bool				Init(const SDL_Rect & screenRegion, const char * buttonDefFile);
	bool				IsTriggered() const;
	bool				IsPressed() const;
	bool				IsMouseOver() const;
	void				Think();
	void				Draw();

private:

	eImageTiler			statesImage;		// contains a pointer to the image of various button states
	eBounds				clickRegion;
	char				state;
	bool				mouseOver;
	bool				pressed;
	bool				triggered;

};

//*************
// eButton::Draw
//**************
inline void eButton::Draw() {
	editor.GetRenderer().AddToRenderPool( renderImage_t{ clickRegion[0], statesImage.Source(), &statesImage.GetFrame(), MAX_LAYER } );
}

//*************
// eButton::IsPressed
//**************
inline bool eButton::IsPressed() const {
	return pressed;
}

//*************
// eButton::IsTriggered
//**************
inline bool eButton::IsTriggered() const {
	return triggered;
}

//*************
// eButton::IsMouseOver
//**************
inline bool eButton::IsMouseOver() const {
	return mouseOver;
}

#endif /* EVIL_BUTTON_H */