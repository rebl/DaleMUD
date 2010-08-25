/*
	Instructions, Randomly written in hopes that others might be able to
	add this code more easily than I did. msw
	
	Make sure the DIMD.H is included in your protos.h
	'		#include "dimd.h"

	You might need to add a few new variables to the char_data struct
	or you might do as MOZART and Shadowdale did and move your PC related
	info to ch->pc struct. Here you can add all the needed info.
	This might mean you will have to re-build your playerfile... but
	that is up to your coding.

	I have included all the code in here that should be all ya need to get
	it working. You will need to place the function dimd_loop() in your
	comm.c code.
	'
	'	dimd_loop();
	'	    for (point = descriptor_list; point; point = point->next)  {
	'	      FD_SET(point->descriptor, &input_set);
	'	      FD_SET(point->descriptor, &exc_set);
	'	      FD_SET(point->descriptor, &output_set);
	'etc..........

	See code for funtion descriptions, remember I am telling you this from
	what I have gathered so it might not be totally correct. Ask the
	MOZART guys if you need to know more.
	
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
  
#include "protos.h"

#define USE_COLOR FALSE		/* do not use DaleMUD color codes. */

void isname_init(const char *str);
bool isname_iter(const char *namelist);

void printhost(struct in_addr *addr, char *buf); /* this is included */

/* these are some functions that MOZART uses but Shadowdale does not */
/* we just make it call Shadowdale versions, see functions later.. */
void cact(char *s,byte boo, struct char_data *ch, struct obj_data *obj, struct char_data *ch2, int i, int ii);
void msg(char *s, struct char_data *ch) ;
void cmsg(char *s, struct char_data *ch, int i);
/* */

/* these should be in comm.c or there abouts... */
extern struct descriptor_data *descriptor_list, *next_to_process;
extern int maxdesc, avail_descs;
/* */


	/* DIMD internally used VARS */  
int	dimd_s;
bool	dimd_on = TRUE;
bool	dimd_gossip = TRUE;
bool	dimd_muse = TRUE;
bool	dimd_think = TRUE;
int	dimd_state = DIMD_OPENING;
int	dimd_port;
int	dimd_local_mud = UNDEFINED;
	/* */
	
/* varaibles used internally */
static char	is_buf[MAX_STRING_LENGTH], is_names[MAX_STRING_LENGTH], *is_argv[100],
		*is_xargv[100];
static int	is_argc,is_xargc,is_exact;		
/* */

/* This is where you make the list of muds that you want to 'talk'/DIMD to */
struct mud_data muds[] =		/*			   /-No Idea	*/
{					/*		      /Mud|  /---/--/---No Idea? */
/*   /- Mud Name      /-- Aliases        /-Address to site   |Port|  ?  ? ?	*/
  {"Shadowdale II","dale2 sd2 outpost","outpost.hsc.unt.edu",4000,2,100,1,DD_AUTOTRY},
  {"Shadowdale","dale sd","dale.hsc.unt.edu",7777,2,100,1,DD_AUTOTRY},
  {"ShadowCode","code dale sd","dale.hsc.unt.edu",4000,2,100,1,DD_AUTOTRY },
  { NULL, NULL, NULL, UNDEFINED } /* this must be here */
}; 



/* this is how dimd determines if you can see the other or the other can */
/* see you (tell and such) */

/* that being said all I know is that if an immortal has it set on one side */
/* and another tries to tell that one (both equaal levels) it will say */
/* Character not found. */

/* needless to ssay the code needs work to function normally */
bool dimd_can_see(int slev, struct char_data *o)
{
  if (!slev && IS_AFFECTED(o, AFF_INVISIBLE))
    return FALSE;
  
  if (o->invis_level && (o->invis_level - (LOW_IMMORTAL-1)) > slev)
    return FALSE;
  
  return TRUE;
  
}

	/* internal code sends data to a mud for various tasks */
void dimd_broadcast(char *buf)
{
  int i;
  
  
  for(i=0; muds[i].address; i++)
    if (IS_SET(muds[i].flags, DD_VERIFIED))
      write_to_descriptor(muds[i].desc, buf);
}

	/* this parses dimd 'packet' commands */
char *one_lc_dimd_argument(char *argument, char *first_arg)
{
  char *scan;
  
  do {
    while(isspace(*argument)) argument++;
    
    scan = first_arg;
    while(*argument && *argument != '^')
      *scan++ = LOWER(*argument), argument++;
    
    *scan = 0;
  }
  while (fill_word(first_arg));
  
  if (*argument=='^')
    argument++; 
  
  return(argument);
}

/* this is used in parsing dimd 'packet' commands */
char *one_dimd_argument(char *argument, char *first_arg)
{
  char *scan;
  
  while(isspace(*argument)) argument++;
  
  scan = first_arg;
  while(*argument && *argument != '^')
    *scan++ = *argument++;
  
  *scan = 0;
  
  if (*argument=='^')
    argument++; 
  
  return(argument);
}

/* does the character have sufficent DIMD Credits to send the DIMD */
/* command. This is how MOZART limits usage of DIMD, might wanna come */
/* up with a better plan :) */

/* Shadowdale gives each user 1 credit per tick. 
	spell_parser.c, affect_update:
'        GET_HIT(i)  = MIN(hit_gain(i) + GET_HIT(i),  hit_limit(i));
'        GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
'        GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
'        
'if (!IS_LINKDEAD(i) && (i->pc))
'        GET_DIMD(i) +=1;		
'
'etc....
*/
bool dimd_credit(struct char_data *ch, int credits)
{
  if (IS_SET(ch->specials.act, PLR_NODIMD))
    {
      msg("Your DIMD privileges were taken away from you.", ch);
      return;
    };
  
  if (credits > GET_DIMD(ch))
    {
      msg("You do not have enough dimd credits to do that.", ch);
      return(FALSE);
    };
  
  GET_DIMD(ch) -= credits;
  return(TRUE);
}


/* does the *mudname match any nick name in our list? */
int matchmud(char *mudname)
{
  int i;
  
  
  isname_init(mudname);
  for(i=0; muds[i].nicknames; i++)
    if (isname_iter(muds[i].nicknames)
	|| isname_iter(muds[i].address))
      return i;
  
  return UNDEFINED;
}


/* HERE */
int getlocalmud(void)
{
  char buf[100];
  
  gethostname(buf, 100);
  
  DIMDLOG(buf);
  
  return matchmud(buf);
}


