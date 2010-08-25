
/*
  DaleMUD v2.0	Released 2/1994
  See license.doc for distribution terms.   DaleMUD is based on DIKUMUD
*/

#include <stdio.h>
#include <string.h>

#include "protos.h"

/* extern variables */

extern struct descriptor_data *descriptor_list;
extern struct dex_app_type dex_app[];
extern char *att_kick_hit_room[];
extern char *att_kick_hit_victim[];
extern char *att_kick_hit_ch[];
extern char *att_kick_miss_room[];
extern char *att_kick_miss_victim[];
extern char *att_kick_miss_ch[];
extern char *att_kick_kill_room[];
extern char *att_kick_kill_victim[];
extern  char *att_kick_kill_ch[];
extern struct char_data *character_list;


void do_hit(struct char_data *ch, char *argument, int cmd)

{
  char arg[80];
  struct char_data *victim;

dlog("in do_hit");

  if (check_peaceful(ch,
      "You feel too peaceful to contemplate violence.\n\r"))
    return;

  only_argument(argument, arg);
  
  if (*arg) {
    victim = get_char_room_vis(ch, arg);
    if (victim) {
      if (victim == ch) {
	send_to_char("You hit yourself..OUCH!.\n\r", ch);
	act("$n hits $mself, and says OUCH!", FALSE, ch, 0, victim, TO_ROOM);
      } else {
	if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
	  act("$N is just such a good friend, you simply can't hit $M.",
	      FALSE, ch,0,victim,TO_CHAR);
	  return;
	}
	if ((GET_POS(ch)>=POSITION_STANDING) &&
	    (!ch->specials.fighting)) {
	  hit(ch, victim, TYPE_UNDEFINED);
	  WAIT_STATE(ch, PULSE_VIOLENCE+2);
	  
	} else {
	  if (victim != ch->specials.fighting) {
	    if (ch->skills && ch->skills[SKILL_SWITCH_OPP].learned) {
	      if (number(1,101) < ch->skills[SKILL_SWITCH_OPP].learned) {
	      stop_fighting(ch);
	      if (victim->attackers < 5) 
		set_fighting(ch, victim);
	      else {
		send_to_char("There's no room to switch!\n\r", ch);
	      }
	      send_to_char("You switch opponents\n\r", ch);
	      act("$n switches targets", FALSE, ch, 0, 0, TO_ROOM);
	      WAIT_STATE(ch, PULSE_VIOLENCE+2);
	    } else {
	      send_to_char("You try to switch opponents, but you become confused!\n\r", ch);
	      stop_fighting(ch);
	      LearnFromMistake(ch, SKILL_SWITCH_OPP, 0, 95);
	      WAIT_STATE(ch, PULSE_VIOLENCE*2);
	    }
	    } else {
	      send_to_char("You do the best you can!\n\r",ch);
	    }
	  } else {
	      send_to_char("You do the best you can!\n\r",ch);
	  }
	}
      }
    } else {
      send_to_char("They aren't here.\n\r", ch);
    }
  } else {
    send_to_char("Hit who?\n\r", ch);
  }
}



void do_kill(struct char_data *ch, char *argument, int cmd)
{
  static char arg[MAX_INPUT_LENGTH];
  struct char_data *victim;

dlog("in do_kill");

  if ((GetMaxLevel(ch) < SILLYLORD) || IS_NPC(ch)) {
    do_hit(ch, argument, 0);
    return;
  }
  
  only_argument(argument, arg);
  
  if (!*arg) {
    send_to_char("Kill who?\n\r", ch);
  } else {
    if (!(victim = get_char_room_vis(ch, arg)))
      send_to_char("They aren't here.\n\r", ch);
    else if (ch == victim)
      send_to_char("Your mother would be so sad.. :(\n\r", ch);
    else {
      act("You chop $M to pieces! Ah! The blood!", FALSE, ch, 0, victim, TO_CHAR);
      act("$N chops you to pieces!", FALSE, victim, 0, ch, TO_CHAR);
      act("$n brutally slays $N", FALSE, ch, 0, victim, TO_NOTVICT);
      raw_kill(victim,NULL);
    }
  }
}



void do_backstab(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  byte percent, base=0;

dlog("in do_backstab");

if (!ch->skills)
	return;
	
  if (check_peaceful(ch, "Naughty, naughty.  None of that here.\n\r"))
    return;
  
  only_argument(argument, name);
  
  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("Backstab who?\n\r", ch);
    return;
  }
  
  if (victim == ch) {
    send_to_char("How can you sneak up on yourself?\n\r", ch);
    return;
  }

  if (!HasClass(ch, CLASS_THIEF)) {
    send_to_char("You're no thief!\n\r", ch);
    return;
  }
  
  if (!ch->equipment[WIELD]) {
    send_to_char("You need to wield a weapon, to make it a succes.\n\r",ch);
    return;
  }

  if (MOUNTED(ch)) {
    send_to_char("Yeah, right!\n", ch);
    return;
  }

  if (ch->attackers) {
    send_to_char("There's no way to reach that back while you're fighting!\n\r", ch);
    return;
  }

  if (victim->attackers >= 3) {
    send_to_char("You can't get close enough to them to backstab!\n\r", ch);
    return;
  }

  if (IS_NPC(victim) && IS_SET(victim->specials.act, ACT_HUGE))   {
    if (!IsGiant(ch))     {
      act("$N is MUCH too large to backstab", FALSE, ch, 0, victim, TO_CHAR);
      return;
    }
  }
  
  if (ch->equipment[WIELD]->obj_flags.value[3] != 11 &&
      ch->equipment[WIELD]->obj_flags.value[3] != 1  &&
      ch->equipment[WIELD]->obj_flags.value[3] != 10) {
    send_to_char("Only piercing or stabbing weapons can be used for backstabbing.\n\r",ch);
    return;
  }
  
  if (ch->specials.fighting) {
    send_to_char("You're too busy to backstab\n\r", ch);
    return;
  }
  
  if (MOUNTED(ch)) {
    send_to_char("How can you surprise someone with all the racket that beast makes?\n\r", ch);
    return;
  }

  if (victim->specials.fighting) {
    base = 0;
  } else {
    base = 4;
  }

  if (victim->skills && victim->skills[SKILL_AVOID_BACK_ATTACK].learned &&
      GET_POS(victim) > POSITION_SITTING)    {
      percent=number(1,101); /* 101% is a complete failure */
      if (percent < victim->skills[SKILL_AVOID_BACK_ATTACK].learned)       {
           act("You sense a back attack from $N and avoid it skillfully!",
	      FALSE, victim, 0, ch, TO_CHAR);
           act("$n avoids a back attack from $N!",
              FALSE, victim, 0, ch, TO_ROOM);	  
              SetVictFighting(ch,victim); /* he avoided, so make him hit! */
              SetCharFighting(ch,victim);
              AddHated(victim, ch);
          return;
         } else {
          act("You failed to sense a back attack from $N!", 
               FALSE, victim, 0, ch, TO_CHAR);	  
          act("$n fails to avoid a back attack from $N!",
              FALSE, victim, 0, ch, TO_ROOM);	                 
	  LearnFromMistake(victim, SKILL_AVOID_BACK_ATTACK, 0, 95);
         } 
         
    }  /* ^ they had skill avoid ba and where awake! ^ */
    
  
  percent=number(1,101); /* 101% is a complete failure */

  if (ch->skills[SKILL_BACKSTAB].learned) {
    if (percent > ch->skills[SKILL_BACKSTAB].learned) {
	char buff[255];
      if (AWAKE(victim))       {
	sprintf(buff,"check 1 %s backstabed %s (hp=%d, in_room=%ld)",
		GET_NAME(ch),GET_NAME(victim),GET_HIT(victim),victim->in_room);
	log(buff);		
	damage(ch, victim, 0, SKILL_BACKSTAB);
	sprintf(buff,"check 2 %s backstabed %s (hp=%d, in_room=%ld)",
		GET_NAME(ch),GET_NAME(victim),GET_HIT(victim),victim->in_room);
	log(buff);		
	AddHated(victim, ch);
      } else {             /* failed but vic is asleep */
	base += 2;
	GET_HITROLL(ch) += base;
	sprintf(buff,"check 3 %s backstabed %s (hp=%d, in_room=%ld)",
		GET_NAME(ch),GET_NAME(victim),GET_HIT(victim),victim->in_room);
	log(buff);		
	hit(ch,victim,SKILL_BACKSTAB);
	GET_HITROLL(ch) -= base;
	sprintf(buff,"check 4 %s backstabed %s (hp=%d, in_room=%ld)",
		GET_NAME(ch),GET_NAME(victim),GET_HIT(victim),victim->in_room);
	log(buff);		

	AddHated(victim, ch);
      }
      
      LearnFromMistake(ch, SKILL_BACKSTAB, 0, 95);
    } 
    
    else {
	char buff[256];    
      GET_HITROLL(ch) += base;
	sprintf(buff,"check 6a %s backstabed %s (hp=%d, in_room=%ld)",
		GET_NAME(ch),GET_NAME(victim),GET_HIT(victim),victim->in_room);
	log(buff);		

      hit(ch,victim,SKILL_BACKSTAB);
      GET_HITROLL(ch) -= base;
	sprintf(buff,"check 6b %s backstabed %s (hp=%d, in_room=%ld)",
		GET_NAME(ch),GET_NAME(victim),GET_HIT(victim),victim->in_room);
	log(buff);		

      AddHated(victim, ch);
      if (IS_PC(ch) && IS_PC(victim))
	GET_ALIGNMENT(ch)-=50;

    }
  } else {
	char buff[256];
	sprintf(buff,"check 6 %s backstabed %s (hp=%d, in_room=%ld)",
		GET_NAME(ch),GET_NAME(victim),GET_HIT(victim),victim->in_room);
	log(buff);		
    damage(ch, victim, 0, SKILL_BACKSTAB);
	sprintf(buff,"check 6 %s backstabed %s (hp=%d, in_room=%ld)",
		GET_NAME(ch),GET_NAME(victim),GET_HIT(victim),victim->in_room);
	log(buff);		
    AddHated(victim, ch);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}



