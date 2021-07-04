// YellowRed 1D 3-state cellular automata //

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

#define ITER  10

  uint16_t coll;
  uint8_t parent[WIDTH]; 
  uint8_t child[WIDTH];
  int count;


void rndseed(){

  memset(framebuffer, 0, 2*SCR);

  for(int x = 0; x < WIDTH; x++){
  
    parent[x] = trngGetRandomNumber()%3;
    child[x] = 0;
    
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

  rndseed();
 
}

void loop() {

  if (arcada.readButtons() & ARCADA_BUTTONMASK_A) rndseed();

  for(int q=0; q<ITER; q++){
  
  for (int y = 0; y < HEIGHT; y++) {
 
    for (int x = 0; x < WIDTH; x++) {
          
      if (x == 0) count = parent[WIDTH-1] + parent[0] + parent[1];
      else if (x == WIDTH-1) count = parent[WIDTH-2] + parent[WIDTH-1] + parent[0];
      else count = parent[x-1] + parent[x] + parent[x+1];        
            
      if (count == 1 || count == 2 || count == 5) child[x] = 0;
      if (count == 4 || count == 3 ) child[x] = 1;
      if (count == 0 || count == 6) child[x] = 2;
               
      if (child[x] == 0) coll = BLACK;
      if (child[x] == 1) coll = YELLOW;
      if (child[x] == 2) coll = RED;

      framebuffer[x+ARCADA_TFT_WIDTH*y] = coll;
                                                   
    }
                                                 
    memcpy(parent, child, WIDTH);
    
  }

  }

  arcada.blitFrameBuffer(0, 0, true, false);
 
}
