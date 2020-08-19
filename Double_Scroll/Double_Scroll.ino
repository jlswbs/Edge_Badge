// Double Scroll chaotic oscillator //

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

  uint16_t xx, yy;

  int iterations = 10000;
  
// "Double Scroll" circuit which shows chaotic behavior

class DoubleScroll
{
public:
  DoubleScroll(float c1);
  virtual ~DoubleScroll();
  void update(float &vc1, float &iL, float &vc2);
private:
  float g(float vc);
  float m_vc1, m_vc2, m_iL;
  float m_c1;
  const float m_c2 = 1.0f;
  const float m_L = 1.0f/7;
  const float m_G = 0.7f;
  const float m_dt = 0.001f;
};

DoubleScroll::DoubleScroll(float c1) : m_c1(c1)
{
  m_vc1 = 0.0f;
  m_vc2 = 0.1f;
  m_iL = 0.0f;
}

DoubleScroll::~DoubleScroll() {}

// non-linear condactance
float DoubleScroll::g(float vc)
{
  if (vc < 1.0f && -1.0f < vc)
    return vc * -0.8f;
  else if (vc > 0)
    return -0.8f -0.5f*(vc - 1.0f);
  else
    return 0.8f + 0.5f*(-vc - 1.0f);
}

void DoubleScroll::update(float &vc1, float &iL, float &vc2)
{
  float dvc1 = m_dt*(1/m_c1)*(m_G*(m_vc2 - m_vc1) - g(m_vc1));
  float dvc2 = m_dt*(1/m_c2)*(m_G*(m_vc1 - m_vc2) + m_iL);
  float diL = m_dt*(1/m_L)* (-m_vc2);
  m_vc1 += dvc1;
  m_vc2 += dvc2;
  m_iL += diL;
  vc1 = m_vc1;
  iL = m_iL;
  vc2 = m_vc2;
}
 
DoubleScroll ds(1/9.0);
int count = 0;

void setup(void) {

  if (!arcada.arcadaBegin()) {while (1);}
  arcada.displayBegin();
  
  arcada.setBacklight(255);

  if (! arcada.createFrameBuffer(ARCADA_TFT_WIDTH, ARCADA_TFT_HEIGHT)) {while (1);}
  framebuffer = arcada.getFrameBuffer();
 
}

void loop() {

  memset(framebuffer, 0, 2*SCR);

  for (int i = 0; i < iterations; i++)  {

    float x, y, z;
    ds.update(y, x, z);
    xx = 80 + (30 * y);
    yy = 64 + (130 * z);

    framebuffer[xx+yy*ARCADA_TFT_WIDTH] = WHITE;
    
  }

  arcada.blitFrameBuffer(0, 0, true, false);
 
}
