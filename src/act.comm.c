/*
  DaleMUD v2.0	Released 2/1994
  See license.doc for distribution terms.   DaleMUD is based on DIKUMUD
*/

#include <stdio.h>
#include <string.h>

#include "protos.h"

/* extern variables */

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;


void do_say(struct char_data *ch, char *argument, int cmd)
{
  int i;
  char buf[MAX_INPUT_LENGTH+40];

dlog("in do_say");  

  if (apply_soundproof(ch))
    return;
  
  for (i = 0; *(argument + i) == ' '; i++);
  
  if (!*(argument + i))
    send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
  else	{
    sprintf(buf,"$n says '%s'", argument + i);
    act(buf,FALSE,ch,0,0,TO_ROOM);
    if (IS_NPC(ch)||(IS_SET(ch->specials.act, PLR_ECHO))) {
      sprintf(buf,"You say '%s'\n\r", argument + i);
      send_to_char(buf, ch);
    }
  }
}


void do_report(struct char_data *ch, char *argument, int cmd)
{
char buf[100];
int i,ii,iii;

dlog("in do_report");

if (apply_soundproof(ch))
   return;

if (IS_NPC(ch))
   return;  
   
if (GET_HIT(ch) > GET_MAX_HIT(ch) ||		/* bug fix */
    GET_MANA(ch) > GET_MAX_MANA(ch) ||
    GET_MOVE(ch) > GET_MAX_MOVE(ch)) {
	send_to_char("Sorry, cannot do that right now.\n\r",ch);
        return;
}

  sprintf(buf,"$c0014[$c0015$n$c0014] reports 'HP:%2.0f%% MANA:%2.0f%% MV:%2.0f%%'",
  ((float)GET_HIT(ch)  / (int)GET_MAX_HIT(ch))  * 100.0 + 0.5,
  ((float)GET_MANA(ch) / (int)GET_MAX_MANA(ch)) * 100.0 + 0.5,
  ((float)GET_MOVE(ch) / (int)GET_MAX_MOVE(ch)) * 100.0 + 0.5);
 act(buf,FALSE,ch,0,0,TO_ROOM);
  sprintf(buf,"$c0014You report 'HP:%2.0f%% MANA:%2.0f%% MV:%2.0f%%'", 
  ((float)GET_HIT(ch)  / (int)GET_MAX_HIT(ch))  * 100.0 + 0.5,
  ((float)GET_MANA(ch) / (int)GET_MAX_MANA(ch)) * 100.0 + 0.5,
  ((float)GET_MOVE(ch) / (int)GET_MAX_MOVE(ch)) * 100.0 + 0.5);

 act(buf,FALSE, ch,0,0,TO_CHAR);
 
}







void do_shout(struct char_data *ch, char *argument, int cmd)
{
  char buf1[MAX_INPUT_LENGTH+40];
  struct descriptor_data *i;
  extern int Silence;

dlog("in do_shout");

  if (!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_NOSHOUT)) {
    send_to_char("You can't shout!!\n\r", ch);
    return;
  }

  if (IS_NPC(ch) && 
      (Silence == 1) &&
      (IS_SET(ch->specials.act, ACT_POLYSELF))) 
  {
    send_to_char("Polymorphed shouting has been banned.\n\r", ch);
    send_to_char("It may return after a bit.\n\r", ch);
    return;
  }

  if (apply_soundproof(ch))
    return;
  
  for (; *argument == ' '; argument++);
  
  if (ch->master && IS_AFFECTED(ch, AFF_CHARM)) {
    if (!IS_IMMORTAL(ch->master)) {
      send_to_char("I don't think so :-)", ch->master);
      return;
    }
  }
  
if (GET_MOVE(ch)<10 || GET_MANA(ch)<10 && GetMaxLevel(ch) < LOW_IMMORTAL) {
   send_to_char("You do not have the strength to shout!\n\r",ch);
   return;
}

  if (!(*argument))
    send_to_char("Shout? Yes! Fine! Shout we must, but WHAT??\n\r", ch);
  else	{
    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
      sprintf(buf1,"$c0009You shout '%s'", argument);
      act(buf1,FALSE, ch,0,0,TO_CHAR);
    }
    sprintf(buf1, "$c0009[$c0015$n$c0009] shouts '%s'", argument);

    act("$c0009[$c0015$n$c0009] lifts up $s head and shouts loudly", FALSE, ch, 0, 0, TO_ROOM);
    
    if (GetMaxLevel(ch)<LOW_IMMORTAL) {
        GET_MOVE(ch) -= 10;
        GET_MANA(ch) -= 10;
        }
    
    for (i = descriptor_list; i; i = i->next)
      if (i->character != ch && !i->connected &&
	  (IS_NPC(i->character) ||
	   (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
	    !IS_SET(i->character->specials.act, PLR_DEAF))) &&
	  !check_soundproof(i->character)) {
	  act(buf1, 0, ch, 0, i->character, TO_VICT);
      }
  }
}

