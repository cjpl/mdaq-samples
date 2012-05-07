
#ifndef _EVT_GEN_H_
#define _EVT_GEN_H_ 1

#ifdef __cplusplus
// Using ROOT TRandom3 to generate randoms
#include "TRandom3.h"
#include "TF1.h"

class evtgen {
protected:
  TRandom3 *ran;
  TF1      *feng;

public:
  evtgen() { init_gen(); };
  ~evtgen() { };

  void init_gen();
  void gen_event(unsigned short *pdata, int len, int nbins=1024);
};

extern "C" {
#endif

  struct evtgen;

#include "midas.h"
  struct evtgen *c_init_gen();
  void c_gen_event(struct evtgen*, WORD *pdata, int len, int nbins);

#ifdef __cplusplus
}
#endif

#endif

