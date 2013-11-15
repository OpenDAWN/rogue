/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 */

#ifndef DSP_DELAY_H
#define DSP_DELAY_H

#include "types.h"

namespace dsp {

/**
 * Delay code based on The Synthesis ToolKit in C++ (STK)
 * by Perry R. Cook and Gary P. Scavone, 1995-2012.
 */

/**
 * non-interpolating delay line class.
 *
 * This class implements a non-interpolating digital delay-line. If
 *  the delay and maximum length are not specified during
 * instantiation, a fixed maximum length of 4095 and a delay of zero
 * is set.
 */
class Delay {

  public:
    Delay(uint l = 4096);
    ~Delay();
    void setDelay(uint d);
    void setMax(uint d);
    void clear();
    float nextOut();
    float process(float in);

  private:
    float* buffer;
    uint length = 4096, delay = 0, inPoint = 0, outPoint = 0;
    float last = 0.0;

};

/**
 * allpass interpolating delay line class.
 *
 * This class implements a fractional-length digital delay-line using
 * a first-order allpass filter.
 */
class DelayA {

  public:
    DelayA(uint l = 4096);
    ~DelayA();
    void setDelay(float d);
    void setMax(uint d);
    void clear();
    float nextOut();
    float process(float in);

  private:
    float* buffer;
    uint length = 4096, inPoint_, outPoint_;
    float delay_, alpha_, coeff_, last_ = 0.0;
    float apInput_ = 0.0, nextOutput_ = 0.0;
    bool doNextOut_ = true;
};

/**
 * linear interpolating delay line class
 *
 * This class implements a fractional-length digital delay-line using
 * first-order linear interpolation.  If the delay and maximum length
 * are not specified during instantiation, a fixed maximum length of
 * 4095 and a delay of zero is set.
 */
class DelayL {

  public:
    DelayL(uint l = 4096);
    ~DelayL();
    void setDelay(float d);
    void setMax(uint d);
    void clear();
    float nextOut();
    float process(float in);

  private:
    float* buffer;
    uint length = 4096, inPoint_, outPoint_;
    float delay_, alpha_, omAlpha_, last_ = 0.0;
    float nextOutput_ = 0.0;
    bool doNextOut_ = true;

};

}

#endif
