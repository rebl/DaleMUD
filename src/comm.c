/*
*** DaleMUD	comm.c main communication routines. Based on DIKU and
***		       SillyMUD.
*/


#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/resource.h>

#include "protos.h"


void str2ansi(char *p2, char *p1, int start, int stop);
char *ParseAnsiColors(int UsingAnsi, char *txt);
int construct_prompt(char *buf, struct char_data *ch);
void identd_test(struct sockaddr_in in_addr) ;


#define MAX_CONNECTS 256  /* max number of descriptors (connections) */
			  /* THIS IS SYSTEM DEPENDANT, use 64 is not sure! */



#define DFLT_PORT 4000        /* default port */
#define MAX_NAME_LENGTH 15
#define MAX_HOSTNAME   256
#define OPT_USEC 250000       /* time delay corresponding to 4 passes/sec */

#define STATE(d) ((d)->connected)

int mud_port;

extern int errno;

/* extern struct char_data *character_list; */
#if HASH
extern struct hash_header room_db;	  /* In db.c */
#else
extern struct room_data *room_db;	  /* In db.c */
#endif

/* extern int top_of_world;             In db.c */




struct descriptor_data *descriptor_list, *next_to_process;
struct txt_block *bufpool = 0;  /* pool of large output buffers */
int     buf_largecount;         /* # of large buffers which exist */
int     buf_overflows;          /* # of overflows of output */
int     buf_switches;           /* # of switches from small to large buf */

/* int slow_nameserver = FALSE; */

int lawful = 0;		/* work like the game regulator */
int slow_death = 0;     /* Shut her down, Martha, she's sucking mud */
int mudshutdown = 0;       /* clean shutdown */
int reboot = 0;         /* reboot the game after a shutdown */
int no_specials = 0;    /* Suppress ass. of special routines */
long Uptime;            /* time that the game has been up */
long SystemFlags;
long TempDis=0;

int pulse;

#if SITELOCK
char hostlist[MAX_BAN_HOSTS][30];  /* list of sites to ban           */
int numberhosts;
#endif

int maxdesc, avail_descs;
int tics = 0;        /* for extern checkpointing */





#define O_LOG_NAME "output.log"
#define O_MAX_LINES 30000
 
  static FILE *debugf=NULL;
 
void writetofile(char *msg)
{
  static long lines=0;
  char *b[5];
  int i,j;
 
  if(!debugf) {
    if(!(debugf=fopen(O_LOG_NAME,"wr"))) {
      perror(O_LOG_NAME);
      exit(1);
    }
    setvbuf(debugf,NULL,_IOFBF,16386);
  }
 
  if(lines>O_MAX_LINES) {
   rewind(debugf);
   for(i=0;i<5;i++) {
     b[i]=(char *)malloc(MAX_STRING_LENGTH);
     if(fgets(b[i], MAX_STRING_LENGTH, debugf)==NULL) break;
   }
   fclose(debugf);
   if(!(debugf=fopen(O_LOG_NAME,"wr"))) {
     perror(O_LOG_NAME);
     exit(1);
   }
   setvbuf(debugf,NULL,_IOFBF,16386);
   for(j=0;j<i;j++) {
     fputs(b[j],debugf);
if (b[j])
     free(b[j]);
   }
   lines=0;
  }
  fprintf(debugf,"%s", msg);
  lines++;
}
 
void testlog(char *msg)
{
        char buf[MAX_STRING_LENGTH*2];
  sprintf(buf,"->%s\n", msg);
  writetofile(buf);
return;
}
 
 
void check_affected(char *msg,char *msg2)
{
  struct affected_type  *hjp, *old_af;
  struct char_data *c;
  char buf[5000], *p;
  int i,j;

  if(msg)
    sprintf(buf,"%s : ", msg);
  else
    sprintf(buf,"check_affected: ");
  writetofile(buf);
 
  p=strdup("a simple test in check_affected blank text meaning\
  		   nothing to anyone intelligent anyways.... go get'em!\
  		   Okay.... lets see if we can wack an link list somewhere"); 

  for(c=character_list;c;c=c->next)
    if(c && c->affected)
      for(hjp=c->affected;hjp;old_af = hjp, hjp=hjp->next)
        if(hjp->type > MAX_EXIST_SPELL || hjp->type < 0) {
          sprintf(buf,"bogus hjp->type for (%s).\r\n", GET_NAME(c));
          writetofile(buf);
          sprintf(buf,"string in old_af: %s\r\n", old_af);
          writetofile(buf);
          sprintf(buf,"string in cur_af: %s\r\n", hjp);
          writetofile(buf);
          fflush(debugf);
          abort();
        }
 
  writetofile("ok\n");
 free(p);
return;
}

/* *********************************************************************
*  main game loop and related stuff				       *
********************************************************************* */

int __main ()
{
  return(1);
}

/* jdb code - added to try to handle all the different ways the connections
   can die, and try to keep these 'invalid' sockets from getting to select
*/

int close_socket_fd( int desc)
{
  struct descriptor_data *d;
/*  extern struct descriptor_data *descriptor_list; */


#if LOG_DEBUG
slog("begin close_socket_fd");
#endif

  for (d = descriptor_list;d;d=d->next) {
    if (d->descriptor == desc) {
      close_socket(d);
    }
  }

#if LOG_DEBUG
slog("end close_socket_fd");
#endif
}

int main (int argc, char **argv)
{
  int pos=1;
  char buf[512], *dir;

  extern int WizLock;

#ifdef SITELOCK
 int a;
#endif

#if defined(sun) || defined(NETBSD)
  struct rlimit rl;
  int res;
#endif

#ifdef MALLOC_DEBUG
	malloc_debug(1);	/* some systems might not have this lib */
#endif
  
  mud_port = DFLT_PORT;
  dir = DFLT_DIR;

	SystemFlags =0;

#if LOG_ALL
	SET_BIT(SystemFlags,SYS_LOGALL);
#endif	
	
#if defined(sun) || defined(NETBSD)
/*
**  this block sets the max # of connections.  
*/
#if defined(sun)
   res = getrlimit(RLIMIT_NOFILE, &rl);
   rl.rlim_cur = MAX_CONNECTS;
   res = setrlimit(RLIMIT_NOFILE, &rl);
#endif

#if defined(NETBSD)
   res = getrlimit(RLIMIT_OFILE, &rl);
   rl.rlim_cur = MAX_CONNECTS;
   res = setrlimit(RLIMIT_OFILE, &rl);
#endif   

#endif

#if DEBUG  
/* never seen this function, must be something john was working on, disabled */
/* msw */
/*  malloc_debug(0);  */
#endif

  while ((pos < argc) && (*(argv[pos]) == '-'))	{
    switch (*(argv[pos] + 1))  {
    case 'l':
      lawful = 1;
      log("Lawful mode selected.");
      break;
    case 'd':
      if (*(argv[pos] + 2))
	dir = argv[pos] + 2;
      else if (++pos < argc)
	dir = argv[pos];
      else   	{
	log("Directory arg expected after option -d.");
	assert(0);
      }
      break;
    case 's':
      no_specials = 1;
      log("Suppressing assignment of special routines.");
      break;

	case 'A':
		SET_BIT(SystemFlags,SYS_NOANSI);
		log("Disabling ALL color");
		break;
	case 'N':
		SET_BIT(SystemFlags,SYS_SKIPDNS);
		log("Disabling DNS");
		break;
	case 'R':
		SET_BIT(SystemFlags,SYS_REQAPPROVE);
		log("Newbie authorizes enabled");
		break;
	case 'L':
		SET_BIT(SystemFlags,SYS_LOGALL);
		log("Logging all users");
		break;
         
    default:
      sprintf(buf, "Unknown option -% in argument string.",
	      *(argv[pos] + 1));
      log(buf);
      break;
    }
    pos++;
  }
  
  if (pos < argc)
    if (!isdigit(*argv[pos]))      	{
      fprintf(stderr, "Usage: %s [-l] [-s] [-d pathname] [ port # ]\n", 
	      argv[0]);
      assert(0);
    }  else if ((mud_port = atoi(argv[pos])) <= 1024)  {
      printf("Illegal port #\n");
      assert(0);
    }
  
  Uptime = time(0);
  
  sprintf(buf, "Running game on port %d.", mud_port);
  log(buf);
  
  if (chdir(dir) < 0)	{
    perror("chdir");
    assert(0);
  }
  
  sprintf(buf, "Using %s as data directory.", dir);
  log(buf);
  
  srandom(time(0));
  WizLock = FALSE;

#if SITELOCK
  log("Blanking denied hosts.");
  for(a = 0 ; a<= MAX_BAN_HOSTS ; a++) 
    strcpy(hostlist[a]," \0\0\0\0");
  numberhosts = 0;

#if LOCKGROVE
  log("Locking out Host: oak.grove.iup.edu.");
  strcpy(hostlist[0],"oak.grove.iup.edu");
  numberhosts = 1; 
  log("Locking out Host: everest.rutgers.edu.");
  strcpy(hostlist[1],"everest.rutgers.edu");
  numberhosts = 2;
#endif /* LOCKGROVE */

#if PERSONAL_PERM_LOCKOUTS
  numberhosts +=0; 
#endif

#endif



  /* close stdin */
  close(0);

  run_the_game(mud_port);
  return(0);
}



#define PROFILE(x)


