// Daniel Gilbert
// loglow@gmail.com
// copyright 2013



#ifndef __I2SOUND_H__
#define __I2SOUND_H__

#include <stdint.h>



class I2Sound {
  private:
    typedef void (*ISR)();
    uint8_t nChans;
    uint8_t nBits;
    uint32_t nSamples;
    bool init_mclk();
    void init_rx();
    void init_tx();
    uint32_t* rx_ch0;
    uint32_t* rx_ch1;
  public:
    bool begin(uint32_t newSamples, uint8_t newBits, uint8_t newChans);
    void start_rx(ISR new_rxISR, uint32_t* ch0, uint32_t* ch1);
    void start_tx(ISR new_txISR);
    void stop_rx();
    void stop_tx();
    void read();
    ISR rxISR;
    ISR txISR;
};

extern I2Sound I2S;



#endif



// EOF