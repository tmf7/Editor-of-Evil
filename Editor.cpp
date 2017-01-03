#include "Editor.h"

eEditor editor;
eImage target;

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

	// hard-coded target image loading
	SDL_Texture * source = textureManager.GetTexture("graphics/Jog_0(14,60,111).png");
	target.Init(source, "graphics/Jog_0(14,60,111).png");

	SliceTarget();

	camera.Init();

	input.Init();		// DEBUG: will crash if it fails around the allocation
	return true;
}

//*****************
// eEditor::Shutdown
//*****************
void eEditor::Shutdown() {
	textureManager.Free();
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

	renderer.Clear();
	// TODO: allow a linear expansion/contraction zoom without affecting the source texture
	// or, if it does then scale how the rectangles are computed as well
	renderer.AddToRenderQueue(renderImage_t(vec2_zero, &target, 0));
	renderer.FlushRenderQueue();

	Uint8 debugColor[] = { 0, 128, 0, 255 };	// opaque green
	renderer.DrawDebugRect(debugColor, SDL_Rect{ 0,0,4,4 }, true);
	renderer.DrawDebugRects(debugColor, outputRects, false);

	renderer.Show();

	Uint32 frameDuration = SDL_GetTicks() - start;	// DEBUG: always positive unless game runs for ~49 days
	int delay = (1000 / fps) - frameDuration;
	SDL_Delay(delay > 0 ? delay : 0);		// FIXME: SDL_Delay isn't the most reliable frame delay method

	return programRunning;
}

//****************
// eEditor::SliceTarget
// adds a series of SDL_Rects to outputRects
// reads the pixel data to determine each size and position
//****************
void eEditor::SliceTarget() {

	// treat the source surface as a 2D array of images
	int textureWidth, textureHeight;
	Uint32 textureFormat;
	SDL_QueryTexture(target.Source(), &textureFormat, NULL , &textureWidth, &textureHeight);

	// check that the renderer supports changing targets (worked on my hp windows 8 laptop)
//	SDL_RendererInfo info;
//	SDL_GetRendererInfo(renderer.GetSDLRenderer(), &info);
//	int success = info.flags & SDL_RENDERER_TARGETTEXTURE;

	int texturePitch = SDL_BYTESPERPIXEL(textureFormat) * textureWidth;
	Uint8 * texturePixels = new Uint8[texturePitch * textureHeight];	// DEBUG: assume 8 bytes per channel
	SDL_SetRenderTarget(renderer.GetSDLRenderer(), target.Source());
	SDL_RenderReadPixels(renderer.GetSDLRenderer(), NULL, textureFormat, texturePixels, texturePitch);
	SDL_SetRenderTarget(renderer.GetSDLRenderer(), NULL);
	

	int initialX = 0;
	int initialY = 0;


	// check for an alpha channel first as
	// that is the pixel channel to scan
	if (SDL_ISPIXELFORMAT_ALPHA(textureFormat)) {
		int increment = SDL_BYTESPERPIXEL(textureFormat);
		// TODO: use these values to set the BIT jump internal
		// and starting bit
		// EG: index += bytesPerChannel * numChannels;
		// or index += 8(bpc) * 4(c) == 32; so jump ahead 32 to get the next alpha chunck
		// NOTE: some may be ARGB1555 meaning the jump is 16 bits (two 8-bit jumps to the next alpha channel)
//		int pixelLayout = SDL_PIXELLAYOUT(textureFormat);
//		int pixelType = SDL_PIXELTYPE(textureFormat);
//		int pixelOrder = SDL_PIXELORDER(textureFormat);

		// TODO: run a debug loop and set the x, y of the first rect at the first non-zero pixel
		// note texturePixels[i] returns an 8 bit integer (not a 1 bit or 5 bit)
		// the starting i is currently assumed to be an alpha channel, but use the SDL_PIXEL* functions
		// to actually set the starting i, AND use them to set how many BITS to be looking at (1, 5, 6, 8, etc)
		// instead of texturePixels default 8
		int i;
		for (i = 0; texturePixels[i] == 0; i += increment);

		// figure out which pixel of the image i is looking at (in rows and columns)
		// and given that the image is planted at 0,0: row = x, column = y
		initialX = (i / increment) % textureWidth;
		initialY = (i / increment) / textureWidth;
	}

	// set a focusFrame w and h, then loop and push_back
	int frameWidth = 60;
	int frameHeight = 111;
	int xPadding = 30;		// DEBUG: no effect on the 0-th row
	int yPadding = 50;		// DEBUG: no effect on the 0-th column

	int targetColumns = textureWidth / frameWidth;
	int targetRows = textureHeight / frameHeight;
	int column = 0;
	int row = 0;
	while (column < targetColumns) {
		int x = initialX + column * frameWidth + xPadding * column;
		int y = initialY + row * frameHeight + yPadding * row;
		int w = frameWidth;
		int h = frameHeight;

		outputRects.push_back( SDL_Rect{ x, y, w, h} );

		row++;
		if (row >= targetRows) {
			row = 0;
			column++;
		}
	}

	delete[] texturePixels;		// delete the copy of the texture pixels made
}