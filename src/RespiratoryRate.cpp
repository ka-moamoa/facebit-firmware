#include "RespiratoryRate.hpp"
#include "Utilites.h"

RespiratoryRate::RespiratoryRate(CapCalc *cap, Si7051 &temp) : _cap(cap),
                                                               _temp(temp)
{
    _bus_control = BusControl::get_instance();
}

RespiratoryRate::~RespiratoryRate()
{
}

RespiratoryRate::RR_t RespiratoryRate::get_buffer_element()
{
    RR_t tmp = respiratory_rate_buffer.at(0);
    respiratory_rate_buffer.erase(respiratory_rate_buffer.begin());
    return tmp;
}

float RespiratoryRate::calc_resp_rate(float samples[], int SAMPLE_SIZE, float mean)
{
    int i = 1;
    uint8_t max_count = 0;
    float local_max = 0;
    int local_max_index = 0;
    float max_average = 0;
    float delta = 0.5;
    int first_peak;
    int last_peak;
    float threshold = 0.05;
    vector<int> peak_indices;
    while (i < SAMPLE_SIZE)
    {
        if (samples[i] - mean > 0)
        {
            if (local_max < samples[i])
            {
                local_max = samples[i];
                local_max_index = i;
            }
        }
        else if (samples[i] - mean <= 0)
        {
            if (local_max != 0)
            {
                if (max_average != 0 && abs(local_max - max_average) < delta)
                {
                    if (max_count == 0)
                    {
                        first_peak = local_max_index;
                    }
                    else
                    {
                        last_peak = local_max_index;
                    }
                    max_count = max_count + 1;
                    peak_indices.push_back(local_max_index);
                    max_average = (max_average + local_max) / 2;
                    local_max = 0;
                }
                else if (max_average == 0)
                {
                    peak_indices.push_back(local_max_index);
                    max_count = max_count + 1;
                    max_average = local_max;
                    local_max = 0;
                }
            }
        }
        i = i + 1;
    }
    float duration = (peak_indices.back() - peak_indices.front()) / _temp.getMeasurementFrequency();
    if(max_count == 1){
        printf("Breathing too slow\r\n");
        return -2;
    }
    else if (abs(max_average - mean) < threshold || max_count == 0 || duration <= 0)
    {
        return -1; //discard the sample
    }
    else
    {        
        float resp_rate = (60 * (max_count - 1)) / duration;
        return resp_rate;
    }
}

bool RespiratoryRate::get_resp_rate()
{
    bool new_resp_rate = false;

    printf("Resp Sensing\n\r");
    // if ((_cap->calc_joules() > SENSING_ENERGY && _cap.read_voltage() > MIN_VOLTAGE))
    {
        _bus_control->i2c_power(true);
        ThisThread::sleep_for(1000ms);

        _temp.initialize();
        const int SAMPLE_SIZE = 150;

        float samples[SAMPLE_SIZE] = {};
        float sum = 0;

        for (int i = 0; i < SAMPLE_SIZE;)
        {
            _temp.update();
            while (!_temp.getBufferFull())
                _temp.update();

            uint16_t *buffer = _temp.getBuffer();
            for (int j = 0; j < _temp.getBufferSize(); j++)
            {
                //printf("Data %d\r\n", buffer[j]);
                samples[i] = (buffer[j] / 100.0);
                sum = sum + samples[i];
                i++;
            }
            _temp.clearBuffer();
            ThisThread::sleep_for(50ms);
        }
        // convert to floating points
        float mean = (sum) / SAMPLE_SIZE;
        //printf("Mean %f\r\n", mean);
        float resp_rate = calc_resp_rate(samples, SAMPLE_SIZE, mean);
        if(resp_rate != -1){
            printf("RR %0.2f bpm\r\n", resp_rate);
        }
        else{
            printf("Sample discarded... Wait for the new reading\r\n");
        }

        if (resp_rate > 4 && resp_rate < 25)
        {
            new_resp_rate = true;
            
            RR_t new_rate;
            new_rate.rate = Utilities::round(resp_rate * 10);
            new_rate.timestamp = time(NULL);

            respiratory_rate_buffer.push_back(new_rate);
        }
        
    }

    return new_resp_rate;
}
