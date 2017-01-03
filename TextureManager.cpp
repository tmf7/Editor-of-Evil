#include <functional>		// std::hash
#include "TextureManager.h"
#include "Editor.h"

//***************
// eTextureManager::GetTexture
// returns a pointer to an SDL_Texture if it exists
// otherwise attempts to initialize an SDL_Texture from the name (filename)
// DEBUG: assert ( name != NULL )
//***************
SDL_Texture * eTextureManager::GetTexture(const char * name) {
	// search for pre-existing texture
	auto hasher = std::hash<const char *>{};
	int hashkey = hasher(name);
	for (int i = textureHash.First(hashkey); i != -1; i = textureHash.Next(i)) {
		if (textures[i].name == name) {
			return textures[i].texture;
		}
	}

	// no mathes found, initilize the image

// BEGIN FREEHILL different texture access test
	SDL_Surface * source = IMG_Load(name);

	// unable to load file
	if (source == NULL)
		return nullptr;

	SDL_Texture * texture = SDL_CreateTexture(editor.GetRenderer().GetSDLRenderer(), 
											  source->format->format,
											  SDL_TEXTUREACCESS_TARGET, 
											  source->w, 
											  source->h);
	// unable to initialize texture
	if (texture == NULL) {
		SDL_FreeSurface(source);
		return nullptr;
	}
	
	// attempt to copy data to the new texture
	if (SDL_UpdateTexture(texture, NULL, source->pixels, source->pitch) == -1) {
		SDL_DestroyTexture(texture);
		SDL_FreeSurface(source);
		return nullptr;
	}
	SDL_FreeSurface(source);
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
// END FREEHILL different texture access test

	// register the requested texture
	textureHash.Add(hashkey, numTextures);
	textures[numTextures] = eTexture_t{ texture, name };
	return textures[numTextures++].texture;
}

//***************
// eTextureManager::Free
// frees all resource image surfaces
//***************
void eTextureManager::Free() {
	for (int i = 0; i < numTextures; i++) {
		if (textures[i].texture)
			SDL_DestroyTexture(textures[i].texture);
	}
	numTextures = 0;
}