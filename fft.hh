#ifndef __FFT_HH__
#define __FFT_HH__

#include <vector>
#include <complex>
#include <math.h>

#ifndef M_PI
	#define M_PI (acos(-1.0))
#endif

#include "bit_hacks.hh"

template <class T>
void fft(std::complex<T> *arr, size_t size, bool inverse) {
	size_t pwr = next_power_of_two(size);

	if (size <= 1) {
		return;
	}

	//FIXME: who should handle this?
	if (size != pwr) {
		return;
	}

	bitreversal_permutation(arr, size);
	
	for (size_t step_size = 2; step_size <= size; step_size <<= 1) {
		std::complex<T> w(1, 0);
		T angle = static_cast<T>(-2 * M_PI / step_size);
		if (inverse) {
			angle = -angle;
		}
		std::complex<T> wn(std::polar<T>(1, angle));
		
		size_t half_step = step_size / 2;
		for (size_t k = 0; k < half_step; k++) {
			for (size_t step = 0; step < size / step_size; step++) {
				size_t idx_even = step * step_size + k;
				size_t idx_odd = idx_even + half_step;
				
				std::complex<T> odd = w * arr[idx_odd];
				arr[idx_odd] = arr[idx_even] - odd;
				arr[idx_even] = arr[idx_even] + odd;
				
				if (inverse) {
					arr[idx_even] /= 2;
					arr[idx_odd] /= 2;
				}
			}
			w *= wn;
		}
	}
}

template <class T>
void fft_skip(std::complex<T> data[], size_t stride, size_t count, bool inverse) {
	auto vec = new std::complex<T>[count];

	for (size_t i = 0; i < count; i++) {
		vec[i] = data[stride * i];
	}
	fft(vec, count, inverse);
	for (size_t i = 0; i < count; i++) {
		data[stride * i] = vec[i];
	}

	delete[] vec;
}

template<class T>
void fft_2d(std::complex<T> *data, size_t width, size_t height, bool inverse) {
	for (size_t i = 0; i < width; i++) {
		//vertical direction
		fft_skip(data + i, width, height, inverse);
	}
	for (size_t i = 0; i < height; i++) {
		//horizontal direction
		fft_skip(data + width * i, 1, width, inverse);
	}
}

#endif
