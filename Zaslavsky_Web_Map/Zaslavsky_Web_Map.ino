// Zaslavsky web map //

#include "Adafruit_Arcada.h"

#define BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

Adafruit_Arcada arcada;

uint16_t *framebuffer;

#define WIDTH  160
#define HEIGHT 128
#define SCR (WIDTH * HEIGHT)

uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) { return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3); }

float randomf(float minf, float maxf) {return minf + (trngGetRandomNumber()%(1UL << 31))*(maxf - minf) / (1UL << 31);}

  float x = 0.1f;
  float y = 0.1f;

  float a = 0.19f;
        
  float dt = 0.0f;
    
 void rndrule() {

  memset(framebuffer, 0, 2*SCR);

  x = 0.1f;
  y = 0.0f;
  dt = 0.0f;
  a = randomf(0.0099f, 0.9999f);

}

volatile uint32_t rNum;
  
uint32_t trngGetRandomNumber(void){
  uint32_t i = rNum;
  TRNG->INTENSET.reg = TRNG_INTENSET_DATARDY;
  while (i == rNum);
  TRNG->INTENCLR.reg = TRNG_INTENCLR_DATARDY;
  return rNum;
}

void TRNG_Handler(){
  rNum = TRNG->DATA.reg;
  TRNG->INTFLAG.bit.DATARDY = 1;
}


void setup(void) {

  MCLK->APBCMASK.reg |= MCLK_APBCMASK_TRNG;
  NVIC_SetPriority(TRNG_IRQn, 0);
  NVIC_EnableIRQ(TRNG_IRQn);
  TRNG->INTENCLR.reg = TRNG_INTENCLR_DATARDY;
  TRNG->CTRLA.reg = TRNG_CTRLA_ENABLE;

  if (!arcada.arcadaBegin()) {while (1);}
  arcada.displayBegin();
  
  arcada.setBacklight(255);

  if (! arcada.createFrameBuffer(ARCADA_TFT_WIDTH, ARCADA_TFT_HEIGHT)) {while (1);}
  framebuffer = arcada.getFrameBuffer();
 
}

void loop() {

  if (arcada.readButtons() & ARCADA_BUTTONMASK_A) rndrule();

  uint16_t coll = color565(random(255),random(255),random(255));
 
  for (int i=0;i<4096;i++) {

    float nx = x;
    float ny = y;
        
    x = ny;
    y = -nx - a * sinf(2.0f * PI * ny - dt);

    dt = dt + 0.005f;

    int ax = 80 + (x/4);
    int ay = 64 + (y/4);
      
    if (ax>0 && ax<WIDTH && ay>0 && ay <HEIGHT) framebuffer[ax+ay*ARCADA_TFT_WIDTH] = coll;
    
  }
  
  arcada.blitFrameBuffer(0, 0, true, false);
 
}
