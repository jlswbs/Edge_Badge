// Water simulation //

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

#define WIDTH  80
#define HEIGHT 64
#define SCR (WIDTH * HEIGHT)

uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) { return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3); }

  uint8_t terrain[WIDTH][HEIGHT];
  float water[WIDTH][HEIGHT];
  float energy[WIDTH][HEIGHT];
  float new_water[WIDTH][HEIGHT];
  float new_energy[WIDTH][HEIGHT];

void init_terrain()
{   
  for(int j = 0; j < HEIGHT; j++){   
    for(int i = 0; i < WIDTH; i++) terrain[i][j] = trngGetRandomNumber()%10;
  }

  for(int j = 0; j < HEIGHT; j++){   
    for(int i = 0; i < WIDTH; i++){      
      water[i][j] = 0;
      energy[i][j] = 0;  
    }
  }
  
  int L = 10;
  int H = 30;
    
  for(int j = HEIGHT/2 -L/2; j < HEIGHT/2 + L/2; j++){  
    for(int i = WIDTH/2 -L/2; i < WIDTH/2 + L/2; i++)  water[i][j] = H;
  } 
}

void drawTerrain()
{  
  for(int j = 0; j < HEIGHT; j++){
    for(int i = 0; i < WIDTH; i++){
      for(int k = 0; k < terrain[i][j]; k++) framebuffer[(2*i)+(2*j)*ARCADA_TFT_WIDTH] = color565(255/k, 255/k, 255/k);
    }
  }
}

void drawWater()
{  
  for(int j = 0; j < HEIGHT; j++){
    for(int i = 0; i < WIDTH; i++) {    
      for(int k = terrain[i][j]; k < terrain[i][j]+int(round(water[i][j])); k++) framebuffer[(2*i)+(2*j)*ARCADA_TFT_WIDTH] = color565(150, 200, 250);
    }
  }
}


void updateWater()
{

  float friction = 0.125;
  
  for(int j = 0; j < HEIGHT; j++){
    for(int i = 0; i < WIDTH; i++){ 
      new_water[i][j] = water[i][j];
      new_energy[i][j] = energy[i][j];
    }
  }
 
  for(int j = 0; j < HEIGHT; j++){
    for(int i = 0; i < WIDTH; i++) {
      float Lp,Rp,Bp,Fp;
      Lp = Rp = Bp = Fp = 0;

    if(i > 0)    Lp = terrain[i-1][j]+water[i-1][j]+energy[i-1][j];
    if(i < WIDTH-1)  Rp = terrain[i+1][j]+water[i+1][j]-energy[i+1][j];
    if(j > 0)    Bp = terrain[i][j-1]+water[i][j-1]+energy[i][j-1];
    if(j < HEIGHT-1)  Fp = terrain[i][j+1]+water[i][j+1]-energy[i][j+1];
    
    if(i > 0 && terrain[i][j]+water[i][j]-energy[i][j] > Lp)
    {
      float flow = min(water[i][j], terrain[i][j]+water[i][j]-energy[i][j] - Lp)/16;
      new_water[i-1][j]  += flow;
      new_water[i][j]    += -flow;
      new_energy[i-1][j] *= (1-friction);
      new_energy[i-1][j] += -flow;
    }
    
    if(i < WIDTH-1 && terrain[i][j]+water[i][j]+energy[i][j] > Rp)
    {
      float flow = min(water[i][j], terrain[i][j]+water[i][j]+energy[i][j] - Rp)/16;
      new_water[i+1][j]  += flow;
      new_water[i][j]    += -flow;
      new_energy[i+1][j] *= (1-friction);
      new_energy[i+1][j] += flow;
    }
    
    if(j > 0 && terrain[i][j]+water[i][j]-energy[i][j] > Bp)
    {
      float flow = min(water[i][j], terrain[i][j]+water[i][j]-energy[i][j] - Bp)/16;
      new_water[i][j-1]  += flow;
      new_water[i][j]    += -flow;
      new_energy[i][j-1] *= (1-friction);
      new_energy[i][j-1] += -flow;
    }
    
    if(j < HEIGHT-1 && terrain[i][j]+water[i][j]+energy[i][j] > Fp)
    {
      float flow = min(water[i][j], terrain[i][j]+water[i][j]+energy[i][j] - Fp)/16;
      new_water[i][j+1]  += flow;
      new_water[i][j]    += -flow;
      new_energy[i][j+1] *= (1-friction);
      new_energy[i][j+1] += flow;
    }
  }
  
  }
  
  for(int j = 0; j < HEIGHT; j++){
    for(int i = 0; i < WIDTH; i++){ 
      water[i][j] = new_water[i][j];
      energy[i][j] = new_energy[i][j];
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

  init_terrain();
 
}

void loop() {

  if (arcada.readButtons() & ARCADA_BUTTONMASK_A) init_terrain();

  memset(framebuffer, 0, 2*SCR);

  drawTerrain();
  drawWater();
  
  for(int i=0; i < 10; i++) updateWater();

  arcada.blitFrameBuffer(0, 0, true, false);
 
}
