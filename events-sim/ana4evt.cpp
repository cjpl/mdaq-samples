//
// ROOT analyzer for experiment "evtgen" based on "analyzer.cxx" by K.Olchanski
//
// Author: Exaos Lee <Exaos.Lee(at)gmail.com>
//
#include <iostream>
using namespace std;

#include <cstdlib>
#include <cassert>
#include <csignal>
#include <cstring>

#include <sys/time.h>
#include <getopt.h>

#include <TSystem.h>
#include <TROOT.h>
#include <TApplication.h>
#include <TTimer.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TCutG.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TFolder.h>

#include "Globals.h"

#include "TMidasOnline.h"
#include "TMidasEvent.h"
#include "TMidasFile.h"
#include "XmlOdb.h"
#ifdef OLD_SERVER
#include "midasServer.h"
#endif
#ifdef HAVE_LIBNETDIRECTORY
#include "netDirectoryServer.h"
#endif

// Global Variables
int  gRunNumber = 0;
bool gIsRunning = false;
bool gIsPedestalsRun = false;
bool gIsOffline = false;
int  gEventCutoff = 0;

TDirectory* gOnlineHistDir = NULL;
VirtualOdb* gOdb = NULL;
TFile*      gOutputFile = NULL;
//TCanvas*   gMainWindow = NULL;  // the online histogram window

double GetTimeSec()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec + 0.000001*tv.tv_usec;
}

class MyPeriodic : public TTimer {
public:
  typedef void (*TimerHandler)(void);

  int          fPeriod_msec;
  TimerHandler fHandler;
  double       fLastTime;

  inline MyPeriodic(int period_msec,TimerHandler handler)  {
    assert(handler != NULL);
    fPeriod_msec = period_msec;
    fHandler  = handler;
    fLastTime = GetTimeSec();
    Start(period_msec,kTRUE);
  };

  inline Bool_t Notify()  {
    double t = GetTimeSec();
    //printf("timer notify, period %f should be %f!\n",t-fLastTime,fPeriod_msec*0.001);

    if (t - fLastTime >= 0.9*fPeriod_msec*0.001)  {
      //printf("timer: call handler %p\n",fHandler);
      if (fHandler) (*fHandler)();
      fLastTime = t;
    }

    Reset();
    return kTRUE;
  };

  inline ~MyPeriodic()  { TurnOff();  };
};


void startRun(int transition,int run,int time)
{
  gIsRunning = true;
  gRunNumber = run;
  gIsPedestalsRun = gOdb->odbReadBool("/experiment/edit on start/Pedestals run");
  printf("Begin run: %d, pedestal run: %d\n", gRunNumber, gIsPedestalsRun);
    
  if(gOutputFile!=NULL) {
    gOutputFile->Write();
    gOutputFile->Close();
    gOutputFile=NULL;
  }

  char filename[1024];
  sprintf(filename, "output%05d.root", run);
  gOutputFile = new TFile(filename,"CREATE");

  char* name[4] = { "hPH", "hT", "hV", "hE"};
  TH1D* hadc[4] = { NULL, NULL, NULL, NULL };
  TH2D* hpht = NULL;
  for(int i=0; i<4; i++) {
    hadc[i] = (TH1D *) gDirectory->Get(name[i]);
    if(!hadc[i])
      printf("Creating histogram %s ...\n",name[i]);
      hadc[i] = new TH1D(name[i], name[i], 1024, 0, 4096);
#ifdef OLD_SERVER
    if (gManaHistosFolder)
      gManaHistosFolder->Add(hadc[i]);
#endif
  }

  hpht = (TH2D *)gDirectory->Get("hPHT");
  if( !hpht ) {
    printf("Creating 2D histogram hPHT ...\n");
    hpht = new TH2D("hPHT","PH-T", 64, 0, 4096, 64, 0, 4096);
#ifdef OLD_SERVER
    if (gManaHistosFolder)
      gManaHistosFolder->Add(hpht);
#endif
  }

#ifdef HAVE_LIBNETDIRECTORY
  NetDirectoryExport(gOutputFile, "outputFile");
#endif
}

void endRun(int transition,int run,int time)
{
  gIsRunning = false;
  gRunNumber = run;

#ifdef OLD_SERVER
  if (gManaHistosFolder)
    gManaHistosFolder->Clear();
#endif

  if (gOutputFile)  {
    gOutputFile->Write();
    gOutputFile->Close(); //close the histogram file
    gOutputFile = NULL;
  }

  printf("End of run %d\n",run);
}

void HandleADC0(void *ptr, int wsize)
{
  uint16_t *samples = (uint16_t *) ptr;
  int sampleN = wsize;

  if( sampleN != 4 ) return;
  char* name[4] = {"hPH", "hT", "hV", "hE"};
  TH1D* hadc[4] = { NULL, NULL, NULL, NULL };
  for(int i=0; i<4; i++) {
    hadc[i] = (TH1D *) gDirectory->Get(name[i]);
    if(hadc[i]) hadc[i]->Fill(samples[i]);
  }

  TH2D *hpht = (TH2D *)gDirectory->Get("hPHT");
  if(hpht) hpht->Fill((double)samples[0], (double)samples[1]);
}

