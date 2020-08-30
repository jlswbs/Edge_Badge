// Diffusion-Reaction 1D //

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

float randomf(float minf, float maxf) {return minf + (trngGetRandomNumber()%(1UL << 31))*(maxf - minf) / (1UL << 31);}

uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) { return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3); }
  
  int i, j;

  float A[WIDTH]; 
  float I[WIDTH];
  float D2A[WIDTH]; 
  float D2I[WIDTH];

  float p[6] = {0.5f, 2.0f, 2.0f, 2.0f, 1.0f, 0.0f};
  float dt = 0.05f;
  bool color = false;

 void rules() {

  p[0] = randomf(0.0f,1.0f);
  p[1] = randomf(0.0f,15.0f);
  p[2] = randomf(0.0f,4.0f);
  p[3] = randomf(0.0f,15.0f);
  p[4] = randomf(0.0f,4.0f);
  p[5] = randomf(0.0f,2.0f);
  
}
    
 void rndrule() {

  memset(framebuffer, 0, 2*SCR);
   
  for(i=0;i<WIDTH; i++) {
    
   A[i] = randomf(0.0f, 1.0f);
   I[i] = randomf(0.0f, 1.0f);
    
  }
  
  i = 0;
  j = 0;

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

  if (arcada.readButtons() & ARCADA_BUTTONMASK_A) { rules(); rndrule(); }
  if (arcada.readButtons() & ARCADA_BUTTONMASK_B) rndrule();
  if (arcada.readButtons() & ARCADA_BUTTONMASK_RIGHT) color = !color;
  
  for(j=0;j<HEIGHT; j++) {
    
    for(i=1;i<WIDTH-1; i++) {
      
      D2A[i] = A[i-1] + A[i+1] - 2.0f * A[i];
      D2I[i] = I[i-1] + I[i+1] - 2.0f * I[i];
 
    }
  
    D2A[0] = A[1] - A[0]; 
    D2I[0] = I[1] - I[0]; 

    for(i=0;i<WIDTH; i++) {
    
      A[i] = A[i] + dt * (5.0f * A[i] *A[i] * A[i] / (I[i] * I[i]) + p[0] - p[1] * A[i] + p[2] * D2A[i]);
      I[i] = I[i] + dt * (A[i] * A[i] *A[i] - p[3] * I[i] + p[4] * D2I[i] + p[5]);  
      
      uint8_t coll = 250 - (50 * A[i]);
      if (color) framebuffer[i+j*ARCADA_TFT_WIDTH] = color565(coll<<1, coll<<2, coll<<3);
      else framebuffer[i+j*ARCADA_TFT_WIDTH] = color565(coll, coll, coll);
 
    }

  }
  
  arcada.blitFrameBuffer(0, 0, true, false);
 
}
