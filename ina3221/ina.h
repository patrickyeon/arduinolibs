#ifndef INA3221
#define INA3221


typedef enum {
  CHAN_1 = 1 << 14,
  CHAN_2 = 1 << 13,
  CHAN_3 = 1 << 12
} chan_t;


typedef enum {
  SHUNT_MEAS = 1,
  BUS_MEAS = 1 << 1,
  CONTINUOUS_MEAS = 1 << 2 // not implemented, but whatever
} meas_mode_t;


typedef enum {
  // TODO make sure compiler does what I think here
  AVG_1 = 0x0,
  AVG_4,
  AVG_16,
  AVG_64,
  AVG_128,
  AVG_256,
  AVG_512,
  AVG_1024
} avg_bits_t;


typedef enum {
  // Conversion Time, in us
  CT_140 = 0x0,
  CT_203,
  CT_332,
  CT_588,
  CT_1100,
  CT_2116,
  CT_4156,
  CT_8244
} conv_time_t;


class ina
{
  public:
    ina(int address, avg_bits_t average,
        conv_time_t busCT, conv_time_t shuntCT);
    // XXX will be deprecated soon
    ina(int address, avg_bits_t average, conv_time_t conversionTime);

    int setMode(int channels, int mode);
    void setAverage(avg_bits_t average);
    void setConversionTime(conv_time_t busCT, conv_time_t shuntCT);
    void setConversionTime(conv_time_t conversionTime);

    // XXX will be deprecated soon
    int measure(chan_t);

    int trigger();
    int getVal(chan_t channel, meas_mode_t mode);

  private:
    int mAddress;
    avg_bits_t mAvg;
    conv_time_t mBusCT;
    conv_time_t mShuntCT;
    int mChannels;
    int mMode;

    static const int avgShifts[8];
    static const int ctLengths[8];
};


#endif