void HandleTDC0(void *ptr, int wsize)
{}

void HandleMidasEvent(TMidasEvent& event)
{
  int eventId = event.GetEventId();

  if((eventId==1) && gIsRunning && !gIsPedestalsRun) { // ADC & TDC data
    //printf("Have an event of type %d\n",eventId);

    void* ptr[2];
    int size;
    size = event.LocateBank(NULL, "ADC0", &ptr[0]);
    if(ptr[0]) HandleADC0(ptr[0], size);
    size = event.LocateBank(NULL, "TDC0", &ptr[1]);
    if(ptr[1]) HandleTDC0(ptr[1], size);
  } else if((eventId == 2) && gIsRunning)  { // Scaler data
    // ...
  }  else {  // unknown event type
    event.Print();
  }
}

void eventHandler(const void*pheader,const void*pdata,int size)
{
  TMidasEvent event;
  memcpy(event.GetEventHeader(), pheader, sizeof(EventHeader_t));
  event.SetData(size, (char*)pdata);
  event.SetBankList();
  HandleMidasEvent(event);
}

int ProcessMidasFile(TApplication*app,const char*fname)
{
  TMidasFile f;
  bool tryOpen = f.Open(fname);

  if (!tryOpen)  {
    printf("Cannot open input file \"%s\"\n",fname);
    return -1;
  }

  int i=0;
  while (1) {
    TMidasEvent event;
    if (!f.Read(&event))  break;

    int eventId = event.GetEventId();
    //printf("Have an event of type %d\n",eventId);

    if ((eventId & 0xFFFF) == 0x8000)	{
      // begin run
      event.Print();

      // Load ODB contents from the ODB XML file
      if (gOdb)	delete gOdb;
      gOdb = new XmlOdb(event.GetData(),event.GetDataSize());

      startRun(0,event.GetSerialNumber(),0);
    }
    else if ((eventId & 0xFFFF) == 0x8001) {
      // end run
      event.Print();
    }
    else {
      event.SetBankList();
      //event.Print();
      HandleMidasEvent(event);
    }
	
    if((i%500)==0) {
      printf("Processing event %d\n",i);
    }
      
    i++;
    if ((gEventCutoff!=0)&&(i>=gEventCutoff))	{
      printf("Reached event %d, exiting loop.\n",i);
      break;
    }
  }
  
  f.Close();

  endRun(0,gRunNumber,0);

  // start the ROOT GUI event loop
  //  app->Run(kTRUE);

  return 0;
}

void MidasPollHandler()
{
  if (!(TMidasOnline::instance()->poll(0)))
    gSystem->ExitLoop();
}

bool OnlineSetupKeys()
{
  if(! gOdb) return false;

  // VirtualOdb::odbReadXXX(...)
  // There are no VirtualOdb::odbCreateXXX(...) exist! So we just use
  // db_xxx_xxx(...) and gOdb->fDB
  
  return true;
}

int ProcessMidasOnline(TApplication*app, const char* hostname, const char* exptname)
{
  TMidasOnline *midas = TMidasOnline::instance();

  int err = midas->connect(hostname, exptname, "rootana");
  if (err != 0)  {
    fprintf(stderr,"Cannot connect to MIDAS, error %d\n", err);
    return -1;
  }

  // Setup ODB keys. If failed, stop
  gOdb = midas;
  if(! OnlineSetupKeys()) return -1;

  midas->setTransitionHandlers(startRun,endRun,NULL,NULL);
  midas->registerTransitions();

  /* reqister event requests */
  midas->setEventHandler(eventHandler);
  midas->eventRequest("SYSTEM",-1,-1,(1<<1));

  /* fill present run parameters */
  gRunNumber = gOdb->odbReadInt("/runinfo/Run number");

  if ((gOdb->odbReadInt("/runinfo/State") == 3))
    startRun(0,gRunNumber,0);
  printf("Startup: run %d, is running: %d, is pedestals run: %d\n",
	 gRunNumber,gIsRunning,gIsPedestalsRun);
   
  MyPeriodic tm(100,MidasPollHandler);

  /*---- start main loop ----*/
  app->Run(kTRUE);

  /* disconnect from experiment */
  midas->disconnect();

  return 0;
}

static bool gEnableShowMem = false;

int ShowMem(const char* label)
{
  if (!gEnableShowMem)
    return 0;

  FILE* fp = fopen("/proc/self/statm","r");
  if (!fp)  return 0;

  int mem = 0;
  fscanf(fp,"%d",&mem);
  fclose(fp);

  if (label)  printf("memory at %s is %d\n", label, mem);

  return mem;
}

void usage()
{
  cout << "Usage: ana4evt [options] [filenams]" << endl
       << "Options:" << endl
       << "  -H <hostname>    Connect to MIDAS server on host <hostname>" << endl
       << "  -e <exptname>    Connect to MIDAS experiment named <exptname>" << endl
       << "  -P <port>        Start histogram tcp server on <port> using TNetDriectory"
       << endl
       << "  -p <port         Start histogram tcp server on <port> using old midas type"
       << endl
       << "  -N <Max events>  Maximum events to be processed" << endl
       << "  -g, --debug      Enable debug mode" << endl
       << "  -G, --gui        Enable graphical UI" << endl
       << "  -T, --test       Running in test mode" << endl << endl;
}

