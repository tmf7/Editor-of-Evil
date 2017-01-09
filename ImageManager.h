#ifndef EVIL_TEXTURE_MANAGER_H
#define EVIL_TEXTURE_MANAGER_H

#include "Definitions.h"
#include "Image.h"
#include "HashIndex.h"
#include "Renderer.h"

//**********************************
//			eImageManager
// Handles all texture allocation and freeing
// DEBUG: --no other object/system should allocate/free textures--
//**********************************
class eImageManager {
public:

	bool			Init();
	bool			BatchLoad(const char * imageBatchLoadFile);
	bool			GetImage(const char * filename, std::shared_ptr<eImage> & result);
	bool			GetImage(int imageID, std::shared_ptr<eImage> & result);
	bool			LoadImage(const char * filename, std::shared_ptr<eImage> & result);
	bool			LoadConstantText(TTF_Font * font, const char * text, Uint8 r, Uint8 g, Uint8 b, Uint8 a, std::shared_ptr<eImage> & result);
	int				GetNumImages() const;
	void			Free();

private:

	std::vector<std::shared_ptr<eImage>>		images;	// list of pointers to dynamically allocated image resources

	eHashIndex		imageFilenameHash;
	int				numImages;
};

//***************
// eImageManager::GetNumImages
//***************
inline int eImageManager::GetNumImages() const {
	return numImages;
}

#endif /* EVIL_TEXTURE_MANAGER_H */

