#include "Renderer.h"
#include "Editor.h"

//***************
// eRenderer::Init
// initialize the window, its backbuffer surface, and a default font
//***************
bool eRenderer::Init() {
	
	window = SDL_CreateWindow("Evil", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

	if (!window)
		return false;

	internal_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (!internal_renderer)
		return false;

	// enable linear anti-aliasing for the renderer context
	int success = SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, "linear" , SDL_HINT_OVERRIDE);

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

	if (!font)
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
// DEBUG: converts the input point float data to integer values
//***************
void eRenderer::DrawOutlineText(char * string, const eVec2 & point, Uint8 r, Uint8 g, Uint8 b, Uint8 a) const {
	SDL_Surface * surfaceText = TTF_RenderText_Solid(font, string, SDL_Color{ r,g,b, a });
	if (surfaceText == NULL)
		return;

	SDL_Texture * renderedText = SDL_CreateTextureFromSurface(internal_renderer, surfaceText);
	if (renderedText == NULL)
		return;

	SDL_Rect destRect{ (int)point.x , (int)point.y };
	SDL_RenderCopy(internal_renderer, renderedText, NULL, &destRect);
	SDL_FreeSurface(surfaceText);
}

//***************
// eRenderer::DrawDebugRect
//***************
void eRenderer::DrawDebugRect(Uint8 * RGBAcolor, const SDL_Rect & rect, bool fill) const {
	Uint8 oldColor[4];
	SDL_GetRenderDrawColor(internal_renderer, &oldColor[0], &oldColor[1], &oldColor[2], &oldColor[3]);
	SDL_SetRenderDrawColor(internal_renderer, RGBAcolor[0], RGBAcolor[1], RGBAcolor[2], RGBAcolor[3]);

	// TODO: scale the screenRect according to camera zoom as well

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
//***************
void eRenderer::DrawDebugRects(Uint8 * RGBAcolor, const std::vector<SDL_Rect> & rects, bool fill) const {
	Uint8 oldColor[4];
	SDL_GetRenderDrawColor(internal_renderer, &oldColor[0], &oldColor[1], &oldColor[2], &oldColor[3]);
	SDL_SetRenderDrawColor(internal_renderer, RGBAcolor[0], RGBAcolor[1], RGBAcolor[2], RGBAcolor[3]);

	// TODO: scale the screenRect according to camera zoom as well

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
// draws a source image's current frame to the screen
// DEBUG: converts the input point float data to integer values
//***************
void eRenderer::DrawImage(eImage * image, const eVec2 & point) const {
	SDL_Rect destRect;

	if (image == NULL || image->Source() == NULL)
		return;

	// adjust the position and zoom of the target according to camera properties
	destRect.x = eMath::NearestInt(point.x - editor.GetCamera().Origin().x);
	destRect.y = eMath::NearestInt(point.y - editor.GetCamera().Origin().y);
	destRect.w = editor.GetCamera().Width();
	destRect.h = editor.GetCamera().Height();

	SDL_RenderCopy(internal_renderer, image->Source(), &image->Frame(), &destRect);
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

