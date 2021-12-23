// 5-state 1D cellular automata //

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

#define RND   11
#define VAL   15
#define COL   5
#define ITER  8

  uint16_t coll;
  uint8_t parent[WIDTH]; 
  uint8_t child[WIDTH];
  int count;
  int a[VAL];
    
void rndrule() {

  memset(framebuffer, 0, 2*SCR);

  for(int i = 0; i < VAL; i++) a[i] = trngGetRandomNumber()%RND;

  for(int i = 0; i < WIDTH; i++){
  
    parent[i] = trngGetRandomNumber()%COL;
    child[i] = 0;

  }

}

void rndseed(){

  memset(framebuffer, 0, 2*SCR);

  for(int i = 0; i < WIDTH; i++){
  
    parent[i] = trngGetRandomNumber()%COL;
    child[i] = 0;
    
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
  if (arcada.readButtons() & ARCADA_BUTTONMASK_B) rndseed();

  for(int k=0; k<ITER; k++){
  
  for (int y = 0; y < HEIGHT; y++) {
 
    for (int x = 0; x < WIDTH; x++) {
          
      if (x == 0) count = parent[WIDTH-1] + parent[0] + parent[1];
      else if (x == WIDTH-1) count = parent[WIDTH-2] + parent[WIDTH-1] + parent[0];
      else count = parent[x-1] + parent[x] + parent[x+1];        
            
      if(count == a[0] || count == a[1] || count == a[2]) child[x] = 0;
      if(count == a[3] || count == a[4] || count == a[5]) child[x] = 1;
      if(count == a[6] || count == a[7] || count == a[8]) child[x] = 2;
      if(count == a[9] || count == a[10] || count == a[11]) child[x] = 3;
      if(count == a[12] || count == a[13] || count == a[14]) child[x] = 4;
               
      if (child[x] == 0) coll = RED;
      if (child[x] == 1) coll = GREEN;
      if (child[x] == 2) coll = BLUE;
      if (child[x] == 3) coll = YELLOW;
      if (child[x] == 4) coll = WHITE;

      framebuffer[x+ARCADA_TFT_WIDTH*y] = coll;
                                                   
    }
                                                 
      memcpy(parent, child, WIDTH);
    }

  }

  arcada.blitFrameBuffer(0, 0, true, false);
 
}
