#ifndef HRFILTER_H_
#define HRFILTER_H_

/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 52 Hz

* 0 Hz - 0.2 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -40.97987127603499 dB

* 0.75 Hz - 2.5 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 3.7158822319997187 dB

* 3 Hz - 26 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -40.97987127603499 dB

*/

#ifdef __cplusplus
extern "C" {
#endif

#define HRFILTER_TAP_NUM 128

typedef struct {
  float history[HRFILTER_TAP_NUM];
  unsigned int last_index;
} HRFilter;

void HRFilter_init(HRFilter* f);
void HRFilter_put(HRFilter* f, float input);
float HRFilter_get(HRFilter* f);

#ifdef __cplusplus
}
#endif

#endif