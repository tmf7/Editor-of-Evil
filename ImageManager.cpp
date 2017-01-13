#include <functional>		// std::hash
//#include "ImageManager.h"
#include "Editor.h"

//***************
// eImageManager::Init
//***************
bool eImageManager::Init() {
	// prepare the hashindex
	imageFilenameHash.ClearAndResize(MAX_IMAGES);

	// initialize error_image, a bright red texture the size of the current render target
	int width = editor.GetRenderer().ViewArea().w;
	int height = editor.GetRenderer().ViewArea().h;
	SDL_Texture * error_texture = SDL_CreateTexture(editor.GetRenderer().GetSDLRenderer(),
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		width,
		height);

	if (!error_texture)
		return false;

	SDL_PixelFormat * pixelFormat = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
	Uint32 redPixel = SDL_MapRGBA(pixelFormat, 255, 0, 0, 255);
	SDL_FreeFormat(pixelFormat);

	// lock the pixels for writing
	void * pixels = nullptr;
	int pitch = 0;
	if (SDL_LockTexture(error_texture, NULL, &pixels, &pitch) == -1) {
		SDL_DestroyTexture(error_texture);
		return false;
	}

	// write the red pixels on the blank texture
	int lastPixel = pitch * height / SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_ARGB8888);
	for (int i = 0; i < lastPixel; i++)
		((Uint32*)pixels)[i] = redPixel;
	SDL_UnlockTexture(error_texture);

	// register the error_image as the first elements of imageList
	auto hasher = std::hash<std::string>{};
	imageFilenameHash.Add(hasher("error_image"), imageList.size());
	imageList.push_back(std::make_shared<eImage>(error_texture, "invalid_file", imageList.size()));	// error image
	globalIDPool++;
	return true;
}

//***************
// eImageManager::BatchLoad
// loads a batch of image resources
// user can optionally call imageManager.Free()
// prior to this to facilitate starting with a fresh set of images
// TODO: allow selective unloading of images (eg: std::shared_ptr already does reference counting
// take those numbers and add/subtract according to the next level's filename batch)
//***************
bool eImageManager::BatchLoad(const char * imageBatchLoadFile) {
	std::shared_ptr<eImage> result;	// DEBUG: not acually used, but necessary for LoadImage
	char filename[MAX_ESTRING_LENGTH];
	std::ifstream	read(imageBatchLoadFile);

	// unable to find/open file
	if(!read.good())
		return false;

	read.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip the first line of the file
	while (!read.eof()) {
		read >> filename;

		// unrecoverable read error or improperly formatted file
		if (read.bad() || read.fail()) {
			read.clear();
			read.close();
			return false;
		}
		LoadImage(filename, SDL_TEXTUREACCESS_STATIC, result);
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip the rest of the line
	}
	read.close();
	return true;
}

//***************
// eImageManager::GetImage
// fill the result with an eImage * if it exists
// if filename is null or the eImage doesn't exist
// then it result fills with an error eImage * (solid red)
//***************
bool eImageManager::GetImage(const char * filename, std::shared_ptr<eImage> & result) {
	if (!filename) {
		result = imageList[0]; // error image
		return false;
	}

	// search for pre-existing texture
	auto hasher = std::hash<std::string>{};
	int hashkey = hasher(filename);
	for (int i = imageFilenameHash.First(hashkey); i != -1; i = imageFilenameHash.Next(i)) {
		if (imageList[i]->GetFilename() == filename) {
			result = imageList[i];
			return true;
		}
	}
	result = imageList[0]; // error image
	return false;
}

//***************
// eImageManager::GetImage
// fill the result with an eImage * if it exists
// if guid is negative or beyond the number of loaded images
// then it result fills with an error eImage * (solid red)
//***************
bool eImageManager::GetImage(int imageID, std::shared_ptr<eImage> & result) {
	if (imageID < 0 && imageID > imageList.size()) {		// DEBUG: imageID will never be larger than max signed int
		result = imageList[0]; // error image
		return false;
	}
	result = imageList[imageID];
	return true;
}