void do_gossip(struct char_data *ch, char *argument, int cmd)
{
  char buf1[MAX_INPUT_LENGTH+40];
  struct descriptor_data *i;
  extern int Silence;

dlog("in do_gossip");
  
  if (!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_NOSHOUT)) {
    send_to_char("You can't shout, gossip or auction.\n\r", ch);
    return;
  }

  if (IS_NPC(ch) && 
      (Silence == 1) &&
      (IS_SET(ch->specials.act, ACT_POLYSELF))) 
  {
    send_to_char("Polymorphed gossiping has been banned.\n\r", ch);
    send_to_char("It may return after a bit.\n\r", ch);
    return;
  }

  if (apply_soundproof(ch))
    return;
  
  for (; *argument == ' '; argument++);
  
  if (ch->master && IS_AFFECTED(ch, AFF_CHARM)) {
    if (!IS_IMMORTAL(ch->master)) {
      send_to_char("I don't think so :-)", ch->master);
      return;
    }
  }
  
  if (!(*argument))
    send_to_char("Gossip? Yes! but what!\n\r", ch);
  else	{
    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
      sprintf(buf1,"$c0011You gossip '%s'", argument);
      act(buf1,FALSE, ch,0,0,TO_CHAR);
    }

	/* I really hate when people gossip about lag and it is not */
	/* caused by this machine NOR IS IT ON this machine. This   */
	/* should grab all the gossips about it and make them think */
	/* that is was punched over the wire to everyone else! 	    */

if (strstr(argument,"lag") || strstr(argument,"LAG") || strstr(argument,"Lag") ||
    strstr(argument,"LAg") || strstr(argument,"laG") || strstr(argument,"lAG")) {
	/* do nothing.... */
   } else 
    	/* end lag checks */
    	
  {    	
    sprintf(buf1, "$c0011[$c0015$n$c0011] gossips '%s'", argument);
    for (i = descriptor_list; i; i = i->next)
      if (i->character != ch && !i->connected &&
	  (IS_NPC(i->character) ||
	   (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
	    !IS_SET(i->character->specials.act, PLR_NOGOSSIP))) &&
	  !check_soundproof(i->character)) {

#if ZONE_COMM_ONLY
			/* gossip in zone only */
  	 if (i->character->in_room != NOWHERE)  {
            if (real_roomp(ch->in_room)->zone == 
		real_roomp(i->character->in_room)->zone
		||GetMaxLevel(i->character) >=LOW_IMMORTAL
		||GetMaxLevel(ch) >= LOW_IMMORTAL ) 	{
		 act(buf1, 0, ch, 0, i->character, TO_VICT);
	    }
	 }
#else
		 act(buf1, 0, ch, 0, i->character, TO_VICT);
#endif

      } /* end for */
   } /* not gosssing about lag */
         
  }
}


void do_auction(struct char_data *ch, char *argument, int cmd)
{
  char buf1[MAX_INPUT_LENGTH+40];
  struct descriptor_data *i;
  extern int Silence;

dlog("in do_auction");
  
  if (!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_NOSHOUT)) {
    send_to_char("You can't shout, gossip or auction.\n\r", ch);
    return;
  }
  if (IS_NPC(ch) && 
      (Silence == 1) &&
      (IS_SET(ch->specials.act, ACT_POLYSELF))) 
  {
    send_to_char("Polymorphed auctioning has been banned.\n\r", ch);
    send_to_char("It may return after a bit.\n\r", ch);
    return;
  }
  

  if (apply_soundproof(ch))
    return;
  
  for (; *argument == ' '; argument++);
  
  if (ch->master && IS_AFFECTED(ch, AFF_CHARM)) {
    if (!IS_IMMORTAL(ch->master)) {
      send_to_char("I don't think so :-)", ch->master);
      return;
    }
  }
  
  if (!(*argument))
    send_to_char("Auction? Yes! But what!\n\r", ch);
  else	{
    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
      sprintf(buf1,"$c0010You auction '%s'", argument);
      act(buf1,FALSE, ch,0,0,TO_CHAR);
    }
    sprintf(buf1, "$c0010[$c0015$n$c0010] auctions '%s'", argument);

    for (i = descriptor_list; i; i = i->next)
      if (i->character != ch && !i->connected &&
	  (IS_NPC(i->character) ||
	   (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
	    !IS_SET(i->character->specials.act, PLR_NOAUCTION))) &&
	  !check_soundproof(i->character)) {
#if ZONE_COMM_ONLY
			/* gossip in zone only */
  	 if (i->character->in_room != NOWHERE)  {
            if (real_roomp(ch->in_room)->zone == 
		real_roomp(i->character->in_room)->zone
		||GetMaxLevel(i->character) >=LOW_IMMORTAL
		||GetMaxLevel(ch) >= LOW_IMMORTAL ) 	{
		 act(buf1, 0, ch, 0, i->character, TO_VICT);
	    }
	 }
#else
	  act(buf1, 0, ch, 0, i->character, TO_VICT);
#endif	  
      }
  }
}






