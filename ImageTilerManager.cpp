#include "ImageTilerManager.h"
#include "Editor.h"

//*****************
// VerifyRead (global)
// tests for unrecoverable read error or improperly formatted file
//*****************
inline bool VerifyRead(std::ifstream & read) {
	if (read.bad() || read.fail()) {
		read.clear();
		read.close();
		return false;
	}
	return true;
}

//***************
// eImageTilerManager::LoadTiler
// sets result to an (TODO: existing or) 
// loaded eImageTiler pointer
//***************
bool eImageTilerManager::LoadTiler(const char * filename, std::shared_ptr<eImageTiler> & result) {
	if (!filename)
		return false;

	char buffer[MAX_ESTRING_LENGTH];
	eHashIndex sequenceHash;
	std::vector<eImageFrame> frameList;
	std::shared_ptr<eImage> source = nullptr;
	auto hasher = std::hash<std::string>{};

	std::ifstream	read(filename);
	// unable to find/open file
	if(!read.good())
		return false;

	// read the source image name
	memset(buffer, 0, sizeof(buffer));
	read.getline(buffer, sizeof(buffer), '\n');
	if(!VerifyRead(read))
		return false;

	// get a pointer to the source image
	if (!editor.GetImageManager().GetImage(buffer, source))
		return false;

	// read frameList and sequenceHash sizes
	int numFrames;
	int numSequences;
	for (int targetData = 0; targetData < 2; targetData++) {
		switch (targetData) {
			case 0: read >> numFrames; break;
			case 1:	read >> numSequences; break;
			default: break;
		}
		if (!VerifyRead(read))
			return false;
	}
	frameList.reserve(numFrames);					// minimize dynamic allocations
	sequenceHash.ClearAndResize(numSequences);		// esure each sequenceName has a unique hash

	while (!read.eof()) {
		// read a sequence name
		memset(buffer, 0, sizeof(buffer));
		read.getline(buffer, sizeof(buffer), '{');
		if (!VerifyRead(read))
			return false;

		// add the sequence name and index to the hash
		sequenceHash.Add(hasher(buffer), frameList.size());
		eImageFrame * firstFrame = frameList.data() + frameList.size();		// used to close sequence loops

		// read and link one frame at at time
		// until the sequence-closing delimeter
		eImageFrame * nextFrame = nullptr;
		while (read.peek() != '}' && nextFrame != firstFrame) {
			SDL_Rect frame;
			for (int targetData = 0; targetData < 4; targetData++) {
				switch (targetData) {
					case 0: read >> frame.x; break;
					case 1: read >> frame.y; break;
					case 2: read >> frame.w; break;
					case 3: read >> frame.h; break;
					default: break;
				}
		
				if (!VerifyRead(read))
					return false;
			}
			nextFrame = (read.peek() != '}' && frameList.size() + 1 < frameList.capacity())
						? frameList.data() + frameList.size() + 1
						: firstFrame;
			frameList.push_back(eImageFrame(frame, nextFrame));
		}
		read.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip the rest of the sequence line
	}
	read.close();

	// register the requested imageTiler
	tilerFilenameHash.Add(hasher(filename), tilerList.size());
	result = std::make_shared<eImageTiler>(source, std::move(frameList), std::move(sequenceHash), filename, tilerList.size());
	tilerList.push_back(result);
	globalIDPool++;
	return true;
}

//***************
// eImageTilerManager::Clear
// clears all pointers to the current set 
// of resource tilers, which allows them
// to be deleted once no object is using them,
// allows for new resource images to load
// without using excessive memory
//***************
void eImageTilerManager::Clear() {
	tilerList.clear();
	tilerFilenameHash.Clear();
}