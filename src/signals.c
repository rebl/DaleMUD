
#include <signal.h>
#include <stdio.h>
#include <sys/time.h>

#include "protos.h"

int checkpointing(void);
int shutdown_request(void);
int logsig(void);
int hupsig(void);

int raw_force_all( char *to_force);

void signal_setup()
{
	struct itimerval itime;
	struct timeval interval;

	signal(SIGUSR2, shutdown_request);

	/* just to be on the safe side: */

	signal(SIGHUP, hupsig);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, hupsig);
	signal(SIGALRM, logsig);
	signal(SIGTERM, hupsig);

	/* set up the deadlock-protection */

	interval.tv_sec = 900;    /* 15 minutes */
	interval.tv_usec = 0;
	itime.it_interval = interval;
	itime.it_value = interval;
	setitimer(ITIMER_VIRTUAL, &itime, 0);
	signal(SIGVTALRM, checkpointing);
}



int checkpointing()
{
	extern int tics;
	
#if 1 
	if (!tics) {
		log("CHECKPOINT shutdown: tics not updated");
		abort();
	}
	else
	 tics = 0;
#else
if (!tics) {
	log("CHECKPOINT tics not updated, setting to 1, be very warry");
	tics=1;
 }
  else
     tics = 0;
#endif
	 
}




int shutdown_request()
{
	extern int mudshutdown;

	log("Received USR2 - shutdown request");
	mudshutdown = 1;
}


/* kick out players etc */
int hupsig()
{
  int i;
  extern int mudshutdown, reboot;

  log("Received SIGHUP, SIGINT, or SIGTERM. Shutting down");

  raw_force_all("return");
  raw_force_all("save");
  for (i=0;i<30;i++) {
    SaveTheWorld();
  }
  mudshutdown = reboot = 1;
}

int logsig()
{
	log("Signal received. Ignoring.");
}