void do_commune(struct char_data *ch, char *argument, int cmd)
{
  static char buf1[MAX_INPUT_LENGTH];
  struct descriptor_data *i;
  
dlog("in do_commune,think");
  
  for (; *argument == ' '; argument++);
  
  if (!(*argument))
    send_to_char("Communing among the gods is fine, but WHAT?\n\r",ch);
  else {
    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
      sprintf(buf1,"$c0012You think '%s'", argument);
      act(buf1,FALSE, ch,0,0,TO_CHAR);
    }
    sprintf(buf1, "$c0012::$c0015$n$c0012:: '%s'", argument);
    
    for (i = descriptor_list; i; i = i->next)
      if (i->character != ch && !i->connected && !IS_NPC(i->character) &&
	  !IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
	  (GetMaxLevel(i->character) >= LOW_IMMORTAL))
	act(buf1, 0, ch, 0, i->character, TO_VICT);
  }
}


void do_tell(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  char name[100], message[MAX_INPUT_LENGTH+20],
  buf[MAX_INPUT_LENGTH+60];

dlog("in do_tell");

	if (apply_soundproof(ch))
	  return;
  
  half_chop(argument,name,message);

  if(!*name || !*message) {
    send_to_char("Who do you wish to tell what??\n\r", ch);
    return;
  } else if (!(vict = get_char_vis(ch, name))) {
    send_to_char("No-one by that name here..\n\r", ch);
    return;
  } else if (ch == vict) {
    send_to_char("You try to tell yourself something.\n\r", ch);
    return;
  } else 
    if (GET_POS(vict) == POSITION_SLEEPING && !IS_IMMORTAL(ch)) 
  {
    act("$E is asleep, shhh.",FALSE,ch,0,vict,TO_CHAR);
    return;
   }  else if (IS_NPC(vict) && !(vict->desc)) {
    send_to_char("No-one by that name here..\n\r", ch);
    return;
  }  else if (!(GetMaxLevel(ch) >= LOW_IMMORTAL) &&IS_SET(vict->specials.act,PLR_NOTELL)) {
     act("$N is not listening for tells right now.",FALSE,ch,0,vict,TO_CHAR);
     return;
   } else if ((GetMaxLevel(vict) >= LOW_IMMORTAL) && (GetMaxLevel(ch) >= LOW_IMMORTAL) &&
  	     (GetMaxLevel(ch) < GetMaxLevel(vict)) && IS_SET(vict->specials.act,PLR_NOTELL)) {
     act("$N is not listening for tells right now!",FALSE,ch,0,vict,TO_CHAR);
     return;
  } else if (!vict->desc) {
    send_to_char("They can't hear you, link dead.\n\r", ch);
    return;
  } 

  if (check_soundproof(vict)) {
	send_to_char("In a silenced room, try again later.\n\r",ch);
	return;
  }

#if ZONE_COMM_ONLY
     if (real_roomp(ch->in_room)->zone != 
	 real_roomp(vict->in_room)->zone
	  && GetMaxLevel(ch) < LOW_IMMORTAL ) 	{
	 send_to_char("That person is not near enough for you to tell.\n\r",ch);
 	 return;
	}
#endif
	
  sprintf(buf,"$c0013[$c0015%s$c0013] tells you '%s'",
	  (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), message);
  act(buf,FALSE, vict,0,0,TO_CHAR);

  if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) { 
     sprintf(buf,"$c0013You tell %s %s'%s'",
	  (IS_NPC(vict) ? vict->player.short_descr : GET_NAME(vict)), \
          (IS_AFFECTED2(vict, AFF2_AFK) ? "(who is AFK) " : ""), message);
     act(buf,FALSE, ch,0,0,TO_CHAR);
  }
  
}



