#ifndef EVIL_EDITOR_H
#define EVIL_EDITOR_H

#include "Definitions.h"
#include "ImageManager.h"
#include "Renderer.h"
#include "Input.h"
#include "Camera.h"
#include "Overlay.h"

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
	eImageManager &		GetImageManager();

private:

	eRenderer			renderer;
	eInput				input;
	eCamera				camera;
	eImageManager		imageManager;
	eOverlay			overlay;
};

extern eEditor editor;						// DEBUG: one instance used by the entire system
extern std::shared_ptr<eImage> target;		// DEBUG: hard-coded image to edit

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

//*****************
// eEditor::GetImageManager
//*****************
inline eImageManager & eEditor::GetImageManager() {
	return imageManager;
}

#endif /* EVIL_EDITOR_H */