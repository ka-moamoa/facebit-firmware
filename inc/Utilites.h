/**
 * @file Utilities.h
 * @author Alexander Curtiss apcurtiss@gmail.com
 * @brief 
 * @version 0.1
 * @date 2022-01-16
 * 
 * @copyright Copyright (c) 2022 Ka Moamoa
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <math.h>
#include <numeric>
#include <algorithm>
#include <queue>

namespace Utilities
{
    inline double map_range(double val, double input_start, double input_end, double output_start, double output_end)
    {
        double slope = (output_end - output_start) / (input_end - input_start);
        return output_start + slope * (val - input_start);
    }

    inline int round(double val)
    {
        return floor(val + 0.5);
    }

    inline double std_dev(vector<double>& v)
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

    inline double mean(vector<double>& v)
    {
        double sum = std::accumulate(v.begin(), v.end(), 0.0);
        double mean = sum / v.size();

        return mean;
    }

    //From http://www.richelbilderbeek.nl/CppReciprocal.htm
    inline void reciprocal(vector<double>& c)
    {
    std::transform(c.begin(),c.end(),c.begin(),
        std::bind1st(std::divides<double>(),1.0));    
    }

    inline void multiply(vector<double>& v, double k)
    {
        std::transform(v.begin(), v.end(), v.begin(), [k](double &c){ return c*k; });
    }

}

#endif // UTILITIES_H_