int getmud(struct char_data *ch, char *mudname, bool checkforup)
{
  int i;
  
  
  if (!*mudname)
    {
      msg("You must pick a mud!", ch);
      return UNDEFINED;
    };
  
  if ((i=matchmud(mudname))==UNDEFINED)
    {
      msg("I am not aware of that mud.", ch);
      return UNDEFINED;
    };
  
  if (i==dimd_local_mud)
    {
      msg("You are ON that mud silly!", ch);
      return UNDEFINED;
    };
  
  if (checkforup && !IS_SET(muds[i].flags, DD_VERIFIED))
    {
      msg("We are not connected to that mud at the moment.", ch);
      return UNDEFINED;
    };
  
  return i;
}


void do_dgossip(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_INPUT_LENGTH], mudname[MAX_INPUT_LENGTH];
  struct descriptor_data *d;
  int i;
  
  
  if (IS_SET(ch->specials.act, PLR_NOSHOUT))
    {
      msg("Ummm, no one can hear you through that gag!", ch);
      return;
    };
  
  if (IS_SET(ch->specials.act, PLR_NOGOSSIP))
    {
      msg("You are not tuned into the gossip channel!", ch);
      return;
    };
  
  one_argument(argument, mudname);
  if ((i=matchmud(mudname))!=UNDEFINED)
    {
      argument = one_argument(argument, mudname);
      if ((i=getmud(ch, mudname, TRUE))==UNDEFINED)
	return;
    };
  
  while(isspace(*argument)) argument++;
  if (!*argument)
    {
      msg("Surely you have something to gossip!", ch);
      return;
    };
  
  if (!dimd_credit(ch, 2+3*(i==UNDEFINED)))
    return;
  
  if (i!=UNDEFINED)
    {
      if (IS_SET(muds[i].flags, DD_NOGOSSIP))
	{
	  msg("They are no longer listening to remote gossips.", ch);
	  return;
	};
      
      sprintf(buf, "^%s^%s^%d^g^%s\n\r",
	      PER(ch), GET_KEYNAME(ch), GetMaxLevel(ch), argument);
      write_to_descriptor(muds[i].desc, buf);
    }
  else
    {
      sprintf(buf, "^%s^%s^%d^gg^%s\n\r",
	      PER(ch), GET_KEYNAME(ch), GetMaxLevel(ch), argument);
      for(i=0; muds[i].address; i++)
	if (IS_SET(muds[i].flags, DD_VERIFIED)
	    && !IS_SET(muds[i].flags, DD_NOGOSSIP))
	  write_to_descriptor(muds[i].desc, buf);
#if USE_COLOR      
      sprintf(buf, "$kW$n $kmdimension gossips, '%s'", argument);
#else
      sprintf(buf, "$n dimension gossips, '%s'", argument);
#endif      
      for(d = descriptor_list; d; d = d->next)
	if (!d->connected && d->character != ch && 
	    !IS_SET(d->character->pc->comm, COMM_NOGOSSIP) && 
	    !check_soundproof(d->character))
	  cact(buf, FALSE, ch, 0, d->character, TO_VICT, CLR_GOSSIP);
    };
  
}


void do_dlink(struct char_data *ch, char *argument, int cmd)
{
  int mud;
  
  
  if (!dimd_on)
    {
      msg("The DIMD server is currently down.", ch);
      return;
    };
  
  if (IS_SET(ch->pc->plr, PLR_NODIMD))
    {
      msg("Your DIMD privileges were taken away!", ch);
      return;
    };
  
  while(isspace(*argument)) argument++;
  if ((mud=getmud(ch, argument, FALSE))==UNDEFINED)
    return;
  
  if (IS_SET(muds[mud].flags, DD_CONNECTED))
    {
      msg("We are already connected to that mud!", ch);
      return;
    };
  
  if (IS_SET(muds[mud].flags, DD_REFUSE))
    {
      msg("We are refusing connections to that mud at the moment.", ch);
      return;
    };
  
  msg("Attempting to connect.", ch);
  if (call_a_mud(mud))
    msg("Connection established.", ch);
  else
    msg("Connection failed.", ch);
}


void do_dunlink(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  int mud;
  
  
  if (IS_SET(ch->pc->plr, PLR_NODIMD))
    {
      msg("Your DIMD privileges were taken away!", ch);
      return;
    };
  
  while(isspace(*argument)) argument++;
  if ((mud=getmud(ch, argument, FALSE))==UNDEFINED)
    return;
  
  if (!IS_SET(muds[mud].flags, DD_CONNECTED))
    {
      msg("We aren't even connected to that mud!", ch);
      return;
    };
  
  sprintf(buf, "Disconnecting %s[%s].",
	  muds[mud].formalname, muds[mud].host);
  hangup_on_a_mud(mud);
}


void do_drestrict(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  int i, mud;
  
  
  if (IS_SET(ch->pc->plr, PLR_NODIMD))
    {
      msg("Your DIMD privileges were taken away!", ch);
      return;
    };
  
  argument = one_argument(argument, buf);
  if ((mud=getmud(ch, buf, FALSE))==UNDEFINED)
    return;
  
  while(isspace(*argument)) argument++;
  
  if (is_abbrev(argument, "refuse"))
    {
      SET_BIT(muds[mud].flags, DD_REFUSE);
      sprintf(buf, "Now refusing new connections with %s.",
	      muds[mud].formalname);
      msg(buf, ch);
      DIMDLOG(buf);
      return;
    };
  
  if (is_abbrev(argument, "accept"))
    {
      REMOVE_BIT(muds[mud].flags, DD_REFUSE);
      sprintf(buf, "Now accepting new connections with %s.",
	      muds[mud].formalname);
      msg(buf, ch);
      DIMDLOG(buf);
      return;
    };
  
  i = atoi(argument);
  muds[mud].min_level = i;
  sprintf(buf, "Setting %s's DIMD incoming level to %d.",
	  muds[mud].formalname, i);
  msg(buf, ch); 
  sprintf(buf, "%s set %s's DIMD incoming level to %d.",
	  PER(ch), muds[mud].formalname, i);
  DIMDLOG(buf); 
}