//***************
// eImageManager::LoadImage
// attempts to load the given image file
// accessType can be
// SDL_TEXTUREACCESS_STATIC,    /**< Changes rarely, not lockable */
// SDL_TEXTUREACCESS_STREAMING, /**< Changes frequently, lockable */
// SDL_TEXTUREACCESS_TARGET     /**< Texture can be used as a render target */
// sets result to found texture on success
// sets result to error texture
// and returns false on failure
//***************
bool eImageManager::LoadImage(const char * filename, int accessType, std::shared_ptr<eImage> & result) {
	// check if the image already exists 
	// and set result to that if it does
	// DEBUG: ignores accessType
	if (GetImage(filename, result))
		return true;

	if (accessType < SDL_TEXTUREACCESS_STATIC ||
		accessType > SDL_TEXTUREACCESS_TARGET) {
		accessType = SDL_TEXTUREACCESS_STATIC;
	}
		
	SDL_Texture * texture = NULL;
	if (accessType != SDL_TEXTUREACCESS_STATIC) {
		SDL_Surface * source = IMG_Load(filename);

		// unable to load file
		if (source == NULL) {
			result = imageList[0]; // error image
			return false;
		}

		texture = SDL_CreateTexture(editor.GetRenderer().GetSDLRenderer(), 
												  source->format->format,
												  accessType, 
												  source->w, 
												  source->h);
		// unable to initialize texture
		if (texture == NULL) {
			result = imageList[0]; // error image
			SDL_FreeSurface(source);
			return false;
		}
	
		// attempt to copy data to the new texture
		if (SDL_UpdateTexture(texture, NULL, source->pixels, source->pitch)) {
			SDL_DestroyTexture(texture);
			SDL_FreeSurface(source);
			result = imageList[0]; // error image
			return false;
		}
		SDL_FreeSurface(source);
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	} else {
		texture = IMG_LoadTexture(editor.GetRenderer().GetSDLRenderer(), filename);

		// unable to initialize texture
		if (texture == NULL) {
			result = imageList[0]; // error image
			return false;
		}
	}

	// register the requested image
	auto hasher = std::hash<std::string>{};
	imageFilenameHash.Add(hasher(filename), imageList.size());
	result = std::make_shared<eImage>(texture, filename, imageList.size());
	imageList.push_back(result);
	globalIDPool++;
	return true;
}

//***************
// eImageManager::LoadConstantText
// cache unchanging text images
//***************
bool eImageManager::LoadConstantText(TTF_Font * font, const char * text, Uint8 r, Uint8 g, Uint8 b, Uint8 a, std::shared_ptr<eImage> & result) {
	// check if the image already exists 
	// and set result to that if it does
	if (GetImage(text, result))
		return true;

	SDL_Surface * surfaceText = TTF_RenderText_Solid(font, text, SDL_Color{ r, g, b, a });
	if (surfaceText == NULL) {
		result = imageList[0]; // error image
		return false;
	}

	SDL_Texture * renderedText = SDL_CreateTextureFromSurface(editor.GetRenderer().GetSDLRenderer(), surfaceText);
	SDL_FreeSurface(surfaceText);
	if (renderedText == NULL) {
		result = imageList[0]; // error image
		return false;
	}

	SDL_SetTextureBlendMode(renderedText, SDL_BLENDMODE_BLEND);

	// register the requested text image
	auto hasher = std::hash<std::string>{};
	imageFilenameHash.Add(hasher(text), imageList.size());
	result = std::make_shared<eImage>(renderedText, text, imageList.size());
	imageList.push_back(result);
	globalIDPool++;
	return true;
}

//***************
// eImageManager::Clear
// clears all pointers to the current set 
// of resource images, which allows them
// to be deleted once no object is using them,
// allows for new resource images to load
// without using excessive memory
//***************
void eImageManager::Clear() {
	imageList.clear();
	imageFilenameHash.Clear();
}