void do_whisper(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  char name[100], message[MAX_INPUT_LENGTH],
  buf[MAX_INPUT_LENGTH];

dlog("in do_whisper");

	if (apply_soundproof(ch))
	  return;
  
  half_chop(argument,name,message);
  
  if(!*name || !*message)
    send_to_char("Who do you want to whisper to.. and what??\n\r", ch);
  else if (!(vict = get_char_room_vis(ch, name)))
    send_to_char("No-one by that name here..\n\r", ch);
  else if (vict == ch) {
    act("$n whispers quietly to $mself.",FALSE,ch,0,0,TO_ROOM);
    send_to_char
      ("You can't seem to get your mouth close enough to your ear...\n\r",ch);
  }  else    {
	if (check_soundproof(vict))
	  return;

      sprintf(buf,"$c0005[$c0015$n$c0005] whispers to you, '%s'",message);
      act(buf, FALSE, ch, 0, vict, TO_VICT);
      if (IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
        sprintf(buf,"$c0005You whisper to %s%s, '%s'",
	      (IS_NPC(vict) ? vict->player.name : GET_NAME(vict)), \
              (IS_AFFECTED2(vict, AFF2_AFK) ? " (who is AFK)" : ""), message);
        act(buf,FALSE, ch,0,0,TO_CHAR);
      }
      act("$c0005$n whispers something to $N.", FALSE, ch, 0, vict, TO_NOTVICT);
    }
}


void do_ask(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  char name[100], message[MAX_INPUT_LENGTH],
  buf[MAX_INPUT_LENGTH];


dlog("in do_ask");

	if (apply_soundproof(ch))
	  return;
  
  half_chop(argument,name,message);
  
  if(!*name || !*message)
    send_to_char("Who do you want to ask something.. and what??\n\r", ch);
  else if (!(vict = get_char_room_vis(ch, name)))
    send_to_char("No-one by that name here..\n\r", ch);
  else if (vict == ch)	{
    act("$c0006[$c0015$n$c0006] quietly asks $mself a question.",FALSE,ch,0,0,TO_ROOM);
    act("$c0006You think about it for a while...",FALSE, ch,0,0,TO_CHAR);
  }  else	{
	if (check_soundproof(vict))
	  return;

    sprintf(buf,"$c0006[$c0015$n$c0006] asks you '%s'",message);
    act(buf, FALSE, ch, 0, vict, TO_VICT);
    
    if (IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
      sprintf(buf,"$c0006You ask %s%s, '%s'",
	    (IS_NPC(vict) ? vict->player.name : GET_NAME(vict)), \
            (IS_AFFECTED2(vict, AFF2_AFK) ? " (who is AFK)" : ""), message);
      act(buf,FALSE, ch,0,0,TO_CHAR);
    }
    act("$c0006$n asks $N a question.",FALSE,ch,0,vict,TO_NOTVICT);
  }
}



#define MAX_NOTE_LENGTH 1000      /* arbitrary */

void do_write(struct char_data *ch, char *argument, int cmd)
{
  struct obj_data *paper = 0, *pen = 0;
  char papername[MAX_INPUT_LENGTH], penname[MAX_INPUT_LENGTH],
       buf[MAX_STRING_LENGTH];

dlog("in do_write");

  argument_interpreter(argument, papername, penname);
  
  if (!ch->desc)
    return;
  
  if (!*papername)  /* nothing was delivered */    {   
      send_to_char("write (on) papername (with) penname.\n\r", ch);
      return;
    }

  if (!*penname) {
      send_to_char("write (on) papername (with) penname.\n\r", ch);
      return;
  }
  if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))	{
	  sprintf(buf, "You have no %s.\n\r", papername);
	  send_to_char(buf, ch);
	  return;
   }
   if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying)))	{
	  sprintf(buf, "You have no %s.\n\r", papername);
	  send_to_char(buf, ch);
	  return;
    }

  /* ok.. now let's see what kind of stuff we've found */
  if (pen->obj_flags.type_flag != ITEM_PEN) {
      act("$p is no good for writing with.",FALSE,ch,pen,0,TO_CHAR);
  } else if (paper->obj_flags.type_flag != ITEM_NOTE)    {
      act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
  } else if (paper->action_description) {
    send_to_char("There's something written on it already.\n\r", ch);
    return;
  } else {
      /* we can write - hooray! */
      send_to_char
	("Ok.. go ahead and write.. end the note with a @.\n\r", ch);
      act("$n begins to jot down a note.", TRUE, ch, 0,0,TO_ROOM);
      ch->desc->str = &paper->action_description;
      ch->desc->max_str = MAX_NOTE_LENGTH;
    }
}