void do_order(struct char_data *ch, char *argument, int cmd)
{
  char name[100], message[256];
  char buf[256];
  char tbuf[80];
  bool found = FALSE;
  int org_room;
  struct char_data *victim;
  struct follow_type *k;
  

dlog("in do_order");

  if (apply_soundproof(ch))
    return;

  half_chop(argument, name, message);
  
  if (!*name || !*message)
    send_to_char("Order who to do what?\n\r", ch);
  else if (!(victim = get_char_room_vis(ch, name)) &&
	   str_cmp("follower", name) && str_cmp("followers", name))
    send_to_char("That person isn't here.\n\r", ch);
  else if (ch == victim)
    send_to_char("You obviously suffer from Multiple Personality Disorder.\n\r", ch);
  
  else {
    if (IS_AFFECTED(ch, AFF_CHARM)) {
      send_to_char("Your superior would not aprove of you giving orders.\n\r",ch);
      return;
    }
    
    if (victim) {
	if (check_soundproof(victim))
	  return;
      sprintf(buf, "$N orders you to '%s'", message);
      act(buf, FALSE, victim, 0, ch, TO_CHAR);
      act("$n gives $N an order.", FALSE, ch, 0, victim, TO_ROOM);
      
      if ( (victim->master!=ch) || !IS_AFFECTED(victim, AFF_CHARM) )
	if (RIDDEN(victim) == ch) {
	  int check;
	  check = MountEgoCheck(ch, victim);
	  if (check > 5) {
	    if (RideCheck(ch, -5)) {
	      act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
	    } else {
	      Dismount(ch, victim, POSITION_SITTING);
	      act("$n gets pissed and $N falls on $S butt!", 
		  FALSE, victim, 0, ch, TO_NOTVICT);
	      act("$n gets pissed you fall off!", 
		  FALSE, victim, 0, ch, TO_VICT);
	    }
	  } else if (check > 0) {
	    act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
	  } else {
	    send_to_char("Ok.\n\r", ch);
	    command_interpreter(victim, message);
	  }
	} else 
        if(IS_AFFECTED2(victim, AFF2_CON_ORDER) && victim->master==ch) {
          send_to_char("Ok.\n\r", ch);
          command_interpreter(victim, message);
        }
        else {
	  act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
	}
      else {
	send_to_char("Ok.\n\r", ch);
	WAIT_STATE(victim, (21-GET_CHR(ch)));
	
	command_interpreter(victim, message);
      }
    } else {  /* This is order "followers" */
      sprintf(buf, "$n issues the order '%s'.", message);
      act(buf, FALSE, ch, 0, victim, TO_ROOM);
      
      org_room = ch->in_room;
      
      for (k = ch->followers; k; k = k->next) {
        if (org_room == k->follower->in_room)
          if (IS_AFFECTED(k->follower, AFF_CHARM)) {
            found = TRUE;
	  }
      }

      if(found) {
	for (k = ch->followers; k; k = k->next) {
	  if (org_room == k->follower->in_room)
	    if (IS_AFFECTED(k->follower, AFF_CHARM)) {
	      command_interpreter(k->follower, message);
	    }
	}
	send_to_char("Ok.\n\r", ch);
      } else {
	send_to_char("Nobody here is a loyal subject of yours!\n\r", ch);
      }
    }
  }
}



