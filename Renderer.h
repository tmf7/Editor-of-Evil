#ifndef EVIL_RENDERER_H
#define EVIL_RENDERER_H

#include <vector>

#include "Definitions.h"
#include "Vector.h"
#include "Bounds.h"
#include "Sort.h"
#include "Image.h"

typedef struct renderImage_s {
	eVec2			position;	// where on the screen
	eImage *		image;		// for entities, this would be which frame of the eSprite is being drawn
	size_t			priority;

	renderImage_s()
		: position(vec2_zero), image(nullptr), priority(UINT_MAX) {};

	renderImage_s(const eVec2 & position, eImage * image, const int layer)
		: position(position), image(image), priority(layer << 16) {};
} renderImage_t;

//**************************************************
//				eRenderer
// Base class for all window/fullscreen drawing. 
// Contains the window, renderer, and font handles
//****************************************************
class eRenderer {
public:

						eRenderer();

	bool				Init();
	void				Free() const;
	void				Clear() const;
	void				Show() const;
	SDL_Rect			ViewArea() const;

	SDL_Renderer *		GetSDLRenderer() const;
	SDL_Window *		GetWindow() const;

	void				AddToRenderQueue(renderImage_t & renderImage);
	void				FlushRenderQueue();

	void				DrawOutlineText(char * string, const eVec2 & point, Uint8 r, Uint8 g, Uint8 b, Uint8 a) const;
	void				DrawImage(eImage * image, const eVec2 & point) const;
	void				DrawDebugRect(Uint8 * RGBAcolor, const SDL_Rect & rect, bool fill) const;
	void				DrawDebugRects(Uint8 * RGBAcolor, const std::vector<SDL_Rect> & rects, bool fill) const;

//	bool				FormatSurface(SDL_Surface ** surface, int * colorKey) const;	// DEBUG: deprecated
//	void				DrawPixel(const eVec2 & point, Uint8 r, Uint8 g, Uint8 b) const;	// DEBUG: deprecated

	bool				OnScreen(const eVec2 & point) const;
	bool				OnScreen(const eBounds & bounds) const;

private:

	static const int				defaultRenderCapacity = 1024;
	std::vector<renderImage_t>		renderQueue;

	SDL_Window *		window;
	SDL_Renderer *		internal_renderer;
	TTF_Font *			font;
};

//***************
// eRenderer::eRenderer
//***************
inline eRenderer::eRenderer() {
	renderQueue.reserve(defaultRenderCapacity);
}

//***************
// eRenderer::Clear
//***************
inline void eRenderer::Clear() const {
	SDL_RenderClear(internal_renderer);
}

//***************
// eRenderer::Show
// updates the visible screen area
//***************
inline void eRenderer::Show() const {
	SDL_RenderPresent(internal_renderer);
}

//***************
// eRenderer::ViewArea
//***************
inline SDL_Rect eRenderer::ViewArea() const {
	SDL_Rect viewArea;
	SDL_RenderGetViewport(internal_renderer, &viewArea);
	return viewArea;
}

//***************
// eRenderer::GetRenderer
//***************
inline SDL_Renderer * eRenderer::GetSDLRenderer() const {
	return internal_renderer;
}

//*****************
// eRenderer::GetWindow
//*****************
inline SDL_Window * eRenderer::GetWindow() const {
	return window;
}

//***************
// eRenderer::OnScreen
//***************
inline bool eRenderer::OnScreen(const eVec2 & point) const {
	SDL_Rect viewArea;
	SDL_RenderGetViewport(internal_renderer, &viewArea);
	eBounds screenBounds = eBounds(eVec2((float)viewArea.x, (float)viewArea.y), 
									eVec2((float)(viewArea.x + viewArea.w), (float)(viewArea.y + viewArea.h)));
	return screenBounds.ContainsPoint(point);
}

//***************
// eRenderer::OnScreen
//***************
inline bool eRenderer::OnScreen(const eBounds & bounds) const {
	SDL_Rect viewArea;
	SDL_RenderGetViewport(internal_renderer, &viewArea);
	eBounds screenBounds = eBounds(eVec2((float)viewArea.x, (float)viewArea.y),
								   eVec2((float)(viewArea.x + viewArea.w), (float)(viewArea.y + viewArea.h)));
	return screenBounds.Overlaps(bounds);
}

//***************
// eRenderer::AddToRenderQueue
// DEBUG: the second 2 bytes of priority were set using the layer during construction,
// the the first 2 bytes are now set according to the order the renderImage was added to the renderQueue
//***************
inline void eRenderer::AddToRenderQueue(renderImage_t & renderImage) {
	renderImage.priority |= renderQueue.size();
	renderQueue.push_back(renderImage);
}

//***************
// eRenderer::FlushRenderQueue
// FIXME/BUG(!): ensure entities never occupy the same layer/depth as world tiles 
// (otherwise the unstable quicksort will put them at RANDOM draw orders relative to the same layer/depth tiles)
//***************
inline void eRenderer::FlushRenderQueue() {
	QuickSort(renderQueue.data(), 
			  renderQueue.size(), 
			  [](auto && a, auto && b) { 
					if (a.priority < b.priority) return -1; 
					else if (a.priority > b.priority) return 1;
					return 0; 
				}
	);
	for (auto && iter : renderQueue)
		DrawImage(iter.image, iter.position);

	renderQueue.clear();
}
// END FREEHILL draw order sort test
#endif /* EVIL_RENDERER_H */
