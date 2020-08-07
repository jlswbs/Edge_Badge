// StarWars cellular automata //

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

  uint8_t current [WIDTH][HEIGHT];
  uint8_t next [WIDTH][HEIGHT];
  uint8_t alive_counts [WIDTH][HEIGHT];
  uint8_t tmp[WIDTH][HEIGHT];
  
  uint8_t ALIVE = 3;
  uint8_t DEATH_1 = 2;
  uint8_t DEATH_2 = 1;
  uint8_t DEAD = 0;

  uint8_t x, y, nx, ny;

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
  
  step();

  memset(framebuffer, 0, 2*SCR);

  draw_type(50, 100, YELLOW);
  draw_type(2, 49, BLUE);
  draw_type(0, 1, WHITE);

  arcada.blitFrameBuffer(0, 0, true, false);
 
}

void draw_type(int min_alive, int max_alive, uint16_t s)
{
  for (x = 0; x < WIDTH; x++) {
      
    for (y = 0; y < HEIGHT; y++) {
    
      int self = current[x][y];
        
      if (self == DEAD) continue;
        
      int alive = alive_counts[x][y];
    
      if (alive < min_alive || alive > max_alive) continue;
      
      framebuffer[y * ARCADA_TFT_WIDTH + x] = s;
   
      }
   }
}

void rndrule() {

  memset(framebuffer, 0, 2*SCR);

  for (x = 0; x < WIDTH; x++) {
    
    for (y = 0; y < HEIGHT; y++) {
          
      current[x][y] = (trngGetRandomNumber()%10) < 2 ? ALIVE : DEAD;
      alive_counts[x][y] = 0;
           
    }
  }
}

void step() {
 
  for (x = 0; x < WIDTH; x++) {
  
    for (y = 0; y < HEIGHT; y++) {
    
      int count = 0;
      int next_val;
    
      int mx = WIDTH-1;
      int my = HEIGHT-1;
    
      int self = current[x][y];
    
      for (nx = x-1; nx <= x+1; nx++) {
  
        for (ny = y-1; ny <= y+1; ny++) {
    
          if (nx == x && ny == y) continue;
      
          if (current[wrap(nx, mx)][wrap(ny, my)] == ALIVE) count++;
      
        }   
      }  

    int neighbors = count;
    
    if (self == ALIVE) next_val = ((neighbors == 3) || (neighbors == 4) || (neighbors == 5)) ? ALIVE : DEATH_1;
  
    else if (self == DEAD) next_val = (neighbors == 2) ? ALIVE : DEAD;
  
    else next_val = self-1;
   
    next[x][y] = next_val;
  
        if (next_val == ALIVE) {
    
      if ((alive_counts[x][y]+1) <= 100) alive_counts[x][y] = (int)(alive_counts[x][y]+1);
        
      } else if (next_val == DEAD) alive_counts[x][y] = 0;
    }
    }
    
    for (x = 0; x < WIDTH; x++) {
  
    for (y = 0; y < HEIGHT; y++) {
        
      tmp[x][y] = current[x][y];
      current[x][y] = next[x][y];
      next[x][y] = tmp[x][y];
    
    }
  }
}
  
int wrap(int v, int m) {

    if (v < 0) return v + m;

    else if (v >= m) return v - m;

    else return v;
}