void do_dlist(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  int i;

#if USE_COLOR  
  strcpy(buf,
	 " $kWCond  $kYMud Name                  $kMNicknames              $kGRestricted\n\r"
	 "$kB------ ------------------------- ---------------------- ----------\n\r");
  
  for(i=0; muds[i].formalname; i++)
    sprintf(buf+strlen(buf), "$kW%s $kY%-25s $kM%-22s $kG%d\n\r",
	    i==dimd_local_mud ? " HERE "
	    : (IS_SET(muds[i].flags, DD_VERIFIED) ? "  UP  "
	       : (IS_SET(muds[i].flags, DD_REFUSE) ? "REFUSE" : " DOWN ")),
	    muds[i].formalname, muds[i].nicknames, muds[i].min_level);
#else
  strcpy(buf,
	 " Cond     Mud Name                  Nicknames              Restricted\n\r"
	 " ------   ------------------------- ---------------------- ----------\n\r");
  
  for(i=0; muds[i].formalname; i++)
    sprintf(buf+strlen(buf), "%s %-25s %-22s %d\n\r",
	    i==dimd_local_mud ? " HERE "
	    : (IS_SET(muds[i].flags, DD_VERIFIED) ? "  UP  "
	       : (IS_SET(muds[i].flags, DD_REFUSE) ? "REFUSE" : " DOWN ")),
	    muds[i].formalname, muds[i].nicknames, muds[i].min_level);
#endif
  
  msg(buf, ch);
}


void do_dmanage(struct char_data *ch, char *argument, int cmd)
{
  
  while(isspace(*argument)) argument++;
  if (!str_cmp(argument, "on"))
    {
      if (dimd_on)
	{
	  msg("The server was already activated.", ch);
	  return;
	};
      
      dimd_on = TRUE;
      msg("Bringing up the DIMD server.", ch);
      return;
    };
  
  if (!str_cmp(argument, "off"))
    {
      if (!dimd_on)
	{
	  msg("The server was already deactivated.", ch);
	  return;
	};
      
      dimd_on = FALSE;
      msg("Bringing down the DIMD server.", ch);
      return;
    };
  
  if (!str_cmp(argument, "gossip"))
    {
      if (dimd_gossip)
	{
	  msg("Dimensional gossips will now be rejected.", ch);
	  dimd_broadcast("^DIMD^DIMD^110^rg\n\r");
	}
      else
	{
	  msg("Dimensional gossips will now be accepted.", ch);
	  dimd_broadcast("^DIMD^DIMD^110^ag\n\r");
	}
      
      dimd_gossip = !dimd_gossip;
      return;
    };
  
  if (!str_cmp(argument, "muse"))
    {
      if (dimd_muse)
	{
	  msg("Dimensional muses will now be rejected.", ch);
	  dimd_broadcast("^DIMD^DIMD^110^rm\n\r");
	}
      else
	{
	  msg("Dimensional muses will now be accepted.", ch);
	  dimd_broadcast("^DIMD^DIMD^110^am\n\r");
	}
      
      dimd_muse = !dimd_muse;
      return;
    };
  
  if (!str_cmp(argument, "think"))
    {
      if (dimd_think)
	{
	  msg("Dimensional thinking will now be rejected.", ch);
	  dimd_broadcast("^DIMD^DIMD^110^rt\n\r");
	}
      else
	{
	  msg("Dimensional thinking will now be accepted.", ch);
	  dimd_broadcast("^DIMD^DIMD^110^at\n\r");
	}
      
      dimd_think = !dimd_think;
      return;
    };
  
  msg("Use either on, off, gossip, muse, or think.", ch);
}


void do_dmuse(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_INPUT_LENGTH], mudname[MAX_INPUT_LENGTH];
  struct descriptor_data *d;
  int i;
  
  
  if (IS_SET(ch->specials.act, PLR_NOSHOUT))
    {
      msg("The gods are sick of listen to you!  You've been gagged!", ch);
      return;
    };
  
  if (HUMAN(ch) && IS_SET(ch->pc->comm, COMM_NOMUSE))
    {
      msg("You are not tuned into the muse channel!", ch);
      return;
    };
  
  one_argument(argument, mudname);
  if ((i=matchmud(mudname))!=UNDEFINED)
    {
      argument = one_argument(argument, mudname);
      if ((i=getmud(ch, mudname, TRUE))==UNDEFINED)
	return;
    };
  
  while(isspace(*argument)) argument++;
  if (!*argument)
    {
      msg("Sure you have something to muse!", ch);
      return;
    };
  
  if (!dimd_credit(ch, 1+2*(i==UNDEFINED)))
    return;
  
  if (i!=UNDEFINED)
    {
      if (IS_SET(muds[i].flags, DD_NOMUSE))
	{
	  msg("They are no longer listening to remote muses.", ch);
	  return;
	};
      
      sprintf(buf, "^%s^%s^%d^mg^%s\n\r",
	      PER(ch), GET_KEYNAME(ch), GetMaxLevel(ch), argument);
      write_to_descriptor(muds[i].desc, buf);
    }
  else
    {
      sprintf(buf, "^%s^%s^%d^mg^%s\n\r",
	      PER(ch), GET_KEYNAME(ch), GetMaxLevel(ch), argument);
      for(i=0; muds[i].address; i++)
	if (IS_SET(muds[i].flags, DD_VERIFIED)
	    && !IS_SET(muds[i].flags, DD_NOMUSE))
	  write_to_descriptor(muds[i].desc, buf);
#if USE_COLOR      
      sprintf(buf, "$kW$n $kLdimension muses, '%s'", argument);
#else
      sprintf(buf, "$n dimension muses, '%s'", argument);
#endif
      for(d = descriptor_list; d; d = d->next)
	if (!d->connected && d->character != ch
	    && GetMaxLevel(d->character) >= LOW_IMMORTAL
	    && !IS_SET(d->character->pc->comm, COMM_NOMUSE))
	  cact(buf, FALSE, ch, 0, d->character, TO_VICT, CLR_MUSE);
    };
}


void do_dtell(struct char_data *ch, char *argument, int cmd)
{
  char name[MAX_INPUT_LENGTH], mudname[MAX_INPUT_LENGTH],
  buf[MAX_STRING_LENGTH];
  int i;
  /*
    for (i=0;i<MAX_MOBOBJ;i++)
    {
    sprintf(buf,"%s has pos %d",mob_index[i].name, mob_index[i].pos);
    slog(buf);
    }*/
  argument = one_argument(argument, name);
  argument = one_argument(argument, mudname);
  
  if ((i=getmud(ch, mudname, TRUE))==UNDEFINED)
    return;
  
  while(isspace(*argument)) argument++;
  if (!*argument)
    {
      msg("Surely you have SOMETHING to say ... ?", ch);
      return;
    };
  
  if (!dimd_credit(ch, 1))
    return;
  
  sprintf(buf, "^%s^%s^%d^t^%s^%s\n\r",
	  PER(ch), GET_KEYNAME(ch), GetMaxLevel(ch), name, argument);
  write_to_descriptor(muds[i].desc, buf);
}


