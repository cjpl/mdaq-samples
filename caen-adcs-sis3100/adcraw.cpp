
#include "cvadcexp.h"
#include <time.h>

/* ROOT includes */
#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"

extern EXPCVADC_SETTINGS exp_set;

/*-------------- Module declaration ---------------*/

INT adc_raw(EVENT_HEADER *, void *);
INT adc_raw_init(void);
INT adc_raw_bor(INT run_num);
INT adc_raw_eor(INT run_num);

ANA_MODULE adc_raw_module = {
  "Raw Histogramming",
  "Exaos Lee",
  adc_raw,
  adc_raw_bor,
  adc_raw_eor,
  adc_raw_init,
  NULL,
  NULL, // Module parameters
  0,    // Parameter size
  NULL, // Initialized string
};

/*--- module local variables ----*/
static TH1D *hRaw0[N_ADC];
static TH2D *h2DHist[6];

/*--- init routine --------------*/

#define ADC_N_BINS 512
#define ADC_X_LOW    -0.5
#define ADC_X_HIGH 4095.5

INT adc_raw_init(void)
{
  int  i;
  const char hname[N_ADC][32] = {
    "hRaw_dE1", "hRaw_E1", "hRaw_P1", "hRaw_Eu",
    "hRaw_Pu",  "hRaw_Ed",  "hRaw_Pd",
    "CADC07", "CADC08", "CADC09", "CADC10", "CADC11",
    "CADC12", "CADC13", "CADC14", "CADC15",
  };
  const char h2name[5][32] = {
    "h2d_dE1_E1", "h2d_E1_P1", "h2d_Eu_Pu", "h2d_Ed_Pd", "h2d_Eu_Ed"
  };
  char title[256];

  /* Book CADC histos */
  if( exp_set.v785n_1.enable ) {
    for( i=0; i < N_ADC; i++ ) {
      sprintf(title, "Raw ADC0 -- CH%02d",  i);
      hRaw0[i] = h1_book<TH1D>(hname[i], title, ADC_N_BINS, ADC_X_LOW, ADC_X_HIGH);
    }
    for( i=0; i < 5; i++ ) {
      h2DHist[i] = h2_book<TH2D>(h2name[i], h2name[i],
				 ADC_N_BINS, ADC_X_LOW, ADC_X_HIGH,
				 ADC_N_BINS, ADC_X_LOW, ADC_X_HIGH);
    }
  } else  {
    for( i=0; i<N_ADC; i++ ) hRaw0[i] = NULL;
    for( i=0; i<5; i++ ) h2DHist[i] = NULL;
  }

  return SUCCESS;
}

/*--- BOR routine ---------------*/

INT adc_raw_bor(INT run_number)
{
  return SUCCESS;
}

/*--- eor routine ---------------*/

INT adc_raw_eor(INT run_number)
{
  return SUCCESS;
}


/*--- event routine -------------*/
INT adc_raw(EVENT_HEADER *pheader, void *pevent)
{
  INT    i;
  WORD  *pdat0;
  
  /* Look for raw ADC banks: ADC0, ADC1; Return if not resent */
  if( exp_set.v785n_1.enable && bk_locate(pevent, "ADC0", &pdat0) )  {
    for( i=0; i<N_ADC; i++ )
      if( pdat0[i]>0 ) hRaw0[i]->Fill( (float)pdat0[i], 1 );
    h2DHist[0]->Fill( (float)pdat0[0], (float)pdat0[1], 1 );
    h2DHist[1]->Fill( (float)pdat0[1], (float)pdat0[2], 1 );
    h2DHist[2]->Fill( (float)pdat0[3], (float)pdat0[4], 1 );
    h2DHist[3]->Fill( (float)pdat0[5], (float)pdat0[6], 1 );
    h2DHist[4]->Fill( (float)pdat0[3], (float)pdat0[5], 1 );
  }

  return SUCCESS;
}