void do_flee(struct char_data *ch, char *argument, int cmd)
{
  int i, attempt, loose, die, percent, charm;
  
  void gain_exp(struct char_data *ch, int gain);
  int special(struct char_data *ch, int cmd, char *arg);

dlog("in do_flee");

  if (IS_AFFECTED(ch, AFF_PARALYSIS))
    return;

if (GET_POS(ch) < POSITION_SLEEPING) {
	 send_to_char("Not like this you can't!\n\r",ch);
	 return;    
	}

  if (IS_SET(ch->specials.affected_by2,AFF2_BERSERK)) {
    send_to_char("You can think of nothing but the battle!\n\r",ch);
    return;
  }
  
  if (affected_by_spell(ch, SPELL_WEB)) {
    if (!saves_spell(ch, SAVING_PARA)) {
       WAIT_STATE(ch, PULSE_VIOLENCE);
       send_to_char("You are ensared in webs, you cannot move!\n\r", ch);
       act("$n struggles against the webs that hold $m", FALSE,
	   ch, 0, 0, TO_ROOM);
       return;
    } else {
      send_to_char("You pull free from the sticky webbing!\n\r", ch);
      act("$n manages to pull free from the sticky webbing!", FALSE,
	  ch, 0, 0, TO_ROOM);
      GET_MOVE(ch) -= 50;
    }
  }
  
  if (GET_POS(ch) <= POSITION_SITTING){
    GET_MOVE(ch) -= 10;
    act("$n scrambles madly to $s feet!", TRUE, ch, 0, 0, TO_ROOM);
    act("Panic-stricken, you scramble to your feet.", TRUE, ch, 0, 0,
	TO_CHAR);
    GET_POS(ch) = POSITION_STANDING;
    WAIT_STATE(ch, PULSE_VIOLENCE);
    return;
  }
  
  if (!(ch->specials.fighting)) {
    for(i=0; i<6; i++) {
      attempt = number(0, 5);  /* Select a random direction */
      if (CAN_GO(ch, attempt) &&
	  !IS_SET(real_roomp(EXIT(ch, attempt)->to_room)->room_flags, 
		  DEATH)) {
	act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);

	if (RIDDEN(ch)) {
	  if ((die = MoveOne(RIDDEN(ch), attempt, FALSE))== 1) {
	    /* The escape has succeded */
	    send_to_char("You flee head over heels.\n\r", ch);
	    return;
	  } else {
	    if (!die) act("$n tries to flee, but is too exhausted!", 
			  TRUE, ch, 0, 0, TO_ROOM);
	    return;
	  }
	} else {
	  if ((die = MoveOne(ch, attempt, FALSE))== 1) {
	    /* The escape has succeded */
	    send_to_char("You flee head over heels.\n\r", ch);
	    return;
	  } else {
	    if (!die) act("$n tries to flee, but is too exhausted!", 
			  TRUE, ch, 0, 0, TO_ROOM);
	    return;
	  }
	}
      }
    } /* for */
    /* No exits was found */
    send_to_char("PANIC! You couldn't escape!\n\r", ch);
    return;
  }
  
  for(i=0; i<3; i++) {
    attempt = number(0, 5);  /* Select a random direction */
    if (CAN_GO(ch, attempt) &&
	!IS_SET(real_roomp(EXIT(ch, attempt)->to_room)->room_flags, DEATH)) {
      int panic, j;

      if (!ch->skills || (number(1,101) > ch->skills[SKILL_RETREAT].learned)) {
	act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);
	panic = TRUE;
	LearnFromMistake(ch, SKILL_RETREAT, 0, 90);
      } else {
	/*
	  find a legal exit
	  */
	for (j =0;j<6;j++) {
	  if (CAN_GO(ch, j) &&	
	      !IS_SET(real_roomp(EXIT(ch, j)->to_room)->room_flags, 
		      DEATH)) {
	    attempt = j;
	    j = 10;
	  }
	}
	act("$n skillfully retreats from battle", TRUE, ch, 0, 0, TO_ROOM);
	panic = FALSE;
      }
      

      if (IS_AFFECTED(ch, AFF_CHARM)) {
	charm = TRUE;
	REMOVE_BIT(ch->specials.affected_by, AFF_CHARM);
      } else 
	charm = FALSE;

      if (RIDDEN(ch)) {
	die = MoveOne(RIDDEN(ch), attempt);
      } else {
	die = MoveOne(ch, attempt);
      }

      if (charm)
	SET_BIT(ch->specials.affected_by, AFF_CHARM);

if (die == 1) { 
	/* The escape has succeded. We'll be nice. */
if (GetMaxLevel(ch) > 3) {
  if (panic || !HasClass(ch, CLASS_WARRIOR|CLASS_BARBARIAN|CLASS_PALADIN|CLASS_RANGER)) {
	    loose = GetMaxLevel(ch)+(GetSecMaxLev(ch)/2)+
	      (GetThirdMaxLev(ch)/3);
	    loose -= GetMaxLevel(ch->specials.fighting)+
	      (GetSecMaxLev(ch->specials.fighting)/2)+
		(GetThirdMaxLev(ch->specials.fighting)/3);
	    loose *= GetMaxLevel(ch);
	  }
	} else {
	  loose = 0;
	}     
	if (loose < 0) loose = 1;
	if (IS_NPC(ch) && !(IS_SET(ch->specials.act, ACT_POLYSELF) &&
			    !(IS_SET(ch->specials.act, ACT_AGGRESSIVE)))) {
	  AddFeared(ch, ch->specials.fighting);
	} else {
	  percent=(int)100 * (float) GET_HIT(ch->specials.fighting) /
	    (float) GET_MAX_HIT(ch->specials.fighting);
	  if (Hates(ch->specials.fighting, ch)) {	      
	      SetHunting(ch->specials.fighting, ch);
	  } else if ((IS_GOOD(ch) && (IS_EVIL(ch->specials.fighting))) ||
		(IS_EVIL(ch) && (IS_GOOD(ch->specials.fighting)))) {
	      AddHated(ch->specials.fighting, ch);
	      SetHunting(ch->specials.fighting, ch);
	  } else if (number(1,101) < percent) {
	      AddHated(ch->specials.fighting, ch);
	      SetHunting(ch->specials.fighting, ch);
	  }
	}
	
if (IS_PC(ch) && panic) {
  if (HasClass(ch, CLASS_MONK) || 
      !HasClass(ch, CLASS_WARRIOR|CLASS_BARBARIAN|CLASS_PALADIN|CLASS_RANGER)) 
	    gain_exp(ch, -loose);
	}

	if (panic) {
	  send_to_char("You flee head over heels.\n\r", ch);
	} else {
	  send_to_char("You retreat skillfully\n\r", ch);
	}
	if (ch->specials.fighting->specials.fighting == ch)
	  stop_fighting(ch->specials.fighting);
	if (ch->specials.fighting)
	  stop_fighting(ch);
	return;
      } else {
	if (!die) act("$n tries to flee, but is too exhausted!", TRUE, ch, 0, 0, TO_ROOM);
	return;
      }
    }
  } /* for */
  
  /* No exits were found */
  send_to_char("PANIC! You couldn't escape!\n\r", ch);
}



void do_bash(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  byte percent;


dlog("in do_bash");
  if (!ch->skills)
    return;

#if 0
  if (!IS_PC(ch) && cmd && !IS_SET(ch->specials.act,ACT_POLYSELF))
    return;
#endif

   if (check_peaceful(ch,"You feel too peaceful to contemplate violence.\n\r"))
    return;

if (IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
 if (!HasClass(ch, CLASS_WARRIOR|CLASS_PALADIN|CLASS_RANGER|CLASS_BARBARIAN)) {
     send_to_char("You're no warrior!\n\r", ch);
     return;
    }

  only_argument(argument, name);
  
  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Bash who?\n\r", ch);
      return;
    }
  }
  
  
  if (victim == ch) {
    send_to_char("Aren't we funny today...\n\r", ch);
    return;
  }

  if (IS_NPC(victim) && IS_SET(victim->specials.act, ACT_HUGE)) {
    if (!IsGiant(ch)) {
      act("$N is MUCH too large to bash!", FALSE, ch, 0, victim, TO_CHAR);
      return;
    }
  }

  if (MOUNTED(victim)) {
    send_to_char("You can't bash a mounted target!\n\r", ch);
    return;
  }

  if (MOUNTED(ch)) {
    send_to_char("You can't bash while mounted!\n\r", ch);
    return;
  }


#if 0
  if (!ch->skills) {
    if (GET_POS(victim) > POSITION_DEAD) {
      damage(ch, victim, 0, SKILL_BASH);
      GET_POS(ch) = POSITION_SITTING;
    }
  }
#endif

  if (ch->attackers > 3) {
    send_to_char("There's no room to bash!\n\r",ch);
    return;
  }

  if (victim->attackers >= 6) {
    send_to_char("You can't get close enough to them to bash!\n\r", ch);
    return;
  }
  
  percent=number(1,101); /* 101% is a complete failure */
  
  /* some modifications to account for dexterity, and level */
  percent -= dex_app[GET_DEX(ch)].reaction*10;
  percent += dex_app[GET_DEX(victim)].reaction*10;

  if (GetMaxLevel(victim) > 12) {
    percent += ((GetMaxLevel(victim)-10) * 5);
  }

  if (percent > ch->skills[SKILL_BASH].learned) {
    if (GET_POS(victim) > POSITION_DEAD) {
      damage(ch, victim, 0, SKILL_BASH);
      GET_POS(ch) = POSITION_SITTING;
    }
    LearnFromMistake(ch, SKILL_BASH, 0, 90);
    WAIT_STATE(ch, PULSE_VIOLENCE*3);
  } else {
    if (GET_POS(victim) > POSITION_DEAD) {
      GET_POS(victim) = POSITION_SITTING;
      damage(ch, victim, 2, SKILL_BASH);
      WAIT_STATE(victim, PULSE_VIOLENCE*2);
      GET_POS(victim) = POSITION_SITTING;
    }
  }
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}




