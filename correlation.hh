#ifndef __CORRELATION_HH__
#define __CORRELATION_HH__

#include <vector>

template <class T>
static void correlate1D(std::vector<T> &f, std::vector <T> &g,
	std::vector<T> &ret) {
	size_t nf = f.size();
	size_t ng = g.size();
	for (size_t i = 0; i < nf; i++) {
		for (size_t j = 0; j < ng; j++) {
			size_t idx_sig = i + j;
			if (idx_sig >= 0 && idx_sig < nf) {
				ret[i] += f[idx_sig] * g[j];
			}
		}
	}
}

template <class T>
static void correlateCircular1D(std::vector<T> &f, std::vector <T> &g,
	std::vector<T> &ret) {
	size_t nf = f.size();
	size_t ng = g.size();
	for (size_t i = 0; i < nf; i++) {
		for (size_t j = 0; j < nf; j++) {
			size_t idx_kern = (i + j) % ng;
			ret[i] += f[j] * g[idx_kern];
		}
	}
}

#endif