char *RandomWord()
{
  static char *rstring[] = {
    "argle",
    "bargle",
    "glop",
    "glyph",
    "hussamah",  /* 5 */
    "rodina",
    "mustafah",
    "angina",
    "the",
    "fribble",  /* 10 */
    "fnort",
    "frobozz",
    "zarp",
    "ripple",
    "yrk",    /* 15 */
    "yid",
    "yerf",
    "oork",
    "grapple",
    "red",   /* 20 */
    "blue",
    "you",
    "me",
    "ftagn",
    "hastur",   /* 25 */
    "brob",
    "gnort",
    "lram",
    "truck",
    "kill",    /* 30 */
    "cthulhu",
    "huzzah",
    "acetacytacylic",
    "hydrooxypropyl",
    "summah",     /* 35 */
    "hummah",
    "cookies",
    "stan",
    "will",
    "wadapatang",   /* 40 */
    "pterodactyl",
    "frob",
    "yuma",
    "gumma",
    "lo-pan",   /* 45 */
    "sushi",
    "yaya",
    "yoyodine",
    "yaazr",
    "bipsnop",   /* 50 */
    "\n"
  };

 int i;
 for (i=0;*rstring[i]!='\n';i++); /* count max number in array */
  return(rstring[number(0,i)]);
}

void do_sign(struct char_data *ch, char *argument, int cmd)
{
  int i;
  char buf[MAX_INPUT_LENGTH+40];
  char buf2[MAX_INPUT_LENGTH];
  char *p;
  int diff;
  struct char_data *t;
  struct room_data *rp;

dlog("in do_sign");

  for (i = 0; *(argument + i) == ' '; i++);
  
  if (!*(argument + i))
    send_to_char("Yes, but WHAT do you want to sign?\n\r", ch);
  else	{

    rp = real_roomp(ch->in_room);
    if (!rp) return;

    if (!HasHands(ch)) {
      send_to_char("Yeah right... WHAT HANDS!!!!!!!!\n\r", ch);
      return;
    }

    strcpy(buf, argument+i);
    buf2[0] = '\0';
    /*
      work through the argument, word by word.  if you fail your
      skill roll, the word comes out garbled.
      */
    p = strtok(buf, " ");  /* first word */

    diff = strlen(buf);

    while (p) {
      if (ch->skills && number(1,75+strlen(p))<ch->skills[SKILL_SIGN].learned){
	strcat(buf2, p);
      } else {
	strcat(buf2, RandomWord());
      }
      strcat(buf2, " ");
      diff -= 1;
      p = strtok(0, " ");  /* next word */
    }
    /*
      if a recipient fails a roll, a word comes out garbled.
      */

    /*
      buf2 is now the "corrected" string.
      */

    sprintf(buf,"$n signs '%s'", buf2);

    for (t = rp->people;t;t=t->next_in_room) {
      if (t != ch) {
	if (t->skills && number(1,diff) < t->skills[SKILL_SIGN].learned) {
	  act(buf, FALSE, ch, 0, t, TO_VICT);
	} else {
	  act("$n makes funny motions with $s hands", 
	      FALSE, ch, 0, t, TO_VICT);	  
	}
      }
    }

    if (IS_NPC(ch)||(IS_SET(ch->specials.act, PLR_ECHO))) {
      sprintf(buf,"You sign '%s'\n\r", argument + i);
      send_to_char(buf, ch);
    }
  }
}

	/* speak elvish, speak dwarvish, etc...		    */
void do_speak(struct char_data *ch, char *argument, int cmd)
{
char buf[255];
int i,ii;

#define MAX_LANGS 8 

char *lang_list[MAX_LANGS] = {
"common",
"elvish",
"halfling",
"dwarvish",
"orcish",
"giantish",
"ogre",
"gnomish"
};

dlog("in do_speak");

only_argument(argument,buf);

if (buf[0] == '\0') {
	send_to_char("Speak what language?\n\r",ch);
	return;
}

if (strstr(buf,"common")) 
    i = SPEAK_COMMON; else
if (strstr(buf,"elvish")) 
    i = SPEAK_ELVISH; else
if (strstr(buf,"halfling")) 
    i = SPEAK_HALFLING; else
if (strstr(buf,"dwarvish")) 
    i = SPEAK_DWARVISH; else
if (strstr(buf,"orcish")) 
    i = SPEAK_ORCISH; else
if (strstr(buf,"giantish")) 
    i = SPEAK_GIANTISH; else
if (strstr(buf,"ogre")) 
    i = SPEAK_OGRE; else
if (strstr(buf,"gnomish")) 
    i = SPEAK_GNOMISH; else 
    i = -1;

if (i == -1) {
  send_to_char("Un-recognized language!\n\r",ch);
  return;
 } 

		/* set language that we're gonna speak */	 
 ch->player.speaks = i; 
 sprintf(buf,"You concentrate on speaking %s.\n\r",lang_list[i-1]);
 send_to_char(buf,ch);
}
	/* this is where we do the language says */
