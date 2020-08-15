// Fizzy Cellular Automata //

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

  int Calm = 233;
  int CellIndex = 0;
  int i, j;

  float CellVal[SCR];
    
 void rndrule() {

  memset(framebuffer, 0, 2*SCR);

  for (i = 0; i < SCR; i++) CellVal[i] = trngGetRandomNumber()%256;

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

  for (i = 0; i < WIDTH; i++) {

    for (j = 0; j < HEIGHT; j++) {

      CellIndex = (CellIndex+1)%SCR;

      uint8_t coll = (uint8_t)(CellVal[CellIndex]*4.7f)%128;
      framebuffer[i+j*ARCADA_TFT_WIDTH] = color565(coll<<1, coll<<2, coll<<3);

      int below      = (CellIndex+1)%SCR;
      int above      = (CellIndex+SCR-1)%SCR;
      int left       = (CellIndex+SCR-HEIGHT)%SCR;
      int right      = (CellIndex+HEIGHT)%SCR;
      int aboveright = ((CellIndex-1) + HEIGHT + SCR)%SCR;
      int aboveleft  = ((CellIndex-1) - HEIGHT + SCR)%SCR;
      int belowright = ((CellIndex+1) + HEIGHT + SCR)%SCR;
      int belowleft  = ((CellIndex+1) - HEIGHT + SCR)%SCR;

      float NeighbourMix = powf((CellVal[left]*CellVal[right]*CellVal[above]*CellVal[below]*CellVal[belowleft]*CellVal[belowright]*CellVal[aboveleft]*CellVal[aboveright]),0.125f);

      CellVal[CellIndex] = fmod(sqrtf(CellVal[CellIndex]*NeighbourMix)+0.5f, Calm);

    }

  }
  
  arcada.blitFrameBuffer(0, 0, true, false);
 
}
