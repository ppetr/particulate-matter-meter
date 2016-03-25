#ifndef _RUNNING_AVERAGE_H
#define _RUNNING_AVERAGE_H

class RunningAverage {
  public:
    RunningAverage(unsigned long average_time_ms);

    void addSample(float sample);

    float current();

  private:
    float _average_time_ms;
    unsigned long _last_sample_ms;
    float _mean;
};

#endif  // _RUNNING_AVERAGE_H