/* Init sockets, run game, and cleanup sockets */
int run_the_game(int port)
{
  int s; 
  PROFILE(extern etext();)
    
    void signal_setup(void);
  int load(void);
  
  PROFILE(monstartup((int) 2, etext);)
    
    descriptor_list = NULL;
  
  log("Signal trapping.");
  signal_setup();
  
  log("Opening mother connection.");
  s = init_socket(port);
  
#ifdef USE_LAWFUL

if (lawful && load() >= 6)
    {
      log("System load too high at startup.");
      coma(1);
    }
 
#endif
 
  boot_db();
  
  log("Entering game loop.");
  
  game_loop(s);
  
  close_sockets(s); 
  
  PROFILE(monitor(0);)
    
  if (reboot)  {
    log("Rebooting.");
    assert(52);           /* what's so great about HHGTTG, anyhow? */
  }
  
  log("Normal termination of game.");
}






/* Accept new connects, relay commands, and call 'heartbeat-functs' */
int game_loop(int s)
{
  fd_set input_set, output_set, exc_set;
#if 0
  fd_set tin, tout, tex;
  fd_set mtin, mtout, mtex;
#endif
  static int cap;
  struct timeval last_time, now, timespent, timeout, null_time;
  static struct timeval opt_time;
  char comm[MAX_INPUT_LENGTH];
  char promptbuf[180];
  struct descriptor_data *point, *next_point;
  int mask;
  struct room_data *rm;

/*  extern struct descriptor_data *descriptor_list; */
  extern int pulse;
  extern int maxdesc;
  extern struct time_info_data time_info;  /* In db.c */  

  null_time.tv_sec = 0;
  null_time.tv_usec = 0;
  
  opt_time.tv_usec = OPT_USEC;  /* Init time values */
  opt_time.tv_sec = 0;
#ifdef NETBSD
  gettimeofday(&last_time, NULL); 
#else
  gettimeofday(&last_time, (struct timeval *) 0);
#endif  
  
  maxdesc = s;


  /* !! Change if more needed !! */
  avail_descs = getdtablesize() -2; /* never used, pointless? */
  
  mask = sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
    sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
      sigmask(SIGURG) | sigmask(SIGXCPU) | sigmask(SIGHUP);
  
  /* Main loop */
  while (!mudshutdown)  {


    /* Check what's happening out there */

    FD_ZERO(&input_set);
    FD_ZERO(&output_set);
    FD_ZERO(&exc_set);

    FD_SET(s, &input_set);
    
#if TITAN
    maxdesc = 0;
    if (cap < 20)
      cap = 20;
    for (point = descriptor_list; point; point = point->next)  {

      if (point->descriptor <= cap && point->descriptor >= cap-20) {
	FD_SET(point->descriptor, &input_set);
	FD_SET(point->descriptor, &exc_set);
	FD_SET(point->descriptor, &output_set);
      
      }

      if (maxdesc < point->descriptor)
	maxdesc = point->descriptor;
    }

    if (cap > maxdesc)
      cap = 0;
    else
      cap += 20;
#else
	dimd_loop();
    for (point = descriptor_list; point; point = point->next)  {
      FD_SET(point->descriptor, &input_set);
      FD_SET(point->descriptor, &exc_set);
      FD_SET(point->descriptor, &output_set);
      
      if (maxdesc < point->descriptor)
	maxdesc = point->descriptor;
    }
#endif

    /* check out the time */
#ifdef NETBSD
    gettimeofday(&now, NULL); 
#else
    gettimeofday(&now, (struct timeval *) 0);
#endif

    
    timespent = timediff(&now, &last_time);
    timeout = timediff(&opt_time, &timespent);
    last_time.tv_sec = now.tv_sec + timeout.tv_sec;
    last_time.tv_usec = now.tv_usec + timeout.tv_usec;
    if (last_time.tv_usec >= 1000000) {
      last_time.tv_usec -= 1000000;
      last_time.tv_sec++;
    }
    
    sigsetmask(mask);

    if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) 
	< 0)   	{
      perror("Select poll");
      
	/* one of the descriptors is broken... */

      for (point = descriptor_list; point; point = next_point)  {
	next_point = point->next;
	write_to_descriptor(point->descriptor, "\n\r");
      }
      
    }

    if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0) {
      perror("Select sleep");
      /*assert(0);*/
    }
    
    sigsetmask(0);
    
    /* Respond to whatever might be happening */
    
    
    /* New connection? */
    if (FD_ISSET(s, &input_set))
      if (new_descriptor(s) < 0) {
	perror("New connection");
      }

    /* kick out the freaky folks */
    for (point = descriptor_list; point; point = next_point)  {
      next_point = point->next;   
      if (FD_ISSET(point->descriptor, &exc_set))  {
	FD_CLR(point->descriptor, &input_set);
	FD_CLR(point->descriptor, &output_set);
	close_socket(point);
      }
    }
    for (point = descriptor_list; point; point = next_point)  {
      next_point = point->next;
      if (FD_ISSET(point->descriptor, &input_set))
	if (process_input(point) < 0)  {
	  close_socket(point);
	  }
    }

    /* process_commands; */
    for (point = descriptor_list; point; point = next_to_process){
	 next_to_process = point->next;

 if ((--(point->wait) <= 0) && get_from_q(&point->input, comm))	{

  if (point->character && point->connected == CON_PLYNG &&
	      point->character->specials.was_in_room !=	NOWHERE) {

	      point->character->specials.was_in_room = NOWHERE;
	      act("$n has returned.",	TRUE, point->character, 0, 0, TO_ROOM);
	    }
	  
	  point->wait = 1;
	  
	  if (point->character) {
	    point->character->specials.timer = 0;
	    }
	    
	  point->prompt_mode = 1;
	  
	  if (point->str) 
	    string_add(point, comm);
	  else 
	  if (!point->connected) 	  {
	    if (point->showstr_point) {
	      show_string(point, comm);
	      }
	    else  {
	    check_affected("Before command issued","");
	     command_interpreter (  point->character, comm);
	    check_affected("After command issued","");
	     }
          } 
	  else if(point->connected == CON_EDITING)
	    RoomEdit(point->character,comm);
          else if(point->connected == CON_OBJ_EDITING)
            ObjEdit(point->character,comm);
          else if(point->connected == CON_MOB_EDITING)
            MobEdit(point->character,comm);
	  else
	    nanny(point, comm); 
	}
    }
    
    
    /* either they are out of the game */
    /* or they want a prompt.          */
memory_check("end for 4, begin for 5");
    for (point = descriptor_list; point; point = next_point) {
	next_point = point->next;

#ifndef BLOCK_WRITE
	if (FD_ISSET(point->descriptor, &output_set) && point->output.head)
#else
	if (FD_ISSET(point->descriptor, &output_set) && *(point->output))
#endif	
	  if (process_output(point) < 0)
	    close_socket(point);
	  else
	    point->prompt_mode = 1;
      }
    
    /* give the people some prompts  */
memory_check("end 5, begin 6");
    for (point = descriptor_list; point; point = point->next)
      if (point->prompt_mode) {
	if (point->str)
	  write_to_descriptor(point->descriptor, "-> ");
	else if (!point->connected)
	  if (point->showstr_point)
	    write_to_descriptor(point->descriptor,
				"[Return to continue/Q to quit]");
	  else { 
 
            if(point->character->term == VT100) {
               struct char_data *ch;
               int update = 0;
 
               ch = point->character;
 
               if(GET_MOVE(ch) != ch->last.move) {
                  SET_BIT(update, INFO_MOVE);
                  ch->last.move = GET_MOVE(ch);
		}
               if(GET_MAX_MOVE(ch) != ch->last.mmove) {
                  SET_BIT(update, INFO_MOVE);
                  ch->last.mmove = GET_MAX_MOVE(ch);
		}
               if(GET_HIT(ch) != ch->last.hit) {
                  SET_BIT(update, INFO_HP);
                  ch->last.hit = GET_HIT(ch);
		}
               if(GET_MAX_HIT(ch) != ch->last.mhit) {
                  SET_BIT(update, INFO_HP);
                  ch->last.mhit = GET_MAX_HIT(ch);
		}
               if(GET_MANA(ch) != ch->last.mana) {
                  SET_BIT(update, INFO_MANA);
                  ch->last.mana = GET_MANA(ch);
		}
               if(GET_MAX_MANA(ch) != ch->last.mmana) {
                  SET_BIT(update, INFO_MANA);
                  ch->last.mmana = GET_MAX_MANA(ch);
		}
               if(GET_GOLD(ch) != ch->last.gold) {
                  SET_BIT(update, INFO_GOLD);
                  ch->last.gold = GET_GOLD(ch);
		}
               if(GET_EXP(ch) != ch->last.exp) {
                  SET_BIT(update, INFO_EXP);
                  ch->last.exp = GET_EXP(ch);
		}
               if(update)
                  UpdateScreen(ch, update);
	       sprintf(promptbuf,"> ");
	       } else {
	       construct_prompt(promptbuf,point->character);
	     }
	  write_to_descriptor(point->descriptor,ParseAnsiColors( \
                              IS_SET(point->character->player.user_flags,USE_ANSI), \
                              promptbuf));
	  }
	point->prompt_mode = 0;
	
      }  
    
    
    /* handle heartbeat stuff */
    /* Note: pulse now changes every 1/4 sec  */

    pulse++;
    
    if (!(pulse % PULSE_ZONE))  {
	dlog("Before zone update pulse");
    memory_check("BEfore Zone update");
      zone_update();
      if (lawful)
	gr(s);
    }
    
    
    if (!(pulse % PULSE_RIVER)) {
	dlog("Before river pulse ");
	memory_check("Before riverpulse");    
      RiverPulseStuff(pulse);
    }
    
    if (!(pulse % PULSE_TELEPORT)) {
	dlog("Before Teleport pulse");
	memory_check("Tele pulse");
      TeleportPulseStuff(pulse);
    }
    
    if (!(pulse % PULSE_VIOLENCE)) {
	dlog("Before violence pulse...");
	memory_check("Pulse violence");
	check_mobile_activity(pulse);    
	perform_violence( pulse );
      }

    
    
    if (!(pulse % (SECS_PER_MUD_HOUR*4))){
	dlog("Before hourly tick pulse");
	memory_check(" before pulse weather");
      weather_and_time(1);
      affect_update(pulse);  /* things have been sped up by combining */
      if ( time_info.hours == 1 )
	update_time();
    }

  memory_check("loop end");
    
    if (pulse >= 2400) {
      pulse = 0;
      if (lawful)
	night_watchman();
      check_reboot();
    }
    
    tics++;        /* tics since last checkpoint signal */
  }
}






