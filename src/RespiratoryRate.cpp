#include "RespiratoryRate.hpp"

RespiratoryRate::RespiratoryRate(CapCalc &cap, Si7051 &temp) : _cap(cap),
                                                               _temp(temp)
{
    _bus_control = BusControl::get_instance();
}

RespiratoryRate::~RespiratoryRate()
{
}

float RespiratoryRate::calc_resp_rate(float samples[], int SAMPLE_SIZE, float mean)
{
    int i = 1;
    uint8_t max_count = 0;
    float local_max = 0;
    int local_max_index=0;
    float max_average = 0;
    float delta = 0.5;
    int first_peak;
    int last_peak;
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
        //printf("%d, %0.2f\r\n",i,samples[i]);
        i = i + 1;
    }
    //for(int i=0; i < peak_indices.size(); i++)
        //printf("I: %d\r\n",peak_indices.at(i));
    //printf("Time : %d %d",last_peak,first_peak);
    //printf("indices : %d %d",peak_indices.back(),peak_indices.front());
    //printf("%d, %f, %d, ",i,sample[i],
    float duration = (peak_indices.back() - peak_indices.front())/_temp.getMeasurementFrequency();
    float resp_rate = (60*(max_count-1))/duration;
    return resp_rate;
}

void RespiratoryRate::get_resp_rate()
{
    
    printf("Resp Sensing\n\r");
    // if ((_cap.calc_joules() > SENSING_ENERGY && _cap.read_voltage() > MIN_VOLTAGE))
    {
        _bus_control->i2c_power(true);
        ThisThread::sleep_for(1000ms);

        _temp.initialize();

        //int total_windows = 1;
        //int breath_count = 0;
        
        //for (int window = 0; window < total_windows; window++)
        //{
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
        //}

        
        printf("RR %f bpm\r\n", resp_rate);
        //respiratory_rate_buffer.push_back(new_rate);
    }
}