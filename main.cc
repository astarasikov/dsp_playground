#include <iostream>
#include <vector>
#include <complex>
#include <algorithm>
#include "convolution.hh"
#include "correlation.hh"
#include "fft.hh"
#include "windowfunction.hh"

using namespace std;

#define NUM_TEST_SAMPLES 8

template <class T>
static void dump(const T& x) {
	cout << "[";
	for (auto &it : x) {
		cout << it << " ";
	}
	cout << "]" << endl;
}

static void test_fft_1d(void) {
	complex<double> ff2[NUM_TEST_SAMPLES] = {0, 1, 2, 3, 4, 5, 6, 7};
	fft(ff2, NUM_TEST_SAMPLES, false);
	cout << "after FFT" << endl;
	dump(ff2);
	fft(ff2, NUM_TEST_SAMPLES, true);
	cout << "after IFFT" << endl;
	dump(ff2);
}

static void test_fft_2d(void) {
	complex<double> arr[] = {
		1, 2, 3, 4,
		5, 6, 7, 8,
		9, 10, 11, 12,
		13, 14, 15, 16,
	};
	
	for (int j = 0; j < 2; j++) {
		fft_2d(arr, 4, 4, j & 1);
	}
	cout << "2d fft" << endl;
	dump(arr);
}

static void test_convolution_1d(void) {
	vector<double> f = {0, 1, 2, 3, 4};
	vector<double> g = {0, 0, 0, 1, 0};
	vector<double> res (f.size(), 0);
	convolve1D(f, g, res);
	cout << "1d convolution" << endl;
	dump(res);
	vector<double> circ (f.size(), 0);
	convolveCircular1D(f, g, circ);
	cout << "1d circular convolution" << endl;
	dump(circ);
}

static void test_correlation_1d(void) {
	vector<double> sig = {1, 2, 3, 1, 3, 3, 4, 5};
	vector<double> pat = {1, 2, 3, 1, 3, 3, 4, 5};
	vector<double> cor (sig.size(), 0);
	cout << "1d circular correlation" << endl;
	correlateCircular1D(pat, sig, cor);
	dump(cor);
}

static void test_lowpass(void) {
	complex<double> sig[NUM_TEST_SAMPLES] =
		{100, 200, 300, 400, 500, 600, 700, 800};
	fft(sig, NUM_TEST_SAMPLES, true);

	double fcut = 300;
	double RC = 1 / (2 * M_PI * fcut);
	double dt = 1.0 / 8000;
	double a = dt / (dt + RC);
	for (size_t i = 1; i < NUM_TEST_SAMPLES; i++) {
		sig[i] = a * sig[i] + (1 - a) * sig[i - 1];
	}
	fft(sig, NUM_TEST_SAMPLES, false);
	cout << "after filtering" << endl;
	dump(sig);
}

static void test_hipass(void) {
	complex<double> sig[NUM_TEST_SAMPLES] =
		{100, 200, 300, 400, 500, 600, 700, 800};
	fft(sig, NUM_TEST_SAMPLES, true);

	double fcut = 300;
	double RC = 1 / (2 * M_PI * fcut);
	double dt = 1.0 / 8000;
	double a = dt / (dt + RC);
	complex<double> ret[NUM_TEST_SAMPLES];
	ret[0] = 0;
	for (size_t i = 1; i < NUM_TEST_SAMPLES; i++) {
		ret[i] = a * ret[i - 1] + a * (sig[i] - sig[i -1]);
	}
	fft(ret, NUM_TEST_SAMPLES, false);
	cout << "after filtering" << endl;
	dump(ret);
}

static void test_window(void) {
	double foo[] = {1, 2, 3, 4};
	HannWindow<double> wnd(4);
	wnd.apply(foo);
	dump(foo);
}

int main() {
	test_convolution_1d();
	test_correlation_1d();
	test_fft_1d();
	test_fft_2d();
	test_lowpass();
	test_hipass();
	test_window();

	return 0;
}
