#ifndef SOUND_WRAPPER_H
#define SOUND_WRAPPER_H

#include <Windows.h>

class SoundWrapper {
public:
	SoundWrapper(PCWSTR path);
	~SoundWrapper();
};

#endif // SOUND_WRAPPER_H