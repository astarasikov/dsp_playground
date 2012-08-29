#ifndef __SOUND_OPENAL_HPP__
#define __SOUND_OPENAL_HPP__

#include "SoundBackend.hpp"

class SoundOpenAlPlayerImpl;
class SoundOpenAlRecorderImpl;

class SoundOpenAlPlayer : public SoundPlayer {
public:
	SoundOpenAlPlayer(unsigned bufferCount, unsigned frequency);
	~SoundOpenAlPlayer();
	virtual bool transmitData(unsigned char* bufferData, unsigned bufferSize);
private:
	SoundOpenAlPlayerImpl *impl;
};

class SoundOpenAlRecorder : public SoundRecorder {
public:
	SoundOpenAlRecorder(unsigned frequency);
	~SoundOpenAlRecorder();
	virtual unsigned receiveData(unsigned char *buffer, unsigned sampleCount);
private:
	SoundOpenAlRecorderImpl *impl;
};
#endif
