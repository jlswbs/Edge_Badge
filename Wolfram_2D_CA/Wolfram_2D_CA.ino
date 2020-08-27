// Wolfram 2D cellular automata //

#include "Adafruit_Arcada.h"

#define BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF
#define DARKGREY        0x7BEF
#define NAVY            0x000F

Adafruit_Arcada arcada;

uint16_t *framebuffer;

#define WIDTH  160
#define HEIGHT 128
#define SCR (WIDTH * HEIGHT)

  bool state[SCR];
  bool newst[SCR];

  uint8_t i, x, y;
  
  bool rules[10] = {0,0,1,1,1,1,0,0,0,0};
    
 void rndrule() {

  rules[0] = trngGetRandomNumber()%2;
  rules[1] = trngGetRandomNumber()%2;
  rules[2] = trngGetRandomNumber()%2;
  rules[3] = trngGetRandomNumber()%2;
  rules[4] = trngGetRandomNumber()%2;
  rules[5] = trngGetRandomNumber()%2;
  rules[6] = trngGetRandomNumber()%2;
  rules[7] = trngGetRandomNumber()%2;
  rules[8] = trngGetRandomNumber()%2;
  rules[9] = trngGetRandomNumber()%2;

  memset(framebuffer, 0, 2*SCR);
  
  memset(newst, 0, SCR);
  memset(state, 0, SCR);
  
  state[(WIDTH/2)+(HEIGHT/2)*WIDTH] = 1;
  state[(WIDTH/2)+((HEIGHT/2)-1)*WIDTH] = 1;
  state[((WIDTH/2)-1)+((HEIGHT/2)-1)*WIDTH] = 1;
  state[((WIDTH/2)-1)+(HEIGHT/2)*WIDTH] = 1;

}

uint8_t neighbors(uint8_t x, uint8_t y) {
  
  uint8_t result = 0;

  if(y > 0 && state[x+(y-1)*WIDTH] == 1) result = result + 1;
  if(x > 0 && state[(x-1)+y*WIDTH] == 1) result = result + 1;
  if(x < WIDTH-1 && state[(x+1)+y*WIDTH] == 1) result = result + 1;
  if(y < HEIGHT-1 && state[x+(y+1)*WIDTH] == 1) result = result + 1;
  
  return result;
 
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

  for(x = 0; x < WIDTH; x++){
    
    for(y = 0; y < HEIGHT; y++){
           
      uint8_t totalNeighbors = neighbors(x,y);
            
      if(state[x+y*WIDTH] == 0 && totalNeighbors == 0)      {newst[x+y*WIDTH] = rules[0]; framebuffer[x+y*ARCADA_TFT_WIDTH] = WHITE;}
      else if(state[x+y*WIDTH] == 1 && totalNeighbors == 0) {newst[x+y*WIDTH] = rules[1]; framebuffer[x+y*ARCADA_TFT_WIDTH] = RED;}
      else if(state[x+y*WIDTH] == 0 && totalNeighbors == 1) {newst[x+y*WIDTH] = rules[2]; framebuffer[x+y*ARCADA_TFT_WIDTH] = GREEN;}
      else if(state[x+y*WIDTH] == 1 && totalNeighbors == 1) {newst[x+y*WIDTH] = rules[3]; framebuffer[x+y*ARCADA_TFT_WIDTH] = BLUE;}
      else if(state[x+y*WIDTH] == 0 && totalNeighbors == 2) {newst[x+y*WIDTH] = rules[4]; framebuffer[x+y*ARCADA_TFT_WIDTH] = YELLOW;}
      else if(state[x+y*WIDTH] == 1 && totalNeighbors == 2) {newst[x+y*WIDTH] = rules[5]; framebuffer[x+y*ARCADA_TFT_WIDTH] = DARKGREY;}
      else if(state[x+y*WIDTH] == 0 && totalNeighbors == 3) {newst[x+y*WIDTH] = rules[6]; framebuffer[x+y*ARCADA_TFT_WIDTH] = MAGENTA;}
      else if(state[x+y*WIDTH] == 1 && totalNeighbors == 3) {newst[x+y*WIDTH] = rules[7]; framebuffer[x+y*ARCADA_TFT_WIDTH] = CYAN;}
      else if(state[x+y*WIDTH] == 0 && totalNeighbors == 4) {newst[x+y*WIDTH] = rules[8]; framebuffer[x+y*ARCADA_TFT_WIDTH] = NAVY;}
      else if(state[x+y*WIDTH] == 1 && totalNeighbors == 4) {newst[x+y*WIDTH] = rules[9]; framebuffer[x+y*ARCADA_TFT_WIDTH] = BLACK;}
      
    }
  }
 
  memcpy(state, newst, SCR);
  
  arcada.blitFrameBuffer(0, 0, true, false);
 
}
