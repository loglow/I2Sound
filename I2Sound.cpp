// Daniel Gilbert
// loglow@gmail.com
// copyright 2013



#include "I2Sound.h"
#include <mk20dx128.h>
#include <stdint.h>



// ------------------------------------------------------------
// pre-initialized class object for the I2S interface (only one)
// ------------------------------------------------------------
I2Sound I2S;



// ------------------------------------------------------------
// wrapper for the default I2S receive ISR, this allows
// the definition of a custom callback function for interrupts.
// the callback must not have any arguments and return void.
// read() populates provided memory addresses with audio data
// ------------------------------------------------------------
void i2s0_rx_isr() {
  I2S.read();
  I2S.rxISR();
}



// ------------------------------------------------------------
// wrapper for the default I2S transmit ISR, this allows
// the definition of a custom callback function for interrupts.
// the callback must not have any arguments and return void.
// ------------------------------------------------------------
void i2s0_tx_isr() {
  I2S.txISR();
}



// ------------------------------------------------------------
// initialize the MCLK divider and enable clock output
// ------------------------------------------------------------
bool I2Sound::init_mclk() {
  
  uint32_t fract, divide;
  if (nSamples == 48000) { fract = 16; divide = 125; }
  else return false;
  
  SIM_SCGC6 |= SIM_SCGC6_I2S;                // enable clock to the I2S module
  I2S0_MDR |= I2S_MDR_FRACT((fract - 1));    // output = input * (FRACT + 1) / (DIVIDE + 1)
  I2S0_MDR |= I2S_MDR_DIVIDE((divide - 1));  // example: 12.288 MHz = 96 MHz * (16 / 125)
  I2S0_MCR |= I2S_MCR_MOE;                   // enable MCLK pin as output
  return true;
  
}



// ------------------------------------------------------------
// initialize all the RX (receive) registers, configure the
// bit and word (frame sync) clocks, and setup interrupts.
// note: SAI stands for "Synchronous Audio Interface"
// ------------------------------------------------------------
void I2Sound::init_rx() {
  
  // SAI Receive Configuration 1 Register
  I2S0_RCR1 =
    I2S_RCR1_RFW((nChans - 1))  // set FIFO watermark
  ;
  
  // SAI Receive Configuration 2 Register
  I2S0_RCR2 =
    I2S_RCR2_MSEL(1)  // use MCLK as BCLK source
  | I2S_RCR2_SYNC(0)  // use asynchronous mode
  | I2S_RCR2_DIV(1)   // (DIV + 1) * 2, example: 12.288 MHz / 4 = 3.072 MHz
  | I2S_RCR2_BCD      // generate BCLK, master mode
  | I2S_TCR2_BCP      // BCLK is active low
  ;
  
  // SAI Receive Configuration 3 Register
  I2S0_RCR3 =
    I2S_RCR3_RCE  // enable receive channel
  ;
  
  // SAI Receive Configuration 4 Register
  I2S0_RCR4 =
    I2S_RCR4_FRSZ((nChans - 1))  // frame size in words
  | I2S_RCR4_SYWD((nBits - 1))   // bit width of WCLK
  | I2S_RCR4_MF                  // MSB (most significant bit) first
  | I2S_RCR4_FSD                 // generate WCLK, master mode
  | I2S_RCR4_FSE                 // extra bit before frame starts
  ;
  
  // SAI Receive Configuration 5 Register
  I2S0_RCR5 =
    I2S_RCR5_W0W((nBits - 1))  // bits per word, first frame
  | I2S_RCR5_WNW((nBits - 1))  // bits per word, nth frame
  | I2S_RCR5_FBT((nBits - 1))  // index shifted for FIFO
  ;
  
  // SAI Receive Control Register
  I2S0_RCSR =
    I2S_RCSR_BCE   // enable the BCLK output
  | I2S_RCSR_RE    // enable receive globally
  | I2S_RCSR_FRIE  // enable FIFO request interrupt
  ;
  
}



// ------------------------------------------------------------
// TODO -- initialize all the TX (receive) registers
// ------------------------------------------------------------
void I2Sound::init_tx() {}



// ------------------------------------------------------------
// public, universal init function. needs to be passed sample
// rate, bit depth, and number of audio channels
// ------------------------------------------------------------
bool I2Sound::begin(uint32_t newSamples, uint8_t newBits, uint8_t newChans) {
  nSamples = newSamples;
  nBits = newBits;
  nChans = newChans;
  if(!init_mclk()) return false;
  init_rx();
  init_tx(); 
}



// ------------------------------------------------------------
// start the I2S receive process. this function needs to be
// passed the name of a user-defined callback funtion to execute
// when an RX interrupt is generated. the callback function
// must take no args and return void. enables RX interrupts.
// also passed pointers to where the audio data will be stored
// ------------------------------------------------------------
void I2Sound::start_rx(ISR new_rxISR, uint32_t* ch0, uint32_t* ch1) {
  rxISR = new_rxISR;
  rx_ch0 = ch0;
  rx_ch1 = ch1;
  NVIC_ENABLE_IRQ(IRQ_I2S0_RX);
}



// ------------------------------------------------------------
// start the I2S transmit process. this function needs to be
// passed the name of a user-defined callback funtion to execute
// when a TX interrupt is generated. the callback function
// must take no args and return void. enables TX interrupts.
// ------------------------------------------------------------
void I2Sound::start_tx(ISR new_txISR) {
  txISR = new_txISR;
  NVIC_ENABLE_IRQ(IRQ_I2S0_TX);
}



// ------------------------------------------------------------
// stop the I2S receive process by disabling RX interrupts
// ------------------------------------------------------------
void I2Sound::stop_rx() {
  NVIC_DISABLE_IRQ(IRQ_I2S0_RX);
}



// ------------------------------------------------------------
// stop the I2S transmit process by disabling TX interrupts
// ------------------------------------------------------------
void I2Sound::stop_tx() {
  NVIC_DISABLE_IRQ(IRQ_I2S0_TX);
}



// ------------------------------------------------------------
// perform a read of two words of data from the top of the FIFO.
// I'm not entirely sure why there needs to be a dummy read in-
// between the two legitimate reads, but there does. once data
// has been read, clear the FIFO for the next time around
// ------------------------------------------------------------
void I2Sound::read() {
  *rx_ch0 = I2S0_RDR0;       // read channel 0
  *rx_ch1 = I2S0_RDR0;       // dummy read
  *rx_ch1 = I2S0_RDR0;       // read channel 1
  I2S0_RCSR |= I2S_RCSR_FR;  // reset fifo
}



// EOF