#include "Renderer.h"
#include "Editor.h"

//***************
// eRenderer::Init
// initialize the window, its rendering context, and a default font
//***************
bool eRenderer::Init() {
	window = SDL_CreateWindow("Evil", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

	if (!window)
		return false;

	// DEBUG: TARGETTEXTURE is used to read pixel data from SDL_Textures
	internal_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

	if (!internal_renderer)
		return false;

	// enable linear anti-aliasing for the renderer context
//	SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, "linear" , SDL_HINT_OVERRIDE);

	if (SDL_SetRenderDrawColor(internal_renderer, 128, 128, 128, 255) == -1)		// opaque grey
		return false;

	// initialize scalableTarget for scaling textures using the camera
	// independent of any static overlay (gui/HUD)
	SDL_Rect viewArea;
	SDL_RenderGetViewport(internal_renderer, &viewArea);
	scalableTarget = SDL_CreateTexture(internal_renderer,
									   SDL_PIXELFORMAT_ARGB8888,		// DEBUG: this format may not work for all images
									   SDL_TEXTUREACCESS_TARGET,
									   viewArea.w,
									   viewArea.h);

	if (!scalableTarget)
		return false;

	// ensure the scalableTarget can alhpa-blend
	SDL_SetTextureBlendMode(scalableTarget, SDL_BLENDMODE_BLEND);

	if (TTF_Init() == -1)
		return false;

	font = TTF_OpenFont("fonts/Alfphabet.ttf", 24);

	if (!font)
		return false;

	return true;
}

//***************
// eRenderer::Free
// close the font and destroy the window
//***************
void eRenderer::Free() const {
	if (font)
		TTF_CloseFont(font);

	TTF_Quit();

	if (internal_renderer)
		SDL_DestroyRenderer(internal_renderer);

	if (window)
		SDL_DestroyWindow(window);
}

//***************
// eRenderer::DrawOutlineText
// Draws the given string on the screen using location and color
// if isStatic is true then the text is considered unchanging and the
// text's image gets cached for later use
// DEBUG: converts the input point float data to integer values
// TODO: potentially scale, rotate, and translate the text for things other than HUD/Toolbox text
// TODO: does not currently accomodate camera translation
//***************
void eRenderer::DrawOutlineText(const char * text, const eVec2 & point, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool constText, bool dynamic) const {
	SDL_Texture * renderedText = NULL;
	if (constText) {
		std::shared_ptr<eImage> result;
		// check if the image already exists, if not then load it and set result
		editor.GetImageManager().LoadConstantText(font, text, r, g, b, a, result);
		renderedText = result->Source();
	} else {
		SDL_Surface * surfaceText = TTF_RenderText_Solid(font, text, SDL_Color{ r, g, b, a });
		if (surfaceText == NULL)
			return;

		renderedText = SDL_CreateTextureFromSurface(internal_renderer, surfaceText);
		SDL_FreeSurface(surfaceText);
		if (renderedText == NULL)
			return;

		SDL_SetTextureBlendMode(renderedText, SDL_BLENDMODE_BLEND);
	}

	SDL_Rect destRect { (int)point.x , (int)point.y };
	SDL_QueryTexture(renderedText, NULL, NULL, &destRect.w, &destRect.h);
	SDL_SetRenderTarget(internal_renderer, dynamic ? scalableTarget : NULL);
	SDL_RenderCopy(internal_renderer, renderedText, NULL, &destRect);
	SDL_SetRenderTarget(internal_renderer, NULL);
}

//***************
// eRenderer::DrawDebugRect
// dynamic == true draws to the scalableTarget
// dynamic == false draws to the default render target
// DEBUG: immediatly draws to the given render target
//***************
void eRenderer::DrawDebugRect(Uint8 * RGBAcolor, const SDL_Rect & rect, bool fill, bool dynamic) const {
	Uint8 oldColor[4];
	SDL_GetRenderDrawColor(internal_renderer, &oldColor[0], &oldColor[1], &oldColor[2], &oldColor[3]);
	SDL_SetRenderDrawColor(internal_renderer, RGBAcolor[0], RGBAcolor[1], RGBAcolor[2], RGBAcolor[3]);

	SDL_Rect screenRect{ eMath::NearestInt(rect.x - editor.GetCamera().Origin().x),
						 eMath::NearestInt(rect.y - editor.GetCamera().Origin().y),
						 rect.w, 
						 rect.h };
	SDL_SetRenderTarget(internal_renderer, dynamic ? scalableTarget : NULL);
	fill ? SDL_RenderFillRect(internal_renderer, &screenRect) 
		 : SDL_RenderDrawRect(internal_renderer, &screenRect);
	SDL_SetRenderTarget(internal_renderer, NULL);
	SDL_SetRenderDrawColor(internal_renderer, oldColor[0], oldColor[1], oldColor[2], oldColor[3]);
}

//***************
// eRenderer::DrawDebugRects
// dynamic == true draws to the scalableTarget
// dynamic == false draws to the default render target
// DEBUG: immediatly draws to the given render target
//***************
void eRenderer::DrawDebugRects(Uint8 * RGBAcolor, const std::vector<SDL_Rect> & rects, bool fill, bool dynamic) const {
	Uint8 oldColor[4];
	SDL_GetRenderDrawColor(internal_renderer, &oldColor[0], &oldColor[1], &oldColor[2], &oldColor[3]);
	SDL_SetRenderDrawColor(internal_renderer, RGBAcolor[0], RGBAcolor[1], RGBAcolor[2], RGBAcolor[3]);

	int(*drawFunc)(SDL_Renderer *, const SDL_Rect *);
	drawFunc = fill ? SDL_RenderFillRect : SDL_RenderDrawRect;

	SDL_SetRenderTarget(internal_renderer, dynamic ? scalableTarget : NULL);
	for (auto && iter : rects) {
		SDL_Rect screenRect{ eMath::NearestInt(iter.x - editor.GetCamera().Origin().x),
							 eMath::NearestInt(iter.y - editor.GetCamera().Origin().y),
							 iter.w,
							 iter.h };
		drawFunc(internal_renderer, &screenRect);
	}
	SDL_SetRenderTarget(internal_renderer, NULL);
	SDL_SetRenderDrawColor(internal_renderer, oldColor[0], oldColor[1], oldColor[2], oldColor[3]);
}

//***************
// eRenderer::DrawImage
// if frame is nullptr the entire image is drawn
// otherwise frame sets a sub-section of the image to draw
// DEBUG: converts the input point float data to integer values
//***************
void eRenderer::DrawImage(std::shared_ptr<eImage> image, const SDL_Rect * srcRect, const SDL_Rect * dstRect) const {
	SDL_RenderCopy(internal_renderer, image->Source(), srcRect, dstRect);
}

//***************
// eRenderer::OnScreen
//***************
bool eRenderer::OnScreen(const eVec2 & point) const {
	SDL_Rect viewArea;
	SDL_RenderGetViewport(internal_renderer, &viewArea);
	eBounds screenBounds = eBounds(eVec2((float)viewArea.x, (float)viewArea.y), 
									eVec2((float)(viewArea.x + viewArea.w), (float)(viewArea.y + viewArea.h)));
	return screenBounds.ContainsPoint(point);
}

//***************
// eRenderer::OnScreen
//***************
bool eRenderer::OnScreen(const eBounds & bounds) const {
	SDL_Rect viewArea;
	SDL_RenderGetViewport(internal_renderer, &viewArea);
	eBounds screenBounds = eBounds(eVec2((float)viewArea.x, (float)viewArea.y),
								   eVec2((float)(viewArea.x + viewArea.w), (float)(viewArea.y + viewArea.h)));
	return screenBounds.Overlaps(bounds);
}

//***************
// eRenderer::AddToRenderPool
// dynamic == false is used for overlays and/or HUD guis
// dynamic == true is used for scaling and translating groups of images together based on camera properties
// DEBUG: most-significant 2 bytes of priority were set using the layer during construction,
// the the least-significant 2 bytes are now set according to the order the renderImage was added to the renderPool
//***************
void eRenderer::AddToRenderPool(renderImage_t & renderImage, bool dynamic) {
	std::vector<renderImage_t> * targetPool = dynamic ? &dynamicPool : &staticPool;
	renderImage.priority |= targetPool->size();
	targetPool->push_back(renderImage);
}

//***************
// eRenderer::Flush
// FIXME/BUG(!): ensure entities never occupy the same layer/depth as world tiles 
// (otherwise the unstable quicksort will put them at RANDOM draw orders relative to the same layer/depth tiles)
//***************
void eRenderer::Flush() {
	// sort the dynamicPool for the scalableTarget
	QuickSort(dynamicPool.data(), 
			  dynamicPool.size(), 
			  [](auto && a, auto && b) { 
					if (a.priority < b.priority) return -1; 
					else if (a.priority > b.priority) return 1;
					return 0; 
				}
	);

	// set the render target, and scale according to camera zoom
	SDL_SetRenderTarget(internal_renderer, scalableTarget);
	SDL_RenderSetScale(internal_renderer, editor.GetCamera().GetZoom(), editor.GetCamera().GetZoom());
	
	// draw to the scalableTarget
	for (auto && iter : dynamicPool)
		DrawImage(iter.image, iter.srcRect, &iter.dstRect);

	dynamicPool.clear();

	// reset the render target to default, 
	// reset the renderer scale,
	// and transfer the scalableTarget
	SDL_SetRenderTarget(internal_renderer, NULL);
	SDL_RenderSetScale(internal_renderer, 1.0f, 1.0f);
	SDL_RenderCopy(internal_renderer, scalableTarget, NULL, NULL);

	// sort the staticPool for the default render target
	QuickSort(staticPool.data(),
			  staticPool.size(), 
			  [](auto && a, auto && b) { 
					if (a.priority < b.priority) return -1; 
					else if (a.priority > b.priority) return 1;
					return 0; 
				}
	);
	for (auto && iter : staticPool)
		DrawImage(iter.image, iter.srcRect, &iter.dstRect);

	staticPool.clear();
}

/*
//***************
// eRenderer::DrawPixel
// Draws a single pixel to the backbuffer
// after confirming the point is on the backbuffer
// DEBUG: converts the input point float data to integer values
// TODO: dont use this anymore
//***************
void eRenderer::DrawPixel(const eVec2 & point, Uint8 r, Uint8 g, Uint8 b) const {
	Uint8 * targetPixel;
	Uint32 color;
	int x, y;

//	static Uint8 * backbufferStart = (Uint8 *)backbuffer->pixels;
//	static Uint8 * backbufferEnd = (Uint8 *)backbuffer->pixels + (backbuffer->h - 1) * backbuffer->pitch + (backbuffer->w - 1) * backbuffer->format->BytesPerPixel;

	if (!OnScreen(point))
		return;

	if (SDL_MUSTLOCK(backbuffer)) {
		if (SDL_LockSurface(backbuffer) < 0)
			return;
	}

	x = (int)point.x;
	y = (int)point.y;

	color = SDL_MapRGB(backbuffer->format, r, g, b);
	targetPixel = (Uint8 *)backbuffer->pixels + y * backbuffer->pitch + x * backbuffer->format->BytesPerPixel;
	*(Uint32 *)targetPixel = color;		// DEBUG: assumes dereferenced targetPixel is a Uint32

	if (SDL_MUSTLOCK(backbuffer))
		SDL_UnlockSurface(backbuffer);
}
*/
/*
//***************
// eRenderer::FormatSurface
// converts the given surface to the backbuffer format
// for color keying
// returns false on failure
//***************
bool eRenderer::FormatSurface(SDL_Surface ** surface, int * colorKey) const {
	SDL_Surface * formattedSurface;
	Uint32 colorMap;

	formattedSurface = SDL_ConvertSurface(*surface, backbuffer->format, SDL_RLEACCEL);
	if (formattedSurface == NULL)
		return false;
	
	SDL_FreeSurface(*surface);		// DEBUG: free the memory allocated for the original surface
	*surface = formattedSurface;		// point to the newly formatted surface memory block

	if (colorKey != nullptr) {
		colorMap = SDL_MapRGB((*surface)->format, colorKey[0], colorKey[1], colorKey[2]);
		SDL_SetColorKey(*surface, SDL_TRUE, colorMap);
	}
	return true;
}
*/