/* ******************************************************************
*  general utility stuff (for local use)									 *
****************************************************************** */




int get_from_q(struct txt_q *queue, char *dest)
{
	struct txt_block *tmp;

 	/* Q empty? */
	if (!queue->head)
		return(0);
	if (!dest) {
	  log_sev("Sending message to null destination.", 5);
	  return(0);
	}
	tmp = queue->head;

	if (dest && queue->head->text)
	  strcpy(dest, queue->head->text);
	queue->head = queue->head->next;

	if (tmp->text)
		free(tmp->text);
	if (tmp)
		free(tmp);
 return(1);
}


#ifndef BLOCK_WRITE
void write_to_q(char *txt, struct txt_q *queue)
{
  struct txt_block *new;
  char tbuf[256];
  int strl;


  if (!queue) {
    log("Output message to non-existant queue");
    return;
  }

  CREATE(new, struct txt_block, 1);
  strl = strlen(txt);


  if (strl < 0 || strl > 35000) {
    log("strlen returned bogus length in write_to_q, string was: ");
    for(strl=0;strl<120;strl++) tbuf[strl]=txt[strl]; tbuf[strl]=0;
    log(strl);
	if (new)
	    free(new);
    return;
  }

#if 0 			/* Changed for test 2-21 added back to test... */
	memory_check("Before CREATE write_to_q",txt);
  CREATE(new->text, char, strl+1);
	memory_check("After CREATE write_to_q",txt);
  strcpy(new->text, txt);
	memory_check("After strcpy write_to_q",txt);
#else
	 if (!(new->text = strdup(txt))){
		log("strdup returned null");
		assert(0);
	}
#endif
	  new->next = NULL;

  /* Q empty? */
  if (!queue->head)  {
    queue->head = queue->tail = new;
  } else	{
    queue->tail->next = new;
    queue->tail = new;
  }
}

#else
void write_to_q(char *txt, struct txt_q *queue)
{
  struct txt_block *new;
  char tbuf[256];
  int strl;

  if (!queue) {
    log("Output message to non-existant queue");
    return;
  }

/*testlog(txt);*/

  CREATE(new, struct txt_block, 1);
  strl = strlen(txt);
  if (strl < 0 || strl > 45000) {
    log("strlen returned bogus length in write_to_q, string was:");
    for(strl=0;strl<120;strl++) tbuf[strl]=txt[strl]; tbuf[strl]=0;
    log(strl);
if (new)
    free(new);
    return;
  }
  new->text = strdup(txt);
  new->next = NULL;

  /* Q empty? */
  if (!queue->head)  {
    queue->head = queue->tail = new;
  } else	{
    queue->tail->next = new;
    queue->tail = new;
  }
}
#endif

#if BLOCK_WRITE
void write_to_output(char *txt, struct descriptor_data *t)
{
   int size;

   size = strlen(txt);

   /* if we're in the overflow state already, ignore this */
   if (t->bufptr < 0) {
      log("over flow stat in write_to_output, comm.c");
      assert(0);
      return;
      }

   /* if we have enough space, just write to buffer and that's it! */
   if (t->bufspace >= size) {
      strcpy(t->output+t->bufptr, txt);
      t->bufspace -= size;
      t->bufptr += size;
   } else {      /* otherwise, try to switch to a large buffer */
      if (t->large_outbuf || ((size + strlen(t->output)) > LARGE_BUFSIZE)) {
         /* we're already using large buffer, or even the large buffer
            in't big enough -- switch to overflow state */
         t->bufptr = -1;
         buf_overflows++;
         return;
      }

      buf_switches++;
      /* if the pool has a buffer in it, grab it */
      if (bufpool) {
         t->large_outbuf = bufpool;
         bufpool = bufpool->next;
      } else { /* else create one */
         CREATE(t->large_outbuf, struct txt_block, 1);
         CREATE(t->large_outbuf->text, char, LARGE_BUFSIZE);
         buf_largecount++;
      }

      strcpy(t->large_outbuf->text, t->output);
      t->output = t->large_outbuf->text;
      strcat(t->output, txt);
      t->bufspace = LARGE_BUFSIZE-1 - strlen(t->output);
      t->bufptr = strlen(t->output);
   }
}
#else
	/* we use write_to_q instead of write_to_output */
#endif


struct timeval timediff(struct timeval *a, struct timeval *b)
{
	struct timeval rslt, tmp;

	tmp = *a;

	if ((rslt.tv_usec = tmp.tv_usec - b->tv_usec) < 0)
	{
		rslt.tv_usec += 1000000;
		--(tmp.tv_sec);
	}
	if ((rslt.tv_sec = tmp.tv_sec - b->tv_sec) < 0)
	{
		rslt.tv_usec = 0;
		rslt.tv_sec =0;
	}
	return(rslt);
}





#ifndef BLOCK_WRITE
/* Empty the queues before closing connection */
void flush_queues(struct descriptor_data *d)
{
	char dummy[MAX_STRING_LENGTH];

	while (get_from_q(&d->output, dummy));
	while (get_from_q(&d->input, dummy));
}
#else
void flush_queues(struct descriptor_data *d)
{
   char buf2[MAX_STRING_LENGTH];

   if (d->large_outbuf) {
      d->large_outbuf->next = bufpool;
      bufpool = d->large_outbuf;
   }

   while (get_from_q(&d->input, buf2))
      ;
}
#endif






/* ******************************************************************
*  socket handling							 *
****************************************************************** */



#if 0
/* trying merc code out */
int init_socket(int port)
{
	int s;
	char *opt;
	char hostname[MAX_HOSTNAME+1]; 
	struct sockaddr_in sa;
	struct hostent *hp;
	struct linger ld;
	
	bzero(&sa, sizeof(struct sockaddr_in));
	gethostname(hostname, MAX_HOSTNAME);

	hp = gethostbyname(hostname);
	if (hp == NULL)	{
		perror("gethostbyname");
 		assert(0); 
	}

	sa.sin_family = hp->h_addrtype;
	sa.sin_port	= htons(port);
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) 	{
		perror("Init-socket");
		assert(0);
 	}
	if (setsockopt (s, SOL_SOCKET, SO_REUSEADDR,
		(char *) &opt, sizeof (opt)) < 0) 	{
		perror ("setsockopt REUSEADDR");
		exit (1);
	}

	ld.l_onoff = 1;
	ld.l_linger = 100;
	if (setsockopt(s, SOL_SOCKET, SO_LINGER, &ld, sizeof(ld)) < 0)	{
		perror("setsockopt LINGER");
		assert(0);
	}


#ifdef NETBSD
if ( bind( s, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
#else
	if (bind(s, &sa, sizeof(sa), 0) < 0)	
#endif	
        {
	    perror("bind");
	    exit(0);
	}

	listen(s, 5);
	return(s);
}
#else
int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
        perror( "Init_socket: socket" );
        exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof(x) ) < 0 )
    {
        perror( "Init_socket: SO_REUSEADDR" );
        close( fd );
        exit( 1 );
    }

    sa              = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port     = htons( port );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
    {
        perror( "Init_socket: bind" );
        close( fd );
        exit( 1 );
    }

    if ( listen( fd, 3 ) < 0 )
    {
        perror( "Init_socket: listen" );
        close( fd );
        exit( 1 );
    }

    return fd;
}
#endif




int new_connection(int s)
{
  struct sockaddr_in isa;
#ifdef sun
   struct sockaddr peer;
#endif
  int i;
  int t;
  char buf[100];
  
  i = sizeof(isa);
#if 0
  getsockname(s, &isa, &i);
#endif
  
  if ((t = accept(s, (struct sockaddr *)&isa, &i)) < 0){
    perror("Accept");
    return(-1);
  }
  nonblock(t);
  
#ifdef sun
    
  i = sizeof(peer);
  if (!getpeername(t, &peer, &i))	{
    *(peer.sa_data + 49) = '\0';
    sprintf(buf, "New connection from addr %s.", peer.sa_data);
    log(buf);
  }

#endif
  
  return(t);
}