void do_rescue(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim, *tmp_ch;
  int percent;
  char victim_name[240];
  

dlog("in do_rescue");
  if (!ch->skills) {
      send_to_char("You fail the rescue.\n\r", ch);
      return;
  }

#if 0
  if (!IS_PC(ch) && cmd && !IS_SET(ch->specials.act,ACT_POLYSELF))
    return;
#endif

  if (check_peaceful(ch,"No one should need rescuing here.\n\r"))
    return;

if (IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
  if (!HasClass(ch, CLASS_WARRIOR|CLASS_BARBARIAN|CLASS_PALADIN|CLASS_RANGER)) {
    send_to_char("You're no warrior!\n\r", ch);
    return;
  }

  only_argument(argument, victim_name);
  
  if (!(victim = get_char_room_vis(ch, victim_name))) {
    send_to_char("Who do you want to rescue?\n\r", ch);
    return;
  }
  
  if (victim == ch) {
    send_to_char("What about fleeing instead?\n\r", ch);
    return;
  }

  if (MOUNTED(victim)) {
    send_to_char("You can't rescue a mounted person!\n\r", ch);
    return;
  }
  
  if (ch->specials.fighting == victim) {
    send_to_char("How can you rescue someone you are trying to kill?\n\r",ch);
    return;
  }

  if (victim->attackers >= 3) {
    send_to_char("You can't get close enough to them to rescue!\n\r", ch);
    return;
  }
  
  for (tmp_ch=real_roomp(ch->in_room)->people; tmp_ch &&
       (tmp_ch->specials.fighting != victim); tmp_ch=tmp_ch->next_in_room)  ;
  
  if (!tmp_ch) {
    act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }
  
  
  percent=number(1,101); /* 101% is a complete failure */

  if ((percent > ch->skills[SKILL_RESCUE].learned)) {
    send_to_char("You fail the rescue.\n\r", ch);
    LearnFromMistake(ch, SKILL_RESCUE, 0, 90);
    WAIT_STATE(ch, PULSE_VIOLENCE);
    return;
  }
  
  send_to_char("Banzai! To the rescue...\n\r", ch);
  act("You are rescued by $N, you are confused!", FALSE, victim,0,ch, TO_CHAR);
  act("$n heroically rescues $N.", FALSE, ch, 0, victim, TO_NOTVICT);
  if (IS_PC(ch) && IS_PC(victim))
      GET_ALIGNMENT(ch)+=20;
    
  if (victim->specials.fighting == tmp_ch)
    stop_fighting(victim);
  if (tmp_ch->specials.fighting)
    stop_fighting(tmp_ch);
  if (ch->specials.fighting)
    stop_fighting(ch);
  
  set_fighting(ch, tmp_ch);
  set_fighting(tmp_ch, ch);
  
  WAIT_STATE(victim, 2*PULSE_VIOLENCE);

}



void do_assist(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim, *tmp_ch;
  char victim_name[240];

dlog("in do_assist");

  if (check_peaceful(ch,"Noone should need assistance here.\n\r"))
    return;
  
  only_argument(argument, victim_name);
  
  if (!(victim = get_char_room_vis(ch, victim_name))) {
    send_to_char("Who do you want to assist?\n\r", ch);
    return;
  }
  
  if (victim == ch) {
    send_to_char("Oh, by all means, help yourself...\n\r", ch);
    return;
  }
  
  if (ch->specials.fighting == victim) {
    send_to_char("That would be counterproductive?\n\r",ch);
    return;
  }
  
  if (ch->specials.fighting) {
    send_to_char("You have your hands full right now\n\r",ch);
    return;
  }

  if (victim->attackers >= 6) {
    send_to_char("You can't get close enough to them to assist!\n\r", ch);
    return;
  }

  
  tmp_ch = victim->specials.fighting;
  /*	for (tmp_ch=real_roomp(ch->in_room)->people; tmp_ch &&
	(tmp_ch->specials.fighting != victim); tmp_ch=tmp_ch->next_in_room)  ;
	*/
  if (!tmp_ch) {
    act("But $E's not fighting anyone.", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }

if (tmp_ch->in_room !=ch->in_room) {
	send_to_char("Woops, they left in a hurry, must have scared them off!\n\r",ch);
        return;
}
  
  hit(ch, tmp_ch, TYPE_UNDEFINED);
  
  WAIT_STATE(victim, PULSE_VIOLENCE+2); /* same as hit */
}



void do_kick(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[80];
  int dam;
  byte percent;

dlog("in do_kick");

  if (!ch->skills)
    return;
  
  if (check_peaceful(ch,
         "You feel too peaceful to contemplate violence.\n\r"))
    return;

if (IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
  if (!HasClass(ch, CLASS_WARRIOR|CLASS_BARBARIAN|CLASS_RANGER|CLASS_PALADIN)
      && !HasClass(ch, CLASS_MONK)) {
      send_to_char("You're no warrior!\n\r", ch);    
      return;
  }

#if 0
 if (!IS_PC(ch) && cmd && !IS_SET(ch->specials.act,ACT_POLYSELF))
     return;
#endif

  only_argument(argument, name);
  
  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Kick who?\n\r", ch);
      return;
    }
  }
  
  if (victim == ch) {
    send_to_char("Aren't we funny today...\n\r", ch);
    return;
  }


  if (MOUNTED(victim)) {
    send_to_char("You can't kick while mounted!\n\r", ch);
    return;
  }

  if (ch->attackers > 3) {
    send_to_char("There's no room to kick!\n\r",ch);
    return;
  }

  if (victim->attackers >= 4) {
    send_to_char("You can't get close enough to them to kick!\n\r", ch);
    return;
  }
  
  percent=((10-(GET_AC(victim)/10))) + number(1,101);
  /* 101% is a complete failure */
  
  if(GET_RACE(victim)==RACE_GHOST) {
    kick_messages(ch,victim,0);
    SetVictFighting(ch,victim);
    return;
  } else 
     if (!IS_NPC(victim) && (GetMaxLevel(victim) > MAX_MORT)) {
    kick_messages(ch,victim,0);
    SetVictFighting(ch,victim);
    SetCharFighting(ch,victim);
    return;
  }

  if (GET_POS(victim) <= POSITION_SLEEPING)
    percent = 1;

  if (percent > ch->skills[SKILL_KICK].learned) {
    if (GET_POS(victim) > POSITION_DEAD) {
      damage(ch, victim, 0, SKILL_KICK);
      kick_messages(ch,victim,0);
    }
    LearnFromMistake(ch, SKILL_KICK, 0, 90);
  } else {
    if (GET_POS(victim) > POSITION_DEAD) {
      dam = GET_LEVEL(ch, BestFightingClass(ch));
      if (!HasClass(ch, CLASS_MONK) || IS_NPC(ch)) /* so guards use fighter dam */
	dam=dam>>1;
      kick_messages(ch,victim,dam);
      damage(ch, victim, dam, SKILL_KICK);
    }
    WAIT_STATE(victim, PULSE_VIOLENCE);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE*3);
}
void do_wimp(struct char_data *ch, char *argument, int cmd)
{
  
  /* sets the character in wimpy mode.  */
  

dlog("in do_wimp");

  if (IS_SET(ch->specials.act, PLR_WIMPY)) {
    REMOVE_BIT(ch->specials.act, PLR_WIMPY);
    send_to_char("Ok, you are no longer a wimp...\n\r",ch);
  } else {
    
    SET_BIT(ch->specials.act, PLR_WIMPY);
    send_to_char("Ok, you are now in wimpy mode.\n\r", ch);
  }
  
}

extern struct breather breath_monsters[];
extern struct index_data *mob_index;

#if 0
void (*bweapons[])() = {
  cast_geyser,
  cast_fire_breath, cast_gas_breath, cast_frost_breath, cast_acid_breath,
  cast_lightning_breath};
#else
void cast_geyser();
void cast_fire_breath();
void cast_gas_breath();
void cast_frost_breath();
void cast_acid_breath();
void cast_lightning_breath();

funcp bweapons[] = {
  cast_geyser,
  cast_fire_breath, cast_gas_breath, cast_frost_breath, cast_acid_breath,
  cast_lightning_breath
 };
#endif

void do_breath(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char	buf[MAX_STRING_LENGTH], name[MAX_STRING_LENGTH];
  int	count, manacost;
  funcp weapon;

dlog("in do_breath");

  if (check_peaceful(ch,"That wouldn't be nice at all.\n\r"))
    return;
  
  only_argument(argument, name);
  
  for (count = FIRST_BREATH_WEAPON;
       count <= LAST_BREATH_WEAPON && !affected_by_spell(ch, count);
       count++)
    ;
  
  if (count>LAST_BREATH_WEAPON) {
    struct breather *scan;
    
    for (scan = breath_monsters;
	 scan->vnum >= 0 && scan->vnum != mob_index[ch->nr].virtual;
	 scan++)
      ;
    
    if (scan->vnum < 0) {
      send_to_char("You don't have a breath weapon, potatohead.\n\r", ch);
      return;
    }
    
    for (count=0; scan->breaths[count]; count++)
      ;
    
    if (count<1) {
      sprintf(buf, "monster %s has no breath weapons",
	      ch->player.short_descr);
      log(buf);
      send_to_char("Hey, why don't you have any breath weapons!?\n\r",ch);
      return;
    }
    
    weapon = scan->breaths[dice(1,count)-1];
    manacost = scan->cost;
    if (GET_MANA(ch) <= -3*manacost) {
      weapon = NULL;
    }
  } else {
    manacost = 0;
    weapon = bweapons[count-FIRST_BREATH_WEAPON];
    affect_from_char(ch, count);
  }
  
  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Breath on who?\n\r", ch);
      return;
    }
  }
  
  breath_weapon(ch, victim, manacost, weapon);
  
  WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

