#ifndef EVIL_OVERLAY_H
#define EVIL_OVERLAY_H

#include "Definitions.h"
#include "SpatialIndexGrid.h"

//****************************
//		eOverlay
// encapsulates user I/O via a series buttons (make a button object)
// also handles the spatial breakdown and drawing of
// sub-areas of the map to be merged/searched/marked-complete
// TODO(?): separate these tasks even more
//*****************************
class eOverlay {
public:



private:

	std::vector<SDL_Rect> outputRects;		// sub-image bounding frames within the target image
	eSpatialIndexGrid<SDL_Rect, MAX_ROWS, MAX_COLUMNS> targetGrid;	// TODO: adjust how many are active across the target

	
};

#endif /* EVIL_OVERLAY_H */