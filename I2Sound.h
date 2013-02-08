// Daniel Gilbert
// loglow@gmail.com
// copyright 2013



#ifndef __I2SOUND_H__
#define __I2SOUND_H__



#include <stdint.h>



typedef volatile uint32_t* reg;



class I2Sound {
  private:
    uint8_t myID;
    uint8_t nChans;
    uint8_t nBits;
    void init_mclk(uint8_t fract, uint8_t divide);
    void init_rx();
    void init_tx();
    reg I2S_MDR;
    reg I2S_MCR;
    reg I2S_RCR1;
    reg I2S_RCR2;
    reg I2S_RCR3;
    reg I2S_RCR4;
    reg I2S_RCR5;
    reg I2S_RCSR;
    reg I2S_RDR0;
    uint8_t IRQ_I2S_RX;
    uint8_t IRQ_I2S_TX;
    uint32_t* rx_ch0;
    uint32_t* rx_ch1;
  public:
    I2Sound(uint8_t newID);
    void init(uint8_t newChans, uint8_t newBits);
    void start_rx(void (*new_rxISR)(), uint32_t* ch0, uint32_t* ch1);
    void start_tx(void (*new_txISR)());
    void stop_rx();
    void stop_tx();
    void read();
    void (*rxISR)();
    void (*txISR)();
};



extern I2Sound I2Sound0;



#endif



// EOF