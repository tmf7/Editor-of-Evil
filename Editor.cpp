#include "Editor.h"
#include <tuple>

eEditor editor;
std::shared_ptr<eImage> target = nullptr;
Uint32 globalIDPool = 0;

//*****************
// eEditor::Init
//*****************
bool eEditor::Init() {
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Editor of Evil", "SDL failed to initialize!", NULL);
		return false;
	}

	if (!renderer.Init()) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Editor of Evil", "Renderer failed to initialize!", NULL);
		return false;
	}

	if (!imageManager.Init() || !imageManager.BatchLoad("batches/map1.imb")) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Editor of Evil", "Image Manager failed to initialize!", NULL);
		return false;
	}

	// TODO/FIXME: hard-coded target image for testing
	if (!imageManager.GetImage("graphics/characters/heroine/jog/Jog_0(14,60,111).png", target)) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Editor of Evil", "Target failed to initialize!", NULL);
		return false;
	}

	// TODO/FIXME: hard-coded button tiler for testing
	std::shared_ptr<eImageTiler> result = nullptr;
	if (!imageTilerManager.LoadTiler("graphics/gui/buttons/evil_three_state_button.tls", result)) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Editor of Evil", "Tiler failed to initialize!", NULL);
		return false;
	}

	// set a static screen location at HALF-SIZE of original 
	// button frame (local scaling, as opposed to RENDERTYPE_DYNAMIC scaling)
	// TODO: change this to a batch initialization for an eOverlay
	testButton.Init(SDL_Rect{50, renderer.ViewArea().h - 50, 150, 40}, result);

//	SliceTarget();		// FIXME: old hack for auto slicing (broken function)

	camera.Init();		// DEUBG: this must come after target initialization
	input.Init();		// DEBUG: will crash if it fails around the allocation
	return true;
}

//*****************
// eEditor::Shutdown
// TODO: incorperate IMG_Init(...) and TTF_Init(...)
// and IMG_Quit(); and TTF_Quit();
//*****************
void eEditor::Shutdown() {
	renderer.Free();
	SDL_Quit();
}

//*****************
// eEditor::RunFrame
// TODO(?): pop up a dialogue to type the name of the image file to load
// TODO(?): pop up a dialogue to type the name of the file to output results to
// TODO: load image file to the window (and in textureManager)
// TODO: read/write rectangle data to a .def file
//*****************
bool eEditor::RunFrame() {
	static const int	fps = 60;
	Uint32 start = SDL_GetTicks();

	bool programRunning = true;
	static SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT)
			programRunning = false;
	}

	input.Update();
	camera.Think();
	testButton.Think();	// FIXME/TODO: have a buttonManager do a batch Think() on the eOverlay Think() call

	renderer.Clear();
	
	// DEBUG: hack for editing target draw
	SDL_Rect dstRect{ eMath::NearestInt(-camera.Origin().x), eMath::NearestInt(-camera.Origin().y), target->GetWidth() , target->GetHeight() };
	renderer.AddToRenderPool(renderImage_t{target, nullptr, dstRect, 0}, RENDERTYPE_DYNAMIC);

	// DEGUG: test draw the button normally
	testButton.Draw();
	renderer.Flush();

	// DEBUG: hack for drawing slices
	Uint8 debugColor[] = { 0, 128, 0, 255 };	// opaque green
	renderer.DrawDebugRect(debugColor, SDL_Rect{ 0,0,4,4 }, true, RENDERTYPE_STATIC);
//	for (int index = 0; index < targetTiler->GetNumFrames(); index++)
//		renderer.DrawDebugRect(debugColor, targetTiler->GetFrame(index), RENDERTYPE_STATIC);

	renderer.Show();

	Uint32 frameDuration = SDL_GetTicks() - start;	// DEBUG: always positive unless game runs for ~49 days
	int delay = (1000 / fps) - frameDuration;		// DEBUG: potentially negative
	SDL_Delay(delay > 0 ? delay : 0);				// FIXME: SDL_Delay isn't the most reliable frame delay method

	return programRunning;
}