void do_shoot(struct char_data *ch, char *argument, int cmd)
{
#if 0
  char arg[80],dirstr[80],name[80];
  char buf[255];
  struct char_data *victim;
  struct room_data *this_room,*to_room,*next_room;
  struct room_direction_data *exitp;
  struct char_data *mob;
  struct obj_data *weapon;
  int i,dir,room_num=0,room_count, MAX_DISTANCE_SHOOT;
  extern char *listexits[];

  if (check_peaceful(ch,"You feel too peaceful to contemplate violence.\n\r"))
    return;

 argument =  one_argument(argument, arg);

slog("begin do_shoot");  

  if (*arg) {
    victim = get_char_room_vis(ch, arg);

if (!victim)  {
 i = ch->in_room;
 room_count=1;
 weapon = ch->equipment[HOLD];

 if (!weapon) {
   send_to_char("You do not hold a missile weapon?!?!\n\r",ch);
   return;
  }
 
#if 0
  MAX_DISTANCE_SHOOT = weapon->obj_flags.value[1];
#else
  MAX_DISTANCE_SHOOT = 1; 
#endif



	switch(*arg) {
		case 'N':
		case 'n':dir=0;
			break;
		case 'S':
		case 's':dir=2;
			break;			
		case 'E':
		case 'e':dir=1;
			break;
		case 'W':
		case 'w':dir=3;
			break;		
		case 'd':
		case 'D':dir=4;
			break;
		case 'u':
		case 'U':dir=5;
			break;

		default :send_to_char("What direction did you wish to fire?\n\r",ch);
			 return;
 			 break;
		} /* end switch */

argument= one_argument(argument,name);
      if (strn_cmp(name,"at",2) && isspace(name[2]))
       	  argument=one_argument(argument,name);


 if (!exit_ok(EXIT_NUM(i,dir),NULL)) {
     send_to_char("You can't shoot in that direction.\n\r",ch);
     return;
    }
    
  while (room_count<=MAX_DISTANCE_SHOOT && !victim &&
  				exit_ok(EXIT_NUM(i,dir),NULL)) {

    this_room = real_roomp(i);
    to_room   = real_roomp(this_room->dir_option[dir]->to_room);
    room_num  = this_room->dir_option[dir]->to_room;

      mob = get_char_near_room_vis(ch,name,room_num);         
      if (mob) {
	sprintf(buf,"You spot your quarry %s.\n",listexits[dir]);
	act(buf,FALSE,ch,0,0,TO_CHAR);
	victim=mob;
      }

    i = room_num; 
    room_count++;

  } /* end while */     
 } /* !victim */

if (victim) {
      if (victim == ch) {
	send_to_char("You can't shoot things at yourself!", ch);
	return;
      } else {
	if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
	  act("$N is just such a good friend, you simply can't shoot at $M.",
	      FALSE, ch,0,victim,TO_CHAR);
	  return;
	}
	if (ch->specials.fighting) {
	  send_to_char("You're at too close range to fire a weapon!\n\r", ch);
	  return;
	}

  if (check_peaceful(victim,"")) {
       send_to_char("That is a peaceful room\n\r",ch);
       return;
      }

	shoot(ch, victim);
	WAIT_STATE(ch, PULSE_VIOLENCE);
      }
    } else {
      send_to_char("They aren't here.\n\r", ch);
    }
  } else {
    send_to_char("Shoot who?\n\r", ch);
  }
  slog("end do_shoot, act.off.c");

#endif
}


void do_springleap(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  byte percent;

dlog("in do_springleap");

  if (!ch->skills)
    return;
  
  if (check_peaceful(ch,
         "You feel too peaceful to contemplate violence.\n\r"))
    return;

if (IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
  if (!HasClass(ch, CLASS_MONK)) {
    send_to_char("You're no monk!\n\r", ch);    
    return;
  }
  
  only_argument(argument, name);
  
  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Spring-leap at who?\n\r", ch);
      return;
    }
  }

  if (GET_POS(ch) > POSITION_SITTING || !ch->specials.fighting) {
    send_to_char("You're not in position for that!\n\r", ch);
    return;
  }
  
  if (victim == ch) {
    send_to_char("Aren't we funny today...\n\r", ch);
    return;
  }

  if (ch->attackers > 3) {
    send_to_char("There's no room to spring-leap!\n\r",ch);
    return;
  }

  if (victim->attackers >= 3) {
    send_to_char("You can't get close enough\n\r", ch);
    return;
  }
  
  percent=number(1,101);
  
  act("$n does a really nifty move, and aims a leg towards $N.", FALSE,
      ch, 0, victim, TO_ROOM);
  act("You leap off the ground at $N.", FALSE,
      ch, 0, victim, TO_CHAR);
  act("$n leaps off the ground at you.", FALSE,
      ch, 0, victim, TO_VICT);


  if (percent > ch->skills[SKILL_SPRING_LEAP].learned) {
    if (GET_POS(victim) > POSITION_DEAD) {
      damage(ch, victim, 0, SKILL_KICK);
      LearnFromMistake(ch, SKILL_SPRING_LEAP, 0, 90);
      send_to_char("You fall on your butt\n\r", ch);
      act("$n falls on $s butt", FALSE, ch, 0, 0, TO_ROOM);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE*3);
    return;
    
  } else {
    if (HitOrMiss(ch, victim, CalcThaco(ch))) {
      if (GET_POS(victim) > POSITION_DEAD)
	damage(ch, victim, GET_LEVEL(ch, BestFightingClass(ch))>>1, 
	       SKILL_KICK);
      kick_messages(ch,victim, GET_LEVEL(ch, BestFightingClass(ch))>>1);
    } else {
	damage(ch, victim, 0, SKILL_KICK);
	kick_messages(ch, victim, 0);
    }
    WAIT_STATE(victim, PULSE_VIOLENCE);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE*1);
  GET_POS(ch)=POSITION_STANDING;
  update_pos(ch);
  
}


