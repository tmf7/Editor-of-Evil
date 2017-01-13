#ifndef EVIL_IMAGE_H
#define EVIL_IMAGE_H

#include "Definitions.h"

//***************************************
//				eImage
// stores access pointer to SDL_Texture 
// and is handled by eImageManager
//***************************************
class eImage {
public:
							eImage();
							eImage(SDL_Texture * source, const char * filename, int id);
							~eImage();

	bool					IsValid() const;
	SDL_Texture *			Source() const;
	int						GetWidth() const;
	int						GetHeight() const;
	std::string				GetFilename() const;
	unsigned int			GetID() const;

private:

	SDL_Texture *			source;
	SDL_Point				size;
	std::string				filename;		// source file loaded from
	int						id;				// index within eImageManager::images
};

//**************
// eImage::eImage
//**************
inline eImage::eImage()
	: source(nullptr),
	  filename("invalid_file"), 
	  id(INVALID_ID) {
	size = SDL_Point{ 0, 0 };
}

//**************
// eImage::eImage
// frame is the size of the texture
//**************
inline eImage::eImage(SDL_Texture * source, const char * filename, int id)
	: source(source), 
	  filename(filename), 
	  id(id) {
	SDL_QueryTexture(source, NULL, NULL, &size.x, &size.y);
}

//**************
// eImage::~eImage
//**************
inline eImage::~eImage() {
	SDL_DestroyTexture(source);
}

//**************
// eImage::IsValid
// returns true if source != NULL
//**************
inline bool eImage::IsValid() const {
	return source != nullptr;
}

//**************
// eImage::Source
//**************
inline SDL_Texture * eImage::Source() const {
	return source;
}

//**************
// eImage::GetWidth
//**************
inline int eImage::GetWidth() const {
	return size.x;
}

//**************
// eImage::GetHeight
//**************
inline int eImage::GetHeight() const {
	return size.y;
}

//**************
// eImage::GetFilename
//**************
inline std::string eImage::GetFilename() const {
	return filename;
}

//**************
// eImage::GetID
//**************
inline unsigned int eImage::GetID() const {
	return id;
}

#endif /* EVIL_IMAGE_H */

