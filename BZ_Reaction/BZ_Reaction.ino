// Belousov-Zabotinsky reaction //

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

  float a [WIDTH][HEIGHT][2];
  float b [WIDTH][HEIGHT][2];
  float c [WIDTH][HEIGHT][2];

  bool p = 0, q = 1;
  int x, y, i, j;

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

void rndrule(){

  memset(framebuffer, 0, 2*SCR);

  for (y = 0; y < HEIGHT; y++) {
    
    for (x = 0; x < WIDTH; x++) {
      
      a[x][y][p] = (trngGetRandomNumber()%10)/10.0f;
      b[x][y][p] = (trngGetRandomNumber()%10)/10.0f;
      c[x][y][p] = (trngGetRandomNumber()%10)/10.0f;
      
    }
  }
  
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

  for (y = 0; y < HEIGHT; y++) {

    for (x = 0; x < WIDTH; x++) {
  
      float c_a = 0;
      float c_b = 0;
      float c_c = 0;
    
      for (i = x - 1; i <= x+1; i++) {

        for (j = y - 1; j <= y+1; j++) {

          c_a += a[(i+WIDTH)%WIDTH][(j+HEIGHT)%HEIGHT][p];
          c_b += b[(i+WIDTH)%WIDTH][(j+HEIGHT)%HEIGHT][p];
          c_c += c[(i+WIDTH)%WIDTH][(j+HEIGHT)%HEIGHT][p];
        }
      }
      
      c_a /= 9.0f;
      c_b /= 9.0f;
      c_c /= 9.0f;

      if (c_a + c_a * (c_b - c_c) <= 1) a[x][y][q] = c_a + c_a * (c_b - c_c);
      if (c_b + c_b * (c_c - c_a) <= 1) b[x][y][q] = c_b + c_b * (c_c - c_a);
      if (c_c + c_c * (c_a - c_b) <= 1) c[x][y][q] = c_c + c_c * (c_a - c_b);
    
      uint8_t l = 256 * a[x][y][q];

      framebuffer[(2*y) * ARCADA_TFT_WIDTH + (2*x)] = color565(l, l, l);
    
    }
  }

  if (p == 0) { p = 1; q = 0; } else { p = 0; q = 1; }

  arcada.blitFrameBuffer(0, 0, true, false);
 
}
