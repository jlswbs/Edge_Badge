// Super Langtons Ant //

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

#define LENGHT 32

  uint16_t state[WIDTH][HEIGHT];
  uint16_t antX = WIDTH/2;
  uint16_t antY = HEIGHT/2;
  uint16_t direction;
  uint16_t stateCount;
  bool type[LENGHT];
  uint16_t stateCols[LENGHT];
    
void rndrule() {

  memset(framebuffer, 0, 2*SCR);

  antX = WIDTH/2;
  antY = HEIGHT/2;
  
  stateCount = 2 + trngGetRandomNumber()%(LENGHT-2);
  direction = trngGetRandomNumber()%4;

  for(int i = 0; i < stateCount; i++) type[i] = trngGetRandomNumber()%2;
  for(int i = 0; i < stateCount; i++) stateCols[i] = trngGetRandomNumber();

  for (int j=0; j < HEIGHT; j++){ for (int i = 0; i < WIDTH; i++) state[i][j]=0; }

}

void turn(int angle){
  
  if(angle == 0){
    if(direction == 0){
      direction = 3;
    } else {
      direction--;
    }
  } else {
    if(direction == 3){
      direction = 0;
    } else {
      direction++;
    }
  }
}

void move(){
  
  if(antY == 0 && direction == 0){
    antY = HEIGHT-1;
  } else {
    if(direction == 0 ){
      antY--;
    }
  }
  if(antX == WIDTH-1 && direction == 1){
    antX = 0;
  } else {
    if(direction == 1){
      antX++;
    }
  }
  if(antY == HEIGHT-1 && direction == 2){
   antY = 0; 
  } else {
    if(direction == 2){
      antY++;
    }
  }
  if(antX == 0 && direction == 3){
    antX = WIDTH-1;
  } else {
    if(direction == 3){
      antX--;
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

  for(int i = 0; i < 500; i++) {

    move();
    turn(type[(state[antX][antY]%stateCount)]);
    state[antX][antY]++;
    framebuffer[antX+antY*ARCADA_TFT_WIDTH] = stateCols[(state[antX][antY]%stateCount)];

  }

  arcada.blitFrameBuffer(0, 0, true, false);
 
}
