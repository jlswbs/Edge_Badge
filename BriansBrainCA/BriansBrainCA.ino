// Brian's Brain cellular automata //

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

  #define DENSITY     7
  #define READY       0
  #define REFRACTORY  1
  #define FIRING      2

  uint8_t world[WIDTH][HEIGHT];
  uint8_t temp[WIDTH][HEIGHT];


uint8_t weighted_randint(uint8_t true_weight){
  
    uint8_t choice = trngGetRandomNumber() % 10;
    
    if (choice > true_weight) return 1;
    else return 0;
}


uint8_t count_neighbours(uint8_t world[WIDTH][HEIGHT], uint8_t x_pos, uint8_t y_pos)
{
    int x, y, cx, cy, cell;
    int count = 0;

    for (y = -1; y < 2; y++) {
        for (x = -1; x < 2; x++) {
            cx = x_pos + x;
            cy = y_pos + y;
            if ( (0 <= cx && cx < WIDTH) && (0 <= cy && cy < HEIGHT)) {
                cell = world[x_pos + x][y_pos + y];
                if (cell == FIRING) count ++;
            }
        }
    }
  return count;
}


void apply_rules(uint8_t world[WIDTH][HEIGHT])
{
  uint8_t x, y, cell, neighbours;

  memcpy(temp, world, SCR);

  for (y = 0; y < HEIGHT; y++) {
    for (x = 0; x < WIDTH; x++){
      cell = temp[x][y];          
      if (cell == READY) {
        neighbours = count_neighbours(temp, x, y);
        if (neighbours == 2) world[x][y] = FIRING; }
      else if (cell == FIRING) world[x][y] = REFRACTORY;
      else world[x][y] = READY;
    }
  }
}

void populate(){

  memset(framebuffer, 0, 2*SCR);
  
  uint8_t x, y, r;
  
  for (y = 0; y < HEIGHT; y++) {
    for (x = 0; x < WIDTH; x++){
      r = weighted_randint(DENSITY);
      if (r == 1) world[x][y] = FIRING;
      else world[x][y] = READY;
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

  populate();
 
}

void loop() {

  if (arcada.readButtons() & ARCADA_BUTTONMASK_A) populate();
  
  apply_rules(world);
    
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++){
      if (world[x][y] == FIRING) framebuffer[y * ARCADA_TFT_WIDTH + x] = WHITE;    
      else if (world[x][y] == REFRACTORY) framebuffer[y * ARCADA_TFT_WIDTH + x] = YELLOW;
      else framebuffer[y * ARCADA_TFT_WIDTH + x] = BLACK; 
    }
  }

  arcada.blitFrameBuffer(0, 0, true, false);
 
}
