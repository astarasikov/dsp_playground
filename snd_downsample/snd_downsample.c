#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static size_t downsample(
		int *buffer,
		size_t num_samples,
		unsigned src_rate,
		unsigned dst_rate)
{
	float ratio = ((float)src_rate) / dst_rate;
	size_t num_low_samples = floor(num_samples / ratio);

	size_t chunk;
	for (chunk = 0; chunk < num_low_samples; chunk++)
	{
		float idx_float = chunk * ratio;
		float weight_first = ceil(idx_float) - idx_float;
		float weight_last = idx_float - floor(idx_float);

		if (weight_first == 0.0) {
			weight_first = 1.0;
		}

		size_t idx_start = floor(idx_float);
		size_t idx_end = ceil(idx_float);

		float sum = 0;
		sum += buffer[idx_start] * weight_first;
		sum += buffer[idx_end] * weight_last;
		size_t i;
		for (i = 1; i < (size_t)ratio; i++)
		{
			sum += buffer[idx_start + i];
		}

		sum /= ratio;
		buffer[chunk] = sum;
	}
	return num_low_samples;
}

int main(int argc, char** argv)
{
    SF_INFO info = {}, dst_info = {};
    SNDFILE *in = NULL, *out = NULL;
    int* buffer = NULL;
	size_t num_low_samples = 0;
    sf_count_t ndata = 0;

    if (argc != 3) {
        printf("usage: %s in.wav out.wav\n", argv[0]);
        return EXIT_FAILURE;
    }

    in = sf_open(argv[1], SFM_READ, &info);
    if (sf_error(0)) {
        fprintf(stderr, "%s\n", sf_strerror(0));
        goto cleanup;
    }

    printf("rate %d, frames %d channels %d\n",
        info.samplerate,
        (int)info.frames,
        info.channels);

    buffer = (int*)malloc(info.channels * info.frames * sizeof(int));
    if (!buffer) {
        perror("");
        goto cleanup;
    }

    ndata = sf_readf_int(in, buffer, info.frames * info.channels);
    if (sf_error(0)) {
        fprintf(stderr, "failed to read input %s\n", sf_strerror(0));
    }
    printf("read %d items\n", (int)ndata);

	dst_info = info;
	dst_info.samplerate = 8000;

	num_low_samples = downsample(buffer, ndata, info.samplerate, dst_info.samplerate);

    out = sf_open(argv[2], SFM_WRITE, &dst_info);
    if (sf_error(0)) {
        fprintf(stderr, "%s\n", sf_strerror(0));
        goto cleanup;
    }

    ndata = sf_writef_int(out, buffer, num_low_samples);
    if (sf_error(0)) {
        fprintf(stderr, "failed to write output %s\n", sf_strerror(0));
		goto cleanup;
    }
    printf("written %d items\n", (int)ndata);

cleanup:
    if (buffer) {
        free(buffer);
    }

    if (in) {
        sf_close(in);
    }

    if (out) {
        sf_close(out);
    }

    return EXIT_SUCCESS;
}
