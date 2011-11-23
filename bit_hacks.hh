#ifndef __BIT_HACKS_HH__
#define __BIT_HACKS_HH__

#include <climits>

template <class T>
T next_power_of_two(T x) {
	if (!x) {
		return 1;
	}
	x--;
	for (size_t i = 1; i < sizeof(T) * CHAR_BIT; i *= 2) {
		x |= x >> i;
	}
	return x + 1;
};

unsigned char bit_reverse_byte(unsigned char x) {
	x = ((x & 0xf) << 4) | ((x & 0xf0) >> 4);
	x = ((x & 0x33) << 2) | ((x & 0xcc) >> 2);
	x = ((x & 0x55) << 1) | ((x & 0xaa) >> 1);
	return x;
}

template<class T>
T bit_reverse(T x) {
	T ret = 0;
	size_t nbits = (sizeof(T) - 1) * CHAR_BIT;
	for (size_t i = 0; i <= nbits; i += CHAR_BIT) {
		T new_byte = bit_reverse_byte((x >> i) & 0xff);
		ret |= new_byte << (nbits - i);
	}

	return ret;
}

template<class T>
void bitreversal_permutation(T *vec, size_t size) {
	size_t j = 0;
	for (size_t i = 0; i < size - 1; i++) {
		if (i < j) {
			swap(vec[i], vec[j]);
		}
		size_t k = size >> 1;
		while (k <= j) {
			j -= k;
			k >>= 1;
		}
		j += k;
	}
}

#endif
