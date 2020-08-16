// Conway's Game of Life - multi rules //

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

uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) { return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3); }

#define WIDTH  160
#define HEIGHT 128
#define SCR (WIDTH * HEIGHT)

  uint8_t universe[WIDTH][HEIGHT];
  uint8_t universePRIME[WIDTH][HEIGHT];

  int cx = 0;
  int cy = 0;
  int cxPrime = 0;
  int cyPrime = 0;
  int neighbors = 0;

  int rule = 0;

    
void rndrule(){

  memset(framebuffer, 0, 2*SCR);

  for (int x = 0; x < WIDTH; x++) {
    for (int y = 0; y < HEIGHT; y++) {
      universePRIME[x][y] = 0;
      universe[x][y] = trngGetRandomNumber()%2; 
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

  rndrule();
 
}

void loop() {

  if (arcada.readButtons() & ARCADA_BUTTONMASK_A) rndrule();
  if (arcada.readButtons() & ARCADA_BUTTONMASK_B) rule = trngGetRandomNumber()%4;

  stepFunc();
    
  arcada.blitFrameBuffer(0, 0, true, false);
 
}

void renderUniverse() {

  uint16_t coll;

  for (int j=0; j<HEIGHT; j++) {
    for (int i=0; i<WIDTH; i++) {
    
      if (universe[i][j] == 0) {
         coll = BLACK;
      } else {
        if (universe[i][j] == 1) {
          coll = WHITE;
        } else {
          coll  = YELLOW;   
        }
      }
      framebuffer[j*ARCADA_TFT_WIDTH+i] = coll;
    }
  }
}

void countNeighbors() {
  neighbors = 0;
  for (int i=-1; i<2; i++) {
    for (int j=-1; j<2; j++) {
      if (i != 0 || j != 0) {
        cxPrime = (cx+i)%WIDTH;
        cyPrime = (cy+j)%HEIGHT;
        if (abs(cxPrime) != cxPrime) {
          cxPrime += WIDTH;
        }
        if (abs(cyPrime) != cyPrime) {
          cyPrime += HEIGHT;
        }
        if (universe[cxPrime][cyPrime] == 1) {
          neighbors++;
        }
      }
    }
    if (universe[cx][cy] > 1) {
      universePRIME[cx][cy] = 0;
    }

    if (rule == 0) {
      if (universe[cx][cy] != 1) { //cell is dead
        if (neighbors == 3) { //birth
          universePRIME[cx][cy] = 1;
        } else {
          universePRIME[cx][cy] = 0;//dead (the same)
        }
      }
      if (universe[cx][cy] == 1) { //cell is alive
        if (neighbors < 2) { //under population
          universePRIME[cx][cy] = 2;
        } else {
          if (neighbors > 3) { //over population
            universePRIME[cx][cy] = 2;
          } else { //sustained
            universePRIME[cx][cy] = 1;
          }
        }
      }
    } else {

      if (rule == 1) {
      if (universe[cx][cy] != 1) { //cell is dead
        if (neighbors == 3) { //birth
          universePRIME[cx][cy] = 1;
        } else {
          universePRIME[cx][cy] = 0;//dead (the same)
        }
      }
      if (universe[cx][cy] == 1) { //cell is alive
        if (neighbors < 2) { //under population
          universePRIME[cx][cy] = 1;
        } else {
          if (neighbors > 3) { //over population
            universePRIME[cx][cy] = 2;
          } else { //sustained
            universePRIME[cx][cy] = 1;
          }
        }
      }
    }

      if (rule == 2) {
        if (neighbors > 4) {
          universePRIME[cx][cy] = 1;
        } else {
          if (neighbors < 4) {
            if (universe[cx][cy] == 1) {
              universePRIME[cx][cy] = 2;
            } else {
              universePRIME[cx][cy] = 0;
            }
          } else {
            if (universe[cx][cy] == 2) {
              universePRIME[cx][cy] = 0;
            } else {
              universePRIME[cx][cy] = universe[cx][cy];
            }
          }
        }
      } else {

        if (rule == 3) {
          if (neighbors < 1) {
            universePRIME[cx][cy] = 0;
          } else {
            if (neighbors < 5) {
              universePRIME[cx][cy] = 2;
            } else {
              universePRIME[cx][cy] = 1;
            }
          }
        }
        
      }
    }
  }
}

void stepFunc() {
  for (int i=0; i<WIDTH; i++) {
    for (int j=0; j<HEIGHT; j++) {
      if (abs(i) == i && i < WIDTH) {
        if (abs(j) == j && j < HEIGHT) {
          cx = i;
          cy = j;
        } else {
          cy = j%HEIGHT;
          if (cy < 0) cy += HEIGHT;
        }
      } else {
        cx = i%WIDTH;
        if (cx < 0) cx += WIDTH;
      }
      countNeighbors();
    }
  }
  
  for (int i=0; i<WIDTH; i++) {
    for (int j=0; j<HEIGHT; j++) universe[i][j] = universePRIME[i][j];
  }
  
  renderUniverse();
}