void do_quivering_palm( struct char_data *ch, char *arg, int cmd)
{
  struct char_data *victim;
  struct affected_type af;
  byte percent;
  char name[256];

dlog("in do_quivering");

  if (!ch->skills)
    return;
  
  if (check_peaceful(ch,
         "You feel too peaceful to contemplate violence.\n\r"))
    return;

if (IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
  if (!HasClass(ch, CLASS_MONK)) {
    send_to_char("You're no monk!\n\r", ch);    
    return;
  }
  
  only_argument(arg, name);
  
  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Use the fabled quivering palm on who?\n\r", ch);
      return;
    }
  }
  
  if (ch->attackers > 3) {
    send_to_char("There's no room to use that skill!\n\r",ch);
    return;
  }

  if (victim->attackers >= 3) {
    send_to_char("You can't get close enough\n\r", ch);
    return;
  }

  if (!IsHumanoid(victim) ) {
    send_to_char("You can only do this to humanoid opponents\n\r", ch);
    return;
  }

  send_to_char("You begin to work on the vibrations\n\r", ch);

  if (affected_by_spell(ch, SKILL_QUIV_PALM)) {
    send_to_char("You can only do this once per week\n\r", ch);
    return;
  }
    
  percent=number(1,101);
  
  if (percent > ch->skills[SKILL_QUIV_PALM].learned) {
    send_to_char("The vibrations fade ineffectively\n\r", ch);
    if (GET_POS(victim) > POSITION_DEAD) {
      LearnFromMistake(ch, SKILL_QUIV_PALM, 0, 95);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE*3);
    return;
    
  } else {
    if (GET_MAX_HIT(victim) > GET_MAX_HIT(ch)*2 || GetMaxLevel(victim) > GetMaxLevel(ch)) {
	damage(ch, victim, 0,SKILL_QUIV_PALM);
      return;
    }
    if (HitOrMiss(ch, victim, CalcThaco(ch))) {
      if (GET_POS(victim) > POSITION_DEAD)
	damage(ch, victim, GET_MAX_HIT(victim)*20,SKILL_QUIV_PALM);
    }
  }

  WAIT_STATE(ch, PULSE_VIOLENCE*1);
 
  af.type = SKILL_QUIV_PALM;
  af.duration = 168;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = 0;
  affect_to_char(ch, &af);
}


void kick_messages(struct char_data *ch, struct char_data *victim, int damage)
{
  int i;
 
  switch(GET_RACE(victim)) {
  case RACE_HUMAN:
  case RACE_ELVEN:
  case RACE_DWARF:
  case RACE_DROW:
  case RACE_ORC:
  case RACE_LYCANTH:
  case RACE_TROLL:
  case RACE_DEMON:
  case RACE_DEVIL:
  case RACE_MFLAYER:
  case RACE_ASTRAL:
  case RACE_PATRYN:
  case RACE_SARTAN:
  case RACE_DRAAGDIM:
  case RACE_GOLEM:
  case RACE_TROGMAN:
  case RACE_LIZARDMAN:
  case RACE_HALF_ELVEN:
  case RACE_HALF_OGRE:
  case RACE_HALF_ORC:
  case RACE_HALF_GIANT:
    i=number(0,3);
    break;
  case RACE_PREDATOR:
  case RACE_HERBIV:
  case RACE_LABRAT:
    i=number(4,6);
    break;
  case RACE_REPTILE:
  case RACE_DRAGON:
 case RACE_DRAGON_RED    :
 case RACE_DRAGON_BLACK  :
 case RACE_DRAGON_GREEN  :
 case RACE_DRAGON_WHITE  :
 case RACE_DRAGON_BLUE   :
 case RACE_DRAGON_SILVER :
 case RACE_DRAGON_GOLD   :
 case RACE_DRAGON_BRONZE :
 case RACE_DRAGON_COPPER :
 case RACE_DRAGON_BRASS	 :
    i=number(4,7);
    break;
  case RACE_TREE:
    i=8;
    break;
  case RACE_PARASITE:
  case RACE_SLIME:
  case RACE_VEGGIE:
  case RACE_VEGMAN:
    i=9;
    break;
  case RACE_ROO:
  case RACE_GNOME:
  case RACE_HALFLING:
  case RACE_GOBLIN:
  case RACE_SMURF:
  case RACE_ENFAN:
    i=10;
    break;
  case RACE_GIANT:
  case RACE_GIANT_HILL   :
  case RACE_GIANT_FROST  :
  case RACE_GIANT_FIRE   :
  case RACE_GIANT_CLOUD  :
  case RACE_GIANT_STORM  :
  case RACE_GIANT_STONE  :

  case RACE_TYTAN:
  case RACE_GOD:
    i=11;
    break;
  case RACE_GHOST:
    i=12;
    break;
  case RACE_BIRD:
  case RACE_SKEXIE:
    i=13;
    break;
  case RACE_UNDEAD:
  case RACE_UNDEAD_VAMPIRE :
  case RACE_UNDEAD_LICH    : 
  case RACE_UNDEAD_WIGHT   :
 case RACE_UNDEAD_GHAST   :
 case RACE_UNDEAD_SPECTRE :
 case RACE_UNDEAD_ZOMBIE  :
 case RACE_UNDEAD_SKELETON :
 case RACE_UNDEAD_GHOUL    :
    i=14;
    break;
  case RACE_DINOSAUR:
    i=15;
    break;
  case RACE_INSECT:
  case RACE_ARACHNID:
    i=16;
    break;
  case RACE_FISH:
    i=17;
    break;
 default:
    i=18;
  };
  if(!damage) {
    act(att_kick_miss_ch[i], FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
    act(att_kick_miss_victim[i],FALSE, ch, ch->equipment[WIELD],victim, 
	TO_VICT);
    act(att_kick_miss_room[i],FALSE, ch, ch->equipment[WIELD], victim,
	TO_NOTVICT);
  } else if( GET_HIT(victim)-DamageTrivia(ch,victim,damage,SKILL_KICK) < -10 ) {
    act(att_kick_kill_ch[i], FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
    act(att_kick_kill_victim[i],FALSE, ch, ch->equipment[WIELD],victim,
        TO_VICT);
    act(att_kick_kill_room[i],FALSE, ch, ch->equipment[WIELD], victim,
        TO_NOTVICT);
  }
  else {
    act(att_kick_hit_ch[i], FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
    act(att_kick_hit_victim[i],FALSE, ch, ch->equipment[WIELD],victim,
        TO_VICT);
    act(att_kick_hit_room[i],FALSE, ch, ch->equipment[WIELD], victim,
        TO_NOTVICT);
  }
 
}

void do_berserk( struct char_data *ch, char *arg, int cmd)
{
  int skillcheck=0;
  char argument[100],name[100];
  struct char_data *victim;

dlog("in do_berserk");

  if (!ch->skills) {
     send_to_char("You do not know any skills!\n\r",ch);
     return;
    }
  
  if (check_peaceful(ch,
         "You feel too peaceful to contemplate violence.\n\r"))
    return;

if (IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF))
  if (!HasClass(ch, CLASS_BARBARIAN) && cmd !=0)  {
    send_to_char("You're no berserker!\n\r", ch);    
    return;
  }
  
#if 0
  if (!IS_PC(ch) && cmd && !IS_SET(ch->specials.act,ACT_POLYSELF))
    return;
#endif

  only_argument(argument, name);
  
  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting)     {
      victim = ch->specials.fighting;
    } else {
      send_to_char("You need to begin fighting before you can go berserk.\n\r", ch);
      return;
    }
  }
  
  if (victim == ch) {
    send_to_char("Aren't we funny today...\n\r", ch);
    return;
  }


  if (MOUNTED(victim)) {
    send_to_char("You can't berserk while mounted!\n\r", ch);
    return;
  }

 if (IS_SET(ch->specials.affected_by2,AFF2_BERSERK)) {
   send_to_char("You are already berserked!\n\r",ch);
   return;
  }

  if (!GET_MANA(ch)>=15)    {
     send_to_char("You do not have the energy to go berserk!\n\r",ch);
     return;
    }
	/* all the checks passed, now get on with it! */
	
   skillcheck = number(0,101);
   
  if (skillcheck > ch->skills[SKILL_BERSERK].learned)   {
    act("$c1012$n growls at $mself, and looks very angry!", FALSE, ch, 0, victim, TO_ROOM);
    act("$c1012You can't seem to get mad enough right now.",FALSE,ch,0,0,TO_CHAR);
    LearnFromMistake(ch, SKILL_BERSERK, 0, 90);
  } else   {
    if (GET_POS(victim) > POSITION_DEAD)     {
     GET_MANA(ch)=(GET_MANA(ch)-15);  /* cost 15 mana to do it.. */
     SET_BIT(ch->specials.affected_by2,AFF2_BERSERK);
     act("$c1012$n growls at $mself, and whirls into a killing frenzy!", FALSE, ch, 0, victim, TO_ROOM);
     act("$c1012The madness overtakes you quickly!",FALSE,ch,0,0,TO_CHAR);
    }
    WAIT_STATE(victim, PULSE_VIOLENCE);
  }
  
  WAIT_STATE(ch, PULSE_VIOLENCE*3);
}

