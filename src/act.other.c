/*
  DaleMUD v2.0	Released 2/1994
  See license.doc for distribution terms.   DaleMUD is based on DIKUMUD
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "protos.h"


/* extern variables */

extern struct str_app_type str_app[];
extern struct descriptor_data *descriptor_list;
extern struct dex_skill_type dex_app_skill[];
extern struct spell_info_type spell_info[];
extern struct char_data *character_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern char  *spells[];
extern struct spell_info_type spell_info[MAX_SPL_LIST];


void do_gain(struct char_data *ch, char *argument, int cmd)
{
dlog("in do_gain");
}

void do_guard(struct char_data *ch, char *argument, int cmd)
{
  char comm[100];
dlog("in do_guard");

  if (!IS_NPC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) {
    send_to_char("Sorry. you can't just put your brain on autopilot!\n\r",ch);
    return;
  }

  for(;isspace(*argument); argument++);

  if (!*argument) {
    if (IS_SET(ch->specials.act, ACT_GUARDIAN)) {
      act("$n relaxes.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("You relax.\n\r",ch);
      REMOVE_BIT(ch->specials.act, ACT_GUARDIAN);
    } else {
      SET_BIT(ch->specials.act, ACT_GUARDIAN);
      act("$n alertly watches you.", FALSE, ch, 0, ch->master, TO_VICT);
      act("$n alertly watches $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
      send_to_char("You snap to attention\n\r", ch);    
    }  
  } else {
     if (!str_cmp(argument,"on")) {
      if (!IS_SET(ch->specials.act, ACT_GUARDIAN)) {
         SET_BIT(ch->specials.act, ACT_GUARDIAN);
         act("$n alertly watches you.", FALSE, ch, 0, ch->master, TO_VICT);
         act("$n alertly watches $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
         send_to_char("You snap to attention\n\r", ch);
       }
     } else if (!str_cmp(argument,"off")) {
       if (IS_SET(ch->specials.act, ACT_GUARDIAN)) {
         act("$n relaxes.", FALSE, ch, 0, 0, TO_ROOM);
         send_to_char("You relax.\n\r",ch);
         REMOVE_BIT(ch->specials.act, ACT_GUARDIAN);
       }
     }
  }

  return;
}


void do_junk(struct char_data *ch, char *argument, int cmd)
{
  char arg[100], buf[100], newarg[100];
  struct obj_data *tmp_object;
  int num, p, count, value=0;

dlog("in do_junk");

/*
 *   get object name & verify
 */

  only_argument(argument, arg);
  if (*arg) {
    if (getall(arg,newarg)!=NULL) {
      num = -1;
      strcpy(arg,newarg);
    } else if ((p = getabunch(arg,newarg))!=NULL) {
      num = p;                     
      strcpy(arg,newarg);
    } else {
      num = 1;  
    }
  } else {
    send_to_char("Junk what?\n\r",ch);
    return;
  }
  count = 0;
  while (num != 0) {
    tmp_object = get_obj_in_list_vis(ch, arg, ch->carrying);
    if (tmp_object) {
      if (IS_OBJ_STAT(tmp_object,ITEM_NODROP)) {
	send_to_char
	  ("You can't let go of it, it must be CURSED!\n\r", ch);
	return;
      }
      value+=(MIN(1000,MAX(tmp_object->obj_flags.cost/4,1)));
      obj_from_char(tmp_object);
      extract_obj(tmp_object);
      if (num > 0) num--;
      count++;
    } else {
      num = 0;
    }
  }
  if (count > 1) {
    sprintf(buf, "You junk %s (%d).\n\r", arg, count);
    act(buf, 1, ch, 0, 0, TO_CHAR);
    sprintf(buf, "$n junks %s.\n\r", arg);
    act(buf, 1, ch, 0, 0, TO_ROOM);
  } else if (count == 1) {
    sprintf(buf, "You junk %s \n\r", arg);
    act(buf, 1, ch, 0, 0, TO_CHAR);
    sprintf(buf, "$n junks %s.\n\r", arg);
    act(buf, 1, ch, 0, 0, TO_ROOM);
  } else {
    send_to_char("You don't have anything like that\n\r", ch);
  }

  value /= 2;

  if (value) 	{
    act("You are awarded for outstanding performance.", 
	FALSE, ch, 0, 0, TO_CHAR);
    
    if (GetMaxLevel(ch) < 3)
      gain_exp(ch, MIN(100,value));
    else
      GET_GOLD(ch) += value;
  }

  return;
}

void do_qui(struct char_data *ch, char *argument, int cmd)
{
	send_to_char("You have to write quit - no less, to quit!\n\r",ch);
	return;
}

void do_set_prompt(struct char_data *ch, char *argument, int cmd)
{
  static struct def_prompt {
    int n;
    char *pr;
  } prompts[] = {
    {1, "Shadowdale> "},
    {2, "H:%h V:%v> "},
    {3, "H:%h M:%m V:%v> "},
    {4, "H:%h/%H V:%v/%V> "},
    {5, "H:%h/%H M:%m/%M V:%v/%v> "},
    {6, "H:%h V:%v C:%C> "},
    {7, "H:%h M:%m V:%v C:%C> "},
    {8, "H:%h V:%v C:%C %S> "},
    {9, "H:%h M:%m V:%v C:%C %S> "},
    {40,"H:%h R:%R> "},
    {41,"H:%h R:%R i%iI+> "},
    {0,NULL}};
  char buf[512];
  int i,n;

dlog("in do_set_prompt");

/*
  if (IS_NPC(ch) || !ch->desc)
    return;
*/
 
  for(;isspace(*argument); argument++)  ;
  if (*argument) {
    if((n=atoi(argument))!=0) {
      if(n>39 && !IS_IMMORTAL(ch)) {
        send_to_char("Eh?\r\n",ch);
        return;
      }
      for(i=0;prompts[i].pr;i++)
        if(prompts[i].n==n) {
/*
          sprintf(buf, "Your prompt now is : <%s>\n\r", argument);
          send_to_char(buf, ch);
*/
          if(ch->specials.prompt) 
            free(ch->specials.prompt);
          ch->specials.prompt = strdup(prompts[i].pr);
          return;
        }
      send_to_char("Invalid prompt number\n\r",ch);
    } else {
/*
      sprintf(buf, "Your prompt now is : <%s>\n\r", argument);
      send_to_char(buf, ch);
*/
      if(ch->specials.prompt) 
        free(ch->specials.prompt);
      ch->specials.prompt = strdup(argument);
    }
  } else {
    sprintf(buf,"Your current prompt is : %s\n\r",ch->specials.prompt);
    send_to_char(buf,ch);
  }
 
}



void do_title(struct char_data *ch, char *argument, int cmd)
{
   char buf[512];


dlog("in do_title");

/*   char *strdup(char *source); */


   if (IS_NPC(ch) || !ch->desc)
       return;

  for(;isspace(*argument); argument++)  ;

  if (*argument) {

    if (strlen(argument) > 150) {
      send_to_char("Line too long, truncated\n", ch);
      *(argument + 151) = '\0';
    }
    sprintf(buf, "Your title has been set to : <%s>\n\r", argument);
    send_to_char(buf, ch);
if (ch->player.title)
    free(ch->player.title);
    ch->player.title = strdup(argument);
  }      

}

void do_quit(struct char_data *ch, char *argument, int cmd)
{
  char buf[256];

dlog("in do_quit");
  if (IS_NPC(ch) || !ch->desc || IS_AFFECTED(ch, AFF_CHARM))
    return;
  
  if (GET_POS(ch) == POSITION_FIGHTING) {
    send_to_char("No way! You are fighting.\n\r", ch);
    return;
  }
  
  if (GET_POS(ch) < POSITION_STUNNED) {
    send_to_char("You die before your time!\n\r", ch);
    sprintf(buf, "%s dies via quit.", GET_NAME(ch));
    log(buf);
    die(ch,NULL);    
    return;
  }
  
  act("Goodbye, friend.. Come back soon!", FALSE, ch, 0, 0, TO_CHAR);
  act("$n has left the game.", TRUE, ch,0,0,TO_ROOM);
  zero_rent(ch);
  extract_char(ch); /* Char is saved in extract char */
  
  close_socket(ch->desc); 	/* testing....*/
}



void do_save(struct char_data *ch, char *argument, int cmd)
{
  struct obj_cost cost;
  struct char_data *tmp;
  struct obj_data *tl;
  struct obj_data *teq[MAX_WEAR], *eq[MAX_WEAR], *o;
  char buf[256];
  int i;

dlog("in do_save");

  if (IS_NPC(ch) && !(IS_SET(ch->specials.act, ACT_POLYSELF)))
    return;
  
if (cmd != 0)
	send_to_char("Saving...\n\r",ch);
  

  if (IS_NPC(ch) && (IS_SET(ch->specials.act, ACT_POLYSELF))) {
    /*  
      swap stuff, and equipment
      */
    if (!ch->desc)  
      tmp = ch->orig;
    else 
      tmp = ch->desc->original;  /* tmp = the original characer */
    
    if (!tmp) return;
    tl = tmp->carrying;
/*
  there is a bug with this:  When you save, the alignment thing is checked,
  to see if you are supposed to be wearing what you are.  if your stuff gets
  kicked off your body, it will end up in room #3, on the floor, and in
  the inventory of the polymorphed monster.  This is a "bad" thing.  So,
  to fix it, each item in the inventory is checked.  if it is in a room,
  it is moved from the room, back to the correct inventory slot. 
  */
    tmp->carrying = ch->carrying;
    for (i = 0; i < MAX_WEAR; i++) {  /* move all the mobs eq to the ch */
      teq[i] = tmp->equipment[i];
      tmp->equipment[i] = ch->equipment[i];
    }
    GET_EXP(tmp) = GET_EXP(ch);
    GET_GOLD(tmp) = GET_GOLD(ch);
    GET_ALIGNMENT(tmp) = GET_ALIGNMENT(ch);
    recep_offer(tmp, NULL, &cost, FALSE);
    save_obj(tmp, &cost, 0);
    save_char(ch, AUTO_RENT);  /* we can't save tmp because they
				  don't have a desc.  */
    write_char_extra(ch);
    tmp->carrying = tl;
    for (i = 0; i < MAX_WEAR; i++) {
      tmp->equipment[i] = teq[i];
      if (ch->equipment[i] && ch->equipment[i]->in_room != -1) {
	o = ch->equipment[i];
	ch->equipment[i] = 0;
	obj_from_room(o);
/*
	obj_to_char(o, ch);
*/
	equip_char(ch, o, i);  /* equip the correct slot */
      }
    }
    return;
  } else {
    recep_offer(ch, NULL, &cost,FALSE);
    save_obj(ch, &cost, 0);
    save_char(ch, AUTO_RENT);
  }
}


void do_not_here(struct char_data *ch, char *argument, int cmd)
{
	send_to_char("Sorry, but you cannot do that here!\n\r",ch);
}


void do_sneak(struct char_data *ch, char *argument, int cmd)
{
  struct affected_type af;
  byte percent;

dlog("in do_sneak");

  if (IS_AFFECTED(ch, AFF_SNEAK)) {
    affect_from_char(ch, SKILL_SNEAK);
    if (IS_AFFECTED(ch, AFF_HIDE))
      REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
    send_to_char("You are no longer sneaky.\n\r",ch);
    return;
  }
  if (!HasClass(ch, CLASS_THIEF|CLASS_MONK|CLASS_RANGER)) {
    send_to_char("You're not trained to walk silently!\n\r", ch);
    return;
  }

if (HasClass(ch,CLASS_RANGER) && !OUTSIDE(ch)) {
  send_to_char("You must do this outdoors!\n\r",ch);
  return;
}

  if (MOUNTED(ch)) {
    send_to_char("Yeah... right... while mounted\n\r", ch);
    return;
  }

  if (!IS_AFFECTED(ch, AFF_SILENCE)) {
    if (EqWBits(ch, ITEM_ANTI_THIEF)) {
      send_to_char("Gonna be hard to sneak around in that!\n\r", ch);
      return;
    }
    if (HasWBits(ch, ITEM_HUM)) {
     send_to_char("Gonna be hard to sneak around with that thing humming\n\r", 
		 ch);
     return;
   }
  }

  send_to_char("Ok, you'll try to move silently for a while.\n\r", ch);
  
  percent=number(1,101); /* 101% is a complete failure */

  if (!ch->skills)
    return;

  if (IS_AFFECTED(ch, AFF_SILENCE))
    percent = MIN(1, percent-35);  /* much easier when silenced */
  
  if (percent > ch->skills[SKILL_SNEAK].learned +
      dex_app_skill[GET_DEX(ch)].sneak) {
      LearnFromMistake(ch, SKILL_SNEAK, 1, 90);
      WAIT_STATE(ch, PULSE_VIOLENCE);
      return;
    }
 
  af.type = SKILL_SNEAK;
  af.duration = GET_LEVEL(ch, BestThiefClass(ch));
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SNEAK;
  affect_to_char(ch, &af);
  WAIT_STATE(ch, PULSE_VIOLENCE);
  
}



void do_hide(struct char_data *ch, char *argument, int cmd)
{
  byte percent;
  

dlog("in do_hide");
  
  if (IS_AFFECTED(ch, AFF_HIDE))
    REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

  if (!HasClass(ch, CLASS_THIEF|CLASS_MONK|CLASS_BARBARIAN|CLASS_RANGER)) {
    send_to_char("You're not trained to hide!\n\r", ch);
    return;
  }

  if (!HasClass(ch,CLASS_BARBARIAN|CLASS_RANGER))
     send_to_char("You attempt to hide in the shadows.\n\r", ch); else
     send_to_char("You attempt to camouflage yourself.\n\r",ch);

  if (HasClass(ch, CLASS_BARBARIAN|CLASS_RANGER) && !OUTSIDE(ch)) {
    send_to_char("You must do this outdoors.\n\r",ch);
    return;
   }
  
  if (MOUNTED(ch)) {
    send_to_char("Yeah... right... while mounted\n\r", ch);
    return;
  }
  
   
  percent=number(1,101); /* 101% is a complete failure */

  if (!ch->skills)
    return;
  
  if (percent > ch->skills[SKILL_HIDE].learned +
      dex_app_skill[GET_DEX(ch)].hide) {
    LearnFromMistake(ch, SKILL_HIDE, 1, 90);
    WAIT_STATE(ch, PULSE_VIOLENCE*1);
    return;
  }
  
  SET_BIT(ch->specials.affected_by, AFF_HIDE);
  WAIT_STATE(ch, PULSE_VIOLENCE*1);

}


void do_steal(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  struct obj_data *obj;
  char victim_name[240];
  char obj_name[240];
  char buf[240];
  int percent;
  int gold, eq_pos;
  bool ohoh = FALSE;

dlog("in do_steal");

  if (!ch->skills)
    return;

  if (check_peaceful(ch, "What if he caught you?\n\r"))
    return;
  
  argument = one_argument(argument, obj_name);
  only_argument(argument, victim_name);
  
  if (!HasClass(ch, CLASS_THIEF)) {
    send_to_char("You're no thief!\n\r", ch);
    return;
  }
 
  if (MOUNTED(ch)) {
    send_to_char("Yeah... right... while mounted\n\r", ch);
    return;
  }
  
  if (!(victim = get_char_room_vis(ch, victim_name))) {
    send_to_char("Steal what from who?\n\r", ch);
    return;
  } else if (victim == ch) {
    send_to_char("Come on now, that's rather stupid!\n\r", ch);
    return;
  }

  if(IS_IMMORTAL(victim) && !IS_IMMORTAL(ch)) {
    send_to_char("Steal from a God?!?  Oh the thought!\n\r", ch);
    sprintf(buf, "NOTE: %s tried to steal from GOD %s", GET_NAME(ch), GET_NAME(victim));
    log(buf);
    return;
  }

  WAIT_STATE(ch, PULSE_VIOLENCE*2);  /* they're gonna have to wait. */
  
  if ((GetMaxLevel(ch) < 2) && (!IS_NPC(victim))) {
    send_to_char("Due to misuse of steal, you can't steal from other players\n\r", ch);
    send_to_char("unless you are at least 2nd level. \n\r", ch);
    return;
  }
  
  if ((!victim->desc) && (!IS_NPC(victim)))
    return;
  
  /* 101% is a complete failure */
  percent=number(1,101) - dex_app_skill[GET_DEX(ch)].p_pocket;
  
  if (GET_POS(victim) < POSITION_SLEEPING || GetMaxLevel(ch) >=IMPLEMENTOR) 
    percent = -1; /* ALWAYS SUCCESS */
  
  percent += GET_AVE_LEVEL(victim);
  
  if (GetMaxLevel(victim)>MAX_MORT && GetMaxLevel(ch)<IMPLEMENTOR)
    percent = 101; /* Failure */
  
  if (str_cmp(obj_name, "coins") && str_cmp(obj_name,"gold")) {
    
    if (!(obj = get_obj_in_list_vis(victim, obj_name, victim->carrying))) {
      
      for (eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
	if (victim->equipment[eq_pos] &&
	    (isname(obj_name, victim->equipment[eq_pos]->name)) &&
	    CAN_SEE_OBJ(ch,victim->equipment[eq_pos])) {
	  obj = victim->equipment[eq_pos];
	  break;
	}
      
      if (!obj) {
	act("$E has not got that item.",FALSE,ch,0,victim,TO_CHAR);
	return;
      } else { /* It is equipment */
	if ((GET_POS(victim) > POSITION_STUNNED)) {
	  send_to_char("Steal the equipment now? Impossible!\n\r", ch);
	  return;
	} else {
	  act("You unequip $p and steal it.",FALSE, ch, obj ,0, TO_CHAR);
	  act("$n steals $p from $N.",FALSE,ch,obj,victim,TO_NOTVICT);
	  obj_to_char(unequip_char(victim, eq_pos), ch);
#if NODUPLICATES
	  do_save(ch, "", 0);
	  do_save(victim, "", 0);
#endif
	  if (IS_PC(ch) && IS_PC(victim))
	    GET_ALIGNMENT(ch)-=20;

	}
      }
    } else {  /* obj found in inventory */

      if (IS_OBJ_STAT(obj,ITEM_NODROP)) {
         send_to_char
      	   ("You can't steal it, it must be CURSED!\n\r", ch);
	 return;
      }
      
      percent += GET_OBJ_WEIGHT(obj); /* Make heavy harder */

      if (AWAKE(victim) && (percent > ch->skills[SKILL_STEAL].learned)) {
	ohoh = TRUE;
	act("Yikes, you fumbled!", FALSE, ch,0,0,TO_CHAR);
	LearnFromMistake(ch, SKILL_STEAL, 0, 90);
	SET_BIT(ch->player.user_flags,STOLE_1);
	act("$n tried to steal something from you!",FALSE,ch,0,victim,TO_VICT);
	act("$n tries to steal something from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
      } else { /* Steal the item */
	if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
	  if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)) {
	    obj_from_char(obj);
	    obj_to_char(obj, ch);
	    send_to_char("Got it!\n\r", ch);
#if NODUPLICATES
	    do_save(ch, "", 0);
	    do_save(victim, "", 0);
#endif
	    if (IS_PC(ch) && IS_PC(victim))
	      GET_ALIGNMENT(ch)-=20;
	    
	  } else {
	    send_to_char("You cannot carry that much.\n\r", ch);
	  }
	} else
	  send_to_char("You cannot carry that much.\n\r", ch);
      }
    }
  } else { /* Steal some coins */
    if (AWAKE(victim) && (percent > ch->skills[SKILL_STEAL].learned)) {
      ohoh = TRUE;
      act("Oops..", FALSE, ch,0,0,TO_CHAR);
      if (ch->skills[SKILL_STEAL].learned < 90) {
        act("Even though you were caught, you realize your mistake and promise to remember.",FALSE, ch, 0, 0, TO_CHAR);
        ch->skills[SKILL_STEAL].learned++;
	if (ch->skills[SKILL_STEAL].learned >= 90)
	  send_to_char("You are now learned in this skill!\n\r", ch);
      }
      act("You discover that $n has $s hands in your wallet.",FALSE,ch,0,victim,TO_VICT);
      act("$n tries to steal gold from $N.",TRUE, ch, 0, victim, TO_NOTVICT);
    } else {
      /* Steal some gold coins */
      gold = (int) ((GET_GOLD(victim)*number(1,10))/100);
      gold = MIN(number(1000,2000), gold);
      if (gold > 0) {
	GET_GOLD(ch) += gold;
	GET_GOLD(victim) -= gold;
	sprintf(buf, "Bingo! You got %d gold coins.\n\r", gold);
	send_to_char(buf, ch);
	if (IS_PC(ch) && IS_PC(victim))
	  GET_ALIGNMENT(ch)-=20;
      } else {
	send_to_char("You couldn't get any gold...\n\r", ch);
      }
    }
  }
  
  if (ohoh && IS_NPC(victim) && AWAKE(victim))
    if (IS_SET(victim->specials.act, ACT_NICE_THIEF)) {
      sprintf(buf, "%s is a bloody thief.", GET_NAME(ch));
      do_shout(victim, buf, 0);
      do_say(victim, "Don't you ever do that again!", 0);
    } else {
      if (CAN_SEE(victim, ch))
	hit(victim, ch, TYPE_UNDEFINED);
      else if (number(0,1))
	hit(victim, ch, TYPE_UNDEFINED);
    }
  
}

void do_practice(struct char_data *ch, char *arg, int cmd) 
{
  char buf[MAX_STRING_LENGTH*2], buffer[MAX_STRING_LENGTH*2], temp[20];
  int i;
  extern char *spells[];
  extern struct spell_info_type spell_info[MAX_SPL_LIST];
  
dlog("in do_practice");

  buffer[0] = '\0';
  
  if ((cmd != 164) && (cmd != 170)) return;
  
  if (!ch->skills)
    return;
  
  for (; isspace(*arg); arg++);
  
  if (!arg) 
  {
    send_to_char("You need to supply a class for that.",ch);
    return;
  }
  
  switch(*arg) {
  case 'w':
  case 'W':
  case 'f':
  case 'F': 
    {
      if (!HasClass(ch, CLASS_WARRIOR)) 
      {
	send_to_char("I bet you think you're a warrior.\n\r", ch);
	return;
      }
      send_to_char("You have knowledge of these skills:\n\r", ch);
      for(i=0; *spells[i] != '\n' && i < MAX_SPL_LIST; i++)
	if (!spell_info[i+1].spell_pointer && ch->skills[i+1].learned 
	&& IS_SET(ch->skills[i+1].flags,SKILL_KNOWN) ) 
      {	  sprintf(buf,"%-30s %s",spells[i],
		  how_good(ch->skills[i+1].learned));
  if (IsSpecialized(ch->skills[i+1].special)) strcat(buf," (special)");		  
  strcat(buf," \n\r");
	  if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
	    break;
	  strcat(buffer, buf);
	  strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 1);
      return;
    } break;

  case 't':
  case 'T':
    {
 
      if (!HasClass(ch, CLASS_THIEF)) {
	send_to_char("I bet you think you're a thief.\n\r", ch);
	return;
      }
      send_to_char("You have knowledge of these skills:\n\r", ch);
      for(i=0; *spells[i] != '\n' && i < MAX_SPL_LIST; i++)
	if (!spell_info[i+1].spell_pointer && ch->skills[i+1].learned 
	    && IS_SET(ch->skills[i+1].flags,SKILL_KNOWN) )  {
	  sprintf(buf,"%-30s %s",spells[i],
		  how_good(ch->skills[i+1].learned));
  if (IsSpecialized(ch->skills[i+1].special)) strcat(buf," (special)");		  
    strcat(buf," \n\r");
	  if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
	    break;
	  strcat(buffer, buf);
	  strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 1);
      return;
    } break;
  case 'M':
  case 'm':
    {
      if (!HasClass(ch, CLASS_MAGIC_USER)) {
	send_to_char("I bet you think you're a magic-user.\n\r", ch);
	return;
      }
      send_to_char("Your spellbook holds these spells:\n\r", ch);
      for(i=0; *spells[i] != '\n'; i++)
	if (spell_info[i+1].spell_pointer &&
	    (spell_info[i+1].min_level_magic<=GET_LEVEL(ch,MAGE_LEVEL_IND))
             && IS_SET(ch->skills[i+1].flags,SKILL_KNOWN) ) {
	  sprintf(buf,"[%d] %s %s",
		  spell_info[i+1].min_level_magic,
		  spells[i],how_good(ch->skills[i+1].learned));
  if (IsSpecialized(ch->skills[i+1].special)) strcat(buf," (special)");
    strcat(buf," \n\r");
	  if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
	    break;
	  strcat(buffer, buf);
	  strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 1);
      return;			  
    } 
    break;

  case 'S':
  case 's':
    {
      if (!HasClass(ch, CLASS_SORCERER)) {
	send_to_char("I bet you think you're a sorcerer.\n\r", ch);
	return;
      }
      sprintf(buf,
"You can memorize one spell %d times, with a total of %d spells memorized.\n\r"
,MaxCanMemorize(ch,0),TotalMaxCanMem(ch));
      send_to_char(buf,ch);
      sprintf(buf,
        "You currently have %d spells memorized.\n\r",TotalMemorized(ch));
        send_to_char(buf,ch);
      send_to_char("Your spellbook holds these spells:\n\r", ch);
      for(i=0; *spells[i] != '\n'; i++)
	if (spell_info[i+1].spell_pointer &&
	    (spell_info[i+1].min_level_sorcerer<=GET_LEVEL(ch,SORCERER_LEVEL_IND))
             && IS_SET(ch->skills[i+1].flags,SKILL_KNOWN) &&
                IS_SET(ch->skills[i+1].flags,SKILL_KNOWN_SORCERER) ) {
	  sprintf(buf,"[%d] %s %s",
		  spell_info[i+1].min_level_sorcerer,
		  spells[i],how_good(ch->skills[i+1].learned));
  if (IsSpecialized(ch->skills[i+1].special)) strcat(buf," (special)");
  if (MEMORIZED(ch,i+1)) {
       sprintf(temp," x%d",ch->skills[i+1].nummem);
       strcat(buf,temp);
      }
    strcat(buf," \n\r");
	  if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
	    break;
	  strcat(buffer, buf);
	  strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 1);
      return;			  
    } 
    break;
    
  case 'C':
  case 'c':
    {
      if (!HasClass(ch, CLASS_CLERIC)) {
	send_to_char("I bet you think you're a cleric.\n\r", ch);
	return;
      }
      send_to_char("You can attempt any of these spells:\n\r", ch);
      for(i=0; *spells[i] != '\n'; i++)
	if (spell_info[i+1].spell_pointer &&
	   (spell_info[i+1].min_level_cleric<=GET_LEVEL(ch,CLERIC_LEVEL_IND))
             && IS_SET(ch->skills[i+1].flags,SKILL_KNOWN) ) {	   
	  sprintf(buf,"[%d] %s %s",
		  spell_info[i+1].min_level_cleric,
		  spells[i],how_good(ch->skills[i+1].learned));
  if (IsSpecialized(ch->skills[i+1].special)) strcat(buf," (special)");		  
  if (MEMORIZED(ch,i+1)) strcat(buf," (memorized)");  
   strcat(buf," \n\r");
	  if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
	    break;
	  strcat(buffer, buf);
	  strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 1);
      return;
    } 
    break;

  case 'D':
  case 'd':
    {
      if (!HasClass(ch, CLASS_DRUID)) {
	send_to_char("I bet you think you're a druid.\n\r", ch);
	return;
      }
      send_to_char("You can attempt any of these spells:\n\r", ch);
      for(i=0; *spells[i] != '\n'; i++)
	if (spell_info[i+1].spell_pointer &&
	   (spell_info[i+1].min_level_druid<=GET_LEVEL(ch, DRUID_LEVEL_IND))
             && IS_SET(ch->skills[i+1].flags,SKILL_KNOWN) ) {	   
	  sprintf(buf,"[%d] %s %s",
		  spell_info[i+1].min_level_druid,
		  spells[i],how_good(ch->skills[i+1].learned));
  if (IsSpecialized(ch->skills[i+1].special)) strcat(buf," (special)");		  
	strcat(buf," \n\r"  );
	  if (strlen(buf)+strlen(buffer) > MAX_STRING_LENGTH-2)
	    break;
	  strcat(buffer, buf);
	  strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 1);
      return;
    } 
    break;
    
  case 'K':
  case 'k': {
      if (!HasClass(ch, CLASS_MONK)) {
	send_to_char("I bet you think you're a monk.\n\r", ch);
	return;
      }
    
      send_to_char("You have knowledge of these skills:\n\r", ch);
      for(i=0; *spells[i] != '\n' && i < MAX_SPL_LIST; i++)
	if (!spell_info[i+1].spell_pointer && ch->skills[i+1].learned 
	    && IS_SET(ch->skills[i+1].flags,SKILL_KNOWN) )  {
	  sprintf(buf,"%-30s %s",spells[i],
		  how_good(ch->skills[i+1].learned));
  if (IsSpecialized(ch->skills[i+1].special)) strcat(buf," (special)");		  
    strcat(buf," \n\r");
	  if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
	    break;
	  strcat(buffer, buf);
	  strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 1);
      return;
    }
    break;

  case 'b':
  case 'B':
    {
      if (!HasClass(ch, CLASS_BARBARIAN)) {
	send_to_char("I bet you think you're a Barbarian.\n\r", ch);
	return;
      }
      send_to_char("You know the following skills:\n\r", ch);
      for(i=0; *spells[i] != '\n' && i < MAX_SPL_LIST; i++)
	if (!spell_info[i+1].spell_pointer && ch->skills[i+1].learned 
	    && IS_SET(ch->skills[i+1].flags,SKILL_KNOWN) )  {

  sprintf(buf,"%-30s %s",spells[i],how_good(ch->skills[i+1].learned));
  if (IsSpecialized(ch->skills[i+1].special)) strcat(buf," (special)");		  
      strcat(buf," \n\r");
	  if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
	    break;
	  strcat(buffer, buf);
	  strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 1);
      return;
    }
    break;      

  case 'R':
  case 'r': {
      if (!HasClass(ch, CLASS_RANGER)) {
	send_to_char("I bet you think you're a ranger.\n\r", ch);
	return;
      }
    
      send_to_char("You have knowledge of these skills:\n\r", ch);
      for(i=0; *spells[i] != '\n' && i < MAX_SPL_LIST; i++)
	if (ch->skills[i+1].learned 
	    && IS_SET(ch->skills[i+1].flags,SKILL_KNOWN) )  {

	  sprintf(buf,"[%d] %-30s %s",
	  spell_info[i+1].min_level_ranger,	  
	  spells[i],how_good(ch->skills[i+1].learned));
  if (IsSpecialized(ch->skills[i+1].special)) strcat(buf," (special)");		  
    strcat(buf," \n\r");
	  if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
	    break;
	  strcat(buffer, buf);
	  strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 1);
      return;
    }
    break;

  case 'i':
  case 'I': {
      if (!HasClass(ch, CLASS_PSI)) {
	send_to_char("I bet you think you're a psionist.\n\r", ch);
	return;
      }
    
      send_to_char("You have knowledge of these skills:\n\r", ch);
      for(i=0; *spells[i] != '\n' && i < MAX_SPL_LIST; i++)
	if (ch->skills[i+1].learned 
	    && IS_SET(ch->skills[i+1].flags,SKILL_KNOWN) )  {
	  sprintf(buf,"[%d] %-30s %s",
	  spell_info[i+1].min_level_psi,	  
	spells[i], how_good(ch->skills[i+1].learned));
  if (IsSpecialized(ch->skills[i+1].special)) strcat(buf," (special)");		  
    strcat(buf," \n\r");
	  if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
	    break;
	  strcat(buffer, buf);
	  strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 1);
      return;
    }
    break;

  case 'P':
  case 'p': {
      if (!HasClass(ch, CLASS_PALADIN)) {
	send_to_char("I bet you think you're a paladin.\n\r", ch);
	return;
      }
    
      send_to_char("You have knowledge of these skills:\n\r", ch);
      for(i=0; *spells[i] != '\n' && i < MAX_SPL_LIST; i++)
	if (ch->skills[i+1].learned 
	    && IS_SET(ch->skills[i+1].flags,SKILL_KNOWN) )  {
	  sprintf(buf,"[%d] %-30s %s",
	  spell_info[i+1].min_level_paladin,	  
	  spells[i],		  how_good(ch->skills[i+1].learned));
  if (IsSpecialized(ch->skills[i+1].special)) strcat(buf," (special)");		  
    strcat(buf," \n\r");
	  if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
	    break;
	  strcat(buffer, buf);
	  strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 1);
      return;
    }
    break;

  default:
    send_to_char("Which class???\n\r", ch);
  }

  send_to_char("Go to your guildmaster to see the spells you don't have.\n\r", ch);
  
}







void do_idea(struct char_data *ch, char *argument, int cmd)
{
	FILE *fl;
	char str[MAX_INPUT_LENGTH+20];

dlog("in do_idea");

	if (IS_NPC(ch))	{
		send_to_char("Monsters can't have ideas - Go away.\n\r", ch);
		return;
	}

	/* skip whites */
	for (; isspace(*argument); argument++);

	if (!*argument)	{
	      send_to_char
		("That doesn't sound like a good idea to me.. Sorry.\n\r",ch);
		return;
	}
	if (!(fl = fopen(IDEA_FILE, "a")))	{
		perror ("do_idea");
		send_to_char("Could not open the idea-file.\n\r", ch);
		return;
	}

	sprintf(str, "**%s: %s\n", GET_NAME(ch), argument);

	fputs(str, fl);
	fclose(fl);
	send_to_char("Ok. Thanks.\n\r", ch);
}







void do_typo(struct char_data *ch, char *argument, int cmd)
{
	FILE *fl;
	char str[MAX_INPUT_LENGTH+20];

dlog("in do_typo");

	if (IS_NPC(ch))	{
		send_to_char("Monsters can't spell - leave me alone.\n\r", ch);
		return;
	}

	/* skip whites */
	for (; isspace(*argument); argument++);

	if (!*argument)	{
		send_to_char("I beg your pardon?\n\r", 	ch);
		return;
	}
	if (!(fl = fopen(TYPO_FILE, "a")))	{
		perror ("do_typo");
		send_to_char("Could not open the typo-file.\n\r", ch);
		return;
	}

	sprintf(str, "**%s[%d]: %s\n",
		GET_NAME(ch), ch->in_room, argument);
	fputs(str, fl);
	fclose(fl);
	send_to_char("Ok. thanks.\n\r", ch);

}





void do_bug(struct char_data *ch, char *argument, int cmd)
{
	FILE *fl;
	char str[MAX_INPUT_LENGTH+20];

dlog("in do_bug");

	if (IS_NPC(ch))	{
		send_to_char("You are a monster! Bug off!\n\r", ch);
		return;
	}

	/* skip whites */
	for (; isspace(*argument); argument++);

	if (!*argument)	{
		send_to_char("Pardon?\n\r",ch);
		return;
	}
	if (!(fl = fopen(BUG_FILE, "a")))	{
		perror ("do_bug");
		send_to_char("Could not open the bug-file.\n\r", ch);
		return;
	}

	sprintf(str, "**%s[%d]: %s\n",
		GET_NAME(ch), ch->in_room, argument);
	fputs(str, fl);
	fclose(fl);
	send_to_char("Ok.\n\r", ch);
      }



void do_brief(struct char_data *ch, char *argument, int cmd)
{

dlog("in do_breif");

  if (IS_NPC(ch))
    return;
  
  if (IS_SET(ch->specials.act, PLR_BRIEF))	{
    send_to_char("Brief mode off.\n\r", ch);
    REMOVE_BIT(ch->specials.act, PLR_BRIEF);
  }	else	{
    send_to_char("Brief mode on.\n\r", ch);
    SET_BIT(ch->specials.act, PLR_BRIEF);
  }
}


void do_compact(struct char_data *ch, char *argument, int cmd)
{

dlog("in do_compact");

  if (IS_NPC(ch))
    return;
  
  if (IS_SET(ch->specials.act, PLR_COMPACT))	{
    send_to_char("You are now in the uncompacted mode.\n\r", ch);
    REMOVE_BIT(ch->specials.act, PLR_COMPACT);
  }	else	{
    send_to_char("You are now in compact mode.\n\r", ch);
    SET_BIT(ch->specials.act, PLR_COMPACT);
  }
}


char *Condition(struct char_data *ch)
{
  float a, b, t;
  int   c;
  static char buf[100];
  static char *p;
  
  a = (float)GET_HIT(ch);
  b = (float)GET_MAX_HIT(ch);
  
  t = a / b;
  c = (int)100.0*t;
  
  strcpy(buf, how_good(c));
  p = buf;
  return(p);
  
}

char *Tiredness(struct char_data *ch)
{
  float a, b, t;
  int   c;
  static char buf[100];
  static char *p;
  
  a = (float)GET_MOVE(ch);
  b = (float)GET_MAX_MOVE(ch);
  
  t = a / b;
  c = (int)100.0*t;
  
  strcpy(buf, how_good(c));
  p = buf;
  return(p);
  
}
void do_group(struct char_data *ch, char *argument, int cmd)
{
  char name[256],buf[256], tmp_buf[1000];
  struct char_data *victim, *k;
  struct follow_type *f;
  bool found;

static char *rand_groupname[] = {
           "The Seekers",             /* 0 */
           "The Subclan of Harpers",
           "The Farwalkers",
           "The God Squad",
           "The Vampire Slayers",
           "The Nobody Crew",         /* 5 */
           "The Dragon Hunters"       /* 6 */
           };

#define MAX_GNAME           5

dlog("in do_group");

  only_argument(argument, name);
  
  if (!*name) {
    if (!IS_AFFECTED(ch, AFF_GROUP)) {
      send_to_char("But you are a member of no group?!\n\r", ch);
    } else {
      if(ch->specials.group_name) 
        sprintf(buf,"$c0015Your group \"%s\" consists of:", ch->specials.group_name);
      else if(ch->master && ch->master->specials.group_name)
        sprintf(buf,"$c0015Your group \"%s\" consists of:", ch->master->specials.group_name);
      else
        sprintf(buf,"$c0015Your group consists of:");
      act(buf,FALSE,ch,0,0,TO_CHAR);
      if (ch->master)
	k = ch->master;
      else
	k = ch;

      if (IS_AFFECTED(k, AFF_GROUP) 
      && GET_MAX_HIT(k) >0
      && GET_MAX_MANA(k) >0
      && GET_MAX_MOVE(k) >0
      ) {
	sprintf(buf, "$c0014    %-15s $c0011(Head of group) $c0006HP:%2.0f%% MANA:%2.0f%% MV:%2.0f%%",
	          fname(k->player.name),
                  ((float)GET_HIT(k) / (int)GET_MAX_HIT(k)) * 100.0+0.5,
                  ((float)GET_MANA(k)/ (int)GET_MAX_MANA(k)) * 100.0+0.5,
                  ((float)GET_MOVE(k)/ (int)GET_MAX_MOVE(k)) * 100.0+0.5);
	act(buf,FALSE,ch, 0, k, TO_CHAR);
	
      }

      for(f=k->followers; f; f=f->next)
	if (IS_AFFECTED(f->follower, AFF_GROUP)
         && GET_MAX_HIT(f->follower) >0
         && GET_MAX_MANA(f->follower) >0
         && GET_MAX_MOVE(f->follower) >0
	) {
	  sprintf(buf, "$c0014    %-15s             $c0011%s $c0006HP:%2.0f%% MANA:%2.0f%% MV:%2.0f%%",
                  fname(f->follower->player.name),
		  (IS_AFFECTED2(f->follower,AFF2_CON_ORDER))?"(O)":"   ",
                  ((float)GET_HIT(f->follower) /(int)GET_MAX_HIT(f->follower)) *100.0+0.5,
                  ((float)GET_MANA(f->follower)/(int)GET_MAX_MANA(f->follower)) * 100.0+0.5,
                  ((float)GET_MOVE(f->follower)/(int)GET_MAX_MOVE(f->follower)) * 100.0+0.5);
	  act(buf,FALSE,ch, 0, f->follower, TO_CHAR);
	}

	  
    }
    
    return;
  }
  
  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("No one here by that name.\n\r", ch);
  } else {
    
    if (ch->master) {
      act("You can not enroll group members without being head of a group.",
	  FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
    
    found = FALSE;
    
    if (victim == ch)
      found = TRUE;
    else {
      for(f=ch->followers; f; f=f->next) {
	if (f->follower == victim) {
	  found = TRUE;
	  break;
	}
      }
    }
    
    if (found) {
      if (IS_AFFECTED(victim, AFF_GROUP)) {
	act("$n has been kicked out of $N's group!", FALSE, victim, 0, ch, TO_ROOM);
	act("You are no longer a member of $N's group!", FALSE, victim, 0, ch, TO_CHAR);
	REMOVE_BIT(victim->specials.affected_by, AFF_GROUP);
	REMOVE_BIT(victim->specials.affected_by2, AFF2_CON_ORDER);
      } else {
	if (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch)) {
	  act("You really don't want $N in your group.", FALSE, ch, 0, victim, TO_CHAR);
	  return;
	}
	if (IS_IMMORTAL(ch) && !IS_IMMORTAL(victim)) {
	  act("Now now.  That would be CHEATING!",FALSE,ch,0,0,TO_CHAR);
	  return;  
	  
	}
	act("$n is now a member of $N's group.", 
	    FALSE, victim, 0, ch, TO_ROOM);
	act("You are now a member of $N's group.", 
	    FALSE, victim, 0, ch, TO_CHAR);
	SET_BIT(victim->specials.affected_by, AFF_GROUP);
  	REMOVE_BIT(victim->specials.affected_by2, AFF2_CON_ORDER);

        /* set group name if not one */
      if (!ch->master && !ch->specials.group_name && ch->followers) {
           char gnum=number(0,MAX_GNAME);
           ch->specials.group_name=strdup(rand_groupname[gnum]);
           sprintf(buf,"You form <%s> adventuring group!",rand_groupname[gnum]);
           act(buf,FALSE,ch,0,0,TO_CHAR);
          }

      }
    } else {
      act("$N must follow you, to enter the group", 
	  FALSE, ch, 0, victim, TO_CHAR);
    }
  }
}

void do_group_name(struct char_data *ch, char *arg, int cmd)
{
  int count;
  struct follow_type *f;

dlog("in do_group_name");

  /* check to see if this person is the master */
  if (ch->master || !IS_AFFECTED(ch, AFF_GROUP)) {
    send_to_char("You aren't the master of a group.\n\r", ch);
    return;
  }
  /* check to see at least 2 pcs in group      */
  for(count=0,f=ch->followers;f;f=f->next) {
    if (IS_AFFECTED(f->follower, AFF_GROUP) && IS_PC(f->follower)) {
      count++;
    }
  }
  if (count < 1) {
    send_to_char("You can't have a group with just one player!\n\r", ch);
    return;
  }
  /* free the old ch->specials.group_name           */
  if (ch->specials.group_name) {
	  free(ch->specials.group_name);
	  ch->specials.group_name=0;
  }
  /* set ch->specials.group_name to the argument    */
  for (;*arg==' ';arg++);
  send_to_char("\n\rSetting your group name to :", ch);send_to_char(arg, ch);
  send_to_char("\n\r",ch);
  ch->specials.group_name =strdup(arg);
  
}

void do_quaff(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct obj_data *temp;
  int i;
  bool equipped;
  
  equipped = FALSE;

dlog("in do_quaff");

  only_argument(argument,buf);
  
  if (!(temp = get_obj_in_list_vis(ch,buf,ch->carrying))) {
    temp = ch->equipment[HOLD];
    equipped = TRUE;
    if ((temp==0) || !isname(buf, temp->name)) {
      act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
      return;
    }
  }
  
  if (!IS_IMMORTAL(ch)) {
    if (GET_COND(ch,FULL)>23) {
      act("Your stomach can't contain anymore!",FALSE,ch,0,0,TO_CHAR);
      return;
    } else {
      GET_COND(ch, FULL)+=1;
    }
  }
  
  if (temp->obj_flags.type_flag!=ITEM_POTION) {
    act("You can only quaff potions.",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  
  act("$n quaffs $p.", TRUE, ch, temp, 0, TO_ROOM);
  act("You quaff $p which dissolves.",FALSE, ch, temp,0, TO_CHAR);
  
  /*  my stuff */
  if (ch->specials.fighting) {
    if (equipped) {
      if (number(1,20) > ch->abilities.dex) {
	act("$n is jolted and drops $p!  It shatters!", 
	    TRUE, ch, temp, 0, TO_ROOM);
	act("You arm is jolted and $p flies from your hand, *SMASH*",
	    TRUE, ch, temp, 0, TO_CHAR);
	if (equipped)
	  temp = unequip_char(ch, HOLD);
	extract_obj(temp);
	return;
      }
    } else {
      if (number(1,20) > ch->abilities.dex - 4) {
	act("$n is jolted and drops $p!  It shatters!", 
	    TRUE, ch, temp, 0, TO_ROOM);
	act("You arm is jolted and $p flies from your hand, *SMASH*",
	    TRUE, ch, temp, 0, TO_CHAR);
	extract_obj(temp);
	return;
      }
    }
  }
  
  for (i=1; i<4; i++)
    if (temp->obj_flags.value[i] >= 1)
      ((*spell_info[temp->obj_flags.value[i]].spell_pointer)
       ((byte) temp->obj_flags.value[0], ch, "", SPELL_TYPE_POTION, ch, temp));
  
  if (equipped)
    temp = unequip_char(ch, HOLD);
  
  extract_obj(temp);
  
  WAIT_STATE(ch, PULSE_VIOLENCE);
  
}


void do_recite(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct obj_data *scroll, *obj;
  struct char_data *victim;
  int i, bits;
  bool equipped;
  
  equipped = FALSE;
  obj = 0;
  victim = 0;

dlog("in do_recite");

  if (!ch->skills)
    return;
  
  argument = one_argument(argument,buf);
  
  if (!(scroll = get_obj_in_list_vis(ch,buf,ch->carrying))) {
    scroll = ch->equipment[HOLD];
    equipped = TRUE;
    if ((scroll==0) || !isname(buf, scroll->name)) {
      act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
      return;
    }
  }
  
  if (scroll->obj_flags.type_flag!=ITEM_SCROLL)  {
    act("Recite is normally used for scrolls.",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  
  if (*argument) {
    bits = generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM |
			FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &victim, &obj);
    if (bits == 0) {
      send_to_char("No such thing around to recite the scroll on.\n\r", ch);
      return;
    }
  } else {
    victim = ch;
  }
  
  if (!HasClass(ch, CLASS_MAGIC_USER) && 
      !HasClass(ch, CLASS_CLERIC) && 
      !HasClass(ch, CLASS_SORCERER)) {
 if (number(1,95) > ch->skills[SKILL_READ_MAGIC].learned || 
      ch->skills[SKILL_READ_MAGIC].learned == 0) {
      	WAIT_STATE(ch, PULSE_VIOLENCE*3);
	send_to_char(
"After several seconds of study, your head hurts trying to understand.\n\r",ch);
	return;
      }
  }
  
  act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
  act("You recite $p which bursts into flame.",FALSE,ch,scroll,0,TO_CHAR);
  
  for (i=1; i<4; i++) {
    if (scroll->obj_flags.value[0] > 0) {  /* spells for casting */
      if (scroll->obj_flags.value[i] >= 1) 
      {
	if (IS_SET(spell_info[scroll->obj_flags.value[i]].targets, 
		   TAR_VIOLENT) && check_peaceful(ch, 
		   "Impolite magic is banned here."))
	  continue;

	if (check_nomagic(ch,"The magic is blocked by unknown forces.\n\r", 
			  "The magic dissolves powerlessly"))
	  continue;
		
	((*spell_info[scroll->obj_flags.value[i]].spell_pointer)
	 ((byte) scroll->obj_flags.value[0], ch, "", SPELL_TYPE_SCROLL, victim, obj));
      }
    } else {
      /* this is a learning scroll */
      if (scroll->obj_flags.value[0] < -30)  /* max learning is 30% */
	scroll->obj_flags.value[0] = -30;

      if (scroll->obj_flags.value[i] > 0) {  /* positive learning */
	if (ch->skills) {
	  if (ch->skills[scroll->obj_flags.value[i]].learned < 45)
	    ch->skills[scroll->obj_flags.value[i]].learned +=
	      (-scroll->obj_flags.value[0]);
	}
      } else {  /* negative learning (cursed */
	if (scroll->obj_flags.value[i] < 0) {  /* 0 = blank */
	  if (ch->skills) {
	    if (ch->skills[-scroll->obj_flags.value[i]].learned > 0)
	      ch->skills[-scroll->obj_flags.value[i]].learned +=
		scroll->obj_flags.value[0];
	    ch->skills[-scroll->obj_flags.value[i]].learned =
	      MAX(0, ch->skills[scroll->obj_flags.value[i]].learned);
	  }
	}
      }
    }
  }
  if (equipped)
    scroll = unequip_char(ch, HOLD);
    
  extract_obj(scroll);

}



void do_use(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct char_data *tmp_char;
  struct obj_data *tmp_object, *stick;
  
  int bits;

dlog("in do_use");

  argument = one_argument(argument,buf);
  
  if (ch->equipment[HOLD] == 0 ||
      !isname(buf, ch->equipment[HOLD]->name)) {
    act("You do not hold that item in your hand.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

#if 0
  if (!IS_PC(ch) && ch->master) {
    act("$n looks confused, and shrugs helplessly", FALSE, ch, 0, 0, TO_ROOM);
    return;
  }
#endif  

  if (RIDDEN(ch)) {
    return;
  }

  stick = ch->equipment[HOLD];
  
  if (stick->obj_flags.type_flag == ITEM_STAFF)  {
    act("$n taps $p three times on the ground.",TRUE, ch, stick, 0,TO_ROOM);
    act("You tap $p three times on the ground.",FALSE,ch, stick, 0,TO_CHAR);
    if (stick->obj_flags.value[2] > 0) {  /* Is there any charges left? */
      stick->obj_flags.value[2]--;

      if (check_nomagic(ch,"The magic is blocked by unknown forces.", 
			"The magic is blocked by unknown forces."))
	return;

      ((*spell_info[stick->obj_flags.value[3]].spell_pointer)
       ((byte) stick->obj_flags.value[0], ch, "", SPELL_TYPE_STAFF, 0, 0));
      WAIT_STATE(ch, PULSE_VIOLENCE);
    } else {
      send_to_char("The staff seems powerless.\n\r", ch);
    }
  } else if (stick->obj_flags.type_flag == ITEM_WAND) {
    bits = generic_find(argument, FIND_CHAR_ROOM | FIND_OBJ_INV | 
			FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
    if (bits) {
      struct spell_info_type	*spellp;

      spellp = spell_info + (stick->obj_flags.value[3]);

      if (bits == FIND_CHAR_ROOM) {
	act("$n point $p at $N.", TRUE, ch, stick, tmp_char, TO_ROOM);
	act("You point $p at $N.",FALSE,ch, stick, tmp_char, TO_CHAR);
      } else {
	act("$n point $p at $P.", TRUE, ch, stick, tmp_object, TO_ROOM);
	act("You point $p at $P.",FALSE,ch, stick, tmp_object, TO_CHAR);
      }

      if (IS_SET(spellp->targets, TAR_VIOLENT) &&
	  check_peaceful(ch, "Impolite magic is banned here."))
	return;
      
      if (stick->obj_flags.value[2] > 0) { /* Is there any charges left? */
	stick->obj_flags.value[2]--;

      if (check_nomagic(ch,"The magic is blocked by unknown forces.", 
			"The magic is blocked by unknown forces."))
	return;

	((*spellp->spell_pointer)
	 ((byte) stick->obj_flags.value[0], ch, "", SPELL_TYPE_WAND, 
	  tmp_char, tmp_object));
	WAIT_STATE(ch, PULSE_VIOLENCE);
      } else {
	send_to_char("The wand seems powerless.\n\r", ch);
      }
    } else {
      send_to_char("What should the wand be pointed at?\n\r", ch);
    }
  } else {
    send_to_char("Use is normally only for wand's and staff's.\n\r", ch);
  }
}

void do_plr_noshout(struct char_data *ch, char *argument, int cmd)
{
  char buf[128];

dlog("in do_plr_noshout");

  if (IS_NPC(ch))
    return;
  
  only_argument(argument, buf);
  
  if (!*buf) {
    if (IS_SET(ch->specials.act, PLR_DEAF)) {
      send_to_char("You can now hear shouts again.\n\r", ch);
      REMOVE_BIT(ch->specials.act, PLR_DEAF);
    } else {
      send_to_char("From now on, you won't hear shouts.\n\r", ch);
      SET_BIT(ch->specials.act, PLR_DEAF);
    }
  } else {
    send_to_char("Only the gods can shut up someone else. \n\r",ch);
  }
  
}

void do_plr_nogossip(struct char_data *ch, char *argument, int cmd)
{
  char buf[128];

dlog("in do_plr_nogossip");

  if (IS_NPC(ch))
    return;
  
  only_argument(argument, buf);
  
  if (!*buf) {
    if (IS_SET(ch->specials.act, PLR_NOGOSSIP)) {
      send_to_char("You can now hear gossips again.\n\r", ch);
      REMOVE_BIT(ch->specials.act, PLR_NOGOSSIP);
    } else {
      send_to_char("From now on, you won't hear gossips.\n\r", ch);
      SET_BIT(ch->specials.act, PLR_NOGOSSIP);
    }
  } else {
    send_to_char("Only the gods can no gossip someone else. \n\r",ch);
  }
  
}

void do_plr_noauction(struct char_data *ch, char *argument, int cmd)
{
  char buf[128];

dlog("in do_plr_noauction");

  if (IS_NPC(ch))
    return;
  
  only_argument(argument, buf);
  
  if (!*buf) {
    if (IS_SET(ch->specials.act, PLR_NOAUCTION)) {
      send_to_char("You can now hear auctions again.\n\r", ch);
      REMOVE_BIT(ch->specials.act, PLR_NOAUCTION);
    } else {
      send_to_char("From now on, you won't hear auctions.\n\r", ch);
      SET_BIT(ch->specials.act, PLR_NOAUCTION);
    }
  } else {
    send_to_char("Only the gods can no auction someone else. \n\r",ch);
  }
  
}

void do_plr_notell(struct char_data *ch, char *argument, int cmd)
{
  char buf[128];

dlog("in do_plr_notell");

  if (IS_NPC(ch))
    return;
  
  only_argument(argument, buf);
  
  if (!*buf) {
    if (IS_SET(ch->specials.act, PLR_NOTELL)) {
      send_to_char("You can now hear tells again.\n\r", ch);
      REMOVE_BIT(ch->specials.act, PLR_NOTELL);
    } else {
      send_to_char("From now on, you won't hear tells.\n\r", ch);
      SET_BIT(ch->specials.act, PLR_NOTELL);
    }
  } else {
    send_to_char("Only the gods can notell up someone else. \n\r",ch);
  }
  
}


void do_alias(struct char_data *ch, char *arg, int cmd)
{
  char buf[512], buf2[512];
  char *p, *p2;
  int i, num;

dlog("in do_alias");

  if (cmd == 260) {
    for (;*arg==' ';arg++);
    if (!*arg) {  /* print list of current aliases */
      if (ch->specials.A_list) {
	for(i=0;i<10;i++) {
	  if (ch->specials.A_list->com[i]) {
	    sprintf(buf,"[%d] %s\n\r",i, ch->specials.A_list->com[i]);
	    send_to_char(buf,ch);
	  }
	}
      } else {
	send_to_char("You have no aliases defined!\n\r", ch);
	return;
      }
    } else {  /* assign a particular alias */
      if (!ch->specials.A_list) {
	ch->specials.A_list = (Alias *)malloc(sizeof(Alias));
	for (i=0;i<10;i++)
	  ch->specials.A_list->com[i] = (char *)0;
      }
      strcpy(buf, arg);
      p = strtok(buf," ");
      num = atoi(p);
      if (num < 0 || num > 9) {
	send_to_char("numbers between 0 and 9, please \n\r", ch);
	return;
      }
      if (GET_ALIAS(ch,num)) {
	free(GET_ALIAS(ch, num));
	GET_ALIAS(ch, num) = 0;
      }
/*
  testing
*/
      p = strtok(0," ");  /* get the command string */
      if (!p) {
	send_to_char("Need to supply a command to alias bu--------dee\n\r",ch);
	return;
      }
      p2 = strtok(p," ");  /* check the command, make sure its not an alias
			      */
      if (!p2) {
	send_to_char("Hmmmmm\n\r", ch);
	return;
      }
      if (*p2 >= '0' && *p2 <= '9') {
	send_to_char("Can't use an alias inside an alias\n\r", ch);
	return;
      }
      if (strncmp(p2,"alias",strlen(p2))==0) {
	send_to_char("Can't use the word 'alias' in an alias\n\r", ch);
	return;
      }
/*
   verified.. now the copy.
*/
      if (strlen(p) <= 80) {
        strcpy(buf2, arg);  /* have to rebuild, because buf is full of
			       nulls */
	p = strchr(buf2,' ');
	p++;
	ch->specials.A_list->com[num] = (char *)malloc(strlen(p)+1);
	strcpy(ch->specials.A_list->com[num], p);
      } else {
	send_to_char("alias must be less than 80 chars, lost\n\r", ch);
	return;
      }
    }
  } else {   /* execute this alias */
    num = cmd - 260;  /* 260 = alias */
    if (num == 10)
      num = 0;
    if (ch->specials.A_list) {
      if (GET_ALIAS(ch, num)) {
	strcpy(buf, GET_ALIAS(ch, num));
	if (*arg) {
	  sprintf(buf2,"%s%s",buf,arg);
	  command_interpreter(ch, buf2);
	} else {
	  command_interpreter(ch, buf);
	}
      }
    }
  }
}

int Dismount(struct char_data *ch, struct char_data *h, int pos)
{

  MOUNTED(ch) = 0;
  RIDDEN(h) = 0;
  GET_POS(ch) = pos;

  check_falling(ch);

}

void do_mount(struct char_data *ch, char *arg, int cmd)
{
  char buf[256];
  char name[112];
  int check;
  struct char_data *horse;


dlog("in do_mount");

  if (cmd == 276 || cmd == 278) {
    only_argument(arg, name);
    
    if (!(horse = get_char_room_vis(ch, name))) {
      send_to_char("Mount what?\n\r", ch);
      return;
    }

    if (!IsHumanoid(ch)) {
      send_to_char("You can't ride things!\n\r", ch);
      return;
    }
    
    if (IsRideable(horse)) {

      if (GET_POS(horse) < POSITION_STANDING) {
	send_to_char("Your mount must be standing\n\r", ch);
	return;
      }
      
      if (RIDDEN(horse)) {
	send_to_char("Already ridden\n\r", ch);
	return;
      } else if (MOUNTED(ch)) {
	send_to_char("Already riding\n\r", ch);
	return;
      }

      check = MountEgoCheck(ch, horse);
      if (check > 5) {
	act("$N snarls and attacks!", 
	    FALSE, ch, 0, horse, TO_CHAR);
	act("as $n tries to mount $N, $N attacks $n!",
	    FALSE, ch, 0, horse, TO_NOTVICT);
	WAIT_STATE(ch, PULSE_VIOLENCE);
	hit(horse, ch, TYPE_UNDEFINED);
	return;
      } else if (check > -1) {
	act("$N moves out of the way, you fall on your butt", 
	    FALSE, ch, 0, horse, TO_CHAR);
	act("as $n tries to mount $N, $N moves out of the way",
	    FALSE, ch, 0, horse, TO_NOTVICT);
	WAIT_STATE(ch, PULSE_VIOLENCE);
	GET_POS(ch) = POSITION_SITTING;
	return;
      }


      if (RideCheck(ch, 50)) {
	act("You hop on $N's back", FALSE, ch, 0, horse, TO_CHAR);
	act("$n hops on $N's back", FALSE, ch, 0, horse, TO_NOTVICT);
	act("$n hops on your back!", FALSE, ch, 0, horse, TO_VICT);
	MOUNTED(ch) = horse;
	RIDDEN(horse) = ch;
	GET_POS(ch) = POSITION_MOUNTED;
	REMOVE_BIT(ch->specials.affected_by, AFF_SNEAK);
      } else {
	act("You try to ride $N, but falls on $s butt", 
	    FALSE, ch, 0, horse, TO_CHAR);
	act("$n tries to ride $N, but falls on $s butt", 
	    FALSE, ch, 0, horse, TO_NOTVICT);
	act("$n tries to ride you, but falls on $s butt", 
	    FALSE, ch, 0, horse, TO_VICT);
	GET_POS(ch) = POSITION_SITTING;
	WAIT_STATE(ch, PULSE_VIOLENCE*2);
      }
    } else {
      send_to_char("You can't ride that!\n\r", ch);
      return;
    }
  } else if (cmd == 277) {
    horse = MOUNTED(ch);

    act("You dismount from $N", FALSE, ch, 0, horse, TO_CHAR);
    act("$n dismounts from $N", FALSE, ch, 0, horse, TO_NOTVICT);
    act("$n dismounts from you", FALSE, ch, 0, horse, TO_VICT);
    Dismount(ch, MOUNTED(ch), POSITION_STANDING);
    return;
  }

}

void do_memorize(struct char_data *ch, char *argument, int cmd)
{

  int spl,qend;
  float duration;
  struct affected_type af;	

dlog("in do_memorize");

  if (IS_NPC(ch) && (!IS_SET(ch->specials.act, ACT_POLYSELF)))
    return;

  if (!IsHumanoid(ch)) {
    send_to_char("Sorry, you don't have the right form for that.\n\r",ch);
    return;
  }
  
  if (!IS_IMMORTAL(ch)) {
    if (BestMagicClass(ch) == WARRIOR_LEVEL_IND || 
        BestMagicClass(ch) == BARBARIAN_LEVEL_IND) {
      send_to_char("Think you had better stick to fighting...\n\r", ch);
      return;
    } else if (BestMagicClass(ch) == THIEF_LEVEL_IND) {
      send_to_char("Think you should stick to stealing...\n\r", ch);
      return;
    } else if (BestMagicClass(ch) == MONK_LEVEL_IND) {
      send_to_char("Think you should stick to meditating...\n\r", ch);
      return;
    } else if (BestMagicClass(ch) == MAGE_LEVEL_IND) {
      send_to_char("Think you better stick to your way of magic....\n\r",ch);
      return;
    } else if (BestMagicClass(ch) ==DRUID_LEVEL_IND) {
      send_to_char("Think you better stick to your way of magic...\n\r",ch);
     return;
    } else if (BestMagicClass(ch) ==CLERIC_LEVEL_IND) {
      send_to_char("Think you better stick to your way of magic...\n\r",ch);
     return;
    }
  }


if (affected_by_spell(ch,SKILL_MEMORIZE)) {
   send_to_char("You can only learn one spell at a time.\n\r",ch);
   return;
  }
   
  argument = skip_spaces(argument);
  
  /* If there is no chars in argument */
  if (!(*argument)) {
#if 1
    char buf[MAX_STRING_LENGTH*2], buffer[MAX_STRING_LENGTH*2],temp[20];
    int i;
    send_to_char("Memorize 'spell name'\n\rCurrent spells in memory:\n\r", ch);

sprintf(buf,
"You can memorize one spell %d times, with a total of %d spells memorized.\n\r"
,MaxCanMemorize(ch,0),TotalMaxCanMem(ch));
      send_to_char(buf,ch);
      sprintf(buf,
        "You currently have %d spells memorized.\n\r",TotalMemorized(ch));
        send_to_char(buf,ch);
      send_to_char("Your spellbook holds these spells:\n\r", ch);
      for(i=0; *spells[i] != '\n'; i++)
	if (spell_info[i+1].spell_pointer &&
	    (spell_info[i+1].min_level_sorcerer<=GET_LEVEL(ch,SORCERER_LEVEL_IND))
             && IS_SET(ch->skills[i+1].flags,SKILL_KNOWN) &&
                IS_SET(ch->skills[i+1].flags,SKILL_KNOWN_SORCERER) &&
                ch->skills[i+1].nummem>0) {
	  sprintf(buf,"[%d] %s %s",
		  spell_info[i+1].min_level_sorcerer,
		  spells[i],how_good(ch->skills[i+1].learned));
  if (IsSpecialized(ch->skills[i+1].special)) strcat(buf," (special)");
  if (MEMORIZED(ch,i+1)) {
       sprintf(temp," x%d",ch->skills[i+1].nummem);
       strcat(buf,temp);
      }
    strcat(buf," \n\r");
	  if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
	    break;
	  strcat(buffer, buf);
	  strcat(buffer, "\r");
      }
      page_string(ch->desc, buffer, 0);
#endif
      
    return;
  }
  
if (GET_POS(ch) > POSITION_SITTING) {
    send_to_char("You cannot do this standing up.\n\r",ch);
    
    if (affected_by_spell(ch,SKILL_MEMORIZE)) {
          SpellWearOff(SKILL_MEMORIZE,ch);
          affect_from_char(ch,SKILL_MEMORIZE); 
        }
    return;
   }

  if (*argument != '\'') {
    send_to_char("Magic must always be enclosed by the holy magic symbols : '\n\r",ch);
    return;
  }
  
  for (qend=1; *(argument+qend) && (*(argument+qend) != '\'') ; qend++)
    *(argument+qend) = LOWER(*(argument+qend));
  
  if (*(argument+qend) != '\'') {
    send_to_char("Magic must always be enclosed by the holy magic symbols : '\n\r",ch);
    return;
  }
  
  spl = old_search_block(argument, 1, qend-1,spells, 0);
  
  if (!spl) {
  send_to_char(
  "You flip through your spell book but do not find that spell.\n\r",ch);
    return;
  }

  if (!ch->skills)
    return;
  
  if ((spl > 0) && (spl < MAX_SKILLS) && spell_info[spl].spell_pointer) {

  if (!IS_IMMORTAL(ch)) {
    if (spell_info[spl].min_level_sorcerer > GET_LEVEL(ch,SORCERER_LEVEL_IND)) {
 	   send_to_char(
 	    "Sorry, you do not have the skills for that spell.\n\r",ch);
	  return;
	}
      }

		/* Non-Sorcerer spell, cleric/druid or something else */
    if (spell_info[spl].min_level_sorcerer == 0) {
 	   send_to_char("Sorry, you do not have the skills for that spell.\n\r",ch);
           return;
	  }
      
  
  /* made it, lets memorize the spell! */

   if (ch->skills[spl].nummem < 0)  /* should not happen */
       ch->skills[spl].nummem = 0;


  if (spell_info[spl].min_level_magic <= 5) 
      duration =0.1; else
   if (spell_info[spl].min_level_magic <= 10) 
      duration = 0.3; else
  if (spell_info[spl].min_level_magic <= 25) 
      duration =.5; else      
  if (spell_info[spl].min_level_magic <= 45) 
      duration =.7; else      
  if (spell_info[spl].min_level_magic <= 47) 
      duration =1; else      
             duration = 1.5;
      
     af.type = SKILL_MEMORIZE;
     af.duration =duration; 
     af.modifier = spl;		 /**/
     af.location = APPLY_NONE;
     af.bitvector = 0;
     affect_to_char(ch, &af);   

    send_to_char("You flip open your spell book then begin to read and meditate.\n\r",ch);
    act("$n opens a spell book then begins to read and meditate."
        , FALSE, ch, 0, 0, TO_ROOM);
  }

 return;
} /* end memorize */

int TotalMaxCanMem(struct char_data *ch)
{ 
extern struct int_app_type int_app[26];
int i;

if (OnlyClass(ch,CLASS_SORCERER))
   i=GET_LEVEL(ch,SORCERER_LEVEL_IND); else	/* Multis get less spells */
   i=(int)(GET_LEVEL(ch,SORCERER_LEVEL_IND)/HowManyClasses(ch)*0.5); 

   i +=(int)int_app[GET_INT(ch)].learn/2;
   return(i);
}

  		/* total amount of spells memorized */
int TotalMemorized(struct char_data *ch)
{
 int i,ii=0;
 for (i=0;i< MAX_SKILLS;i++) {
  if (ch->skills[i].nummem && IS_SET(ch->skills[i].flags,SKILL_KNOWN_SORCERER))
      ii +=ch->skills[i].nummem;
  }

 return(ii);
}

void check_memorize(struct char_data *ch, struct affected_type *af)
{
  if (af->type == SKILL_MEMORIZE) {

 if (ch->skills[af->modifier].nummem >= MaxCanMemorize(ch,af->modifier)) {
   send_to_char(
   "You cannot memorize this spell anymore than you already have.\n\r",ch);
    return;
   }
   
  if (TotalMemorized(ch) >= TotalMaxCanMem(ch)) {
  send_to_char(
  "You cannot memorize any more spells, your mind cannot contain it!\n\r",ch);
   return;
  }
     ch->skills[af->modifier].nummem +=1;
   }

}

void do_set_afk(struct char_data *ch, char *argument, int cmd)
{

dlog("in do_set_afk");

  if (!ch)
    return;
  if (IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF))
    return;

  act("$c0006$n quietly goes Away From Keyboard.", TRUE, ch, 0, 0, TO_ROOM);
  act("$c0006You quietly go AFK.", TRUE, ch, 0, 0, TO_CHAR);
  SET_BIT(ch->specials.affected_by2, AFF2_AFK);
  if (ch->pc)
  	SET_BIT(ch->pc->comm,COMM_AFK);
}

#define RACE_WAR_MIN_LEVEL 10	/* this is the level a user can turn race war ON */
void do_set_flags(struct char_data *ch, char *argument, int cmd)
{
 char type[255],field[255];

dlog("in do_set_flags");

 if (!ch) 
    return;

 argument = one_argument(argument,type);
 
 if (!*type) {
 	send_to_char("Set, but set what?!?!?\n\r",ch);
 	return;
  }

 argument = one_argument(argument,field);

if (!strcmp("war",type) &&  (!*field)) {
 send_to_char("Use 'set war enable', REMEMBER ONCE THIS IS SET YOU CANNOT REMOVE IT!\n\r",ch);
 send_to_char("Be sure to READ the help on RACE WAR.\n\r",ch);
 return;
}

if (!*field) {
	send_to_char("Set it to what? (Enable,Disable/Off)\n\r",ch);
	return;
  } 	

if (!strcmp(type,"war") && GetMaxLevel(ch)>=RACE_WAR_MIN_LEVEL) {
 if (!strcmp("enable",field)) {
    SET_BIT(ch->player.user_flags,RACE_WAR);
    send_to_char("YOU CAN BE ATTACKED BY YOUR RACIAL ENEMIES!\n\r",ch);
    return;
  } else
  send_to_char("READ HELP on RACE WAR.\n\r",ch);
 return;
}

if (!strcmp(type,"ansi")) {		/* turn ansi stuff ON/OFF */
     if (is_abbrev(field,"enable")) {
         send_to_char("Setting ansi colors enabled.\n\r",ch);
         SET_BIT(ch->player.user_flags,USE_ANSI);
        } else {
          act("Setting ansi colors off.",FALSE,ch,0,0,TO_CHAR);
          if (IS_SET(ch->player.user_flags,USE_ANSI))
              REMOVE_BIT(ch->player.user_flags,USE_ANSI);
     }
    } /* was ansi */
else 
  if (!strcmp(type,"color")) {	/* set current screen color */
        char buf[128];
    	sprintf(buf,"%sChanging screen colors!",ansi_parse(field));
    	act(buf,FALSE,ch,0,0,TO_CHAR);
  }		/* was color*/
else 
if (!strcmp(type,"pause")) {             /* turn page mode ON/OFF */
     if (strstr(field,"enable")) {
         send_to_char("Setting page pause mode enabled.\n\r",ch);
         SET_BIT(ch->player.user_flags,USE_PAGING);
     } else {
         act("Turning page pause off.",FALSE,ch,0,0,TO_CHAR);
         if (IS_SET(ch->player.user_flags,USE_PAGING))
              REMOVE_BIT(ch->player.user_flags,USE_PAGING);
     }
}

else
  if (!strcmp(type,"group")) {
    if (!strcmp(field,"name")) {
      if (argument)  
        do_group_name(ch,argument,0);
     } 
#if 0
      else 
     if (!strcmp(field,"order")) {
	if (IS_SET(ch->specials.affected_by2,AFF2_CON_ORDER)) {
	   act("You no longer recieve orders from your leader.",FALSE,ch,0,0,TO_CHAR);
	   act("$n stops accepting orders from $s group leader.",FALSE,ch,0,0,TO_ROOM);
	   REMOVE_BIT(ch->specials.affected_by2,AFF2_CON_ORDER);	   
	   } else
      if(!ch->master) {
        act("You already can accept orders from YOURSELF",FALSE,ch,0,0,TO_CHAR);
      } else {
        act("You now can receive orders from your group leader",FALSE,ch,0,0,TO_CHAR);
        act("$N just give you permission to order $m", FALSE,ch->master,0,ch,TO_CHAR);
        SET_BIT(ch->specials.affected_by2,AFF2_CON_ORDER);
      }
     }  /* end order */
/* i disabled for not, allows people to order followers to bash 50 times */
/* with no lag, as well as cast 50 times with no lag */
#endif
 
       else {
       send_to_char("Unknown set group command\n",ch);
     }
  } /* end was a group command */

  else
  if (!strcmp(type,"autoexits")) {
    if (strstr(field,"enable")) {
      act("Setting autodisplay exits on.",FALSE,ch,0,0,TO_CHAR);
      if (!IS_SET(ch->player.user_flags,SHOW_EXITS))
        SET_BIT(ch->player.user_flags,SHOW_EXITS);
    } else {
      act("Setting autodisplay exits off.",FALSE,ch,0,0,TO_CHAR);
      if (IS_SET(ch->player.user_flags,SHOW_EXITS))
        REMOVE_BIT(ch->player.user_flags,SHOW_EXITS);
    }      
  }
else 
	if (!strcmp(type,"email")) {
		if (*field) {
		/* set email to field */
	          ch->specials.email= strdup(field);
	if (cmd)
		send_to_char("Email address set.\n\r",ch);
		 } else {
	if (cmd)
		send_to_char("Set email address to what?\n\r",ch);
		   }
		} else /* end email */
     {
      send_to_char("Unknown type to set.\n\r",ch);
      return;
     }
	
}

void do_finger(struct char_data *ch, char *argument, int cmd)
{
 char name[128],buf[254];
 struct char_data *finger;

dlog("in do_finger");

 argument= one_argument(argument,name);

 if (!*name) {
  send_to_char("Whois whom?!?!\n\r",ch);
  return;
  }

if (! (finger=get_char(name)) )  {
	send_to_char("No person by that name\n\r",ch);
 } else {
 
 if (IS_NPC(finger)) {
 	send_to_char("No person by that name\n\r",ch);
 	return;
 	}
 	
  act("\n\r$N's adventurer information:",FALSE,ch,0,finger,TO_CHAR);
  sprintf(buf,"Known message drop: %-50s",finger->specials.email);
  act(buf,FALSE,ch,0,0,TO_CHAR);
  sprintf(buf,"Last time sited	: Unknown");
  act(buf,FALSE,ch,0,0,TO_CHAR); 
  sprintf(buf,"Rumored Info	: Unknown");
  act(buf,FALSE,ch,0,0,TO_CHAR);	
  } /* end found finger'e */
   
}