void do_new_say(struct char_data *ch, char *argument, int cmd)
{
  int i,ii,learned,skill_num;
  char buf[MAX_INPUT_LENGTH+40];
  char buf2[MAX_INPUT_LENGTH];
  char buf3[MAX_INPUT_LENGTH+40];  
  char *p;
  int diff;
  struct char_data *t;
  struct room_data *rp;

dlog("in do_new_say");

  if (!argument) 
	return;

  for (ii = 0; *(argument + ii) == ' '; ii++);
  
  if (!argument[ii])
    send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
  else	{

  if (apply_soundproof(ch))
    return;

    rp = real_roomp(ch->in_room);
    if (!rp) return;


if (!ch->skills) {
    learned = 0;
    skill_num = LANG_COMMON;
   } else {
		/* find the language we are speaking */

	switch(ch->player.speaks) {
		case SPEAK_COMMON:learned = ch->skills[LANG_COMMON].learned;
				skill_num=LANG_COMMON;
				  break;
		case SPEAK_ELVISH: learned = ch->skills[LANG_ELVISH].learned;
				skill_num=LANG_ELVISH;
				break;
		case SPEAK_HALFLING:learned = ch->skills[LANG_HALFLING].learned;
				skill_num=LANG_HALFLING;
				break;
		case SPEAK_DWARVISH: learned = ch->skills[LANG_DWARVISH].learned;
				skill_num=LANG_DWARVISH;
				break;
		case SPEAK_ORCISH:learned = ch->skills[LANG_ORCISH].learned;
				skill_num=LANG_ORCISH;
				break;
		case SPEAK_GIANTISH:learned = ch->skills[LANG_GIANTISH].learned;
				skill_num=LANG_GIANTISH;
				break;
		case SPEAK_OGRE:learned = ch->skills[LANG_OGRE].learned;
				skill_num=LANG_OGRE;
				break;
		case SPEAK_GNOMISH:learned = ch->skills[LANG_GNOMISH].learned;
				skill_num=LANG_GNOMISH;
				break;

		default:learned = ch->skills[LANG_COMMON].learned;
			skill_num = LANG_COMMON;
			break;
 		} /* end switch */
     }
/* end finding language */


    strcpy(buf, argument+ii);
    buf2[0] = '\0'; 

   /* we use this for ESP and immortals and comprehend lang */
    sprintf(buf3,"$c0015[$c0005$n$c0015] says '%s'",buf);
    
    /*
      work through the argument, word by word.  if you fail your
      skill roll, the word comes out garbled.
      */
    p = strtok(buf, " ");  /* first word */

    diff = strlen(buf);


    while (p) {
      if (number(1,75+strlen(p))<learned || GetMaxLevel(ch) >= LOW_IMMORTAL){
	strcat(buf2, p);
      } else {
	/* add case statement here to use random words from clips of elvish */
	/* dwarvish etc so the words look like they came from that language */
	strcat(buf2, RandomWord());
      }
      strcat(buf2, " ");
      diff -= 1;
      p = strtok(0, " ");  /* next word */
    }
    /*
      if a recipient fails a roll, a word comes out garbled.
      */

    /*
      buf2 is now the "corrected" string.
      */
if (!*buf2 || !buf2) {
    send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
    return;
    }

    sprintf(buf,"$c0015[$c0005$n$c0015] says '%s'", buf2);
    
    for (t = rp->people;t;t=t->next_in_room) {
   if (t != ch) {
if ((t->skills) && number(1,diff) < t->skills[skill_num].learned || 
    GetMaxLevel(t) >= LOW_IMMORTAL || IS_NPC(t) ||
    affected_by_spell(t,SKILL_ESP) || affected_by_spell(t,SPELL_COMP_LANGUAGES)) {

			/* these guys always understand */
    if (GetMaxLevel(t) >= LOW_IMMORTAL || affected_by_spell(t,SKILL_ESP) ||
       affected_by_spell(t,SPELL_COMP_LANGUAGES) || IS_NPC(t) )
    	  act(buf3, FALSE,ch,0,t,TO_VICT); else
    	  		/* otherwise */
    	  		
	  act(buf, FALSE, ch, 0, t, TO_VICT);
	} else {
        act("$c0010$n speaks in a language you can't quite understand.", FALSE, ch, 0, t, TO_VICT);	  
	}
      }
    }

    if (IS_NPC(ch)||(IS_SET(ch->specials.act, PLR_ECHO))) {
      sprintf(buf,"$c0015You say '%s'", argument + ii);
      act(buf,FALSE, ch,0,0,TO_CHAR);
    }
  }
}



