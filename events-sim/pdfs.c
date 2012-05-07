#include <stdlib.h>
#include <time.h>
#include <math.h>

void init_rand();
double nrand();
double rand_gauss(double mean, double var);
double rand_sin();

void init_rand() {
  srand(time(0));
}

double nrand() {
  return (rand()%10000)/10000.0;
}

double rand_sin()
{
  double r1, r2, rtmp;
  do {
    r1 = nrand();
    r2 = nrand();
    rtmp = (2*r1-1);
    rtmp = rtmp*rtmp + r2*r2;
  } while (rtmp > 1);

  if(rtmp==0) return 0.0;

  return 2*(2*r1-1)*r2/rtmp;
}

double rand_gauss(double mean, double var)
{
  double r1, r2;
  r1 = nrand();
  r2 = rand_sin();
  return mean + var * sqrt(-2*log(r1))*r2;
}

