#ifndef __SOUND_BACKEND_HPP__
#define __SOUND_BACKEND_HPP__

class SoundRecorder {
public:
	SoundRecorder(unsigned _frequency)
	: frequency(_frequency) {};
	virtual ~SoundRecorder() {};
	virtual unsigned receiveData(unsigned char *buffer, unsigned sampleCount) = 0;
	unsigned getFrequency() const {return frequency;};
protected:
	unsigned frequency;
};

class SoundPlayer {
public:
	SoundPlayer(unsigned bufferCount, unsigned _frequency)
	:frequency(_frequency) {};
	virtual ~SoundPlayer() {};
	virtual bool transmitData(unsigned char *buffer, unsigned bufferSize) = 0;
	unsigned getFrequency() const {return frequency;};
protected:
	unsigned frequency;
};
#endif
