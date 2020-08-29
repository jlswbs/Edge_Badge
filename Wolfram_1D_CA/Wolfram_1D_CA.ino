// Wolfram 1D cellular automata //

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

  bool state[WIDTH];
  bool newst[WIDTH];

  int x, y, k;
  bool rules[8] = {0, 1, 1, 1, 1, 0, 0, 0};
  bool center = false;
    
 void rndrule() {

  memset(framebuffer, 0, 2*SCR);
  memset(state, 0, WIDTH);
  
  if (center) state[WIDTH/2] = 1;
  else for (x=0;x<WIDTH;x++) state[x] = trngGetRandomNumber()%2;

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

  if (arcada.readButtons() & ARCADA_BUTTONMASK_A) { rndrule(); for (x=0;x<8;x++) rules[x] = trngGetRandomNumber()%2; }
  if (arcada.readButtons() & ARCADA_BUTTONMASK_B) rndrule();
  if (arcada.readButtons() & ARCADA_BUTTONMASK_RIGHT) { rndrule(); center = !center; }

  for (y = 0; y < HEIGHT; y++) {

    memset (newst, 0, WIDTH);

    for (x=0;x<WIDTH;x++) {     
      k = 4*state[(x-1+WIDTH)%WIDTH] + 2*state[x] + state[(x+1)%WIDTH];
      newst[x] = rules[k];
    }

    memcpy (state, newst, WIDTH);
  
    for (x = 0; x < WIDTH; x++) {
      if (state[x] == 1) framebuffer[x+y*ARCADA_TFT_WIDTH] = WHITE;       
      else framebuffer[x+y*ARCADA_TFT_WIDTH] = BLACK;
    }

    delayMicroseconds(200);

  }

  arcada.blitFrameBuffer(0, 0, true, false);
 
}