int new_descriptor(int s)
{

  int desc,size,i;
  struct descriptor_data *newd;
  struct hostent *from;
  struct sockaddr_in sock;
  char buf[200],buf2[200];
 extern char login[];  
  if ((desc = new_connection(s)) < 0)
    return (-1);
  
  
  if ((desc + 1) >= MAX_CONNECTS) {
    struct descriptor_data *d;
    sprintf(buf,"Sorry.. The game is full. (max players %d) Try again later.\n\r",desc);
    write_to_descriptor(desc,buf);
    close(desc);
    
    for (d = descriptor_list; d; d = d->next) {
      if (!d->character)
	close_socket(d);
    }
    return(0);
  }
  else
    if (desc > maxdesc)
      maxdesc = desc;
  
  CREATE(newd, struct descriptor_data, 1);

#if 0
		/* older code replaced */
		
  *newd->host = '\0';
  /* find info */
  size = sizeof(sock);
  if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0)    {
    perror("getpeername");
    *newd->host = '\0';
  }
  
  if(*newd->host == '\0') {
#ifndef sun
    if ((long) strncpy(newd->host, inet_ntoa(sock.sin_addr), 49) > 0)  {
      *(newd->host + 49) = '\0';
      sprintf(buf, "New connection from addr %s: %d: %d", newd->host, desc, maxdesc);
      log_sev(buf,3);
    }
#else
    strcpy(newd->host, (char *)inet_ntoa(&sock.sin_addr));
#endif
  }
	
	/* end old code */
#endif

/*newer code */
   /* find info */
   size = sizeof(sock);
   if (getpeername(desc, (struct sockaddr *) & sock, &size) < 0) {
      perror("getpeername");
      *newd->host = '\0';
   } else if (IS_SET(SystemFlags,SYS_SKIPDNS) ||
      !(from = gethostbyaddr((char *)&sock.sin_addr,
      sizeof(sock.sin_addr), AF_INET))) {
         if (!IS_SET(SystemFlags,SYS_SKIPDNS))
            perror("gethostbyaddr");
         i = sock.sin_addr.s_addr;
          sprintf(newd->host, "%d.%d.%d.%d", (i & 0xFF000000) >> 24,
          (i & 0x00FF0000) >> 16, (i & 0x0000FF00) >> 8,
          (i & 0x000000FF));
   } else {
      strncpy(newd->host, from->h_name, 49);
      *(newd->host + 49) = '\0';
   }

#if 0
   if (isbanned(newd->host) == BAN_ALL) {
      close(desc);
      sprintf(buf2, "Connection attempt denied from [%s]", newd->host);
      log(buf2);
if (newd)
      free(newd);
      return(0);
   }
#endif 

   sprintf(buf2, "New connection from addr %s: %d: %d", newd->host, desc, maxdesc);
   log(buf2);

#if 0
  identd_test(sock);	/* test stuff */
#endif
  
/* end newer code */

  /* init desc data */
  newd->descriptor = desc;
  newd->connected  = CON_NME;
  newd->wait = 1;
  newd->prompt_mode = 0;
  *newd->buf = '\0';
  newd->str = 0;
  newd->showstr_head = 0;
  newd->showstr_point = 0;
  *newd->last_input= '\0';
#ifndef BLOCK_WRITE
   newd->output.head = NULL;
#else
   newd->output=newd->small_outbuf;
   *(newd->output)='\0';
   newd->bufspace=SMALL_BUFSIZE-1;
   newd->large_outbuf=NULL;
#endif
  newd->input.head = NULL;
  newd->next = descriptor_list;
  newd->character = 0;
  newd->original = 0;
  newd->snoop.snooping = 0;
  newd->snoop.snoop_by = 0;
  
  /* prepend to list */
  
  descriptor_list = newd;
  
  SEND_TO_Q(login, newd);
  SEND_TO_Q("What is thy name? ", newd);

  return(0);
}



#ifndef BLOCK_WRITE
int process_output(struct descriptor_data *t)
{
  char i[MAX_STRING_LENGTH + MAX_STRING_LENGTH];
  
  if (!t->prompt_mode && !t->connected)
    if (write_to_descriptor(t->descriptor, "\n\r") < 0)
      return(-1);
  
  
  /* Cycle thru output queue */
  while (get_from_q(&t->output, i))	{  
    if ((t->snoop.snoop_by) && (t->snoop.snoop_by->desc)) {
      SEND_TO_Q("% ",t->snoop.snoop_by->desc);
      SEND_TO_Q(i,t->snoop.snoop_by->desc);
    }
    if (write_to_descriptor(t->descriptor, i))
      return(-1);
  }
  
  if (!t->connected && !(t->character && !IS_NPC(t->character) && 
	 IS_SET(t->character->specials.act, PLR_COMPACT)))
    if (write_to_descriptor(t->descriptor, "\n\r") < 0)
      return(-1);
  
  return(1);
}
#else

/* SEARCH HERE */
int process_output(struct descriptor_data *t)
{
	static char i[LARGE_BUFSIZE + 20];
  struct descriptor_data *point, *next_point;

   /* start writing at the 2nd space so we can prepend "% " for snoop */
   if (!t->prompt_mode && !t->connected) {
      strcpy(i+2, "\n\r");
      strcat(i+2, t->output);
   } else
      strcpy(i+2, t->output);

   if (t->bufptr < 0) {
       log("***** OVER FLOW **** in process_output, comm.c");
       strcat(i+2, "**OVERFLOW**");
      }

  if (!t->connected && !(t->character && !IS_NPC(t->character) && 
	 IS_SET(t->character->specials.act, PLR_COMPACT)))
	strcat(i+2, "\n\r");

   if (write_to_descriptor(t->descriptor, i+2) < 0)
      return -1;

   if (t->snoop.snoop_by) {
      i[0] = '%';
      i[1] = ' ';
      SEND_TO_Q(i, t->snoop.snoop_by->desc);
   }

   /* if we were using a large buffer, put the large buffer on the buffer
      pool and switch back to the small one */
   if (t->large_outbuf) {
      t->large_outbuf->next = bufpool;
      bufpool = t->large_outbuf;
      t->large_outbuf = NULL;
      t->output = t->small_outbuf;
   }

   /* reset total bufspace back to that of a small buffer */
   t->bufspace = SMALL_BUFSIZE-1;
   t->bufptr = 0;
   *(t->output) = '\0';

	return(1);
}

#endif

#if 1	 /* reset to use this code, lets see if it helps */
int write_to_descriptor(int desc, char *txt)
{
  int sofar, thisround, total;
  
  total = strlen(txt);
  sofar = 0;
  
  do {
      thisround = write(desc, txt + sofar, total - sofar);
      if (thisround < 0)	{
	  if (errno == EWOULDBLOCK)
	     break;
	  perror("Write to socket");
 /* 
  lets see if this stops it from crashing close_socket_fd(desc);
  arioch 
 */
	  return(-1);
	}
      sofar += thisround;
  } 
  while (sofar < total);

  
  return(0);
}
#else
/* merc code */
#define UMIN(a, b)              ((a) < (b) ? (a) : (b))
int write_to_descriptor( int desc, char *txt)
{
    int iStart;
    int nWrite;
    int nBlock, length;
    char buf[256];
    
        length = strlen(txt);

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
        nBlock = UMIN( length - iStart, 4096 );
        if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
            {
	     if (errno == EWOULDBLOCK)
	       break;
       sprintf(buf,"<#=%d> had a error (%d) in write to descriptor (Broken Pipe?)",
       desc, errno);
       log(buf);
             perror( "Write_to_descriptor" );
	     /* close_socket_fd(desc); */
	     return(-1); 
	    }
    }

    return(0); 
}

#endif




/* SEARCH HERE */
#ifndef BLOCK_WRITE
/* base silly process_input */
int process_input(struct descriptor_data *t)
{
  int sofar, thisround, begin, squelch, i, k, flag;
  char tmp[MAX_INPUT_LENGTH+2], buffer[MAX_INPUT_LENGTH + 60];
  
  sofar = 0;
  flag = 0;
  begin = strlen(t->buf);
  
  /* Read in some stuff */
  do  {
    if ((thisround = read(t->descriptor, t->buf + begin + sofar, 
			  (8*MAX_INPUT_LENGTH) - (begin + sofar) - 1)) > 0) {
#if 0
			  MAX_STRING_LENGTH - (begin + sofar) - 1)) > 0) {
#endif			  
      sofar += thisround;
    } else {
      if (thisround < 0) {
	if (errno != EWOULDBLOCK) {
	  perror("Read1 - ERROR");
	  return(-1);
	} else {
	  break;
	}
      } else {
	log("EOF encountered on socket read.");
	return(-1);
      }
    }
  } while (!ISNEWL(*(t->buf + begin + sofar - 1)));	
  
  *(t->buf + begin + sofar) = 0;
  
  /* if no newline is contained in input, return without proc'ing */
  for (i = begin; !ISNEWL(*(t->buf + i)); i++)
    if (!*(t->buf + i))
      return(0);
  
  /* input contains 1 or more newlines; process the stuff */
  for (i = 0, k = 0; *(t->buf + i);)	{
    if (!ISNEWL(*(t->buf + i)) && !(flag=(k>=(MAX_INPUT_LENGTH - 2))))
      if (*(t->buf + i) == '\b') {	 /* backspace */
	if (k) { /* more than one char ? */
	  if (*(tmp + --k) == '$')
	    k--;				
	  i++;
	} else {
	  i++;  /* no or just one char.. Skip backsp */
	}
      } else {
	if (isascii(*(t->buf + i)) && isprint(*(t->buf + i))) {
	  /* 
	    trans char, double for '$' (printf)	
	    */
	  if ((*(tmp + k) = *(t->buf + i)) == '$')
	    *(tmp + ++k) = '$';
	  k++;
	  i++;
	} else {
	  i++;
	}
      } else 	{
	*(tmp + k) = 0;
	if(*tmp == '!')
	  strcpy(tmp,t->last_input);
	else
	  strcpy(t->last_input,tmp);
	
	write_to_q(tmp, &t->input);
	
	if ((t->snoop.snoop_by) && (t->snoop.snoop_by->desc)){
	  write_to_q("% ",&t->snoop.snoop_by->desc->output);
	  write_to_q(tmp,&t->snoop.snoop_by->desc->output);
	  write_to_q("\n\r",&t->snoop.snoop_by->desc->output);
	}
	
	if (flag) {
	  sprintf(buffer, 
		  "Line too long. Truncated to:\n\r%s\n\r", tmp);
	  if (write_to_descriptor(t->descriptor, buffer) < 0)
	    return(-1);
	  
	  /* skip the rest of the line */
	  for (; !ISNEWL(*(t->buf + i)); i++);
	}
	
	/* find end of entry */
	for (; ISNEWL(*(t->buf + i)); i++);
	
	/* squelch the entry from the buffer */
	for (squelch = 0;; squelch++)
	  if ((*(t->buf + squelch) = 
	       *(t->buf + i + squelch)) == '\0')
	    break;
	k = 0;
	i = 0;
      }
  }
  return(1);
}

