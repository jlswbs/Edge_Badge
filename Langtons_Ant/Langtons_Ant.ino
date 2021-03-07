// Langtons Ant Multi //

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

#define NUMANTS 5
#define ITER 32
  
  uint16_t coll[NUMANTS];
  
  int x[NUMANTS];
  int y[NUMANTS];
  int antsdir[NUMANTS];
    
void rndrule() {

  memset(framebuffer, 0, 2*SCR);

  for(int i = 0; i < NUMANTS; i++){
  
    x[i] = trngGetRandomNumber()%WIDTH;
    y[i] = trngGetRandomNumber()%HEIGHT;
    antsdir[i] = trngGetRandomNumber()%4;
    coll[i] = 1 + trngGetRandomNumber();
    
  }

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

  for(int k = 0; k < ITER; k++){
  
    for(int i = 0; i < NUMANTS; i++){
    
      if (framebuffer[x[i]+ARCADA_TFT_WIDTH*y[i]] > BLACK){ antsdir[i] = antsdir[i] - 1; framebuffer[x[i]+ARCADA_TFT_WIDTH*y[i]] = BLACK; }
      else { antsdir[i] = antsdir[i] + 1; framebuffer[x[i]+ARCADA_TFT_WIDTH*y[i]] = coll[i]; }

      if (antsdir[i] > 3) antsdir[i] = 0;   
      if (antsdir[i] < 0) antsdir[i] = 3;   
    
      if (antsdir[i] == 0) x[i] = x[i] - 1;
      if (antsdir[i] == 1) y[i] = y[i] + 1;
      if (antsdir[i] == 2) x[i] = x[i] + 1;
      if (antsdir[i] == 3) y[i] = y[i] - 1;
    
      if (x[i] > WIDTH-1) x[i] = 0;
      if (x[i] < 0) x[i] = WIDTH-1;
      if (y[i] > HEIGHT-1) y[i] = 0;
      if (y[i] < 0) y[i] = HEIGHT-1;
    
    }
  }

  arcada.blitFrameBuffer(0, 0, true, false);
 
}