void do_gtell(struct char_data *ch, char *argument, int cmd)
{
  int i;
  struct char_data *victim, *k;
  struct follow_type *f;
  char buf[MAX_STRING_LENGTH];

dlog("in do_gtell");

 if (apply_soundproof(ch))
     return;
  
  for (i = 0; *(argument + i) == ' '; i++);

  if(!*(argument+i)) {
    send_to_char("What do you want to group tell!??\n\r", ch);
    return;
  } 

 if (!IS_AFFECTED(ch, AFF_GROUP)) {
      send_to_char("But you are a member of no group?!\n\r", ch);
      return;
    } else {
      if (ch->master)
	k = ch->master;
      else
	k = ch;

      
 for(f=k->followers; f; f=f->next)
 {
  if (IS_AFFECTED(f->follower, AFF_GROUP))
    if (!f->follower->desc) {
      /* link dead */
   } else
  if (ch == f->follower) {
      /* can't tell yourself! */
    } else
   if (!check_soundproof(f->follower))  {
   sprintf(buf,"$c0012[$c0015%s$c0012] group tells you '%s'",
 	  (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), argument+i);
   act(buf, FALSE,f->follower,0,0,TO_CHAR);
  } /* !soundproof */
 } /* end for loop */

		/* send to master now */
if (ch->master) {
  if (IS_AFFECTED(ch->master, AFF_GROUP))
    if (!ch->master->desc) {
      /* link dead */
   } else
  if (ch == ch->master) {
      /* can't tell yourself! */
    } else
   if (!check_soundproof(ch->master))  {
   sprintf(buf,"$c0012[$c0015%s$c0012] group tells you '%s'",
 	  (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), argument+i);
   act(buf, FALSE,ch->master,0,0,TO_CHAR);
  } /* !soundproof */
 } 	/* end master send */		
 
  if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) { 
      sprintf(buf,"$c0012You group tell '%s'",argument+i);
      act(buf, FALSE,ch,0,0,TO_CHAR);
    } /* if echo */
    
  } /* they where grouped... */   
} /* end of gtel */

/*
 * 'Split' originally by Gnort, God of Chaos. I stole it from Merc
 * and changed it to work with mine :) Heh msw
 */
 
void do_split(struct char_data *ch, char *argument, int cmd)
{

bool is_same_group( struct char_data *ach, struct char_data *bch );

    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    struct char_data *gch;
    int members, amount, share, extra;

dlog("in do_split");

    one_argument( argument, arg );

    if ( arg[0] == '\0' ) {
	send_to_char( "Split how much?\n\r", ch );
	return;
    }
    
    amount = atoi( arg );

    if ( amount < 0 )    {
	send_to_char( "Your group wouldn't like that.\n\r", ch );
	return;
    }

    if ( amount == 0 )    {
	send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
	return;
    }

    if ( ch->points.gold < amount )    {
	send_to_char( "You don't have that much gold.\n\r", ch );
	return;
    }
  
    members = 0;
    for ( gch = real_roomp(ch->in_room)->people;gch != NULL; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
	    members++;
    }

    if ( members < 2 )    {
	send_to_char( "Just keep it all.\n\r", ch );
	return;
    }
	    
    share = amount / members;
    extra = amount % members;

    if ( share == 0 )    {
	send_to_char( "Don't even bother, cheapskate.\n\r", ch );
	return;
    }

    ch->points.gold -= amount;
    ch->points.gold += share + extra;

    sprintf( buf,
	"You split %d gold coins.  Your share is %d gold coins.\n\r",
	amount, share + extra );
    send_to_char( buf, ch );

    sprintf( buf, "$n splits %d gold coins.  Your share is %d gold coins.",
	amount, share );

    for ( gch = real_roomp(ch->in_room)->people; gch != NULL; gch = gch->next_in_room )
    {
	if ( gch != ch && is_same_group( gch, ch ) )
	{
	    act( buf,FALSE,ch, NULL, gch, TO_VICT );
	    gch->points.gold += share;
	}
    }

    return;
}


