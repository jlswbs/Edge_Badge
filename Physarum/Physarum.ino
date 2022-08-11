// Physarum growth //

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

#define ITER  5000
#define NUM   8

  uint16_t coll[NUM];
  uint16_t grid[WIDTH][HEIGHT];
  uint16_t image;
  int t, q;


void rndseed(){

  memset(framebuffer, 0, 2*SCR);

  for (int i = 0; i < NUM; i++) coll[i] = rand();

  for (int y = 0; y < HEIGHT; y++){  
    for (int x = 0; x < WIDTH; x++){
      
      if(x == 0 || x == 1 || x == WIDTH-2 || x == WIDTH-1 || y == 0 || y == 1 || y == HEIGHT-2 || y == HEIGHT-1) grid[x][y] = 1;
      else grid[x][y] = 0;

    }
  }
  
  for (int i = 1; i < NUM; i++){
    
    int x = 2 * (5 + trngGetRandomNumber()%(WIDTH/2)-5);
    int y = 2 * (5 + trngGetRandomNumber()%(HEIGHT/2)-5);
    if(grid[x][y] == 0) grid[x][y] = 1000+(i*100);

  }

}

void nextstep(){

  for (int i = 0; i < ITER; i++){
  
    int x = 2 * (1 + trngGetRandomNumber()%(WIDTH/2)-1);
    int y = 2 * (1 + trngGetRandomNumber()%(HEIGHT/2)-1);
    
    if(grid[x][y] >= 100 && grid[x][y] < 1000){
      
      q = grid[x][y]/100;
      int p = grid[x][y] - (q*100);
      
      if(p < 30){
        
        t = 1 + trngGetRandomNumber()%5;
        if(t == 1 && grid[x+2][y] == 0){ grid[x+2][y] = q*100; grid[x+1][y] = q*100; } 
        if(t == 2 && grid[x][y+2] == 0){ grid[x][y+2] = q*100; grid[x][y+1] = q*100; } 
        if(t == 3 && grid[x-2][y] == 0){ grid[x-2][y] = q*100; grid[x-1][y] = q*100; } 
        if(t == 4 && grid[x][y-2] == 0){ grid[x][y-2] = q*100; grid[x][y-1] = q*100; } 
        grid[x][y] = grid[x][y] + 1;
        
      } else {
        
        t = 0;
        if(grid[x+1][y] > 1) t = t + 1;
        if(grid[x][y+1] > 1) t = t + 1;
        if(grid[x-1][y] > 1) t = t + 1;
        if(grid[x][y-1] > 1) t = t + 1;
        if(t <= 1){
          grid[x][y] = 9100;
          grid[x+1][y] = 0;
          grid[x][y+1] = 0;
          grid[x-1][y] = 0;
          grid[x][y-1] = 0; 
        }
      }      
    }
    
    if(grid[x][y] >= 1000 && grid[x][y] < 2000){
      
      q = (grid[x][y]/100)-10;
      if(grid[x+2][y] == 0){ grid[x+2][y] = q*100; grid[x+1][y] = q*100; }
      if(grid[x][y+2] == 0){ grid[x][y+2] = q*100; grid[x][y+1] = q*100; }
      if(grid[x-2][y] == 0){ grid[x-2][y] = q*100; grid[x-1][y] = q*100; }
      if(grid[x][y-2] == 0){ grid[x][y-2] = q*100; grid[x][y-1] = q*100; }
    
    }
    
    if(grid[x][y] >= 9000){
      
      grid[x][y] = grid[x][y] - 1;
      if(grid[x][y] < 9000) grid[x][y] = 0;
    
    }
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

  nextstep();

  for (int y = 0; y < HEIGHT; y++){  
    for (int x = 0; x < WIDTH; x++){
    
      if(grid[x][y] >= 100 && grid[x][y] < 1000){
        q = (grid[x][y]/100)%NUM;
        image = coll[q];    
      } else image = BLACK;
      
      framebuffer[x+ARCADA_TFT_WIDTH*y] = image;
      
    }
  }
    
  arcada.blitFrameBuffer(0, 0, true, false);
 
}