#else

int process_input(struct descriptor_data *t)
{
  int sofar, thisround, begin, squelch, i, k, flag;
  char tmp[MAX_INPUT_LENGTH+2], buffer[MAX_INPUT_LENGTH + 60];
  
  sofar = 0;
  flag = 0;
  begin = strlen(t->buf);
  
  /* Read in some stuff */
  do  {
    if ((thisround = read(t->descriptor, t->buf + begin + sofar, 
			  (8*MAX_INPUT_LENGTH) - (begin + sofar) - 1)) > 0) {
      sofar += thisround;
    } else {
      if (thisround < 0) {
	if (errno != EWOULDBLOCK) {
	  perror("Read1 - ERROR");
	  return(-1);
	} else {
	  break;
	}
      } else {
	log("EOF encountered on socket read.");
	return(-1);
      }
    }
  } while (!ISNEWL(*(t->buf + begin + sofar - 1)));	
  
  *(t->buf + begin + sofar) = 0;
  
  /* if no newline is contained in input, return without proc'ing */
  for (i = begin; !ISNEWL(*(t->buf + i)); i++)
    if (!*(t->buf + i))
      return(0);
  
  /* input contains 1 or more newlines; process the stuff */
  for (i = 0, k = 0; *(t->buf + i);)	{
    if (!ISNEWL(*(t->buf + i)) && !(flag=(k>=(MAX_INPUT_LENGTH - 2))))
      if (*(t->buf + i) == '\b') {	 /* backspace */
	if (k) { /* more than one char ? */
	  if (*(tmp + --k) == '$')
	    k--;				
	  i++;
	} else {
	  i++;  /* no or just one char.. Skip backsp */
	}
      } else {
	if (isascii(*(t->buf + i)) && isprint(*(t->buf + i))) {
	  /* 
	    trans char, double for '$' (printf)	
	    */
	  if ((*(tmp + k) = *(t->buf + i)) == '$')
	    *(tmp + ++k) = '$';
	  k++;
	  i++;
	} else {
	  i++;
	}
      } else 	{
	*(tmp + k) = 0;
	if(*tmp == '!')
	  strcpy(tmp,t->last_input);
	else
	  strcpy(t->last_input,tmp);
	
	write_to_q(tmp, &t->input);
	
	if ((t->snoop.snoop_by) && (t->snoop.snoop_by->desc)){
	  SEND_TO_Q("% ",t->snoop.snoop_by->desc);
	  SEND_TO_Q(tmp,t->snoop.snoop_by->desc);
	  SEND_TO_Q("\n\r",t->snoop.snoop_by->desc);
	}
	
	if (flag) {
	  sprintf(buffer, 
		  "Line too long. Truncated to:\n\r%s\n\r", tmp);
	  if (write_to_descriptor(t->descriptor, buffer) < 0)
	    return(-1);
	  
	  /* skip the rest of the line */
	  for (; !ISNEWL(*(t->buf + i)); i++);
	}
	
	/* find end of entry */
	for (; ISNEWL(*(t->buf + i)); i++);
	
	/* squelch the entry from the buffer */
	for (squelch = 0;; squelch++)
	  if ((*(t->buf + squelch) = 
	       *(t->buf + i + squelch)) == '\0')
	    break;
	k = 0;
	i = 0;
      }
  }
  return(1);
}
#endif



void close_sockets(int s)
{
  log("Closing all sockets.");
  
  while (descriptor_list)
    close_socket(descriptor_list);
  
  close(s);
}





void close_socket(struct descriptor_data *d)
{
  char buf[MAX_STRING_LENGTH];
  struct txt_block *txt, *txt2;


  if (!d) {
	log("!d in close_socket");
	  return;
  }

#if LOG_DEBUG  
slog("begin close_socket");
#endif

  close(d->descriptor);
  flush_queues(d);
  if (d->descriptor == maxdesc)
    --maxdesc;
  
  /* Forget snooping */
  if (d->snoop.snooping)
    d->snoop.snooping->desc->snoop.snoop_by = 0;
  
  if (d->snoop.snoop_by)		{
    send_to_char("Your victim is no longer among us.\n\r",d->snoop.snoop_by);
    d->snoop.snoop_by->desc->snoop.snooping = 0;
  }
  
  if (d->character)
    if (d->connected == CON_PLYNG) 	{
       do_save(d->character, "", 0);
      act("$n has lost $s touch with reality.", TRUE, d->character, 0, 0, TO_ROOM);
      sprintf(buf, "Closing link to: %s.", GET_NAME(d->character));
      log(buf);
      if (IS_NPC(d->character)) { /* poly, or switched god */
	if (d->character->desc)
	  d->character->orig = d->character->desc->original;
      }
      d->character->desc = 0;

/*      d->character->invis_level = LOW_IMMORTAL;  msw 8/9/94 */
      
     if (!IS_AFFECTED(d->character, AFF_CHARM)) {
	if (d->character->master) {
	  stop_follower(d->character);
	}
      }

    } else {
      if (GET_NAME(d->character)) {
	sprintf(buf, "Losing player: %s.", GET_NAME(d->character));
	log(buf);
      }
      free_char(d->character);
    }
  else
    log("Losing descriptor without char.");
  
  
  if (next_to_process == d)    	/* to avoid crashing the process loop */
    next_to_process = next_to_process->next;   
  
  if (d == descriptor_list) /* this is the head of the list */
    descriptor_list = descriptor_list->next;
  else  /* This is somewhere inside the list */    
{

  struct descriptor_data *tmp;
    				/* Locate the previous element */
    for (tmp = descriptor_list; tmp && tmp->next != d;tmp = tmp->next);

         if (tmp != NULL) 
             tmp->next = d->next;
          else   {
	log(" ERROR< ERROR< ERROR< ERROR< corrupted list...");
  /* not sure where this gets fried, but it keeps poping up now and */
  /* then, let me know if you figure it out. msw 		    */
  
  /* DENIS see if you can figure this out */
  
  /* I know that when a new user cuts link in the login process the first */
  /* time it will get here, how to stop it I do not know		  */
    } /* end bug */

  }/* end inside the list */ 

	if (d->showstr_head)   /* this piece of code causes core dumps on */
	 free(d->showstr_head);   /* ardent titans */
	if (d)
	 free(d);
   
#if LOG_DEBUG
  slog("end close_socket");
#endif
}





void nonblock(int s)
{
  if (fcntl(s, F_SETFL, FNDELAY) == -1)    {
    perror("Noblock");
    assert(0);
  }
}




#define COMA_SIGN \
"\n\r\
DikuMUD is currently inactive due to excessive load on the host machine.\n\r\
Please try again later.\n\r\n\
\n\r\
   Sadly,\n\r\
\n\r\
    the DikuMUD system operators\n\r\n\r"


/* sleep while the load is too high */
void coma(int s)
{
#ifdef USE_LAWFUL

  fd_set input_set;
  static struct timeval timeout =
    {
      60, 
      0
      };
  int conn;
  
  int workhours(void);
  int load(void);
  
  log("Entering comatose state.");
  
  sigsetmask(sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
	     sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
	     sigmask(SIGURG) | sigmask(SIGXCPU) | sigmask(SIGHUP));
  
  
  while (descriptor_list)
    close_socket(descriptor_list);
  
  FD_ZERO(&input_set);
  do {
    FD_SET(s, &input_set);
    if (select(64, &input_set, 0, 0, &timeout) < 0){
      perror("coma select");
      assert(0);
    }
    if (FD_ISSET(s, &input_set))	{
      if (load() < 6){
	log("Leaving coma with visitor.");
	sigsetmask(0);
	return;
      }
      if ((conn = new_connection(s)) >= 0)     {
	write_to_descriptor(conn, COMA_SIGN);
	sleep(2);
	close(conn);
      }
    }			
    
    tics = 1;
    if (workhours())  {
      log("Working hours collision during coma. Exit.");
      assert(0);
    }
  }
  while (load() >= 6);
  
  log("Leaving coma.");
  sigsetmask(0);
#endif
}



