#ifndef SOUND_WRAPPER_H
#define SOUND_WRAPPER_H

#include <Windows.h>

/*
 * Wrapper for PlaySound function.
 * Plays sound on construction and stops it on destruction.
 */
class SoundWrapper {
public:
	SoundWrapper(PCWSTR path);
	~SoundWrapper();
};

#endif // SOUND_WRAPPER_H