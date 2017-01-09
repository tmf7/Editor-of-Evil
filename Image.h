#ifndef EVIL_IMAGE_H
#define EVIL_IMAGE_H

#include "Definitions.h"

//***************************************
//				eImage
// stores access pointer to SDL_Texture 
// that is handled by eImageManager
//***************************************
class eImage {
public:
							eImage();
							eImage(SDL_Texture * source, const char * filename, unsigned int id);

	bool					IsValid() const;
	SDL_Texture *			Source() const;
	int						GetWidth() const;
	int						GetHeight() const;
	std::string				GetFilename() const;
	unsigned int			GetID() const;
	void					Free();

private:

	SDL_Texture *			source;
	SDL_Point				size;
	std::string				filename;
	unsigned int			id;
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
inline eImage::eImage(SDL_Texture * source, const char * filename, unsigned int id)
	: source(source), 
	  filename(filename), 
	  id(id) {
	SDL_QueryTexture(source, NULL, NULL, &size.x, &size.y);
}

//**************
// eImage::IsValid
// returns true if source != NULL
//**************
inline bool eImage::IsValid() const {
	return source != nullptr;
}

//**************
// eImage::Free
//**************
inline void eImage::Free() {
	if (source)
		SDL_DestroyTexture(source);
	filename.clear();
	id = INVALID_ID;
	size = SDL_Point{ 0, 0 };
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

//***************************************
//				eImageTiler
// sub-sectioning of eImage data
// for use by animations and tilesets
// TODO: modify calls render.AddToRenderQueue(renderImage{point, image_ptr, frame_ptr, layer_byte});
// where frame_ptr is either nullptr for an entire eImage, or &imageTiler.GetFrame()
//***************************************
class eImageTiler {
public:

	void					Init(std::shared_ptr<eImage> image, std::vector<SDL_Rect> && subFrameList);
	std::shared_ptr<eImage>	Source() const;
	SDL_Rect				GetFrame();
	const SDL_Rect	&		GetFrame() const;
	void					SetFrame(const int frameNumber);
	int						GetNumFrames() const;

private:

	std::shared_ptr<eImage> source;	
	std::vector<SDL_Rect>	subFrames;			// subsections of image to focus on
	SDL_Rect *				focusFrame;			// currently used image subsection
};

//**************
// eImageTiler::Init
//**************
inline void eImageTiler::Init(std::shared_ptr<eImage> image, std::vector<SDL_Rect> && subFrameList) {
	source = image;
	subFrames = std::move(subFrameList);
	focusFrame = &subFrames[0];
}

//**************
// eImageTiler::Source
//**************
inline std::shared_ptr<eImage> eImageTiler::Source() const {
	return source;
}

//**************
// eImageTiler::GetFrame
//**************
inline SDL_Rect eImageTiler::GetFrame() {
	return *focusFrame;
}

//**************
// eImageTiler::GetFrame
//**************
inline const SDL_Rect & eImageTiler::GetFrame() const {
	return *focusFrame;
}

//**************
// eImageTiler::SetFrame
//**************
inline void eImageTiler::SetFrame(const int frameNumber) {
	focusFrame = &subFrames[frameNumber];
}
//**************
// eImageTiler::GetNumFrames
//**************
inline int eImageTiler::GetNumFrames() const {
	return subFrames.size();
}

#endif /* EVIL_IMAGE_H */

