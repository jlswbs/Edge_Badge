// Biham-Middleton-Levine Traffic Model //

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

  int x, y, i;
  uint8_t density = 35;

  uint8_t rule[] = {0,0,0,0,2,2,1,1,1,2,2,2,0,2,2,1,1,1,0,0,0,0,2,2,1,1,1};
  uint8_t pixles[SCR];
  uint8_t slexip[SCR];
  

uint16_t color2state(uint16_t c){ return c == RED ? 1 : (c == BLUE ? 2 : 0); }

uint16_t state2color(uint16_t s){ return s == 1 ? RED : (s == 2 ? BLUE : WHITE); }

void rndrule() { for(int i=0; i<27; i++) rule[i] = trngGetRandomNumber()%3; }

void drawtext(uint8_t value) {
  
  arcada.display->fillScreen(ARCADA_BLACK);
  arcada.display->setCursor(0, 0);
  arcada.display->setTextColor(WHITE);
  arcada.display->setTextWrap(true);
  arcada.display->println(value);
  delay(25);
  
}

void trafficSet(){

  memset(framebuffer, 0, 2*SCR);

  for(i=0; i<SCR; i++){
    if((trngGetRandomNumber()%100)<density){
      if((trngGetRandomNumber()%100)<50) framebuffer[i] = RED;
      else framebuffer[i] = BLUE;
    }
  }
  
  for(i=0; i<SCR; i++) pixles[i] = color2state(framebuffer[i]);

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

  trafficSet();
 
}

void loop() {

  if (arcada.readButtons() & ARCADA_BUTTONMASK_A) { rndrule(); trafficSet(); }
  if (arcada.readButtons() & ARCADA_BUTTONMASK_B) trafficSet();
  if (arcada.readButtons() & ARCADA_BUTTONMASK_UP) { drawtext(density); density = density + 1; trafficSet(); }
  if (arcada.readButtons() & ARCADA_BUTTONMASK_DOWN) { drawtext(density); density = density - 1; trafficSet(); }

  if (density <= 1) density = 1;
  if (density >= 99) density = 99;

  for(x = 0; x<WIDTH; x++){
    for(y = 0; y<HEIGHT; y++) slexip[y*WIDTH+x] = rule[pixles[y*WIDTH+(x+1)%WIDTH]+3*pixles[y*WIDTH+x]+9*pixles[y*WIDTH+(x+WIDTH-1)%WIDTH]];
  }

  for(x = 0; x<WIDTH; x++){
    for(y = 0; y<HEIGHT; y++) pixles[y*WIDTH+x] = rule[slexip[x+((y+1)%HEIGHT)*WIDTH]+3*slexip[y*WIDTH+x]+9*slexip[x+((y+HEIGHT-1)%HEIGHT)*WIDTH]];
  }
  
  for(i = 0; i<SCR; i++) framebuffer[i] = state2color(pixles[i]);
  
  arcada.blitFrameBuffer(0, 0, true, false);
 
}