void do_pray(struct char_data *ch, char *argument, int cmd)
{
  struct affected_type af;
  char buf1[MAX_INPUT_LENGTH];
  struct descriptor_data *i;
  int ii=0; 

dlog("in do_pray");

if (IS_NPC(ch))
   return;  

  if (affected_by_spell(ch, SPELL_PRAYER)) {
    send_to_char("You have already prayed today.\n\r",ch);
    return;
  }
   
  for (; *argument == ' '; argument++);
  
if (!(*argument))
  send_to_char("Pray to a deity, but what?!?! (pray <DeityName> <prayer>)\n\r",ch);
  else 
  {
   ii =((int)GetMaxLevel(ch)*1.5);

   if (HasClass(ch,CLASS_CLERIC|CLASS_DRUID))
       ii +=10;  /* clerics get a 10% bonus :) */

   if (ii>number(1,101)) {
    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
      sprintf(buf1,"You pray '%s'\n\r", argument);
      send_to_char(buf1, ch);
    }
    sprintf(buf1, ":*:$n:*: prays '%s'", argument);
    
    for (i = descriptor_list; i; i = i->next) {
      if (i->character != ch && !i->connected && !IS_NPC(i->character) &&
	  !IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
	  (GetMaxLevel(i->character) >= LOW_IMMORTAL))
	act(buf1, 0, ch, 0, i->character, TO_VICT);
   } /* end for */
   
  } /* failed prayer */
   else
      send_to_char("You prayer is ignored at this time.\n\r",ch);

    af.type = SPELL_PRAYER;
    af.duration = 24;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char(ch, &af);    
   return;
 }
} 

/* modified by Aarcerak */
bool is_same_group( struct char_data *ach, struct char_data *bch )
{
    if( !IS_AFFECTED(ach,AFF_GROUP) || !IS_AFFECTED(bch,AFF_GROUP))
       return 0;
    if ( ach->master != NULL ) 
       ach = ach->master;
    if ( bch->master != NULL ) 
       bch = bch->master;
    return (ach == bch);
}


void do_telepathy(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  char name[100], message[MAX_INPUT_LENGTH+20],
  buf[MAX_INPUT_LENGTH+60];

dlog("in do_telepathy");

  half_chop(argument,name,message);

if (!HasClass(ch,CLASS_PSI) && !IS_AFFECTED(ch,AFF_TELEPATHY)) {
   send_to_char("What do you think you are? A Telepath?\n\r",ch);
   return;
}

if (GET_MANA(ch) < 5 && !IS_AFFECTED(ch,AFF_TELEPATHY)) {
   send_to_char("You do not have the mental power to bespeak anyone.\n\r",ch);
   return;
}

  if(!*name || !*message) {
    send_to_char("Who do you wish to bespeak what??\n\r", ch);
    return;
  } else if (!(vict = get_char_vis(ch, name))) {
    send_to_char("No-one by that name here..\n\r", ch);
    return;
  } else if (ch == vict) {
    send_to_char("You try to bespeak yourself something.\n\r", ch);
    return;
  } else 
    if (GET_POS(vict) == POSITION_SLEEPING && !IS_IMMORTAL(ch)) 
  {
    act("$E is asleep, shhh.",FALSE,ch,0,vict,TO_CHAR);
    return;
   }  else if (IS_NPC(vict) && !(vict->desc)) {
    send_to_char("No-one by that name here..\n\r", ch);
    return;
  }  else if (!(GetMaxLevel(ch) >= LOW_IMMORTAL) &&IS_SET(vict->specials.act,PLR_NOTELL)) {
     act("$N is not listening for thoughts right now.",FALSE,ch,0,vict,TO_CHAR);
     return;
   } else if ((GetMaxLevel(vict) >= LOW_IMMORTAL) && (GetMaxLevel(ch) >= LOW_IMMORTAL) &&
  	     (GetMaxLevel(ch) < GetMaxLevel(vict)) && IS_SET(vict->specials.act,PLR_NOTELL)) {
     act("$N is not listening for thoughts right now!",FALSE,ch,0,vict,TO_CHAR);
     return;
  } else if (!vict->desc) {
    send_to_char("They can't hear you, link dead.\n\r", ch);
    return;
  }

/*
  if (check_soundproof(vict)) {
	send_to_char("In a silenced room, try again later.\n\r",ch);
	return;
  }
*/

  if (!IS_AFFECTED(ch,AFF_TELEPATHY))
  	GET_MANA(ch) -=5;  
 
  sprintf(buf,"$c0013[$c0015%s$c0013] bespeaks you '%s'",
	  (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), message);
  act(buf,FALSE, vict,0,0,TO_CHAR);

  if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) { 
     sprintf(buf,"$c0013You bespeak %s '%s'",
	  (IS_NPC(vict) ? vict->player.short_descr : GET_NAME(vict)), message);
     act(buf,FALSE, ch,0,0,TO_CHAR);
  }


}

