#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PI 3.14159265
//Filter kernel is multiplied by (1 << FLT_NORM_BITS)
//to use integer multiplication
#define FLT_NORM_BITS 20

struct biquad {
	long long int a[3];
	//B coefficients are negated to use multiply-add in convolution
	long long int neg_b[2];
};

static struct biquad lowpass(double slope, double nfreq)
{
	struct biquad flt;
	double nf_sq = nfreq * nfreq;
	double nf_slope = nfreq / slope;
	double norm = 1.0 * (1 << FLT_NORM_BITS) / (1 + nf_slope + nf_sq);
	flt.a[0] = nf_sq * norm;
	flt.a[1] = 2 * flt.a[0];
	flt.a[2] = flt.a[0];
	flt.neg_b[0] = -2 * (nf_sq - 1) * norm;
	flt.neg_b[1] = -(1 - nf_slope + nf_sq) * norm;
	return flt;
}

static struct biquad highpass(double slope, double nfreq) {
	struct biquad flt;
	double nf_sq = nfreq * nfreq;
	double nf_slope = nfreq / slope;
	double norm = (1 << FLT_NORM_BITS) / (1 + nf_slope + nf_sq);
	flt.a[0] = norm;
	flt.a[1] = -2 * norm;
	flt.a[2] = flt.a[0];
	flt.neg_b[0] = -2 * (nf_sq - 1) * norm;
	flt.neg_b[1] = -(1 - nf_slope + nf_sq) * norm;
	return flt;
}

static void convolve(int *buffer, int frames, struct biquad f) {
	int i;
	int _x1;
	int x1 = buffer[0], x2 = buffer[1];

	printf("%lld %lld %lld %lld %lld\n",
		f.a[0], f.a[1], f.a[2], f.neg_b[0], f.neg_b[1]);

	for (i = 2; i < frames; i++) {
		_x1 = buffer[i];

		buffer[i] = (f.a[0] * _x1) >> FLT_NORM_BITS;
		buffer[i] += (f.a[1] * x1) >> FLT_NORM_BITS;
		buffer[i] += (f.a[2] * x2) >> FLT_NORM_BITS;
		buffer[i] += (f.neg_b[0] * buffer[i - 1]) >> FLT_NORM_BITS;
		buffer[i] += (f.neg_b[1] * buffer[i - 2]) >> FLT_NORM_BITS;
		
		x2 = x1;
		x1 = _x1;
	}
}

int main(int argc, char **argv) {
	SF_INFO info;
	SNDFILE *in = NULL, *out = NULL;
	int *buffer = NULL;
	sf_count_t ndata;
	double slope, normalized_fq;
	struct biquad bq;
	unsigned fcut;

	if (argc != 5) {
		printf("usage: %s in.wav out.wav low|high frequency\n", argv[0]);
		return EXIT_FAILURE;
	}

	memset((void*)&info, 0, sizeof(info));

	in = sf_open(argv[1], SFM_READ, &info);
	if (sf_error(0)) {
		fprintf(stderr, "%s\n", sf_strerror(0));
		goto cleanup;
	}
	
	printf("rate %d, frames %d channels %d\n",
		info.samplerate,
		(int)info.frames,
		info.channels
	);

	buffer = (int*)malloc(info.channels * info.frames * sizeof(int));
	if (!buffer) {
		perror("");
		goto cleanup;
	}

	ndata = sf_readf_int(in, buffer, info.frames);
	if (sf_error(0)) {
		fprintf(stderr, "failed to read input %s\n", sf_strerror(0));
	}
	printf("read %d items\n", (int)ndata);

	slope = 0.9;
	fcut = atoi(argv[4]);

	normalized_fq = tan((PI * fcut) / info.samplerate);
	if (!strcmp(argv[3], "low")) {
		bq = lowpass(slope, normalized_fq);
	}
	else {
		bq = highpass(slope, normalized_fq);
	}
	convolve(buffer, info.frames, bq);

	out = sf_open(argv[2], SFM_WRITE, &info);
	if (sf_error(0)) {
		fprintf(stderr, "%s\n", sf_strerror(0));
		goto cleanup;
	}

	ndata = sf_writef_int(out, buffer, ndata);
	if (sf_error(0)) {
		fprintf(stderr, "failed to write output %s\n", sf_strerror(0));
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
