// Sand simulation //

#include "Adafruit_Arcada.h"

#define BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CyAN            0x07FF
#define MAGENTA         0xF81F
#define yELLOW          0xFFE0
#define WHITE           0xFFFF

Adafruit_Arcada arcada;

uint16_t *framebuffer;

#define WIDTH  160
#define HEIGHT 128
#define SCR (WIDTH * HEIGHT)

uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) { return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3); }

  int M = 40 ;

  uint8_t C[WIDTH][HEIGHT]; 
  int x; int y; int L; int R; int K;
    
 void rndrule(){

  memset(framebuffer, 0, 2*SCR);

  for (x = 0 ; x < WIDTH ; x++){
    for (y = 0 ; y < HEIGHT ; y++){
      C[x][y] = 0;
      if (y < HEIGHT/4) C[x][y] = 255*y/(HEIGHT/4);
      if ((y == HEIGHT/2 && (trngGetRandomNumber()%100) > 10 ) || x == 0 || x == WIDTH-1 || y == 0 || y == HEIGHT-1) C[x][y] = 255;
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

  memset(framebuffer, 0, 2*SCR);
  
  for (y = HEIGHT-1; y > 1; y--){
    if (int (y/2)*2 == y){
      for (x = 1; x < WIDTH-1; x++){
        if (C[x][y] == 0){
          R = C[x][y-1]; K = 0; 
          L = C[x-1][y+1];
          move();
        }
      }
    }
    if (int (y/2)*2 != y){
      for (x = WIDTH-2; x > 0; x--){
        if (C[x][y] == 0){
          L = C[x][y-1]; K = 1; 
          R = C[x+1][y-1]; 
          move() ;
        }
      }
    }        
  }  
  
  for (x = 0; x < WIDTH; x++ ){
    for (y = 0; y < HEIGHT; y++ ){
      if (C[x][y] > 0) framebuffer[y*ARCADA_TFT_WIDTH+x] = color565(C[x][y], C[x][y], C[x][y]);
    }
  }
  
  x = 1 + trngGetRandomNumber()%(WIDTH-1);
  y = (HEIGHT/2-10)+trngGetRandomNumber()%(HEIGHT/2+10);
  if (C[x][y] == 255) C[x][y] = 0;

  arcada.blitFrameBuffer(0, 0, true, false);
 
}

void move(){
  
    if ( ( L > R+M || R == 255 || R == 0) && L < 255){
      C[x][y] = C[x+(K-1)][y-1] ;
      C[x+(K-1)][y-1] = 0 ;
    }
    if ( ( R > L+M || L == 255 || L == 0) && R < 255){
      C[x][y] = C[x+K][y-1];
      C[x+K][y-1] = 0; 
    }
    if (abs(L-R) <= M && L < 255 && R < 255){
      if ((trngGetRandomNumber()%100) < 50+((L-R)*(50/M))){
        C[x][y] = C[x+(K-1)][y-1];
        C[x+(K-1)][y-1] = 0;
      }else{
        C[x][y] = C[x+K][y-1];
        C[x+K][y-1] = 0;
      }
    }
}