/* ****************************************************************
*	Public routines for system-to-player-communication	  *
**************************************************************** */


char *ParseAnsiColors(int UsingAnsi, char *txt)
{
  static char buf[MAX_STRING_LENGTH] = "";
  char tmp[MAX_INPUT_LENGTH];
  
  register int i,l,f=0;

  buf[0]=0;	
  for(i=0,l=0;*txt;) {
    if(*txt=='$' && (toupper(*(txt+1)) == 'C' || 
                    (*(txt+1)=='$' && toupper(*(txt+2)) == 'C'))) {
      if(*(txt+1)=='$')
        txt+=3;
      else
        txt+=2;
      str2ansi(tmp,txt,0,3);
      
      /* if using ANSI */
      if (UsingAnsi)
           strcat(buf,ansi_parse(tmp));
           else
      /* if not using ANSI   */
          strcat(buf,"");   
      
      txt+=4;
      l=strlen(buf);
      f++;
    }
    else {
      buf[l++]=*txt++;
    }
    buf[l]=0;
  }
  if(f && UsingAnsi)
    strcat(buf,ansi_parse("0007"));
 
return buf;
}


void send_to_char(char *messg, struct char_data *ch)
{
  if (ch)
     if (ch->desc && messg)
	SEND_TO_Q(
	 ParseAnsiColors(IS_SET(ch->player.user_flags, USE_ANSI),messg),
	  ch->desc); 
}


void save_all()
{
  struct descriptor_data *i;
  
  for (i = descriptor_list; i; i = i->next)
    if (i->character)
      save_char(i->character,AUTO_RENT);
}

void send_to_all(char *messg)
{
  struct descriptor_data *i;
  
  if (messg)
    for (i = descriptor_list; i; i = i->next)
      if (!i->connected)
/*	SEND_TO_Q(messg, &i->output); */
	SEND_TO_Q(
	 ParseAnsiColors(IS_SET(i->character->player.user_flags, USE_ANSI),messg),
	  i); 

}


void send_to_outdoor(char *messg)
{
  struct descriptor_data *i;
  
  if (messg)
    for (i = descriptor_list; i; i = i->next)
      if (!i->connected)
	if (OUTSIDE(i->character))
/*	  SEND_TO_Q(messg, &i->output); */
	SEND_TO_Q(
	 ParseAnsiColors(IS_SET(i->character->player.user_flags, USE_ANSI),messg),
	  i); 
}

void send_to_desert(char *messg)
{
  struct descriptor_data *i;
  struct room_data *rp;
  extern struct zone_data *zone_table;

  if (messg) {
    for (i = descriptor_list; i; i = i->next) {
      if (!i->connected) {
	if (OUTSIDE(i->character)) {
	  if ((rp = real_roomp(i->character->in_room))!=NULL) {
	    if (IS_SET(zone_table[rp->zone].reset_mode, ZONE_DESERT) ||
		rp->sector_type == SECT_DESERT) {
/*	      SEND_TO_Q(messg, &i->output); */
	SEND_TO_Q(
	 ParseAnsiColors(IS_SET(i->character->player.user_flags, USE_ANSI),messg),
	  i); 

	    }
	  }
	}
      }
    }
  }
}

void send_to_out_other(char *messg)
{
  struct descriptor_data *i;
  struct room_data *rp;
  extern struct zone_data *zone_table;

  if (messg) {
    for (i = descriptor_list; i; i = i->next) {
      if (!i->connected) {
	if (OUTSIDE(i->character)) {
	  if ((rp = real_roomp(i->character->in_room))!=NULL) {
	    if (!IS_SET(zone_table[rp->zone].reset_mode, ZONE_DESERT) &&
	        !IS_SET(zone_table[rp->zone].reset_mode, ZONE_ARCTIC) &&
		rp->sector_type != SECT_DESERT) {
/*	      SEND_TO_Q(messg, &i->output); */
	SEND_TO_Q(
	 ParseAnsiColors(IS_SET(i->character->player.user_flags, USE_ANSI),messg),
	  i); 

	    }
	  }
	}
      }
    }
  }
}


void send_to_arctic(char *messg)
{
  struct descriptor_data *i;
  struct room_data *rp;
  extern struct zone_data *zone_table;

  if (messg) {
    for (i = descriptor_list; i; i = i->next) {
      if (!i->connected) {
	if (OUTSIDE(i->character)) {
	  if ((rp = real_roomp(i->character->in_room))!=NULL) {
	    if (IS_SET(zone_table[rp->zone].reset_mode, ZONE_ARCTIC)) {
/*	      SEND_TO_Q(messg, &i->output); */
	SEND_TO_Q(
	 ParseAnsiColors(IS_SET(i->character->player.user_flags, USE_ANSI),messg),
	  i); 

	    }
	  }
	}
      }
    }
  }
}


void send_to_except(char *messg, struct char_data *ch)
{
  struct descriptor_data *i;
  
  if (messg)
    for (i = descriptor_list; i; i = i->next)
      if (ch->desc != i && !i->connected)
/*	SEND_TO_Q(messg, &i->output); */
	SEND_TO_Q(
	 ParseAnsiColors(IS_SET(i->character->player.user_flags, USE_ANSI),messg),
	  i); 

}


void send_to_zone(char *messg, struct char_data *ch)
{
  struct descriptor_data *i;
  
  if (messg)
    for (i = descriptor_list; i; i = i->next)
      if (ch->desc != i && !i->connected)
	if (real_roomp(i->character->in_room)->zone == 
	    real_roomp(ch->in_room)->zone)
/* 	  SEND_TO_Q(messg, &i->output); */
	SEND_TO_Q(
	 ParseAnsiColors(IS_SET(i->character->player.user_flags, USE_ANSI),messg),
	  i); 

}



void send_to_room(char *messg, int room)
{
  struct char_data *i;
  
  if (messg)
    for (i = real_roomp(room)->people; i; i = i->next_in_room)
      if (i->desc)
/* 	SEND_TO_Q(messg, &i->desc->output);*/
	SEND_TO_Q(
	 ParseAnsiColors(IS_SET(i->player.user_flags, USE_ANSI),messg),
	  i->desc); 

}




void send_to_room_except(char *messg, int room, struct char_data *ch)
{
  struct char_data *i;
  
  if (messg)
    for (i = real_roomp(room)->people; i; i = i->next_in_room)
      if (i != ch && i->desc)
/* 	SEND_TO_Q(messg, &i->desc->output); */
	SEND_TO_Q(
	 ParseAnsiColors(IS_SET(i->player.user_flags, USE_ANSI),messg),
	  i->desc); 

}

void send_to_room_except_two
  (char *messg, int room, struct char_data *ch1, struct char_data *ch2)
{
  struct char_data *i;
  
  if (messg)
    for (i = real_roomp(room)->people; i; i = i->next_in_room)
      if (i != ch1 && i != ch2 && i->desc)
/* 	SEND_TO_Q(messg, &i->desc->output); */
	SEND_TO_Q(
	 ParseAnsiColors(IS_SET(i->player.user_flags, USE_ANSI),messg),
	  i->desc); 

}


void str2ansi(char *p2, char *p1, int start, int stop)
{
	int i,j;

	if((start > stop) || (start < 0))
		p2[0] = '\0';    /* null terminate string */
	else
	{
		if (start == stop)        /* will copy only 1 char at pos=start */
		{
			p2[0] = p1[start];
			p2[1] = '\0';
		}
		else {
			j = 0;

            /* start or (start-1) depends on start index */
            /* if starting index for arrays is 0 then use start */
            /* if starting index for arrays is 1 then use start-1 */

            for (i=start;i<=stop;i++)   
				 p2[j++] = p1[i];
			p2[j] = '\0';    /* null terminate the string */
		}
	}

if (strlen(p2)+1 > 5)
	log("DOH!");  /* remove this after test period */
}

/* higher-level communication */

/* ACT */
void act(char *str, int hide_invisible, struct char_data *ch,
	 struct obj_data *obj, void *vict_obj, int type)
{
  register char *strp, *point, *i;
  int KLUDGE = FALSE;
  struct char_data *to;
  char buf[MAX_STRING_LENGTH], tmp[MAX_INPUT_LENGTH];

  extern int WizLock;
  
  if (!str)
    return;
  if (!*str)
    return;
  
  if (ch->in_room <= -1)
    return;  /* can't do it. in room -1 */

/*testlog(str);*/
	
  if (type == TO_VICT)
    to = (struct char_data *) vict_obj;
  else if (type == TO_CHAR)
    to = ch;
  else
    to = real_roomp(ch->in_room)->people;
  
