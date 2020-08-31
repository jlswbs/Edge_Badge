// Lorenz chaotic attractor //

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

  int iterations = 5000;

  float dx = -1.0f;
  float dy = 1.0f;
  float dz = 1.5f;
  
  float s = 10.0f;
  float r = 28.0f;
  float b = 8/3;

  float x = 0.1f;
  float y = 0.0f;
  float z = 0.0f;

  float dt = 0.005;

void setup(void) {

  if (!arcada.arcadaBegin()) {while (1);}
  arcada.displayBegin();
  
  arcada.setBacklight(255);

  if (! arcada.createFrameBuffer(ARCADA_TFT_WIDTH, ARCADA_TFT_HEIGHT)) {while (1);}
  framebuffer = arcada.getFrameBuffer();
 
}

void loop() {

  if (arcada.readButtons() & ARCADA_BUTTONMASK_RIGHT) dx = dx + 0.02f;
  if (arcada.readButtons() & ARCADA_BUTTONMASK_LEFT) dx = dx - 0.02f;
  if (arcada.readButtons() & ARCADA_BUTTONMASK_UP) dy = dy + 0.02f;
  if (arcada.readButtons() & ARCADA_BUTTONMASK_DOWN) dy = dy - 0.02f;
  if (arcada.readButtons() & ARCADA_BUTTONMASK_A) dz = dz + 0.02f;
  if (arcada.readButtons() & ARCADA_BUTTONMASK_B) dz = dz - 0.02f;

  memset(framebuffer, 0, 2*SCR);

  for (int i = 0; i < iterations; i++)  {

    float nx = x;
    float ny = y;
    float nz = z;

    x = nx + dt * (s*(ny-nx));
    y = ny + dt * ((r*nx)-ny-(nx*nz));
    z = nz + dt * ((nx*ny)-(b*nz));

    float new_x = x - dx * z / dz ;
    float new_y = y - dy * z / dz ;

    uint8_t xout = 26 + (2.5f * new_x);
    uint8_t yout = 96 + (1.8f * new_y);

    if(xout > 0 && xout < WIDTH && yout > 0 && yout < HEIGHT) framebuffer[xout+yout*ARCADA_TFT_WIDTH] = WHITE;
    
  }

  arcada.blitFrameBuffer(0, 0, true, false);
 
}
