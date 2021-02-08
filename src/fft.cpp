/* Fast Fourier Transform
 * Cooley-Tukey algorithm with 2-radix DFT
 */

#include "fft.hpp"

#define PI 3.14159265358979323846

FFT::FFT(/* args */)
{
}

FFT::~FFT()
{
}

void FFT::_fft_radix2(float* x, std::complex<double>* X, unsigned int N, unsigned int s) {
    unsigned int k;
    std::complex<double> t;

    // At the lowest level pass through (delta T=0 means no phase).
    if (N == 1) {
        X[0] = x[0];
        return;
    }

    // Cooley-Tukey: recursively split in two, then combine beneath.
    _fft_radix2(x, X, N/2, 2*s);
    _fft_radix2(x+s, X + N/2, N/2, 2*s);

    for (k = 0; k < N/2; k++) {
        t = X[k];
        X[k] = t + exp(-2 * PI * 1i * (double)k / (double)N) * X[k + N/2];
        X[k + N/2] = t - exp(-2 * PI * 1i * (double)k / (double)N) * X[k + N/2];
    }
}

void FFT::fft(float* x, std::complex<double>* X, uint16_t N) {
    _fft_radix2(x, X, N, 1);
}
