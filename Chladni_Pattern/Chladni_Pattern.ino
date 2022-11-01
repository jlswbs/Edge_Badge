// Chladni patterns //

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

#define WIDTH   80
#define HEIGHT  64
#define SCR     (WIDTH * HEIGHT)
#define SCR2    (ARCADA_TFT_WIDTH * ARCADA_TFT_HEIGHT)

uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) { return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3); }
float randomf(float minf, float maxf) {return minf + (rand()%(1UL << 31))*(maxf - minf) / (1UL << 31);}
  
  float p[WIDTH][HEIGHT];
  float v[WIDTH][HEIGHT];
  float a = 0.01f;
  float b = 0.1f;
  float c = 8.0f;

void rndrule(){

  memset(framebuffer, 0, 2*SCR2);

  a = randomf(0.005f, 0.099f);
  b = randomf(0.049f, 0.199f);
  c = randomf(1.0f, 12.0f);
  
  for (int y = 0; y < HEIGHT; y++){
    for (int x = 0; x < WIDTH; x++){
      p[x][y] = 0;
      v[x][y] = 0;
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

void setup(void){

  MCLK->APBCMASK.reg |= MCLK_APBCMASK_TRNG;
  NVIC_SetPriority(TRNG_IRQn, 0);
  NVIC_EnableIRQ(TRNG_IRQn);
  TRNG->INTENCLR.reg = TRNG_INTENCLR_DATARDY;
  TRNG->CTRLA.reg = TRNG_CTRLA_ENABLE;
 
  srand(trngGetRandomNumber());

  if (!arcada.arcadaBegin()) {while (1);}
  arcada.displayBegin();
  
  arcada.setBacklight(255);

  if (! arcada.createFrameBuffer(ARCADA_TFT_WIDTH, ARCADA_TFT_HEIGHT)) {while (1);}
  framebuffer = arcada.getFrameBuffer();

  rndrule();
  
}

void loop() {

  if (arcada.readButtons() & ARCADA_BUTTONMASK_A) rndrule();

  int frame = millis() / 32;

  v[WIDTH/2][HEIGHT/2] = 0;
  p[WIDTH/2][HEIGHT/2] = (sinf(frame * a) + sinf(frame * b)) * c;
  
  for (int y = 1; y < HEIGHT-1; y++) {
    for (int x = 1; x < WIDTH-1; x++) {
      v[x][y] += (p[x-1][y] + p[x+1][y] + p[x][y-1] + p[x][y+1]) * 0.25f - p[x][y];
    }
  }
  
  for (int y = 0; y < HEIGHT; y++){
    for (int x = 0; x < WIDTH; x++){
      p[x][y] += v[x][y];
      uint8_t coll = 255.0f * fabs(constrain(v[x][y], -1.0f, 1.0f));
      framebuffer[(2*x)+(2*y)*ARCADA_TFT_WIDTH] = color565(coll, coll, coll);
    }
  }

  arcada.blitFrameBuffer(0, 0, true, false);
 
}
