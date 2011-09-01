#include <iostream>
#include <vector>
#include <complex>
#include "convolution.hh"
#include "correlation.hh"
#include "fft.hh"

using namespace std;

template <class T>
static void dump(const T& x) {
	cout << "[";
	for (auto &it : x) {
		cout << it << " ";
	}
	cout << "]" << endl;
}

int main() {
	vector<double> f = {0, 1, 2, 3, 4};
	vector<double> g = {0, 0, 0, 1, 0};
	vector<double> res (f.size(), 0);
	convolve1D(f, g, res);
	dump(res);
	vector<double> circ (f.size(), 0);
	convolveCircular1D(f, g, circ);
	dump(circ);

	vector<double> sig = {1, 2, 3, 1, 3, 3, 4, 5};
	vector<double> pat = {1, 2, 3, 1, 3, 3, 4, 5};
	vector<double> cor (sig.size(), 0);
	correlateCircular1D(pat, sig, cor);
	dump(cor);

	vector<complex<double>> ff2 = {0, 1, 2, 3, 4, 5, 6, 7};
	fft(ff2, false);
	cout << "after FFT" << endl;
	dump(ff2);
	fft(ff2, true);
	cout << "after IFFT" << endl;
	dump(ff2);

	return 0;
}
