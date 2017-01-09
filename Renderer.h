#ifndef EVIL_RENDERER_H
#define EVIL_RENDERER_H

#include <vector>

#include "Definitions.h"
#include "Vector.h"
#include "Bounds.h"
#include "Sort.h"
#include "Image.h"

typedef struct renderImage_s {
	eVec2 *						position;	// where on the screen
	std::shared_ptr<eImage>		image;		// for entities, this would be which frame of the eSprite is being drawn
	SDL_Rect *					frame;		// what part of the source image to draw
	Uint32						priority;

	renderImage_s()
		: position(nullptr), 
		  image(nullptr),
		  frame(nullptr),
		  priority(MAX_LAYER << 16) {};
	
	renderImage_s(eVec2 & position, std::shared_ptr<eImage> image, SDL_Rect * frame, const Uint8 layer)
		: position(&position), 
		  image(image), 
		  frame(frame),
		  priority(layer << 16) {};	// DEBUG: most-significant 2 bytes are layer
									// least-significant 2 bytes are renderPool push order
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

	void				AddToRenderPool(renderImage_t & renderImage);
	void				FlushRenderPool();

	void				DrawOutlineText(const char * text, const eVec2 & point, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool isStatic) const;
	void				DrawImage(const eVec2 & point, std::shared_ptr<eImage> image, const SDL_Rect * frame) const;
	void				DrawDebugRect(Uint8 * RGBAcolor, const SDL_Rect & rect, bool fill) const;
	void				DrawDebugRects(Uint8 * RGBAcolor, const std::vector<SDL_Rect> & rects, bool fill) const;

//	bool				FormatSurface(SDL_Surface ** surface, int * colorKey) const;	// DEBUG: deprecated
//	void				DrawPixel(const eVec2 & point, Uint8 r, Uint8 g, Uint8 b) const;	// DEBUG: deprecated

	bool				OnScreen(const eVec2 & point) const;
	bool				OnScreen(const eBounds & bounds) const;

private:

	static const int				defaultRenderCapacity = 1024;
	std::vector<renderImage_t>		renderPool;

	SDL_Window *		window;
	SDL_Renderer *		internal_renderer;
	TTF_Font *			font;
};

//***************
// eRenderer::eRenderer
//***************
inline eRenderer::eRenderer() {
	renderPool.reserve(defaultRenderCapacity);
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
// DEBUG: most-significant 2 bytes of priority were set using the layer during construction,
// the the least-significant 2 bytes are now set according to the order the renderImage was added to the renderPool
//***************
inline void eRenderer::AddToRenderPool(renderImage_t & renderImage) {
	renderImage.priority |= renderPool.size();
	renderPool.push_back(renderImage);
}

//***************
// eRenderer::FlushRenderQueue
// FIXME/BUG(!): ensure entities never occupy the same layer/depth as world tiles 
// (otherwise the unstable quicksort will put them at RANDOM draw orders relative to the same layer/depth tiles)
//***************
inline void eRenderer::FlushRenderPool() {
	QuickSort(renderPool.data(), 
			  renderPool.size(), 
			  [](auto && a, auto && b) { 
					if (a.priority < b.priority) return -1; 
					else if (a.priority > b.priority) return 1;
					return 0; 
				}
	);
	for (auto && iter : renderPool)
		DrawImage(*(iter.position), iter.image, iter.frame);

	renderPool.clear();
}

#endif /* EVIL_RENDERER_H */
