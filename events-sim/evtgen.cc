#include "evtgen.h"
#include "math.h"

struct evtgen* c_init_gen()
{  return new evtgen();  }

void c_gen_event(struct evtgen* evt, WORD *pdata, int len, int nbins)
{  evt->gen_event(pdata, len, nbins); }

void evtgen::init_gen()
{
  ran  = new TRandom3();
  feng = new TF1("feng","[0]*expo(1)+gaus(3)+gaus(6)", 0, 16.0);
  feng->SetParameters(3.0, 0., -5., 1.5, 3.0, 1.6, 2.0, 14.0, 0.12);
}

/** Generated events:
 * - Bins: ==> [0,1024]
 * - ADC0: PH
 *   - Random from PDF "hph"
 *   - range = [0,16.0]
 * - ADC1: T
 *   - gaus(72.3/sqrt(E), sigma)
 *   - range = [0, 250] (ns)
 * - ADC2: V
 *   - gaus(13.83*sqrt(E),sigma)
 *   - range = [0, 60]  (m/ns)
 * - ADC3: E
 *   - range = [0, 16.0] (MeV)
 */
void evtgen::gen_event(unsigned short int *pdata, int len, int nbins)
{
  double ph, t, v, e;
  WORD   adc[4];

  e  = feng->GetRandom();
  ph = ran->Rndm() * e;
  if(e>0.08)
    t = ran->Gaus(72.3*sqrt(e), 1.5);
  else t = 0;
  v  = ran->Gaus(13.83*sqrt(e),1.2);

  adc[0] = (WORD) (ph * nbins/16.0);
  adc[1] = (WORD) ( t * nbins/250.);
  adc[2] = (WORD) ( v * nbins/60.);
  adc[3] = (WORD) ( e * nbins/16.);

  if(len>4) len = 4;
  for(int i=0; i<len; i++) *(pdata+i) = adc[i];
}

