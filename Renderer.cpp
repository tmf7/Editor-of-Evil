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
	SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, "linear" , SDL_HINT_OVERRIDE);

	if (SDL_SetRenderDrawColor(internal_renderer, 128, 128, 128, 255) == -1)		// opaque grey
		return false;

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
//***************
void eRenderer::DrawOutlineText(const char * text, const eVec2 & point, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool isStatic) const {
	SDL_Texture * renderedText = NULL;
	if (isStatic) {
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
	SDL_RenderCopy(internal_renderer, renderedText, NULL, &destRect);
}

//***************
// eRenderer::DrawDebugRect
// TODO: scale the screenRect according to camera zoom as well
//***************
void eRenderer::DrawDebugRect(Uint8 * RGBAcolor, const SDL_Rect & rect, bool fill) const {
	Uint8 oldColor[4];
	SDL_GetRenderDrawColor(internal_renderer, &oldColor[0], &oldColor[1], &oldColor[2], &oldColor[3]);
	SDL_SetRenderDrawColor(internal_renderer, RGBAcolor[0], RGBAcolor[1], RGBAcolor[2], RGBAcolor[3]);

	SDL_Rect screenRect{ eMath::NearestInt(rect.x - editor.GetCamera().Origin().x),
						 eMath::NearestInt(rect.y - editor.GetCamera().Origin().y),
						 rect.w, 
						 rect.h };
	fill ? SDL_RenderFillRect(internal_renderer, &screenRect) 
		 : SDL_RenderDrawRect(internal_renderer, &screenRect);
	SDL_SetRenderDrawColor(internal_renderer, oldColor[0], oldColor[1], oldColor[2], oldColor[3]);
}

//***************
// eRenderer::DrawDebugRects
// TODO: scale the screenRect according to camera zoom as well
//***************
void eRenderer::DrawDebugRects(Uint8 * RGBAcolor, const std::vector<SDL_Rect> & rects, bool fill) const {
	Uint8 oldColor[4];
	SDL_GetRenderDrawColor(internal_renderer, &oldColor[0], &oldColor[1], &oldColor[2], &oldColor[3]);
	SDL_SetRenderDrawColor(internal_renderer, RGBAcolor[0], RGBAcolor[1], RGBAcolor[2], RGBAcolor[3]);

	int(*drawFunc)(SDL_Renderer *, const SDL_Rect *);
	drawFunc = fill ? SDL_RenderFillRect : SDL_RenderDrawRect;

	for (auto && iter : rects) {
		SDL_Rect screenRect{ eMath::NearestInt(iter.x - editor.GetCamera().Origin().x),
							 eMath::NearestInt(iter.y - editor.GetCamera().Origin().y),
							 iter.w,
							 iter.h };
		drawFunc(internal_renderer, &screenRect);
	}

	SDL_SetRenderDrawColor(internal_renderer, oldColor[0], oldColor[1], oldColor[2], oldColor[3]);
}

//***************
// eRenderer::DrawImage
// if frame is nullptr the entire image is drawn
// otherwise frame sets a sub-section of the image to draw
// DEBUG: converts the input point float data to integer values
//***************
void eRenderer::DrawImage(const eVec2 & point, std::shared_ptr<eImage> image, const SDL_Rect * frame) const {
	if (image == nullptr || !image->IsValid())
		return;
	
	// adjust the position and zoom of the target according to camera properties
	// TODO: migrate this to EngineOfEvil and also incorperate isometric calculation (maybe)
	SDL_Rect destRect;
	destRect.x = eMath::NearestInt(point.x - editor.GetCamera().Origin().x);
	destRect.y = eMath::NearestInt(point.y - editor.GetCamera().Origin().y);
	destRect.w = editor.GetCamera().Width();
	destRect.h = editor.GetCamera().Height();

	SDL_RenderCopy(internal_renderer, image->Source(), frame, &destRect);
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

