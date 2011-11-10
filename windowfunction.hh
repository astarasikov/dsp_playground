#ifndef __WINDOWFUNCTION_HH__
#define __WINDOWFUNCTION_HH__

#define _USE_MATH_DEFINES
#include <math.h>

template <class T>
class WindowFunction {
public:
	virtual ~WindowFunction() {}
	virtual void apply(T* data) = 0;
};

template<class T>
class HannWindow : public WindowFunction<T> {
public:
	HannWindow(size_t length):
		length(length), coeffs(new double[length]) {
		double phi = 2 * M_PI / (length - 1);
		for (size_t i = 0; i < length; i++) {
			coeffs[i] = 0.5 - 0.5 * cos(i * phi);
		}
	}
	~HannWindow() {
		delete[] coeffs;
	}
	
	void apply(T* data) {
		for (size_t i = 0; i < length; i++) {
			data[i] *= coeffs[i];
		}
	}
protected:
	size_t length;
	double *coeffs;
};

#endif