void do_dthink(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_INPUT_LENGTH], mudname[MAX_INPUT_LENGTH];
  struct descriptor_data *d;
  int i;
  
  
  one_argument(argument, mudname);
  if ((i=matchmud(mudname))!=UNDEFINED)
    {
      argument = one_argument(argument, mudname);
      if ((i=getmud(ch, mudname, TRUE))==UNDEFINED)
	return;
    };
  
  while(isspace(*argument)) argument++;
  if (!*argument)
    {
      msg("Sure you have something to think!", ch);
      return;
    };
  
  if (!dimd_credit(ch, 1+1*(i==UNDEFINED)))
    return;
  
  if (i!=UNDEFINED)
    {
      if (IS_SET(muds[i].flags, DD_NOTHINK))
	{
	  msg("They are no longer listening to remote thinks.", ch);
	  return;
	};
      
      sprintf(buf, "^%s^%s^%d^*^%s\n\r",
	      PER(ch), GET_KEYNAME(ch), GetMaxLevel(ch), argument);
      write_to_descriptor(muds[i].desc, buf);
    }
  else
    {
      sprintf(buf, "^%s^%s^%d^*g^%s\n\r",
	      PER(ch), GET_KEYNAME(ch), GetMaxLevel(ch), argument);
      for(i=0; muds[i].address; i++)
	if (IS_SET(muds[i].flags, DD_VERIFIED)
	    && !IS_SET(muds[i].flags, DD_NOTHINK))
	  write_to_descriptor(muds[i].desc, buf);
#if USE_COLOR      
      sprintf(buf, "$kW$n $kLdimension thinks, '%s'", argument);
#else
      sprintf(buf, "$n dimension thinks, '%s'", argument);
#endif      
      for(d = descriptor_list; d; d = d->next)
	if (!d->connected && d->character != ch
	    && GetMaxLevel(d->character) >= DEMIGOD)
	  cact(buf, FALSE, ch, 0, d->character, TO_VICT, CLR_THINK);
    };
}

void do_dwho(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  int i;
  
  
  argument = one_argument(argument, buf);
  if ((i=getmud(ch, buf, TRUE))==UNDEFINED)
    return;
  
  if (!dimd_credit(ch, 2))
    return;
  
  while(isspace(*argument)) argument++;
  sprintf(buf, "^%s^%s^%d^w^%s\n\r",
	  PER(ch), GET_KEYNAME(ch), GetMaxLevel(ch), argument);
  write_to_descriptor(muds[i].desc, buf);
}


bool call_a_mud(int mud)
{
  int size, connectresult;
  struct sockaddr_in sockaddr;
  struct hostent *hp;
  char buf[200];
  
  
  if (mud == dimd_local_mud)
    return FALSE;
  
  if (isdigit(*muds[mud].address))
    sockaddr.sin_addr.s_addr=inet_addr(muds[mud].address);
  else
    {
      if (!(hp=gethostbyname(muds[mud].address)))
	{
	  sprintf(buf, "Couldn't decipher %s's address.",
		  muds[mud].formalname);
	  DIMDLOG(buf);
	  return FALSE;
	};
      
      memcpy((char *) &sockaddr.sin_addr, hp->h_addr,
	     sizeof(sockaddr.sin_addr));
    };
  
  sockaddr.sin_port = htons(muds[mud].port+2);
  
  if ((muds[mud].desc=socket(AF_INET, SOCK_STREAM, 0))<0)
    {
      DIMDLOG("Could not obtain a socket.");
      return FALSE;
    };
  
  sockaddr.sin_family = AF_INET;
  
  connectresult =
    connect(muds[mud].desc, (struct sockaddr *) &sockaddr, sizeof(sockaddr));
  
  if (connectresult)
    {
      close(muds[mud].desc);
      sprintf(buf, "Couldn't connect to %s[%s].",
	      muds[mud].formalname, muds[mud].address);
      switch(errno) {
      case 61:
	strcat(buf, "  Connection refused.");
	break;
	
      default:
	sprintf(buf+strlen(buf), "  Error(%d).", errno);
      };
      DIMDLOG(buf);
      return FALSE;
    };
  
  if ((maxdesc+1) >= avail_descs)
    {
      sprintf(buf, "Tried to open a connection to %s but ran out of sockets.",
	      muds[mud].formalname);
      DIMDLOG(buf);
      close(muds[mud].desc);
      return FALSE;
    }
  else
    if (muds[mud].desc > maxdesc)
      maxdesc = muds[mud].desc;
  
  
  size = sizeof(sockaddr);
  if (getpeername(muds[mud].desc, (struct sockaddr *) &sockaddr, &size) < 0)
    {
      perror("getpeername");
      *muds[mud].host = '\0';
    }
  else 
    printhost(&sockaddr.sin_addr, muds[mud].host);
  
  
  muds[mud].output.head = NULL;
  muds[mud].input.head = NULL;
  *muds[mud].buf = 0;
  muds[mud].flags |= DD_CONNECTED|DD_VERIFIED;
  *muds[mud].pcname = 0;
  
  sprintf(buf, "DIMD connection to %s[%s] established.",
	  muds[mud].formalname, muds[mud].address);
  DIMDLOG(buf);
  
  write_to_descriptor(muds[mud].desc, DIMD_PASSWORD "\n\r");
  return TRUE;
}


