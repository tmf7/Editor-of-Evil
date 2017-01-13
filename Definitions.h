#ifndef EVIL_DEFINITIONS_H
#define EVIL_DEFINITIONS_H

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"
#include <cstring>			// memset
#include <memory>			// std::unique_ptr, std::shared_ptr, std::weak_ptr
#include <ios>
#include <fstream>
#include <random>
#include <limits>
//#include <stdio.h>
#include "Math.h"

#define BIT(a) (1<<a)

#define INVALID_ID -1
#define MAX_ESTRING_LENGTH 128
#define MAX_IMAGES 1024
#define MAX_ROWS 128
#define MAX_COLUMNS 128
#define MAX_LAYER 128

extern Uint32 globalIDPool;	// globally unique identifier: each resource gets, regardless of copying, moving, or type

#endif /* EVIL_DEFINITIONS_H */
