#include "SoundOpenAl.hpp"

#include <AL/al.h>
#include <AL/alc.h>

#include <cmath>
#include <cstdio>

class SoundOpenAlRecorderImpl {
public:
	ALCdevice *device;
};

SoundOpenAlRecorder :: SoundOpenAlRecorder(unsigned frequency)
	: SoundRecorder(frequency),
	impl(new SoundOpenAlRecorderImpl()) {
	impl->device = alcCaptureOpenDevice(0,
		frequency, AL_FORMAT_MONO8, frequency);

	fprintf(stderr, "alcCaptureOpenDevice: device=%08x, err=%d\n",
		reinterpret_cast<int>(impl->device),
		alGetError());
	alcCaptureStart(impl->device);
	fprintf(stderr, "alcCaptureStart: err=%d\n", alGetError());
}

SoundOpenAlRecorder :: ~SoundOpenAlRecorder() {
	alcCaptureStop(impl->device);
	alcCaptureCloseDevice(impl->device);
	delete impl;
}

unsigned SoundOpenAlRecorder :: receiveData(unsigned char *destBuffer,
	unsigned sampleCount) {
	ALCint rec_samples = 0, needed_samples = sampleCount, err;
	
	do {
		alcGetIntegerv(impl->device, ALC_CAPTURE_SAMPLES, 1, &rec_samples);
	} while (rec_samples < needed_samples);
	alcCaptureSamples(impl->device, destBuffer, rec_samples);
	
	if ((err = alcGetError(impl->device))) {
		fprintf(stderr, "-%s err=%d\n", __func__, err);
	}
	return rec_samples;
}

class SoundOpenAlPlayerImpl {
public:
	SoundOpenAlPlayerImpl(unsigned _bufferCount, unsigned _frequency)
		: buffers(new ALuint[_bufferCount]),
		bufferCount(_bufferCount),
		bufferSize(_frequency)
		{}

	~SoundOpenAlPlayerImpl() {
		delete[] buffers;
	}
	ALCdevice *device;
	ALuint source;
	ALuint *buffers;
	ALCcontext* context;
	unsigned bufferCount;
	ALCuint bufferSize;
};

SoundOpenAlPlayer :: SoundOpenAlPlayer(unsigned bufferCount, unsigned frequency)
	: SoundPlayer(bufferCount, frequency),
	impl(new SoundOpenAlPlayerImpl(bufferCount, frequency)) {

	impl->device = alcOpenDevice(0);
	fprintf(stderr, "%s: dev=%08x, err=%d\n", __func__,
		reinterpret_cast<int>(impl->device),
		alcGetError(impl->device));
	impl->context = alcCreateContext(impl->device, 0);
	fprintf(stderr, "%s: context=%08x, err=%d\n", __func__,
		reinterpret_cast<int>(impl->context), alGetError());
	alcMakeContextCurrent(impl->context);
	alGenBuffers(impl->bufferCount, impl->buffers);
	alGenSources(1, &impl->source);
}

SoundOpenAlPlayer :: ~SoundOpenAlPlayer() {
	alDeleteSources(1, &impl->source);
	alDeleteBuffers(impl->bufferCount, impl->buffers);
	alcMakeContextCurrent(0);
	alcDestroyContext(impl->context);
	alcCloseDevice(impl->device);
	delete impl;
}

bool SoundOpenAlPlayer :: transmitData(unsigned char* sstvBuffer,
	unsigned sstvBufferSize) {
	int err;
	unsigned sstvBufferCount = sstvBufferSize / impl->bufferSize;
	unsigned lastSstvBuffer = 0;

	if (sstvBufferSize % impl->bufferSize)
		++sstvBufferCount;

	while(lastSstvBuffer < sstvBufferCount) {
		ALCuint buffersToProcess = sstvBufferCount - lastSstvBuffer;
		if (buffersToProcess > impl->bufferCount)
			buffersToProcess = impl->bufferCount;

		for (unsigned i = 0; i < buffersToProcess; i++) {
			alBufferData(impl->buffers[i], AL_FORMAT_MONO8,
				sstvBuffer + frequency * (lastSstvBuffer + i),
				frequency, frequency);
		}
		lastSstvBuffer += buffersToProcess;

		err = alGetError();
		if (err) {
			fprintf(stderr, "%s: err buffering=%d\n", __func__, err);
			return false;
		}
		alSourceQueueBuffers(impl->source, buffersToProcess, impl->buffers);
		alSourcePlay(impl->source);

		ALint state;
		do {
			alGetSourcei(impl->source, AL_SOURCE_STATE, &state);
		} while (state == AL_PLAYING);

		alSourceStop(impl->source);
		alSourceUnqueueBuffers(impl->source, buffersToProcess, impl->buffers);
	};
	
	return true;
}
