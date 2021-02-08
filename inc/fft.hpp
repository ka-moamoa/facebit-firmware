/* Fast Fourier Transform
 * Cooley-Tukey algorithm with 2-radix DFT
 */

#ifndef FFT_H_
#define FFT_H_

#include <stdint.h>
#include "mbed.h"
#include <complex.h>

class FFT
{
private:
    void _fft_radix2(float* x, std::complex<double>* X, unsigned int N, unsigned int s);
public:
    FFT(/* args */);
    ~FFT();

    void fft(float* x, std::complex<double>* X, uint16_t N);
};


#endif // FFT_H_
