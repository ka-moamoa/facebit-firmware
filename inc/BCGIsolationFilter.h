#ifndef BCGISOLATIONFILTER_H_
#define BCGISOLATIONFILTER_H_

/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 50 Hz

* 0 Hz - 9 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -42.32212038784483 dB

* 10 Hz - 13 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 3.170893963653967 dB

* 14 Hz - 25 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -42.32212038784483 dB

*/

#ifdef __cplusplus
extern "C" {
#endif

#define BCGISOLATIONFILTER_TAP_NUM 64

typedef struct {
  float history[BCGISOLATIONFILTER_TAP_NUM];
  unsigned int last_index;
} BCGIsolationFilter;

void BCGIsolationFilter_init(BCGIsolationFilter* f);
void BCGIsolationFilter_put(BCGIsolationFilter* f, float input);
float BCGIsolationFilter_get(BCGIsolationFilter* f);

#ifdef __cplusplus
}
#endif

#endif