  for (; to; to = to->next_in_room)	{
    if (to->desc && 
    ((to != ch) || (type == TO_CHAR)) && (CAN_SEE(to, ch) 
    || !hide_invisible) && AWAKE(to) &&
    !((type == TO_NOTVICT) &&
      (to==(struct char_data *) vict_obj))){
      for (strp = str, point = buf;;)
	if (*strp == '$') {
	  switch (*(++strp)) {

#if 1

/*	  parse ansi colors here 
    $CMBFG, where M is modier, B is back ground color and FG is fore 
    $C0001 would be normal, black back, red fore.
    $C1411 would be bold, blue back, light yellow fore 
*/    
act_switch_c:
	  case 'C': 
	  case 'c': if (IS_SET(to->player.user_flags, USE_ANSI)) {
	  		KLUDGE=TRUE;
		       str2ansi(tmp,strp,1,4);
	  	       i = ansi_parse(tmp); 
		     } else
	  		i = "";
		       ++strp;++strp;++strp;++strp; /* delete nums */
                     break;
#endif	  	   
	  case 'n': i = PERS(ch, to); 
	    break;
	  case 'N': i = PERS((struct char_data *) vict_obj, to); 
	    break;
	  case 'm': i = HMHR(ch); 
	    break;
	  case 'M': i = HMHR((struct char_data *) vict_obj); 
	    break;
	  case 's': i = HSHR(ch); 
	    break;
	  case 'S': i = HSHR((struct char_data *) vict_obj); 
	    break;
	  case 'e': i = HSSH(ch); 
	    break;
	  case 'E': i = HSSH((struct char_data *) vict_obj); 
	    break;
	  case 'o': i = OBJN(obj, to); 
	    break;
	  case 'O': i = OBJN((struct obj_data *) vict_obj, to); 
	    break;
	  case 'p': i = OBJS(obj, to); 
	    break;
	  case 'P': i = OBJS((struct obj_data *) vict_obj, to); 
	    break;
	  case 'a': i = SANA(obj); 
	    break;
	  case 'A': i = SANA((struct obj_data *) vict_obj); 
	    break;
	  case 'T': i = (char *) vict_obj; 
	    break;
	  case 'F': i = fname((char *) vict_obj); 
	    break;
	  case '$': 
            if(*strp=='$' && toupper(*(strp+1))=='C' && 
                 isdigit(*(strp+2)) && isdigit(*(strp+3)) ) {
               strp++;
               goto act_switch_c;
            }
            i = "$"; 
	    break;
	  default:
	    log("Illegal $-code to act():");
	    log(str);
	    break;
	  }
	  
	  while (*point = *(i++))
	    ++point;
	  
	  ++strp;
	  
	}	else if (!(*(point++) = *(strp++)))
	  break;
      
      *(--point) = '\n';
      *(++point) = '\r';
      *(++point) = '\0';
      
      SEND_TO_Q(CAP(buf), to->desc);
      
/* kludge to turn the color back grey, try to move to prompt */
  if (KLUDGE && IS_SET(to->player.user_flags,USE_ANSI))
       SEND_TO_Q(ansi_parse("0007"),to->desc);
    }
    
    if ((type == TO_VICT) || (type == TO_CHAR)) {
      return;
      }
  }
}

int raw_force_all( char *to_force)
{
  struct descriptor_data *i;
  char buf[400];

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected) {
      sprintf(buf, "The game has forced you to '%s'.\n\r", to_force);
      send_to_char(buf, i->character);
      command_interpreter(i->character, to_force);
    }
}

int _affected_by_s(struct char_data *ch, int skill)
{
    struct affected_type *hjp;
    int  fs=0,fa=0;
 
    switch(skill) {
        case SPELL_FIRESHIELD:
            if(IS_AFFECTED(ch,AFF_FIRESHIELD)) fa=1; break;
        case SPELL_SANCTUARY:
            if(IS_AFFECTED(ch,AFF_SANCTUARY))  fa=1; break;
        case SPELL_INVISIBLE:
            if(IS_AFFECTED(ch,AFF_INVISIBLE))  fa=1; break;
        case SPELL_TRUE_SIGHT:
            if(IS_AFFECTED(ch,AFF_TRUE_SIGHT)) fa=1; break;
        case SPELL_PROT_ENERGY_DRAIN:
            if(IS_SET(ch->immune,IMM_DRAIN) || IS_SET(ch->M_immune,IMM_DRAIN))
                fa=1;
            break;
    }
    if(ch->affected)
        for (hjp = ch->affected; hjp; hjp = hjp->next)
            if ( hjp->type == skill )
                fs = (hjp->duration + 1);   /* in case it's 0 */
 
         if(!fa && !fs) return -1;
    else if( fa && !fs) return 999;
    else return fs-1;
}

int construct_prompt(char *outbuf, struct char_data *ch)
{
    struct room_data *rm;
    extern const struct title_type titles[MAX_CLASS][ABS_MAX_LVL];
    char tbuf[255],*pr_scan,*mask;
    long l,exp,texp;
    int i,s_flag=0;
 
    *outbuf=0;
 
    if(ch->specials.prompt==NULL) { /* use default prompts */
        if(IS_IMMORTAL(ch))
            mask="Shadowdale: (type help prompt) H:%h R:%R i%iI+> ";
        else
            mask="Shadowdale: (type help prompt) H:%h M:%m V:%v> ";
    } else {
        mask=ch->specials.prompt;
    }
    for(pr_scan = mask; *pr_scan; pr_scan++) {
        if(*pr_scan == '%') {
            if(*(++pr_scan)=='%') {
                tbuf[0]='%';
                tbuf[1]=0;
            } else {
                switch(*pr_scan) {
                            /* stats for character */
                case 'H': sprintf(tbuf,"%d",GET_MAX_HIT(ch)); break;
                case 'h': sprintf(tbuf,"%d",GET_HIT(ch)); break;
                case 'M': sprintf(tbuf,"%d",GET_MAX_MANA(ch)); break;
                case 'm': sprintf(tbuf,"%d",GET_MANA(ch)); break;
                case 'V': sprintf(tbuf,"%d",GET_MAX_MOVE(ch)); break;
                case 'v': sprintf(tbuf,"%d",GET_MOVE(ch)); break;
                case 'G': sprintf(tbuf,"%ld",GET_BANK(ch)); break;
                case 'g': sprintf(tbuf,"%ld",GET_GOLD(ch)); break;
                case 'X': /* xp stuff */
                    sprintf(tbuf,"%ld",GET_EXP(ch));
                    break;
                case 'x': /* xp left to level (any level, btw..) */
                    for(l=1,i=0,exp=999999999;i<PSI_LEVEL_IND;i++,l<<=1) {
                        if(HasClass(ch,l)) {
                            texp=(titles[i][GET_LEVEL(ch,i)+1].exp)-GET_EXP(ch);
                            if(texp<exp)
                                exp=texp;
                        }
                    }
                    sprintf(tbuf,"%ld",exp);
                    break;
                case 'C':   /* mob condition */
#if 1
		/* we get float errors here! msw */
                    if(ch->specials.fighting) {
	     		if (GET_MAX_HIT(ch->specials.fighting)==0)
     				strcpy(tbuf,"unknown"); else
		{     				
                        i=(100*GET_HIT(ch->specials.fighting))/ \
                        GET_MAX_HIT(ch->specials.fighting);
                             if(i>=100)strcpy(tbuf,"excellent");
                        else if(i>=90) strcpy(tbuf,"few scratches");
                        else if(i>=75) strcpy(tbuf,"small wounds");
                        else if(i>=50) strcpy(tbuf,"wounded");
                        else if(i>=30) strcpy(tbuf,"big nasty");
                        else if(i>=15) strcpy(tbuf,"badly wounded");
                        else if(i>=0)  strcpy(tbuf,"awful");
                        else           strcpy(tbuf,"bleeding");
		}                        
                    } else {
                        strcpy(tbuf,"*");
                    }
#endif

                    break;
                case 'c':   /* tank condition */
#if 0

                    if(ch->specials.fighting && ch->specials.fighting->specials.fighting) {
	if (GET_MAX_HIT(ch->specials.fighting->specials.fighting)==0)
		strcpy(tbuf,"Unknown"); else
		{
                        i=(100*GET_HIT(ch->specials.fighting->specials.fighting))/ \
                        GET_MAX_HIT(ch->specials.fighting->specials.fighting);
                             if(i>=100)strcpy(tbuf,"excellent");
                        else if(i>=90) strcpy(tbuf,"few scratches");
                        else if(i>=75) strcpy(tbuf,"small wounds");
                        else if(i>=50) strcpy(tbuf,"wounded");
                        else if(i>=30) strcpy(tbuf,"big nasty");
                        else if(i>=15) strcpy(tbuf,"badly wounded");
                        else if(i>=0)  strcpy(tbuf,"awful");
                        else           strcpy(tbuf,"bleeding");
		}                        
                    } else {
                        strcpy(tbuf,"*");
                    }
#endif

                    break;
                case 's':
		    s_flag = 1;
                case 'S':   /* affected spells */
                    *tbuf=0;
		    if((i=_affected_by_s(ch,SPELL_FIRESHIELD))!=-1) 
		        strcat(tbuf,(i>1)?"F":"f");
      		    else if(s_flag) 
			strcat(tbuf,"-"); 
		    if((i=_affected_by_s(ch,SPELL_SANCTUARY))!=-1) 
		        strcat(tbuf,(i>1)?"S":"s");
      		    else if(s_flag) 
			strcat(tbuf,"-"); 
		    if((i=_affected_by_s(ch,SPELL_INVISIBLE))!=-1) 
		        strcat(tbuf,(i>1)?"I":"i");
      		    else if(s_flag) 
			strcat(tbuf,"-"); 
		    if((i=_affected_by_s(ch,SPELL_TRUE_SIGHT))!=-1) 
		        strcat(tbuf,(i>1)?"T":"t");
      		    else if(s_flag) 
			strcat(tbuf,"-"); 
		    if((i=_affected_by_s(ch,SPELL_PROT_ENERGY_DRAIN))!=-1) 
		        strcat(tbuf,(i>1)?"D":"d");
      		    else if(s_flag) 
			strcat(tbuf,"-"); 
		    if((i=_affected_by_s(ch,SPELL_ANTI_MAGIC_SHELL))!=-1) 
		        strcat(tbuf,(i>1)?"A":"a");
      		    else if(s_flag) 
			strcat(tbuf,"-"); 
                    break;
                case 'T':   /* did't implemented.. yet */
                    break;
                case 'R': /* room number for immortals */
                    if(IS_IMMORTAL(ch)) {
                        rm = real_roomp(ch->in_room);
                        if (!rm) {
                            char_to_room(ch, 0);
                            rm = real_roomp(ch->in_room);
                        }
                        sprintf(tbuf,"%ld",rm->number);
                    } else {
                        *tbuf=0;
                    }
                    break;
                case 'i':   /* immortal stuff going */
                    pr_scan++;
                    if(!IS_IMMORTAL(ch)) { 
                      *tbuf=0; 
                      break; 
                    }
                    switch(*pr_scan) {
                    case 'I':   /* invisible status */
                        sprintf(tbuf,"%d",ch->invis_level);
                        break;
                    case 'S':   /* stealth mode */
                        strcpy(tbuf,IS_SET(ch->specials.act, PLR_STEALTH)?"On":"Off");
                        break;
                    case 'N':   /* snoop name */
                        if(ch->desc->snoop.snooping)
                            strcpy(tbuf,ch->desc->snoop.snooping->player.name);
			else 
			    *tbuf=0;
                        break;
                    default:
/*
                        sprintf(tbuf,"Invalid Immmortal Prompt code '%c'",*pr_scan);
                        log(tbuf);
*/
                        *tbuf=0;
                        break;
                    }
                    break;
                default:
/*  
                    sprintf(tbuf,"Invalid Prompt code '%c'",*pr_scan);
                    log(tbuf);
*/
                    *tbuf=0;
                    break;
                }
            }
        } else { /* end of if ch=='%'  */
            tbuf[0]=*pr_scan;
            tbuf[1]=0;
        }
        strcat(outbuf,tbuf);
    }
}