bool answer_a_mud(void)
{
  int desc, size, mud;
  struct sockaddr_in sock;
  char buf[1024], host[100];
  
  
  if ((desc = new_connection(dimd_s)) < 0)
    return FALSE;
  
  if ((maxdesc+1) >= avail_descs)
    {
      write_to_descriptor(desc, "Sorry, the game is full.");
      close(desc);
      return FALSE;
    }
  else
    if (desc > maxdesc)
      maxdesc = desc;
  
  
  size = sizeof(sock);
  if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0)
    {
      perror("getpeername");
      *host = '\0';
    }
    else  printhost(&sock.sin_addr, host); 
   
  for(mud=0; muds[mud].address; mud++) 
    if (!str_cmp(host, muds[mud].address))
      break;
  
  if (!muds[mud].address)
    {
      write_to_descriptor(desc, "Sorry, I don't recognize your address.");
      close(desc);
      sprintf(buf,"DIMD requestor[%s](%d) was refused -- couldn't recognize.", host,mud);
      DIMDLOG(buf);
      return FALSE;
    };
  
  if (IS_SET(muds[mud].flags, DD_CONNECTED))
    {
      write_to_descriptor(desc, "We are already connected to your mud.");
      close(desc);
      sprintf(buf, "%s[%s] tried to open a SECOND DIMD connection.",
	      muds[mud].formalname, muds[mud].address);
      DIMDLOG(buf);
      return FALSE;
    };
  
  if (IS_SET(muds[mud].flags, DD_REFUSE))
    {
      write_to_descriptor(desc, "We are not accepting connections from your mud at the moment.");
      close(desc);
      sprintf(buf, "%s[%s] tried to connect but is being refused.",
	      muds[mud].formalname, muds[mud].address);
      DIMDLOG(buf);
      return FALSE;
    };
  
  muds[mud].desc = desc;
  strcpy(muds[mud].host, host);
  muds[mud].output.head = NULL;
  muds[mud].input.head = NULL;
  *muds[mud].buf = 0;
  muds[mud].flags |= DD_CONNECTED;
  *muds[mud].pcname = 0;
  
  sprintf(buf, "DIMD connection established with %s[%s].",
	  muds[mud].formalname, host);
  DIMDLOG(buf);
  
  return TRUE;
}


void hangup_on_a_mud(int mud)
{
  char buf[MAX_INPUT_LENGTH];
  
  if (!IS_SET(muds[mud].flags, DD_CONNECTED))
    return;
  
  close(muds[mud].desc);
  while(get_from_q(&muds[mud].input, buf));
  while(get_from_q(&muds[mud].output, buf));
  if (muds[mud].desc == maxdesc)
    --maxdesc;
  
  REMOVE_BIT(muds[mud].flags, DD_CONNECTED|DD_VERIFIED);
  sprintf(buf, "DIMD connection with %s[%s] closed.",
	  muds[mud].formalname, muds[mud].address);
  DIMDLOG(buf);
}


void close_dimd(void)
{
  int mud;
  
  
  if (dimd_state != DIMD_CLOSED)
    {
      dimd_state = DIMD_CLOSED ;
      DIMDLOG("Closing all DIMD sockets and DIMD server.");
      
      for(mud=0; muds[mud].address; mud++)
	hangup_on_a_mud(mud);
      
      close(dimd_s);
    };
}


