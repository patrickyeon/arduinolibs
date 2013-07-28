#if ARDUINO >= 100
  #include <Arduino.h>
#else
  #include "WProgram.h"
#endif

#include <Wire.h>
#include "ina.h"

// averaging modes from table 3 in datasheet
const int ina::avgShifts[8] = {0, 2, 4, 6, 7, 8, 9, 10};
// conversion times, in us, for measurements as in table 5
const int ina::ctLengths[8] = {140, 204, 332, 588, 1100, 2116, 4156, 8244};


ina::ina(int address, avg_bits_t average,
         conv_time_t busCT, conv_time_t shuntCT)
{
  // only lower 7 bits of the address matter anyway
  mAddress = address & 0x7f;
  mAvg = average;
  mBusCT = busCT;
  mShuntCT = shuntCT;
  mChannels = mMode = 0;
}


// XXX this should be deprecated soon
ina::ina(int address, avg_bits_t average, conv_time_t conversionTime)
{
    // in legacy code, we only measure shunt voltage, so bus conversion time
    // doesn't matter, but we need to pass _something_ along
    ina(address, average, conversionTime, conversionTime);
}


// Measure the shunt voltage for channel. Return value LSB = 5uV.
// Return values {1, 2, 3, 4} are the errors from the Wire library.
// TODO make that a more sensible return condition.
// XXX this is going to be deprecated.
int ina::measure(chan_t channel)
{
  int err;
  err = setMode(channel, SHUNT_MEAS);
  if (err != 0) {
    return err;
  }
  // trigger();
  return getVal(channel, SHUNT_MEAS);
}


int ina::setMode(int channels, int mode)
{
  channels &= (CHAN_1 | CHAN_2 | CHAN_3);
  mode &= (SHUNT_MEAS | BUS_MEAS | CONTINUOUS_MEAS);
  mChannels = channels;
  mMode = mode;
  return trigger();
}


int ina::trigger()
{
  Wire.beginTransmission(mAddress);
  Wire.write((byte)0x00); // configuration register
  // set channel, number of averages, conversion time
  // hard-coded for single-shot measurement
  int mask = (mAvg << 9) | (mBusCT << 6) | (mShuntCT << 3) | mMode;
  // two bytes, MSB first
  Wire.write(((mChannels | mask) >> 8) & 0xFF);
  Wire.write(mask & 0xFF);
  int err = Wire.endTransmission();

  byte cvrf = 0;
  while (cvrf == 0) // polling to make sure we get a fresh measurement
  {
    Wire.requestFrom(mAddress, 2);
    while(!Wire.available()) { delay(20); }
    Wire.read();
    while(!Wire.available()) { delay(20); }
    cvrf = Wire.read() & 1;
  }
  // will return the error code from the Wire library
  return err;
}


int ina::getVal(chan_t channel, meas_mode_t mode)
{
  // only one measurement + channel per call
  if ((channel != CHAN_1 && channel != CHAN_2 && channel != CHAN_3)
      || (mode != SHUNT_MEAS && mode != BUS_MEAS))
  {
    return -1;
  }

  Wire.beginTransmission(mAddress);
  Wire.write(0x0f); // status register we can poll to see if measurement is done
  int err = Wire.endTransmission();
  if (err != 0)
  {
    return err;
  }

  Wire.beginTransmission(mAddress);
  char reg = mode == SHUNT_MEAS ? 0 : 1;
  // set register pointer for later read
  switch (channel)
  {
    case CHAN_1:
      Wire.write(0x01 + reg);
      break;
    case CHAN_2:
      Wire.write(0x03 + reg);
      break;
    case CHAN_3:
      Wire.write(0x05 + reg);
      break;
  }

  err = Wire.endTransmission();
  if (err != 0)
  {
    return err;
  }

  Wire.requestFrom(mAddress, 2);

  int retval = 0;
  byte curHi, curLo;
  // TODO is there a more sensible way to read data?
  while (!Wire.available())
  {
    delay(20); // arbitrary delay
  }
  curHi = Wire.read();
  while (!Wire.available())
  {
    delay(20); // arbitrary delay
  }
  curLo = Wire.read();

  return (curHi << 8) | curLo;
}


void ina::setAverage(avg_bits_t average)
{
  mAvg = average;
}


void ina::setConversionTime(conv_time_t busCT, conv_time_t shuntCT)
{
  mBusCT = busCT;
  mShuntCT = shuntCT;
}


void ina::setConversionTime(conv_time_t conversionTime)
{
    setConversionTime(conversionTime, conversionTime);
}