void throw_weapon(struct obj_data *o, int dir, struct char_data *targ, 
   struct char_data *ch)
{
    int w = o->obj_flags.weight, sz, max_range, range, there, ps, ps2;
    int rm = ch->in_room, opdir[] = {2, 3, 0, 1, 5, 4};
    int broken=FALSE;
    char buf[MAX_STRING_LENGTH];
    struct char_data *spud, *next_spud;
    char *dir_name[] = {
       "the north",
       "the east",
       "the south",
       "the west",
       "above",
       "below"};

    if (w>100) {
       sz = 3;
    } else if (w>25) {
       sz = 2;
    } else if (w>5) {
       sz = 1;
    } else sz = 0;
    max_range = (((GET_STR(ch)+GET_ADD(ch)/30)-3)/8)+2;
    max_range = max_range / (sz+1);
    if ((o->obj_flags.type_flag==ITEM_MISSILE)&&(ch->equipment[WIELD])&&(ch->equipment[WIELD]->obj_flags.type_flag==ITEM_FIREWEAPON)) {
       /* Add bow's range bonus */
       max_range += ch->equipment[WIELD]->obj_flags.value[2];
    }
    if (max_range==0) {
       act("$p immediately hits the ground.  A truly pitiful throw.",TRUE,ch,o,0,TO_ROOM);
       act("$p flew 2 feet then fell to earth.  Maybe a lighter weapon...",TRUE,ch,o,0,TO_CHAR);
       obj_to_room(o,ch->in_room);
       return;
    }
    range = 0;
    while (range<max_range && broken==FALSE) {
        /* Check for target */
        there = 0;
        for (spud=real_roomp(rm)->people;spud;spud=next_spud) {
           next_spud = spud->next_in_room;
           if (spud==targ) {
              there = 1;
              if (range_hit(ch,targ,range,o,dir,max_range)) {
                 if ((targ)&&(GET_POS(targ)>POSITION_DEAD)) {
                 if (number(1,100)<o->obj_flags.value[0]) {
                    act("$p breaks into splinters.",TRUE,targ,o,0,TO_ROOM); 
		    broken=TRUE;
                    obj_to_room(o,3); /* storage for broken arrows */
		    /* for some reason this causes the obj to get placed
			in very weird places
		    obj_from_room(o);
		    extract_obj(o);
		    */
                 } else {
                    obj_to_room(o,rm);
                 }
                 }
                 slog("Gobble 2");
                 return;
              }
           }
       }
       if(broken==FALSE) {
        if (clearpath(ch, rm,dir)) {
           if ((!there)&&(rm!=ch->in_room)) { 
              sprintf(buf,"%s from %s flies past!\n\r",o->short_description,dir_name[opdir[dir]]);
              send_to_room(buf,rm);
           } else {
              there = 0;
           }
           rm = real_roomp(rm)->dir_option[dir]->to_room;
        } else {
           if (range>1) {
              sprintf(buf,"%s flies into the room from %s and hits a wall.\n\r",o->short_description,dir_name[opdir[dir]]);
           } else {
              sprintf(buf,"%s hits a wall.\n\r",o->short_description);
           }
           send_to_room(buf,rm);
           if (number(1,100)<o->obj_flags.value[0]) {
              sprintf(buf,"%s breaks into splinters.\n\r",o->short_description);
              obj_to_room(o,3); /* storage for broken arrows */
	      broken=TRUE;
           } else {
              sprintf(buf,"%s falls to the ground.\n\r",o->short_description);
              obj_to_room(o,rm);
           }
           send_to_room(buf,rm);
           return;
        }
       }
       range++;
    }
    if(broken==FALSE) {
      sprintf(buf,"%s falls to the ground.\n\r",o->short_description);
      send_to_room(buf,rm);
      obj_to_room(o,rm);
    }
}

void throw_object(struct obj_data *o, int dir, int from)
{
	struct char_data *catcher;
	const char *directions[][2]= {
	{"north","south"},
	{"east","west"},
	{"south","north"},
	{"west","east"},
	{"up","down"},
	{"down","up"}};

	char buf1[100],buf2[100];
	int distance=0;
	
	while((distance<20)&&(real_roomp(from)->dir_option[dir])&&(real_roomp(from)->dir_option[dir]->exit_info<2)&&(real_roomp(from)->dir_option[dir]->to_room>0)) {
		if(distance) {
			sprintf(buf1,"%s flies into the room from the %s.\n\r",o->short_description,directions[dir][1]);
			send_to_room(buf1,from);

#if 0
			for(catcher=real_roomp(from)->people;catcher;catcher=catcher->next_in_room) {
				if(!strcmp(catcher->catch,o->name)) {
					switch (number(1,3)) {
					case 1:act("$n dives and catches $p",FALSE,catcher,o,0,TO_ROOM);
						break;
					default:act("$n catches $p",FALSE,catcher,o,0,TO_ROOM);
					}
					send_to_char("You caught it!\n\r",catcher);
					obj_from_room(o);
					obj_to_char(o,catcher);
					return;
					}
				}
#endif
				
			sprintf(buf1,"%s flies out of the room to the %s.\n\r",o->short_description,directions[dir][0]);
			send_to_room(buf1,from);
			}
		distance++;
		obj_from_room(o);
		from=real_roomp(from)->dir_option[dir]->to_room;
		obj_to_room(o,from);
		}
	if(distance==20) {
		sprintf(buf1,"%s flies into the room from the %s and lands here.\n\r",o->short_description,directions[dir][1]);
		send_to_room(buf1,from);
		return;
		}
	sprintf(buf1,"%s flies into the room from the %s and smacks a wall.\n\r",o->short_description,directions[dir][1]);
	send_to_room(buf1,from);
	}



int clearpath(struct char_data *ch, long room, int direc)
{
   int opdir[] = {2, 3, 0, 1, 5, 4};
   struct room_direction_data	*exitdata;
  
   exitdata = (real_roomp(room)->dir_option[direc]);

   if ((exitdata) &&
      (!real_roomp(exitdata->to_room))) return 0;
   if (!CAN_GO(ch, direc)) return 0;
   if (!real_roomp(room)->dir_option[direc]) return 0;
   if (real_roomp(room)->dir_option[direc]->to_room<1) return 0;
   if (real_roomp(room)->zone!=real_roomp(real_roomp(room)->dir_option[direc]->to_room)->zone)
      return 0;
   if (IS_SET(real_roomp(room)->dir_option[direc]->exit_info,EX_CLOSED)) return 0;
   if ((!IS_SET(real_roomp(room)->dir_option[direc]->exit_info,EX_ISDOOR))&&
      (real_roomp(room)->dir_option[direc]->exit_info>0)) return 0;
   /* One-way windows are allowed... no see through 1-way exits */
   if (!real_roomp(real_roomp(room)->dir_option[direc]->to_room)->dir_option[opdir[direc]]) return 0;

   if (real_roomp(real_roomp(room)->dir_option[direc]->to_room)->dir_option[opdir[direc]]->to_room<1) return 0;

   if (real_roomp((real_roomp(room)->dir_option[direc]->to_room))->dir_option[opdir[direc]]->to_room!=room) return 0;


   return real_roomp(room)->dir_option[direc]->to_room;
}