void UpdateScreen(struct char_data *ch, int update)
{
 char buf[255];
 int size;
 
 size = ch->size;

if (size<=0)
    return;
   
 if(IS_SET(update, INFO_MANA)) {
    sprintf(buf, VT_CURSAVE);
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, VT_CURSPOS, size - 2, 7);
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, "          ");
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, VT_CURSPOS, size - 2, 7);
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, "%d(%d)", GET_MANA(ch), GET_MAX_MANA(ch));
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, VT_CURREST);
    write_to_descriptor(ch->desc->descriptor, buf);
  }
 
 if(IS_SET(update, INFO_MOVE)) {
    sprintf(buf, VT_CURSAVE);
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, VT_CURSPOS, size - 3, 58);
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, "          ");
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, VT_CURSPOS, size - 3, 58);
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, "%d(%d)", GET_MOVE(ch), GET_MAX_MOVE(ch));
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, VT_CURREST);
    write_to_descriptor(ch->desc->descriptor, buf);
  }
 
 if(IS_SET(update, INFO_HP)) {
    sprintf(buf, VT_CURSAVE);
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, VT_CURSPOS, size - 3, 13);
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, "          ");
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, VT_CURSPOS, size - 3, 13);
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, "%d(%d)", GET_HIT(ch), GET_MAX_HIT(ch));
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, VT_CURREST);
    write_to_descriptor(ch->desc->descriptor, buf);
  }
 
 if(IS_SET(update, INFO_GOLD)) {
    sprintf(buf, VT_CURSAVE);
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, VT_CURSPOS, size - 2, 47);
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, "                ");
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, VT_CURSPOS, size - 2, 47);
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, "%d", GET_GOLD(ch));
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, VT_CURREST);
    write_to_descriptor(ch->desc->descriptor, buf);
  }
 
 if(IS_SET(update, INFO_EXP)) {
    sprintf(buf, VT_CURSAVE);
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, VT_CURSPOS, size - 1, 20);
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, "                ");
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, VT_CURSPOS, size - 1, 20);
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, "%d", GET_EXP(ch));
    write_to_descriptor(ch->desc->descriptor, buf);
    sprintf(buf, VT_CURREST);
    write_to_descriptor(ch->desc->descriptor, buf);
  }
}
 
 
void InitScreen(struct char_data *ch)
{
 char buf[255];
 int size;

 size = ch->size; 
 sprintf(buf, VT_HOMECLR);
 send_to_char(buf, ch);
 sprintf(buf, VT_MARGSET, 0, size - 5);
 send_to_char(buf, ch);
 sprintf(buf, VT_CURSPOS, size - 4, 1);
 send_to_char(buf, ch);
 sprintf(buf, "-===========================================================================-");
 send_to_char(buf, ch);
 sprintf(buf, VT_CURSPOS, size - 3, 1);
 send_to_char(buf, ch);
 sprintf(buf, "Hit Points: ");
 send_to_char(buf, ch);
 sprintf(buf, VT_CURSPOS, size - 3, 40);
 send_to_char(buf, ch);
 sprintf(buf, "Movement Points: ");
 send_to_char(buf, ch);
 sprintf(buf, VT_CURSPOS, size - 2, 1);
 send_to_char(buf, ch);
 sprintf(buf, "Mana: ");
 send_to_char(buf, ch);
 sprintf(buf, VT_CURSPOS, size - 2, 40);
 send_to_char(buf, ch);
 sprintf(buf, "Gold: ");
 send_to_char(buf, ch);
 sprintf(buf, VT_CURSPOS, size - 1, 1);
 send_to_char(buf, ch);
 sprintf(buf, "Experience Points: ");
 send_to_char(buf, ch);
 
 ch->last.mana = GET_MANA(ch);
 ch->last.mmana = GET_MAX_MANA(ch);
 ch->last.hit = GET_HIT(ch);
 ch->last.mhit = GET_MAX_HIT(ch);
 ch->last.move = GET_MOVE(ch);
 ch->last.mmove = GET_MAX_MOVE(ch);
 ch->last.exp = GET_EXP(ch);
 ch->last.gold = GET_GOLD(ch);
 
 /* Update all of the info parts */
 sprintf(buf, VT_CURSPOS, size - 3, 13);
 send_to_char(buf, ch);
 sprintf(buf, "%d(%d)", GET_HIT(ch), GET_MAX_HIT(ch));
 send_to_char(buf, ch);
 sprintf(buf, VT_CURSPOS, size - 3, 58);
 send_to_char(buf, ch);
 sprintf(buf, "%d(%d)", GET_MOVE(ch), GET_MAX_MOVE(ch));
 send_to_char(buf, ch);
 sprintf(buf, VT_CURSPOS, size - 2, 7);
 send_to_char(buf, ch);
 sprintf(buf, "%d(%d)", GET_MANA(ch), GET_MAX_MANA(ch));
 send_to_char(buf, ch);
 sprintf(buf, VT_CURSPOS, size - 2, 47);
 send_to_char(buf, ch);
 sprintf(buf, "%d", GET_GOLD(ch));
 send_to_char(buf, ch);
 sprintf(buf, VT_CURSPOS, size - 1, 20);
 send_to_char(buf, ch);
 sprintf(buf, "%d", GET_EXP(ch));
 send_to_char(buf, ch);

 sprintf(buf, VT_CURSPOS, 0, 0);
 send_to_char(buf, ch);

}

void identd_test(struct sockaddr_in in_addr)
{
        struct sockaddr_in addr;
        int fd, addrlen, lport, fport;
        FILE *fp_in, *fp_out;
        char buf[8192], reply_type[81], opsys[81], ident[1024];
        int i;
  
  if( (fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("identd socket");
    return;
  }
                            
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = in_addr.sin_addr.s_addr;
  addr.sin_port = htons(113);
  addrlen = sizeof(addr);

  if (connect(fd, (struct sockaddr *)&addr, addrlen) == -1) {
    sprintf(buf,"identd server not responding, errno: %d\n", errno);
    log(buf);
    return;
  }
 
  addrlen = sizeof(addr);
  if (getsockname(fd, (struct sockaddr *)&addr, &addrlen) == -1)
    perror("getsockname");
 
  fp_in  = fdopen(fd, "r");
  fp_out = fdopen(fd, "w");
  if (!fp_in || !fp_out)
    perror("fdopen");
 
  fprintf(fp_out, "%d , %d\n", ntohs(in_addr.sin_port), mud_port);
  fflush(fp_out);
 
  if(fgets(buf, sizeof(buf)-1, fp_in) == NULL)
    perror("fgets");
 
  shutdown(fd, 1);
 
  i = sscanf(buf, "%d , %d : %[^ \t\n\r:] : %[^\t\n\r:] : %[^\n\r]",
                &lport, &fport, reply_type, opsys, ident);
  if(i < 3) {
    fprintf(stderr, "fscanf: too few arguments (%d)\n", i);
    return;
  }
  if(strcmp(reply_type, "ERROR") == 0) {
    sprintf(buf, "Ident error: error code: %s\n", opsys);
    log(buf);
  } else if(strcmp(reply_type, "USERID") != 0) {
    sprintf(buf, "Ident error: illegal reply type: %s\n", reply_type);
    log(buf);
  } else {
    sprintf(buf, "ident data -- system:%s user:%s\n", opsys, ident);
    log(buf);
  }
 
  fclose(fp_out);
  fclose(fp_in);
 
  return;
}