//****************
// eEditor::SliceTarget
// adds a series of SDL_Rects to outputRects
// reads the pixel data to determine each size and position
//****************
void eEditor::SliceTarget() {

	// get basic target image information
	int textureWidth, textureHeight;
	Uint32 textureFormat;
	SDL_QueryTexture(target->Source(), &textureFormat, NULL , &textureWidth, &textureHeight);

	// only split targets that have alpha channels
	if (!SDL_ISPIXELFORMAT_ALPHA(textureFormat))
		return;

	// get a copy of the target image's pixel data
	const int texturePitch = SDL_BYTESPERPIXEL(textureFormat) * textureWidth;
	Uint32 * texturePixels = new Uint32[texturePitch * textureHeight];	// DEBUG: allocate space for 32bpp even if only 8bpp
	SDL_SetRenderTarget(renderer.GetSDLRenderer(), target->Source());
	SDL_RenderReadPixels(renderer.GetSDLRenderer(), NULL, textureFormat, texturePixels, texturePitch);
	SDL_SetRenderTarget(renderer.GetSDLRenderer(), NULL);
	
	// determine relevant pixel indexing values
	const int lastPixel = texturePitch * textureHeight;
	int pixelRows = lastPixel / textureWidth;
	int pixelColumns = lastPixel % textureWidth;
	
	// traverse the pixel data and construct SDL_Rects 
	// around each sub-image of the target image

	std::vector<std::tuple<int, int>> startPoints;		// DEBUG: list of pixel row, colum pairs that define new search areas
//	startPoints.push_back(std::make_tuple<int, int>(row, column));
//	int startRow = std::get<0>(startPoints[i]);
//	int startCol = std::get<1>(startPoints[i]);

	// TODO: first focus on finding a bottom-right corner for the rect
	// assuming the top-left is at 0,0 (or startPoints[i])
	// THEN focus on shifting the top-left corner 
	SDL_PixelFormat * pixelFormat = SDL_AllocFormat(textureFormat);
	for (int count = 0; count < lastPixel; count++) {	// DEBUG: this loop ensures every transparent pixel gets checked at least once
		// TODO: make a maxRow and maxCol
		// row and column will be jumping around alot during the search
		// and incrementing differently depending on which edge of the expanding rect is being checked
		// DO NOT search a row/column if maxRow/maxColumn hasnt changed
		// DO NOT allow maxRow/maxColumn to go beyond pixelRows/pixelColumns

		int maxRow = 1;
		int maxColumn = 1;
		int row, column;

		Uint8 red, green, blue, alpha;

		// search down the right edge of the rect
		for (row = 0, column = maxColumn; row < maxRow; row++) {
			SDL_GetRGBA(texturePixels[row * pixelColumns + column], pixelFormat, &red, &green, &blue, &alpha);
			// continue to increase maxRow until an alpha > 0 is found, then set a bool waitForClearRow
			// if waitForClear == true (ie an alpha > 0 pixel was previously found moving vertically)
			// AND row == maxRow (ie loop finishes) then DO NOT INCREASE maxRow anymore (yet)
		}

		// search across the bottom edge of the rect
		for (row = maxRow, column = 0; column < maxColumn; column++) {
			SDL_GetRGBA(texturePixels[row * pixelColumns + column], pixelFormat, &red, &green, &blue, &alpha);
			// continue to increase maxColumn until an alpha > 0 is found, then set a bool waitForClearColumn
			// if waitForClear == true (ie an alpha > 0 pixel was previously found moving horizontally)
			// AND column == maxColumn (ie loop finishes) then DO NOT INCREASE maxColumn anymore (yet)
		}

		// if either maxRow or maxColumn has stopped moving, but row/column DON'T get to maxRow/maxColumn
		// that means 
		

		// TODO: now check if the alpha > SDL_ALPHA_TRANSPARENT
		// if it is, then trigger something, then potentially wait until another trigger point

		// TODO: search by a row at fixed column, then a column by fixed row
		// as in the edges of a rectangle, not the full insides
		// but start the rectangle at x,y,w,h == 0,0,1,1

		//	int x = column;
		//	int y = row;
		//	int w = frameWidth;
		//	int h = frameHeight;
		//	outputRects.push_back( SDL_Rect{ x, y, w, h} );
	} 

	delete[] texturePixels;		// delete the copy of the texture pixels made
	SDL_FreeFormat(pixelFormat);
}