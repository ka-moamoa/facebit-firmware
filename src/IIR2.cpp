#include "IIR2.hpp"

IIR2 ::IIR2 ()
{
    //ctor
    m_pNum = NULL;
    m_pDen = NULL;
    m_pW = NULL;
    m_num_order = -1;
    m_den_order = -1;
    m_N = 0;
};
 
/** \brief Zero the internal state of the filter and preserve the coefficients of the filter
 * \return
 */
void IIR2 ::reset()
{
    for(int i = 0; i < m_N; i++)
    {
        m_pW[i] = 0.0;
    }
}
/** \brief
 *
 * \param num Coefficient of molecule polynomial, ascending order, num[0] is a constant term
 * \param m Order of Molecular Polynomial
 * \param den Denominator polynomial coefficients, ascending order, den[0] is a constant term
 * \param m Order of denominator polynomial
 * \return
 */
void IIR2 ::setBiquadParameters(const double *b, int num_order, const double *a, int den_order)
{
    delete[] m_pNum;
    delete[] m_pDen;
    delete[] m_pW;
    m_num_order = num_order;
    m_den_order = den_order;
    m_N = std::max(num_order, den_order) + 1;
    m_pNum = new double[m_N];
    m_pDen = new double[m_N];
    m_pW = new double[m_N];
    for(int i = 0; i < m_N; i++)
    {
        m_pNum[i] = 0.0;
        m_pDen[i] = 0.0;
        m_pW[i] = 0.0;
    }
    for(int i = 0; i <= num_order; i++)
    {
         m_pNum[i] = b[i];
    }
    for(int i = 0; i <= den_order; i++)
    {
        m_pDen[i] = a[i];
    }
}
/** \brief Calculates the time domain response of the IIR filter without affecting the internal state of the filter
 * \param data_in For filter input, input before 0 minutes defaults to 0, data_in[M] and after defaults to data_in[M-1]
 * \param data_out Output of filter
 * \param M Length of input data
 * \param N Length of output data
 * \return
 */
void IIR2 ::resp(double data_in[], int M, double data_out[], int N)
{
    int i, k, il;
    for(k = 0; k < N; k++)
    {
        data_out[k] = 0.0;
        for(i = 0; i <= m_num_order; i++)
        {
            if( k - i >= 0)
            {
                il = ((k - i) < M) ? (k - i) : (M - 1);
                data_out[k] = data_out[k] + m_pNum[i] * data_in[il];
            }
        }
        for(i = 1; i <= m_den_order; i++)
        {
            if( k - i >= 0)
            {
                data_out[k] = data_out[k] - m_pDen[i] * data_out[k - i];
            }
        }
    }
}
/** \brief Filter function with direct type II structure
 *
 * \param data Input data
 * \return Filtered results
 */
double IIR2 ::filter(double data)
{
    m_pW[0] = data;
    for(int i = 1; i <= m_den_order; i++) // Update the status of w[n] first
    {
        m_pW[0] = m_pW[0] - m_pDen[i] * m_pW[i];
    }
    data = 0.0;
    for(int i = 0; i <= m_num_order; i++)
    {
        data = data + m_pNum[i] * m_pW[i];
    }
    for(int i = m_N - 1; i >= 1; i--)
    {
        m_pW[i] = m_pW[i-1];
    }
    return data;
}
/** \brief Filter function with direct type II structure
 *
 * \param data[] Input data, returned with filtered results
 * \param len data[] The length of the array
 * \return
 */
void IIR2 ::filter(float data[], int len)
{
    int i, k;
    for(k = 0; k < len; k++)
    {
        m_pW[0] = data[k];
        for(i = 1; i <= m_den_order; i++) // Update the status of w[n] first
        {
            m_pW[0] = m_pW[0] - m_pDen[i] * m_pW[i];
        }
        data[k] = 0.0;
        for(i = 0; i <= m_num_order; i++)
        {
            data[k] = data[k] + m_pNum[i] * m_pW[i];
        }
 
        for(i = m_N - 1; i >= 1; i--)
        {
            m_pW[i] = m_pW[i-1];
        }
    }
}
/** \brief Filter function with direct type II structure
 *
 * \param data_in[] Input data
 * \param data_out[] Save filtered data
 * \param len The length of the array
 * \return
 */
void IIR2 ::filter(double data_in[], double data_out[], int len)
{
    int i, k;
    for(k = 0; k < len; k++)
    {
        m_pW[0] = data_in[k];
        for(i = 1; i <= m_den_order; i++) // Update the status of w[n] first
        {
            m_pW[0] = m_pW[0] - m_pDen[i] * m_pW[i];
        }
        data_out[k] = 0.0;
        for(i = 0; i <= m_num_order; i++)
        {
            data_out[k] = data_out[k] + m_pNum[i] * m_pW[i];
        }
 
        for(i = m_N - 1; i >= 1; i--)
        {
            m_pW[i] = m_pW[i-1];
        }
    }
}