void dimd_loop(void)
{
  int	  i, fromlevel, fromgodlevel;
  fd_set  input_set, output_set, exc_set;
  static char
    buf[MAX_STRING_LENGTH], extra[MAX_STRING_LENGTH],
    fromname[MAX_INPUT_LENGTH], fromkeyname[MAX_INPUT_LENGTH],
    dcommand[MAX_INPUT_LENGTH], toname[MAX_INPUT_LENGTH], *scan;
  struct  char_data *vict;
  char    comm[MAX_INPUT_LENGTH];
  struct  timeval null_time;
  struct  descriptor_data *d;
  
  
  switch (dimd_state) {
    /****************************************************************/
  case DIMD_OPENING:
    if ((dimd_local_mud=getlocalmud())==UNDEFINED)
      {
	DIMDLOG("Couldn't figure out what this mud's name is!  DIMD NOT opened.");
	dimd_state = DIMD_CLOSED;
	dimd_on = FALSE;
	return;
      };
    
    dimd_port = DIMD_PORT;
    dimd_s=init_socket(dimd_port);
    DIMDLOG("DIMD port opened.");
    
    for(i=0; muds[i].address; i++)
      if (IS_SET(muds[i].flags, DD_AUTOTRY))
	call_a_mud(i);
    
    dimd_state = DIMD_OPEN;
    break;
    
    /****************************************************************/
  case DIMD_OPEN:
    
    null_time.tv_sec = 0;
    null_time.tv_usec = 0;
    
    FD_ZERO(&input_set);
    FD_ZERO(&output_set);
    FD_ZERO(&exc_set);
    FD_SET(dimd_s, &input_set);
    for(i=0; muds[i].address; i++)
      if (IS_SET(muds[i].flags, DD_CONNECTED))
	{
	  FD_SET(muds[i].desc, &input_set);
	  FD_SET(muds[i].desc, &exc_set);
	  FD_SET(muds[i].desc, &output_set);
	};
    
    if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0)
      {
	perror("Tell select poll");
	return;
      };
    
    
    /* Respond to whatever might be happening */
    
    /* New connection? */
    if (FD_ISSET(dimd_s, &input_set))
      if (!answer_a_mud())
	perror("New connection");
    
    for(i=0; muds[i].address; i++)
      if (IS_SET(muds[i].flags, DD_CONNECTED))
	{
	  if (FD_ISSET(muds[i].desc, &exc_set))
	    {
	      FD_CLR(muds[i].desc, &input_set);
	      FD_CLR(muds[i].desc, &output_set);
	      hangup_on_a_mud(i);
	    };
	};
    
    for(i=0; muds[i].address; i++)
      if (IS_SET(muds[i].flags, DD_CONNECTED))
	{
	  if (FD_ISSET(muds[i].desc, &input_set))
	    if (process_dimd_input(i) < 0) 
	      hangup_on_a_mud(i);
	};
    
    for(i=0; muds[i].address; i++)
      if (IS_SET(muds[i].flags, DD_CONNECTED))
	{
	  while(get_from_q(&muds[i].input, comm))
	    {
	      if (!IS_SET(muds[i].flags, DD_VERIFIED))
		{
		  if (str_cmp(comm, DIMD_PASSWORD))
		    {
		      sprintf(buf, "The mud is located at port %d.\n\r",
			      dimd_port-2);
		      write_to_descriptor(muds[i].desc, buf);
		      sprintf(buf, "%s[%s]'s server didn't know our password.",
			      muds[i].formalname, muds[i].host);
		      DIMDLOG(buf);
		      hangup_on_a_mud(i);
		      break;
		    };
		  
		  SET_BIT(muds[i].flags, DD_VERIFIED);
		  break;
		};
	      
	      if (*comm != '^')
		{
		  if (!(vict=get_char(muds[i].pcname)))
		    {
		      sprintf(buf,
			      "DIMD stream for unknown player %s rejected.",
			      muds[i].pcname);
		      DIMDLOG(buf);
		      DIMDLOG(comm);
		    }
		  else
		    msg(comm, vict);
		  
		  continue;
		};
	      
	      scan = comm+1;
	      scan = one_dimd_argument(scan, fromname);
	      scan = one_dimd_argument(scan, fromkeyname);
	      scan = one_dimd_argument(scan, buf);
	      fromlevel = atoi(buf);
	      if (!*fromname || !*fromkeyname || !fromlevel)
		{
		  write_to_descriptor(muds[i].desc, "Your server isn't working properly.");
		  sprintf(buf, "%s[%s] didn't supply PC names or level.",
			  muds[i].formalname, muds[i].host);
		  DIMDLOG(buf);
		  hangup_on_a_mud(i);
		  break;
		};
	      
	      if (fromlevel < muds[i].min_level)
		sprintf(buf, "You must be at least %d level to use DIMD on %s.",
			muds[i].min_level, muds[i].formalname);
	      else {
		fromgodlevel = (fromlevel-muds[i].immort_level)
		  * muds[i].immort_level_mult;
		scan = one_lc_dimd_argument(scan, dcommand);
		switch(*dcommand)
		  {
		  case '#':
		    strcpy(muds[i].pcname, fromkeyname);
		    continue;
		    
		  case 'a':
		    switch(*(dcommand+1))
		      {
		      case 'g':
			sprintf(buf, "%s is now accepting remote gossips.",
				muds[i].formalname);
			REMOVE_BIT(muds[i].flags, DD_NOGOSSIP);
			break;
			
		      case 'm':
			sprintf(buf, "%s is now accepting remote muses.",
				muds[i].formalname);
			REMOVE_BIT(muds[i].flags, DD_NOMUSE);
			break;
			
		      case 't':
			sprintf(buf, "%s is now accepting remote thinks.",
				muds[i].formalname);
			REMOVE_BIT(muds[i].flags, DD_NOTHINK);
			break;
			
		      default:
			sprintf(buf, "Unknown accept command from %s.",
				muds[i].formalname);
		      };
		    DIMDLOG(buf);
		    continue;
		    
		  case 'r':
		    switch(*(dcommand+1))
		      {
		      case 'g':
			sprintf(buf, "%s is now refusing remote gossips.",
				muds[i].formalname);
			SET_BIT(muds[i].flags, DD_NOGOSSIP);
			break;
			
		      case 'm':
			sprintf(buf, "%s is now refusing remote muses.",
				muds[i].formalname);
			SET_BIT(muds[i].flags, DD_NOMUSE);
			break;
			
		      case 't':
			sprintf(buf, "%s is now refusing remote thinks.",
				muds[i].formalname);
			SET_BIT(muds[i].flags, DD_NOTHINK);
			break;
			
		      default:
			sprintf(buf, "Unknown refuse command from %s.",
				muds[i].formalname);
		      };
		    DIMDLOG(buf);
		    continue;
		    
		  case 'w':
		    sprintf(buf, "Who request from %s at %s served.",
			    fromname, muds[i].formalname);
		    DIMDLOG(buf);
/*
		    construct_who_list(buf, 39, NULL, fromgodlevel, scan);
*/		    
		    break;
		    
		  case 't':
	            scan = one_lc_dimd_argument(scan, toname);
		    while(isspace(*scan)) scan++;
		    
	            if (!(vict = get_char(toname))
			|| !dimd_can_see(fromgodlevel, vict))
		      strcpy(buf, "No char by that name.");
	            else
		      {
#if USE_COLOR
			sprintf(buf, "$kW%s $kMfrom $kW%s $kMtells you '%s'.",
				fromname, muds[i].formalname, scan);
#else
			sprintf(buf, "%s from %s tells you '%s'.",
				fromname, muds[i].formalname, scan);
#endif				
			cmsg(buf, vict, CLR_TELL);
			if (IS_SET(vict->pc->comm, COMM_AFK))
#if USE_COLOR
			  sprintf(buf, "$kMYou tell $kW%s$kM (who is AFK), '%s'",
				  PER(vict), scan);
#else
			  sprintf(buf, "You tell %s (who is AFK), '%s'",
				  PER(vict), scan);

#endif				  
			else
#if USE_COLOR
			  sprintf(buf, "$kMYou tell $kW%s$kM, '%s'",
				  PER(vict), scan);
#else
			  sprintf(buf, "You tell %s, '%s'",
				  PER(vict), scan);
#endif				  
		      };
		    break;
		    
		  case 'g':
		    if (!dimd_gossip)
		      {
			write_to_descriptor(muds[i].desc, "^DIMD^DIMD^110^rg\n\r");
			sprintf(buf, "%s is no longer accepting remote gossips.", muds[dimd_local_mud].formalname);
			break;
		      };
		    if (*(dcommand+1)=='g')
#if USE_COLOR
		      sprintf(buf, "$kW%s $kYfrom $kW%s $kYdimension gossips, '%s'",
			      fromname, muds[i].formalname, scan);
#else
		      sprintf(buf, "%s from %s dimension gossips, '%s'",
			      fromname, muds[i].formalname, scan);
#endif			      
		    else
#if USE_COLOR
		      sprintf(buf, "$kW%s $kYfrom $kW%s $kYgossips, '%s'",
			      fromname, muds[i].formalname, scan);
#else
		      sprintf(buf, "%s from %s gossips, '%s'",
			      fromname, muds[i].formalname, scan);
#endif			      
		    
		    for(d = descriptor_list; d; d = d->next)
		      if (!d->connected
			  && !IS_SET(d->character->pc->comm, COMM_NOGOSSIP)
			  && !check_soundproof(d->character))
			cmsg(buf, d->character, CLR_GOSSIP);
		    
#if USE_COLOR
		    sprintf(buf, "$kYYou gossip on $kW%s$kY, '%s'",
			    muds[dimd_local_mud].formalname, scan);
#else
		    sprintf(buf, "You gossip on %s, '%s'",
			    muds[dimd_local_mud].formalname, scan);

#endif			    
		    break;
		    
		  case 'm':
		    if (!dimd_muse)
		      {
			write_to_descriptor(muds[i].desc, "^DIMD^DIMD^110^rm\n\r");
			sprintf(buf, "%s is no longer accepting remote muses.", muds[dimd_local_mud].formalname);
			break;
		      };
		    
		    if (*(dcommand+1)=='g')
#if USE_COLOR
		      sprintf(buf, "$kW%s $kLfrom $kW%s $kLdimension muses, '%s'",
			      fromname, muds[i].formalname, scan);
#else
		      sprintf(buf, "%s from %s dimension muses, '%s'",
			      fromname, muds[i].formalname, scan);
#endif			      
		    else
#if USE_COLOR
		      sprintf(buf, "$kW%s $kLfrom $kW%s $kLmuses, '%s'",
			      fromname, muds[i].formalname, scan);
#else
		      sprintf(buf, "%s from %s muses, '%s'",
			      fromname, muds[i].formalname, scan);
#endif			      
		    for(d = descriptor_list; d; d = d->next)
		      if (!d->connected
			  && GetMaxLevel(d->character) >= LOW_IMMORTAL
			  && !IS_SET(d->character->pc->comm, COMM_NOMUSE))
			cmsg(buf, d->character, CLR_MUSE);
		    
#if USE_COLOR
		    sprintf(buf, "$kLYou muse on $kW%s$kL, '%s'",
			    muds[dimd_local_mud].formalname, scan);
#else
		    sprintf(buf, "You muse on %s, '%s'",
			    muds[dimd_local_mud].formalname, scan);
#endif
		    break;
		    
		  case '*':
		    if (!dimd_think)
		      {
			write_to_descriptor(muds[i].desc, "^DIMD^DIMD^110^rt\n\r");
			sprintf(buf, "%s is no longer accepting remote thinks.", muds[dimd_local_mud].formalname);
			break;
		      };
		    if (*(dcommand+1)=='g')
#if USE_COLOR
		      sprintf(buf, "$kW%s $kLfrom $kW%s $kLdimension thinks, '%s'",
			      fromname, muds[i].formalname, scan);
#else
		      sprintf(buf, "%s from %s dimension thinks, '%s'",
			      fromname, muds[i].formalname, scan);

#endif			      
		    else
#if USE_COLOR
		      sprintf(buf, "$kW%s $kLfrom $kW%s $kLthinks, '%s'",
			      fromname, muds[i].formalname, scan);
#else
		      sprintf(buf, "%s from %s thinks, '%s'",
			      fromname, muds[i].formalname, scan);

#endif			      
		    for(d = descriptor_list; d; d = d->next)
		      if (!d->connected
			  && GetMaxLevel(d->character) >= DEMIGOD
			  && !IS_SET(d->character->pc->comm, COMM_NOMUSE))
			cmsg(buf, d->character, CLR_THINK);
		    
#if USE_COLOR
		    sprintf(buf, "$kLYou think on $kW%s$kL, '%s'",
			    muds[dimd_local_mud].formalname, scan);
#else
		    sprintf(buf, "You think on %s, '%s'",
			    muds[dimd_local_mud].formalname, scan);

#endif			    
		    break;
		    
		  default:
#if USE_COLOR
		    sprintf(buf, "$kW%s $kRfrom $kW%s $kRissued invalid command '%s'.",
			    fromname, muds[i].formalname, dcommand);
#else
		    sprintf(buf, "%s from %s issued invalid command '%s'.",
			    fromname, muds[i].formalname, dcommand);
#endif			    
		    DIMDLOG(buf);
#if USE_COLOR
		    strcpy(buf, "$k+BRYour server isn't functioning properly.");
#else
		    strcpy(buf, "Your server isn't functioning properly.");
#endif		    
		  };
	      };
	      
	      strcat(buf, "\n\r");
	      sprintf(extra, "^%s^%s^%d^#\n\r%s\n\r", fromname, fromkeyname, 1000, buf);
	      write_to_descriptor(muds[i].desc, extra);
	    };
	};
    
    /* Are they gone yet? */    
    for(i=0; muds[i].address; i++)
      if (IS_SET(muds[i].flags, DD_CONNECTED))
	{
	  if (FD_ISSET(muds[i].desc, &output_set) && muds[i].output.head)
	    if (process_dimd_output(i) < 0)
	      hangup_on_a_mud(i);
	};
    
    
    if (!dimd_on)
      dimd_state = DIMD_CLOSING;
    
    break;
    
    /****************************************************************/
  case DIMD_CLOSING:
    close_dimd();
    dimd_state = DIMD_CLOSED ;
    break;
    
    /****************************************************************/
  case DIMD_CLOSED:
    if (dimd_on)
      dimd_state = DIMD_OPENING;
    break;
    
  }
  return;
}