// For command line processing
struct globalArgs_t {
  char*  hostname; // -H, --hostname <...> :: hostname
  char*  exptname; // -e, --exptname <...> :: experiment name
  int    portn;    // -P, --portN <port> :: port using TNetDirectory
  int    portm;    // -p, --portM <port> :: port using midas server
  long   nmax;     // -N <max event> :: maximum events to be processed
  int    debug;    // -g, --debug :: debug mode
  int    gui;      // -G, --gui   :: GUI mode
  int    test;     // -T, --test  :: test mode
  char** inputs;   // Input files
  int    inpn;     // Number of input files
} globalOpts;

static const char* optString = "H:e:P:p:N:gGTh?";

static const struct option cmdOpts[] = {
  {"hostname", required_argument, NULL, 'H'},
  {"exptname", required_argument, NULL, 'e'},
  {"portN",    required_argument, NULL, 'P'},
  {"portM",    required_argument, NULL, 'p'},
  {"maxevent", required_argument, NULL, 'N'},
  {"debug",    no_argument, NULL, 'g'},
  {"gui",      no_argument, NULL, 'G'},
  {"test",     no_argument, NULL, 'T'},
};

void cmd_parsing(int argc, char** argv)
{
  /* Initialize globalOpts before processing args passed to main() */
  globalOpts.hostname = NULL;
  globalOpts.exptname = NULL;
  globalOpts.portn = 0; // 9091 -- default
  globalOpts.portm = 0; // 9090 -- default
  globalOpts.nmax  = -1;
  globalOpts.debug = 0;
  globalOpts.gui   = 0;
  globalOpts.test  = 0;

  int longIdx = 0;
  int opt = getopt_long(argc, argv, optString, cmdOpts, &longIdx);
  while( opt != -1 ) {
    switch(opt) {
    case 'H':  globalOpts.hostname = optarg; break;
    case 'e':  globalOpts.exptname = optarg; break;
    case 'P':  globalOpts.portn = atoi(optarg); break;
    case 'p':  globalOpts.portm = atoi(optarg); break;
    case 'N':  globalOpts.nmax  = atol(optarg); break;
    case 'g':  globalOpts.debug = 1; break;
    case 'G':  globalOpts.gui   = 1; break;
    case 'T':  globalOpts.test  = 1; break;
    case 'h':
    case '?':
    case 0:    usage();  break;
    default: break;
    }
    opt = getopt_long(argc, argv, optString, cmdOpts, &longIdx);
  }
  globalOpts.inpn = argc - optind;
  if(globalOpts.inpn>0) {
    globalOpts.inputs = argv + optind;
  }

  // for(int i=0;i<globalOpts.inpn;i++)
  //   cout << "globalOpts.inputs[" << i << "] = " << globalOpts.inputs[i] << endl;
  // for(int i=0; i<argc; i++) cout << "Arg[" << i << "] = " << argv[i] << endl;
}

// Main function call
static bool gVerbose = true;

int main(int argc, char** argv)
{
  setbuf(stdout,NULL);
  setbuf(stderr,NULL);
 
  signal(SIGILL,  SIG_DFL);
  signal(SIGBUS,  SIG_DFL);
  signal(SIGSEGV, SIG_DFL);

  cmd_parsing(argc, argv); // Parsing command options

  TApplication *app = new TApplication("rootana", &argc, argv);

  if(gROOT->IsBatch()) {
    printf("Cannot run in batch mode\n");
    return 1;
  }

  gROOT->cd();
  gOnlineHistDir = new TDirectory("rootana", "rootana online plots");

#ifdef OLD_SERVER
  if (globalOpts.portm)  StartMidasServer(globalOpts.portm);
#else
  if (globalOpts.portm)
    fprintf(stderr,"ERROR: No support for the old midas server!\n");
#endif
#ifdef HAVE_LIBNETDIRECTORY
  if (globalOpts.portn)
    StartNetDirectoryServer(globalOpts.portn, gOnlineHistDir);
#else
  if (globalOpts.portn)
    fprintf(stderr,"ERROR: No support for the TNetDirectory server!\n");
#endif

  // Test mode
  if (globalOpts.test)  {
    gOnlineHistDir->cd();
    TH1D* hh = new TH1D("test", "test", 100, 0, 100);
    hh->Fill(1);
    hh->Fill(10);
    hh->Fill(50);

    app->Run(kTRUE);
    return 0;
  }

  // Offline mode
  gIsOffline = false;
  if(globalOpts.inpn > 0) {
    gIsOffline = true;
    for(int i=0; i<globalOpts.inpn; i++)
      ProcessMidasFile(app, globalOpts.inputs[i]);

    // if we processed some data files,
    // do not go into online mode.
    return 0;
  }

#ifdef HAVE_MIDAS   // Online mode
  ProcessMidasOnline(app, globalOpts.hostname, globalOpts.exptname);
#endif
   
  return 0;
}

//end

