/**< IIR Filter Direct Type II Implementation */

#ifndef IIR2_H_
#define IIR2_H_

#include <complex.h>

class IIR2
{
public:
    IIR2();
    void reset();
    void setBiquadParameters(const double *b, int num_order, const double *a, int den_order);
    void resp(double data_in[], int m, double data_out[], int n);
    double filter(double data);
    void filter(float data[], int len);
    void filter(double data_in[], double data_out[], int len);
protected:
    private:
    double *m_pNum;
    double *m_pDen;
    double *m_pW;
    int m_num_order;
    int m_den_order;
    int m_N;
};

#endif // IIR2_H_