/**********************************************************************/



int process_dimd_output(int mud)
{
  char i[MAX_STRING_LENGTH + 1];
  
  /* Cycle thru output queue */
  while(get_from_q(&muds[mud].output, i))
    if (write_to_descriptor(muds[mud].desc, i))
      return(-1);
}


int process_dimd_input(int mud)
{
  int sofar, thisround, begin, squelch, i, k, flag;
  char tmp[MAX_INPUT_LENGTH+2], buffer[MAX_INPUT_LENGTH + 60];
  
  sofar = 0;
  flag = 0;
  begin = strlen(muds[mud].buf);
  
  /* Read in some stuff */
  do  {
    if ((thisround = read(muds[mud].desc, muds[mud].buf + begin + sofar, 
			  MAX_STRING_LENGTH - (begin + sofar) - 1)) > 0) {
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
	DIMDLOG("EOF encountered on DIMD socket read.");
	return(-1);
      }
    }
  } while (!ISNEWL(*(muds[mud].buf + begin + sofar - 1)));	
  
  *(muds[mud].buf + begin + sofar) = 0;
  
  /* if no newline is contained in input, return without proc'ing */
  for (i = begin; !ISNEWL(*(muds[mud].buf + i)); i++)
    if (!*(muds[mud].buf + i))
      return(0);
  
  /* input contains 1 or more newlines; process the stuff */
  for (i = 0, k = 0; *(muds[mud].buf + i);)	{
    if (!ISNEWL(*(muds[mud].buf + i)) && !(flag=(k>=(MAX_INPUT_LENGTH - 2))))
      if (*(muds[mud].buf + i) == '\b') {	 /* backspace */
	if (k) { /* more than one char ? */
	  if (*(tmp + --k) == '$')
	    k--;				
	  i++;
	} else {
	  i++;  /* no or just one char.. Skip backsp */
	}
      } else {
	if (isascii(*(muds[mud].buf + i)) && isprint(*(muds[mud].buf + i))) {
	  /* 
	    trans char, double for '$' (printf)	
	    */
	  if ((*(tmp + k) = *(muds[mud].buf + i)) == '$')
	    *(tmp + ++k) = '$';
	  k++;
	  i++;
	} else {
	  i++;
	}
      } else 	{
	*(tmp + k) = 0;
	
	write_to_q(tmp, &muds[mud].input);
	
	if (flag) {
	  sprintf(buffer, 
		  "Line too long. Truncated to:\n\r%s\n\r", tmp);
	  if (write_to_descriptor(muds[mud].desc, buffer) < 0)
	    return(-1);
	  
	  /* skip the rest of the line */
	  for (; !ISNEWL(*(muds[mud].buf + i)); i++);
	}
	
	/* find end of entry */
	for (; ISNEWL(*(muds[mud].buf + i)); i++);
	
	/* squelch the entry from the buffer */
	for (squelch = 0;; squelch++)
	  if ((*(muds[mud].buf + squelch) = 
	       *(muds[mud].buf + i + squelch)) == '\0')
	    break;
	k = 0;
	i = 0;
      }
  }
  return(1);
}




