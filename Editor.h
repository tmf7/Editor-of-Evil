#ifndef EVIL_EDITOR_H
#define EVIL_EDITOR_H

#include "Definitions.h"
#include "TextureManager.h"
#include "Renderer.h"
#include "Input.h"
#include "Camera.h"

class eEditor {
public:

						eEditor();

	bool				Init();
	void				Shutdown();
	bool				RunFrame();
	void				SliceTarget();

	eRenderer &			GetRenderer();
	eInput &			GetInput();
	eCamera &			GetCamera();

private:

	std::vector<SDL_Rect> outputRects;		// sub-image bounding frames within the target image

	eRenderer			renderer;
	eInput				input;
	eCamera				camera;
	eTextureManager		textureManager;
};

extern eEditor editor;		// DEBUG: one instance used by the entire system
extern eImage target;		// DEBUG: hard-coded image to edit

//*****************
// eEditor::eEditor
//*****************
inline eEditor::eEditor() {
}

//*****************
// eEditor::GetInput
//*****************
inline eInput & eEditor::GetInput() {
	return input;
}

//*****************
// eEditor::GetCamera
//*****************
inline eCamera & eEditor::GetCamera() {
	return camera;
}

//*****************
// eEditor::GetRenderer
//*****************
inline eRenderer & eEditor::GetRenderer() {
	return renderer;
}

#endif /* EVIL_EDITOR_H */