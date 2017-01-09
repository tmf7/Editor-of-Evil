#ifndef EVIL_CAMERA_H
#define EVIL_CAMERA_H

#include "Bounds.h"

class eCamera {
public:
					
						eCamera();

	void				Think();
	void				Init();

	void				SetOrigin(const eVec2 & point);		// TODO: make this part of a physics class
	const eVec2 &		Origin() const;						// TODO: make this part of a physics class
	void				UpdateOrigin();						// TODO: make this part of a physics class

	int					Width() const;
	int					Height() const;

	float				GetZoom() const;
	void				SetZoom(const float level);
	const eVec2 &		GetAbsBounds(bool minMax = false) const;

	static constexpr const float zoomIncrement = 0.1f;
	static constexpr const float maxZoom = 2.0f;
	static constexpr const float minZoom = 0.1f;

private:

	eBounds				localBounds;						// using local coordinates
	eBounds				absBounds;							// using map coordinates
	eVec2				origin;
	eVec2				oldOrigin;							// for use with collision response
	eVec2				velocity;
	float				speed;
	float				zoomLevel;
};	

//***************
// eCamera::eCamera
//***************
inline eCamera::eCamera() 
	: speed(10.0f), zoomLevel(1) {
}

//*************
// eCamera::UpdateOrigin
//*************
inline void eCamera::UpdateOrigin() {
	oldOrigin = origin;
	origin += velocity;
	absBounds = localBounds + origin;
}

//*************
// eCamera::SetOrigin
//*************
inline void eCamera::SetOrigin(const eVec2 & point) {
	oldOrigin = point;
	origin = point;
	absBounds = localBounds + origin;
}

//*************
// eCamera::Origin
//*************
inline const eVec2 & eCamera::Origin() const {
	return origin;
}

//***************
// eCamera::Width
//***************
inline int eCamera::Width() const {
	return (int)localBounds.Width();
}

//***************
// eCamera::Height
//***************
inline int eCamera::Height() const {
	return (int)localBounds.Height();
}

//***************
// eCamera::GetZoom
//***************
inline float eCamera::GetZoom() const {
	return zoomLevel;
}

//***************
// eCamera::GetAbsBounds
// minMax == false returns absBounds mins, true returns maxs
//***************
inline const eVec2 & eCamera::GetAbsBounds(bool minMax) const {
	return minMax ? absBounds[1] : absBounds[0];
}

#endif /* EVIL_CAMERA_H */