void
isname_init(const char *str)
{
        char           *s;
        strcpy(is_buf, str);
        is_argc = split_string(is_buf, "- \t\n\r,", is_argv);

        s = is_argv[is_argc - 1];
        s += strlen(s);
        if (*(--s) == '.') {
                is_exact = 1;
                *s = 0;
        } else {
                is_exact = 0;
        }
}


bool
isname_iter(const char *namelist)
{
        int             i, j;

        strcpy(is_names, namelist);
        is_xargc = split_string(is_names, "- \t\n\r,", is_xargv);


        if (is_exact && is_argc != is_xargc)
                return FALSE;

        for (i = 0; i < is_argc; i++) {
                for (j = 0; j < is_xargc; j++) {
                        if (!str_cmp(is_argv[i], is_xargv[j])) {
                                is_xargv[j] = NULL;
                                break;
                        };
                };
                if (j >= is_xargc)
                        return FALSE;
        };

        return TRUE;
}



void cact(char *s,byte boo, struct char_data *ch, struct obj_data *obj, struct char_data *ch2, int i, int ii)
{
	act(s,boo,ch,obj,ch2,i);
}

void msg(char *s, struct char_data *ch)
{
	send_to_char(s,ch);
	send_to_char("\n\r",ch);
}

void cmsg(char *s, struct char_data *ch, int i)
{
	send_to_char(s,ch);
	send_to_char("\n\r",ch);
}

void printhost(struct in_addr *addr, char *buf)
{
	int i;
	struct hostent *from;

if ((from = gethostbyaddr((char *)addr,sizeof(addr), AF_INET))) {
    strcpy(buf,from->h_name);  
    } else {
    log("!from!");
    i = addr->s_addr;
    sprintf(buf, "%d.%d.%d.%d", (i&0xff), (i&0xff00)>>8, 
                             (i&0xff0000)>>16, (i&0xff000000)>>24);
        }
	log(buf);
}

void construct_who_list(char *buf, int cmd,
			struct char_data *ch, int godlevel,
			char *argument)
{
    struct char_data *person;
    struct descriptor_data *d;
    char levels[40]="", classes[20]="";
    char *classname[]={"Mu","Cl","Wa","Th","Dr","Mo","Ba","So","Pa","Ra","Ps"};
    char buffer[MAX_STRING_LENGTH],tbuf[MAX_STRING_LENGTH];
    int i,total,classn,count,color_cnt=0; long bit;

	if (godlevel<0) {
	  godlevel=0;
	  log("godlevel < 0 in construct_who list");
	}
      sprintf(buffer,"                        Shadowdale Players\n\r");
      strcat(buffer,       "                           ------------\n\r");
      count=0;
    for (d = descriptor_list; d; d = d->next) {
  if ((person=(d->original?d->original:d->character)))
     if (godlevel>=person->invis_level && real_roomp(person->in_room)) {
          count++; color_cnt = (color_cnt++ % 9);  /* range 1 to 9 */
            if(!IS_IMMORTAL(person)) {
              for(bit=1,i=total=classn=0;i<PSI_LEVEL_IND+1;i++, bit<<=1) {
                if(HasClass(person,bit)) {
                  classn++; total+=person->player.level[i];
                  if(strlen(classes)!=0) strcat(classes,"/");
                  sprintf(classes+strlen(classes),"%s",classname[i]);
                }
              }
             if (total <=0)
                total =1;
	if (classn <= 0)
		classn =1;                
              total/=classn;
                   if(total<11) strcpy(levels,"Newbie");
              else if(total<21) strcpy(levels,"Chump");
              else if(total<31) strcpy(levels,"Medium");
              else if(total<41) strcpy(levels,"Expert");
              else if(total<51) strcpy(levels,"Adept");
              sprintf(tbuf, "%s %s",levels, classes);
              sprintf(levels,"%30s","");
              strcpy(levels+8-((strlen(tbuf)-12)/2),tbuf);
              sprintf(tbuf, "%-28s : %s %s",levels,
                      GET_NAME(person),person->player.title?person->player.title:"(Null)");
            }
            else {
              switch(GetMaxLevel(person)) {
                case 51: sprintf(levels,"Newbie Immortal"); break;
                case 52: sprintf(levels,"Saint"); break;
                case 53: sprintf(levels,"Creator"); break;
                case 54: sprintf(levels,"Demi God"); break;
                case 55: sprintf(levels,"God"); break;
                case 56: sprintf(levels,"Greater God"); break;
                case 57: sprintf(levels,"Overseer"); break;
                case 58: sprintf(levels,"SD Lord"); break;
                case 59: sprintf(levels,"Lord of Realm"); break;
                case 60: sprintf(levels,"God of Realm"); break;
              }
              sprintf(tbuf, "%s",levels);
              sprintf(levels,"%30s","");
              strcpy(levels+8-(strlen(tbuf)/2),tbuf);
              sprintf(tbuf, "%-16s : %s %s",levels,GET_NAME(person),
                      person->player.title?person->player.title:"(Null)");
            }
            if(IS_SET(person->specials.affected_by2,AFF2_AFK))
              sprintf(tbuf+strlen(tbuf)," [AFK] ");
	if (IS_LINKDEAD(person))
		sprintf(tbuf+strlen(tbuf)," [LINKDEAD] ");
            sprintf(tbuf+strlen(tbuf),"\n\r");
            if (strlen(buffer)+strlen(tbuf) < (MAX_STRING_LENGTH*2)-512)
              strcat(buffer,tbuf);
          }
        }
    sprintf(tbuf, "\n\rTotal visible players: %d\n\r", count);
    if (strlen(buffer)+strlen(tbuf) < MAX_STRING_LENGTH*2-512)
      strcat(buffer,tbuf);

	strcpy(buf,buffer);
}

