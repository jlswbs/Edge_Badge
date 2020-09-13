// Hopalong orbit fractal //

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

float randomf(float minf, float maxf) {return minf + (trngGetRandomNumber()%(1UL << 31))*(maxf - minf) / (1UL << 31);}

  uint16_t coll;
  float a, b, c, x, y, t;
    
 void rndrule() {

  memset(framebuffer, 0, 2*SCR);

  x = 0.1f;
  y = 0.1f;
  t = 0.0f;
  a = 0.0f;
  b = 0.0f;
  c = 0.0f;
  
  float pmax = WIDTH, logpmax = logf(pmax);
  
  a = expf(randomf(-1.0f, 1.0f)*logpmax);
  b = expf(randomf(-1.0f, 1.0f)*logpmax);
  c = randomf(-1.0f, 1.0f)*pmax;

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

  rndrule();
 
}

void loop() {

  if (arcada.readButtons() & ARCADA_BUTTONMASK_A) rndrule();

  coll = trngGetRandomNumber();

  for (int i=0;i<10000;i++) {

    float nx = x;
    float ny = y;

    t = sqrtf(fabs(b*nx-c));
    x = ny - ((nx<0) ? t : -t);
    y = a - nx;

    int ax = (WIDTH/2) + (x/6);
    int ay = (HEIGHT/2) + (y/6);
      
    if (ax>0 && ax<WIDTH && ay>0 && ay <HEIGHT) framebuffer[ax+ay*ARCADA_TFT_WIDTH] = coll;

  }
  
  arcada.blitFrameBuffer(0, 0, true, false);
 
}
