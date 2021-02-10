#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <math.h>

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

}

#endif // UTILITIES_H_