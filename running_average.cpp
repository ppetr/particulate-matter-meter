#include "running_average.h"

#include "Arduino.h"

RunningAverage::RunningAverage(unsigned long average_time_ms) {
    _mean = NAN;
    _last_sample_ms = 0;  // ignored as _mean is NaN
    _average_time_ms = average_time_ms;
}

void RunningAverage::addSample(float sample) {
    const unsigned long now = millis();
    if (isnan(_mean)) {
        _mean = sample;
    } else {
        const float alpha = pow(0.5, (now - _last_sample_ms) / _average_time_ms);
        // _mean += (1 - alpha) * (sample - mean)
        _mean = sample + alpha * (_mean - sample);
    }
    _last_sample_ms = now;
}

float RunningAverage::current() {
    return _mean;
}
