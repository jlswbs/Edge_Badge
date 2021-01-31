// Turmites cellular automata //

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

  int posx,posy;
  int oldposx, oldposy;
  int state;
  int dir;
  int world[WIDTH][HEIGHT];
  int last_filled;
  int current_col;
  int next_col[4][4];
  int next_state[4][4];
  int directions[4][4];
    
void rndrule() {

  memset(framebuffer, 0, 2*SCR);
  
  state = trngGetRandomNumber()%4;
  dir = 0;
  posx = WIDTH/2;
  posy = HEIGHT/2;
  
  for(int j=0; j<4; j++){   
    for(int i=0; i<4; i++){         
      next_col[i][j] = trngGetRandomNumber()%4;
      next_state[i][j] = trngGetRandomNumber()%4;
      directions[i][j] = trngGetRandomNumber()%8;
    }   
  }

  for (int j=0; j < HEIGHT; j++){ for (int i = 0; i < WIDTH; i++) world[i][j]=0; } 

  world[posx][posy] = trngGetRandomNumber()%4;

}

void move_turmite(){
  
  int cols = world[posx][posy];
  
  oldposx = posx;
  oldposy = posy;
  current_col = next_col[cols][state];
  world[posx][posy] = next_col[cols][state];
  state = next_state[cols][state];    

  dir = (dir + directions[cols][state]) % 8;

  switch(dir){
    case 0: posy--; break;
    case 1: posy--; posx++; break;
    case 2: posx++; break;
    case 3: posx++; posy++; break;
    case 4: posy++; break;
    case 5: posy++; posx--; break;
    case 6: posx--; break;
    case 7: posx--; posy--; break;
  }

  if(posy < 0) posy = HEIGHT-1;
  if(posy >= HEIGHT) posy = 0;
  if(posx < 0) posx = WIDTH-1;
  if(posx >= WIDTH) posx=0;
  
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

  for(int i=0; i<50; i++){
    
    move_turmite();
    
    switch(current_col){
      case 0: framebuffer[oldposx + oldposy * ARCADA_TFT_WIDTH] = RED; break;
      case 1: framebuffer[oldposx + oldposy * ARCADA_TFT_WIDTH] = GREEN; break;
      case 2: framebuffer[oldposx + oldposy * ARCADA_TFT_WIDTH] = BLUE; break;
      case 3: framebuffer[oldposx + oldposy * ARCADA_TFT_WIDTH] = WHITE; break;
    }
    
  }

  arcada.blitFrameBuffer(0, 0, true, false);
 
}
