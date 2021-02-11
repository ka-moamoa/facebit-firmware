#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <math.h>
#include <numeric>

namespace Utilities
{

    inline double map_range(double val, double input_start, double input_end, double output_start, double output_end)
    {
        double slope = (output_end - output_start) / (input_end - input_start);
        return output_start + slope * (val - input_start);
    }

    inline double round(double val)
    {
        return floor(val + 0.5);
    }

    inline double std_dev(vector<double> v)
    {    
        double sum = std::accumulate(v.begin(), v.end(), 0.0);
        double mean = sum / v.size();

        std::vector<double> diff(v.size());
        std::transform(v.begin(), v.end(), diff.begin(),
                    std::bind2nd(std::minus<double>(), mean));
        double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
        double stdev = std::sqrt(sq_sum / v.size());

        return stdev;
    }

}

#endif // UTILITIES_H_