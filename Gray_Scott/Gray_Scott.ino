// Gray-Scott Reaction-Diffusion //

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

float randomf(float minf, float maxf) {return minf + (trngGetRandomNumber()%(1UL << 31))*(maxf - minf) / (1UL << 31);}

  int i, j;
  
  float diffU = 0.16f;
  float diffV = 0.08f;
  float paramF = 0.035f;
  float paramK = 0.06f;

  float U[WIDTH][HEIGHT];
  float V[WIDTH][HEIGHT];

  float dU[WIDTH][HEIGHT];
  float dV[WIDTH][HEIGHT];
    
 void rndrule() {

  diffU = randomf(0.0999f, 0.1999f);
  diffV = randomf(0.0749f, 0.0849f);
  paramF = randomf(0.0299f, 0.0399f);
  paramK = randomf(0.0549f, 0.0649f);

  memset(framebuffer, 0, 2*SCR);
  
  for (int j = 0; j < HEIGHT; j++) { 
    for (int i = 0; i < WIDTH; i++) {    
      U[i][j] = 1.0f;
      V[i][j] = 0.0f;       
    }
  }

  for(int j=(HEIGHT-3)/2;j<(HEIGHT+3)/2;++j){      
    for(int i=(WIDTH-3)/2;i<(WIDTH+3)/2;++i){          
      U[i][j] = 0.5f * (1.0f + randomf(-1.0f, 1.0f));
      V[i][j] = 0.25f * (1.0f + randomf(-1.0f, 1.0f));     
    }
  }

}

void timestep(float F, float K, float diffU, float diffV) {

  for (int j = 1; j < HEIGHT-1; j++) {
    for (int i = 1; i < WIDTH-1; i++) {
            
      float u = U[i][j];
      float v = V[i][j];
          
      float uvv = u * v * v;     
       
      float lapU = (U[i-1][j] + U[i+1][j] + U[i][j-1] + U[i][j+1] - 4.0f * u);
      float lapV = (V[i-1][j] + V[i+1][j] + V[i][j-1] + V[i][j+1] - 4.0f * v);
          
      dU[i][j] = diffU*lapU - uvv + F*(1.0f-u);
      dV[i][j] = diffV*lapV + uvv - (K+F)*v;
          
    }
  }
      
  for (int j = 0; j < HEIGHT; j++){   
    for (int i= 0; i < WIDTH; i++) {
         
      U[i][j] += dU[i][j];
      V[i][j] += dV[i][j];
      
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

  for (int k = 0; k < 50; k++) timestep(paramF, paramK, diffU, diffV);
  
  for(int j=0;j<HEIGHT;++j){
    for(int i=0;i<WIDTH;++i){       
      uint8_t col = 255 * U[i][j];
      framebuffer[(2*i)+(2*j)*ARCADA_TFT_WIDTH] = color565(col<<1, col<<2, col<<3);        
    }
  }
  
  arcada.blitFrameBuffer(0, 0, true, false);
 
}