void do_weapon_load(struct char_data *ch, char *argument, int cmd)
{
   struct obj_data *fw, *ms;
   struct affected_type *laf;
   char arg1[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH];

dlog("in do_weapon_load");


   fw = ch->equipment[WIELD];
   if ((!fw)||(fw->obj_flags.type_flag!=ITEM_FIREWEAPON)) { 
      send_to_char("You must be wielding the projectile weapon you want to load.\n\r",ch);
      return;
   }
   if ((GET_STR(ch)+(GET_ADD(ch)/3))<fw->obj_flags.value[0]) {
     sprintf(arg1,"(%s) can't load (%s) because it requires (%d) strength to wield",
     GET_NAME(ch),fw->name,fw->obj_flags.value[0]);
     log(arg1);
      send_to_char("You aren't strong enough to draw such a mighty weapon.\n\r",ch);
      return;
   }
   if (ch->equipment[LOADED_WEAPON]) {
      if (CAN_CARRY_N(ch)!=IS_CARRYING_N(ch)) {
         ms = unequip_char(ch, LOADED_WEAPON);
         act("You first unload $p.",TRUE,ch,ms,0,TO_CHAR);
         obj_to_char(ms, ch);
         act("$n unloads $p.",FALSE,ch,ms,0,TO_ROOM);
      } else {
         send_to_char("Your hands are to full to unload.\n\r",ch);
         return;
      }
   }

   half_chop(argument, arg1, arg2);
   if (!*arg1) {
      send_to_char("You must specify the projectile to load!\n\r",ch);
      return;
   }
   ms = get_obj_in_list_vis(ch, arg1, ch->carrying); 
   if (!ms) {
      send_to_char("You don't seem to have that.\n\r",ch);
      return;
   }
   if (ms->obj_flags.type_flag!=ITEM_MISSILE) {
      act("$p is not a valid missile.",TRUE,ch,ms,0,TO_CHAR);
      return;
   }
   if (ms->obj_flags.value[3]!=fw->obj_flags.value[2]) {
      act("You can't load $p in that sort of weapon.",TRUE,ch,ms,0,TO_CHAR);
      return;
   }

   obj_from_char(ms);
   equip_char(ch,ms,LOADED_WEAPON);
   act("You load $p.",TRUE,ch,ms,0,TO_CHAR);
   act("$n loads $p.",FALSE,ch,ms,0,TO_ROOM);
   WAIT_STATE(ch, PULSE_VIOLENCE*3);
}

void do_fire(struct char_data *ch, char *argument, int cmd)
{
   struct obj_data *fw, *ms;
   char arg[MAX_STRING_LENGTH];
   char buf[MAX_STRING_LENGTH];
   char *keywords[] = {
        "north",
        "east",
        "south",
        "west",
        "up",
        "down",
        "\n"};
   struct char_data *targ;
   int tdir, rng, dr;


dlog("in do_fire");

   fw = ch->equipment[WIELD];
   if ((!fw)||(fw->obj_flags.type_flag!=ITEM_FIREWEAPON)) {
      send_to_char("You must be using a projectile weapon to fire things!\n\r",ch);
      return;
   }
   only_argument(argument, arg);

   if (!*arg) {
      send_to_char("The proper format for fire is: fire <target>\n\r",ch);
      return;
   }
   (char *)targ = get_char_linear(ch, arg, &rng, &dr);
        if (targ && targ == ch) {
          send_to_char("You can't fire that at yourself!\n\r",ch);
          return;
        }

   if (!targ) {
     send_to_char("You don't see anybody around meeting that description.\n\r",ch);
     return;
   } else {
	
	/* add spot check before letting mortals get father */
	
     if (rng > fw->obj_flags.value[1]) {
   	 send_to_char("You just can't fire that far!\n\r",ch);
	 return;
        }
        
      tdir = dr;
    }
   if (check_peaceful(targ, "Sorry but you can not fire a range weapon into a peaceful room.")) {

     send_to_char("Sorry but you can not fire a range weapon into a peaceful room.", ch);

     return;
   }

   if (ch->equipment[LOADED_WEAPON]) {
   ms = unequip_char(ch,LOADED_WEAPON); 
   } else {
      act("$p is not loaded!",TRUE,ch,fw,0,TO_CHAR);
      return;
   }

   send_to_char("OK.\n\r",ch);
   act("$n fires $p!",TRUE,ch,ms,0,TO_ROOM);
   throw_weapon(ms,tdir,targ,ch);

}
			


void do_throw(struct char_data *ch, char *argument, int cmd)
{
	struct obj_data *throw;
	char arg1[100],arg2[100];
        char *keywords[] = {
           "north",
           "east",
           "south",
           "west",
           "up",
           "down",
           "\n"};
	int i, rng, dr, tdir;
        struct char_data *targ;

dlog("in do_throw");

	half_chop(argument, arg1, arg2);
        if ((!*arg1)||(!*arg2)) {
           send_to_char("The correct format for 'throw' is: throw <object> <target>\n\r",ch);
           return;
        }
	throw = get_obj_in_list_vis(ch, arg1, ch->carrying);
        /* Check if second argument is a character or direction */
        (char *)targ = get_char_linear(ch, arg2, &rng, &dr);
        if (targ && targ == ch) {
          send_to_char("You can't throw that at yourself!\n\r",ch);
	  return;
	}
        if (!targ) {
           tdir = search_block(arg1, keywords, FALSE);   
           if (tdir==-1) {
              send_to_char("You don't see anybody around meeting that description.\n\r",ch);
              return;
           } else { 
           
		/* add spot check before letting mortals get father */
		
		if (rng > throw->obj_flags.value[1]) {
		    send_to_char("You just can't throw it that far!\n\r",ch);
		     return;
		   }
 		}
        } else {
           tdir = dr;
        }
	if (!throw) {
      		send_to_char("You don't have that!\n\r",ch); 
		return;
	}
	if (!IS_SET(throw->obj_flags.wear_flags,ITEM_THROW)) {
		send_to_char("You cant throw that!\n\r",ch);
		return;
	}
        if (throw->obj_flags.type_flag!=ITEM_WEAPON) {
           /* Friendly throw */
	   /* if (cmd!=263)  */
	      send_to_char("OK.\n\r",ch);
	   obj_from_char(throw);
	   /* if (cmd!=263) */
	      act("$n throws $p!",TRUE,ch,throw,0,TO_ROOM);
	   obj_to_room(throw,ch->in_room);
	   throw_object(throw,tdir,ch->in_room);
        } else {
           if (check_peaceful(targ,"Someone just tried to throw a weapon at you, and you are in peaceful room! HA!")) {
             send_to_char("You can not throw a weapon into a peaceful room!", ch);
             return;
           }
	   /* if (cmd!=263) */ 
	      send_to_char("OK.\n\r",ch);
	   obj_from_char(throw);
	  /*  if (cmd!=263) */
	     act("$n throws $p!",TRUE,ch,throw,0,TO_ROOM);
           throw_weapon(throw,tdir,targ,ch);
           /* if (cmd!=263) */
              WAIT_STATE(ch,1);
        }
}

