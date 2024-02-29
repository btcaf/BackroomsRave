#include "SoundWrapper.h"

#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

SoundWrapper::SoundWrapper(PCWSTR path) {
	PlaySound(path, nullptr, SND_FILENAME | SND_ASYNC | SND_LOOP);
}

SoundWrapper::~SoundWrapper() {
	PlaySound(nullptr, nullptr, 0);
}