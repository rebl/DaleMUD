
#include <stdio.h>
#include <assert.h>

#include "protos.h"

/* Extern structures */
#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif
extern struct obj_data  *object_list;
extern struct char_data *character_list;
extern long SystemFlags;

/* For future use in blinding those with infravision who are fireballed
   or otherwise subjected to lotsa heat quickly in dark rooms. */

void heat_blind(struct char_data *ch)
{
  struct affected_type af;
  byte tmp;

  tmp=number(1,4);

  if(!ch)		/* Dunno if this does anything */
    return;

  if ( IS_AFFECTED(ch, AFF_BLIND) ) {
    return;			/* no affect */
  }
  else if  ((IS_DARK(ch->in_room)) && (!IS_IMMORTAL(ch)) &&
	    (!IS_AFFECTED(ch, AFF_TRUE_SIGHT)) && 
	    (IS_AFFECTED(ch, AFF_INFRAVISION)))  {
    send_to_char("Aaarrrggghhh!!  The heat blinds you!!\n\r", ch);
    af.type      = SPELL_BLINDNESS;
    af.location  = APPLY_HITROLL;
    af.modifier  = -4;  /* Make hitroll worse */
    af.duration  = tmp;
    af.bitvector = AFF_BLIND;
    affect_to_char(ch, &af);
  }
}
	   

/* Offensive Spells */

void spell_magic_missile(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(victim && ch);

if (level <0 || level >ABS_MAX_LVL)
	return;

  dam = dice((int)(level / 2)+1,4)+(level / 2)+1;

  if (affected_by_spell(victim,SPELL_SHIELD))
    dam = 0;

  MissileDamage(ch, victim, dam, SPELL_MAGIC_MISSILE);
}



void spell_chill_touch(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  int dam;

  assert(victim && ch);
if (level <0 || level >ABS_MAX_LVL)
	return;

  dam = number(level, 3*level);

  if ( !saves_spell(victim, SAVING_SPELL) )
  {
    af.type      = SPELL_CHILL_TOUCH;
    af.duration  = 6;
    af.modifier  = -1;
    af.location  = APPLY_STR;
    af.bitvector = 0;
    affect_join(victim, &af, TRUE, FALSE);
  } else {
    dam >>= 1;
  }
  damage(ch, victim, dam, SPELL_CHILL_TOUCH);
}

void spell_burning_hands(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
  struct char_data *tmp_victim, *temp;

  assert(ch);
if (level <0 || level >ABS_MAX_LVL)
	return;

  dam = dice(1,4) + level/2 + 1;

  send_to_char("Searing flame fans out in front of you!\n\r", ch);
  act("$n sends a fan of flame shooting from the fingertips!\n\r",
	  FALSE, ch, 0, 0, TO_ROOM);

  for ( tmp_victim = real_roomp(ch->in_room)->people; tmp_victim; 
       tmp_victim = temp ) {
    temp = tmp_victim->next_in_room;
    if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
      if ((GetMaxLevel(tmp_victim)>LOW_IMMORTAL) && (!IS_NPC(tmp_victim)))
	return;
      if (!in_group(ch, tmp_victim)) {
	act("You are seared by the burning flame!\n\r",
	    FALSE, ch, 0, tmp_victim, TO_VICT);
	heat_blind(tmp_victim);
	if ( saves_spell(tmp_victim, SAVING_SPELL) )
	  dam = 0;
	MissileDamage(ch, tmp_victim, dam, SPELL_BURNING_HANDS);
      } else {
	act("You are able to avoid the flames!\n\r",
	    FALSE, ch, 0, tmp_victim, TO_VICT);
	heat_blind(tmp_victim);
      }
    }
  }
}



void spell_shocking_grasp(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(victim && ch);
if (level <0 || level >ABS_MAX_LVL)
	return;

  dam = number(1,8)+level;

  if ((GET_HIT(victim) < -4) && IsHumanoid(victim) &&
      !IsUndead(victim)) {
    act ("$n utters the words 'clear', and touches $N's chest", 
	 FALSE,ch, 0, victim, TO_ROOM);
    GET_HIT(victim) += dam;
    update_pos(victim);
    return;
  }

  if (!HitOrMiss(ch, victim, CalcThaco(ch)))
    dam = 0;

  damage(ch, victim, dam, SPELL_SHOCKING_GRASP);
}



void spell_lightning_bolt(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(victim && ch);
if (level <0 || level >ABS_MAX_LVL)
	return;

  dam = dice(level,6);

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  MissileDamage(ch, victim, dam, SPELL_LIGHTNING_BOLT);
}



void spell_colour_spray(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  assert(victim && ch);
if (level <0 || level >ABS_MAX_LVL)
	return;

  dam = 4 * level;

  if ( saves_spell(victim, SAVING_SPELL) )
    dam >>= 1;

  MissileDamage(ch, victim, dam, SPELL_COLOUR_SPRAY);

}


/* Drain XP, MANA, HP - caster gains HP and MANA */
void spell_energy_drain(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
  int tmp;

  void set_title(struct char_data *ch);
  void gain_exp(struct char_data *ch, int gain);

  assert(victim && ch);
if (level <0 || level >ABS_MAX_LVL)
	return;

  if ( !saves_spell(victim, SAVING_SPELL) ) {
    GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch)-200);

 if (GetMaxLevel(victim) <= 1) {
      damage(ch, victim, 100, SPELL_ENERGY_DRAIN); /* Kill the sucker */
    } 
 
 else if ((!IS_NPC(victim)) && (GetMaxLevel(victim) >= LOW_IMMORTAL)) {
      send_to_char("Some puny mortal just tried to drain you...\n\r",victim);
    } 
 
 else {
   if (!IS_SET(victim->M_immune, IMM_DRAIN) ||
       (IS_SET(victim->immune, IMM_DRAIN) && !saves_spell(victim, SAVING_SPELL)) ) {

         send_to_char("Your life energy is drained!\n\r", victim);
	 dam = 1;
	 damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
	 if (!IS_NPC(victim)) {
	    drop_level(victim, BestClassBIT(victim),FALSE);
	    set_title(victim);
	  } else {
	    tmp = GET_MAX_HIT(victim)/GetMaxLevel(victim);
	    victim->points.max_hit -=tmp;
	    GET_HIT(victim) -= tmp;
	    tmp = GET_EXP(victim)/GetMaxLevel(victim);
	    GET_EXP(ch)+=tmp;
	    GET_EXP(victim)-=tmp;
	    victim->points.hitroll+=1;
	  }
      } else {
	 if (!IS_SET(ch->M_immune, IMM_DRAIN)) {
	    send_to_char("Your spell backfires!\n\r",ch);
	    dam = 1;
	    damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
	    if (!IS_NPC(ch)) {
	       drop_level(ch, BestClassBIT(ch),FALSE);
               set_title(ch);
	     } else {
	       tmp = GET_MAX_HIT(victim)/GetMaxLevel(victim);
	       victim->points.max_hit -=tmp;
	       GET_HIT(victim) -= tmp;
	       victim->points.hitroll+=1;
	       tmp = GET_EXP(victim)/GetMaxLevel(victim);
	       GET_EXP(ch)+=tmp;
	       GET_EXP(victim)-=tmp;
	     }
         } else {
	   send_to_char("Your spell fails utterly.\n\r",ch);
	 }
       }

     }
  } else {
    damage(ch, victim, 0, SPELL_ENERGY_DRAIN); /* Miss */
  }
}



void spell_fireball(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
  struct char_data *tmp_victim, *temp;

  assert(ch);
if (level <0 || level >ABS_MAX_LVL)
	return;

  dam = dice(level,8);
   for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) 
   {
      temp = tmp_victim->next;
      if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) 
      {
         if (!in_group(ch,tmp_victim) && !IS_IMMORTAL(tmp_victim)) 
         {
	   if ( saves_spell(tmp_victim, SAVING_SPELL) )
	     dam >>= 1;
	   heat_blind(tmp_victim);
	   MissileDamage(ch, tmp_victim, dam, SPELL_FIREBALL);
	 } else 
	 
	 {
            act("You dodge the mass of flame!!\n\r",
                 FALSE, ch, 0, tmp_victim, TO_VICT);
	    heat_blind(tmp_victim);
	 }
      } else 
      {
	 if (tmp_victim->in_room != NOWHERE) 
	 {
            if (real_roomp(ch->in_room)->zone == 
		real_roomp(tmp_victim->in_room)->zone) 
		{
                send_to_char("You feel a blast of hot air.\n\r", tmp_victim);
	    }
	 }
      }
   } 
}


void spell_earthquake(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;

  struct char_data *tmp_victim, *temp;

  assert(ch);
if (level <0 || level >ABS_MAX_LVL)
	return;

  dam =  dice(1,4) + level + 1;

  send_to_char("The earth trembles beneath your feet!\n\r", ch);
  act("$n makes the earth tremble and shiver",
	  FALSE, ch, 0, 0, TO_ROOM);

  for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next;
    if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
      if (!in_group(ch,tmp_victim) && !IS_IMMORTAL(tmp_victim)) {
	
	 if (GetMaxLevel(tmp_victim) > 4) {
	   act("You fall and hurt yourself!!\n\r",
	       FALSE, ch, 0, tmp_victim, TO_VICT);
	   MissileDamage(ch, tmp_victim, dam, SPELL_EARTHQUAKE);
	 } else {
	   act("You are sucked into a huge hole in the ground!", FALSE,
	       ch, 0, tmp_victim, TO_VICT);
	   act("$N is sucked into a huge hole in the ground!", FALSE,
	       ch, 0, tmp_victim, TO_NOTVICT);
	   MissileDamage(ch, tmp_victim, GET_MAX_HIT(tmp_victim)*12, 
			 SPELL_EARTHQUAKE);
	 }
       } else {
	 act("You almost fall and hurt yourself!!\n\r",
	     FALSE, ch, 0, tmp_victim, TO_VICT);
       }
    } else {
      if (real_roomp(ch->in_room)->zone == 
	  real_roomp(tmp_victim->in_room)->zone)
	send_to_char("The earth trembles...\n\r", tmp_victim);
    }
  } 
}



void spell_dispel_evil(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam=1;
  assert(ch && victim);
if (level <0 || level >ABS_MAX_LVL)
	return;
    
  
  if (IsExtraPlanar(victim)) {
    if (IS_EVIL(ch)) {
      victim = ch;
    } else {
      if (IS_GOOD(victim)) {
	act("Good protects $N.", FALSE, ch, 0, victim, TO_CHAR);
	return;
      }
    }
    if (!saves_spell(victim, SAVING_SPELL) ) {
      act("$n forces $N from this plane.", TRUE, ch, 0, victim, TO_ROOM);
      act("You force $N from this plane.", TRUE, ch, 0, victim, TO_CHAR);
      act("$n forces you from this plane.", TRUE, ch, 0, victim,TO_VICT);
      gain_exp(ch, GET_EXP(victim)/2);
      extract_char(victim);
    } else {
      act("$N resists the attack",TRUE, ch, 0, victim, TO_CHAR);
      act("you resist $n's attack.", TRUE, ch, 0, victim, TO_VICT);
      damage(ch, victim, dam, SPELL_EARTHQUAKE);
    }
  } else {
    act("$N laughs at you.", TRUE, ch, 0, victim, TO_CHAR);
    act("$N laughs at $n.", TRUE,ch, 0, victim, TO_NOTVICT);
    act("You laugh at $n.", TRUE,ch,0,victim,TO_VICT);
  }
}


void spell_call_lightning(byte level, struct char_data *ch,
			  struct char_data *victim, struct obj_data *obj)
{
  int dam;
  
  extern struct weather_data weather_info;
  
  assert(victim && ch);
if (level <0 || level >ABS_MAX_LVL)
	return;
  
  dam = dice( level+2, 8);
  
  if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
    
    if ( saves_spell(victim, SAVING_SPELL) )
      dam >>= 1;
    
    MissileDamage(ch, victim, dam, SPELL_CALL_LIGHTNING);
  } else {
    send_to_char("The proper atmospheric conditions are not at hand.\n\r", ch);
    return;
  } 
}



void spell_harm(byte level, struct char_data *ch,
		struct char_data *victim, struct obj_data *obj)
{
  int dam;
  
  assert(victim && ch);
if (level <0 || level >ABS_MAX_LVL)
	return;
  
  dam = GET_HIT(victim) - dice(1,4);
  
  if (dam < 0)
    dam = 100; /* Kill the suffering bastard */
  else {
    if (GET_RACE(ch) == RACE_GOD)
      dam = 0;
    if (!HitOrMiss(ch, victim, CalcThaco(ch)))
      dam = 0;
  }
  dam = MIN(dam, 100);
  
  damage(ch, victim, dam, SPELL_HARM);

  if (IS_PC(ch) && IS_PC(victim))
    GET_ALIGNMENT(ch)-=4;

}



/* spells2.c - Not directly offensive spells */

void spell_armor(byte level, struct char_data *ch,
		 struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  
  assert(victim);
if (level <0 || level >ABS_MAX_LVL)
	return;
  
  if (!affected_by_spell(victim, SPELL_ARMOR)) {
    af.type      = SPELL_ARMOR;
    af.duration  = 24;
    af.modifier  = -20;
    af.location  = APPLY_AC;
    af.bitvector = 0;
    
    affect_to_char(victim, &af);
    send_to_char("You feel someone protecting you.\n\r", victim);
  } else {
    send_to_char("Nothing New seems to happen\n\r", ch);
  }
}

#define ASTRAL_ENTRANCE   2701

void spell_astral_walk(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj)
{
  struct char_data *tmp, *tmp2;
  struct room_data *rp;

if (IS_SET(SystemFlags,SYS_NOASTRAL)) {
   send_to_char("The astral planes are shifting, you cannot!\n",ch);
   return;
 }

  rp = real_roomp(ch->in_room);
  
  for (tmp = rp->people;tmp;tmp=tmp2) {
    tmp2 = tmp->next_in_room;
if ( in_group(ch, tmp) && (!tmp->specials.fighting) &&
    (IS_PC(tmp)|| IS_SET(tmp->specials.act,ACT_POLYSELF)) &&
     IS_AFFECTED(tmp,AFF_GROUP)) {
      act("$n wavers, fades and dissappears", FALSE, tmp, 0, 0, TO_ROOM);
      char_from_room(tmp);
      char_to_room(tmp, ASTRAL_ENTRANCE);      
      do_look(tmp, "\0", 0);
    }
  }  
}

void spell_teleport(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int to_room, try = 0;
  extern int top_of_world;      /* ref to the top element of world */
  struct room_data *room;
  
  assert(ch && victim);
  
  if (victim != ch) {
    if (saves_spell(victim,SAVING_SPELL)) {
      send_to_char("Your spell has no effect.\n\r",ch);
      if (IS_NPC(victim)) {
	if (!victim->specials.fighting)
	  set_fighting(victim, ch);
      } else {
	send_to_char("You feel strange, but the effect fades.\n\r",victim);
      }
      return;
    } else {
      ch = victim;  /* the character (target) is now the victim */
    }
  }

  if (!IsOnPmp(victim->in_room)) {
    send_to_char("You're on an extra-dimensional plane!\n\r", ch);
    return;
  }

  
  do {
    to_room = number(0, top_of_world);
    room = real_roomp(to_room);
    if (room) {
      if ((IS_SET(room->room_flags, PRIVATE)) ||
	  (IS_SET(room->room_flags, TUNNEL)) ||
	  (IS_SET(room->room_flags, NO_SUM)) ||
	  (IS_SET(room->room_flags, NO_MAGIC)) ||
	  !IsOnPmp(to_room)) {
	room = 0;
        try++;
	}
    }
    
  } while (!room && try < 10);
  
  if (try >= 10) {
    send_to_char("The magic fails.\n\r", ch);
    return;
  }

  act("$n slowly fade out of existence.", FALSE, ch,0,0,TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, to_room);
  act("$n slowly fade in to existence.", FALSE, ch,0,0,TO_ROOM);
  
  do_look(ch, "", 0);
  
  if (IS_SET(real_roomp(to_room)->room_flags, DEATH) && 
      GetMaxLevel(ch) < LOW_IMMORTAL) {
    NailThisSucker(ch);
    return;
  }

  check_falling(ch);

}



void spell_bless(byte level, struct char_data *ch,
		 struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  
  assert(ch && (victim || obj));
if (level <0 || level >ABS_MAX_LVL)
	return;
  
  if (obj) {
    if ( (5*GET_LEVEL(ch,CLERIC_LEVEL_IND) > GET_OBJ_WEIGHT(obj)) &&
	(GET_POS(ch) != POSITION_FIGHTING) &&
	!IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_BLESS);
      act("$p briefly glows.",FALSE,ch,obj,0,TO_CHAR);
    }
  } else {
    
    if ((GET_POS(victim) != POSITION_FIGHTING) &&
	(!affected_by_spell(victim, SPELL_BLESS))) {
      
      send_to_char("You feel righteous.\n\r", victim);
      af.type      = SPELL_BLESS;
      af.duration  = 6;
      af.modifier  = 1;
      af.location  = APPLY_HITROLL;
      af.bitvector = 0;
      affect_to_char(victim, &af);
      
      af.location = APPLY_SAVING_SPELL;
      af.modifier = -1;                 /* Make better */
      affect_to_char(victim, &af);
    }
  }
}



void spell_blindness(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(ch && victim);
if (level <0 || level >ABS_MAX_LVL)
	return;


  if (saves_spell(victim, SAVING_SPELL) ||
	   affected_by_spell(victim, SPELL_BLINDNESS))
		return;

  act("$n seems to be blinded!", TRUE, victim, 0, 0, TO_ROOM);
  send_to_char("You have been blinded!\n\r", victim);

  af.type      = SPELL_BLINDNESS;
  af.location  = APPLY_HITROLL;
  af.modifier  = -4;  /* Make hitroll worse */
  af.duration  = level / 2;
  af.bitvector = AFF_BLIND;
  affect_to_char(victim, &af);


  af.location = APPLY_AC;
  af.modifier = +20; /* Make AC Worse! */
  affect_to_char(victim, &af);

  if ((!victim->specials.fighting)&&(victim!=ch))
     set_fighting(victim,ch);

}



void spell_clone(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{

  assert(ch && (victim || obj));
if (level <0 || level >ABS_MAX_LVL)
	return;

	send_to_char("Clone is not ready yet.", ch);

  if (obj) {

	} else {
		/* clone_char(victim); */
	}
}



void spell_control_weather(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
   /* Control Weather is not possible here!!! */
   /* Better/Worse can not be transferred     */
}



void spell_create_food(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct obj_data *tmp_obj;

  assert(ch);
if (level <0 || level >ABS_MAX_LVL)
	return;

  CREATE(tmp_obj, struct obj_data, 1);
  clear_object(tmp_obj);

  tmp_obj->name = (char *)strdup("mushroom");
  tmp_obj->short_description =  (char *)strdup("A Magic Mushroom");
  tmp_obj->description =  (char *)strdup("A really delicious looking magic mushroom lies here.");

  tmp_obj->obj_flags.type_flag = ITEM_FOOD;
  tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
  tmp_obj->obj_flags.value[0] = 5+level;
  tmp_obj->obj_flags.weight = 1;
  tmp_obj->obj_flags.cost = 10;
  tmp_obj->obj_flags.cost_per_day = 1;

  tmp_obj->next = object_list;
  object_list = tmp_obj;

  obj_to_room(tmp_obj,ch->in_room);

  tmp_obj->item_number = -1;

  act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
  act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_CHAR);
}



void spell_create_water(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int water;

  extern struct weather_data weather_info;
	void name_to_drinkcon(struct obj_data *obj,int type);
	void name_from_drinkcon(struct obj_data *obj);

  assert(ch && obj);

	if (GET_ITEM_TYPE(obj) == ITEM_DRINKCON) {
		if ((obj->obj_flags.value[2] != LIQ_WATER)
		     && (obj->obj_flags.value[1] != 0)) {

			name_from_drinkcon(obj);
			obj->obj_flags.value[2] = LIQ_SLIME;
			name_to_drinkcon(obj, LIQ_SLIME);

		} else {

			water = 2*level * ((weather_info.sky >= SKY_RAINING) ? 2 : 1);

			/* Calculate water it can contain, or water created */
			water = MIN(obj->obj_flags.value[0]-obj->obj_flags.value[1], water);

			if (water > 0) {
			  obj->obj_flags.value[2] = LIQ_WATER;
				obj->obj_flags.value[1] += water;

				weight_change_object(obj, water);

				name_from_drinkcon(obj);
				name_to_drinkcon(obj, LIQ_WATER);
				act("$p is partially filled.", FALSE, ch,obj,0,TO_CHAR);
			}
		}
	}
}



void spell_cure_blind(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{

  if (!victim || level <0 || level > ABS_MAX_LVL)
   return;

  if (IS_AFFECTED(victim, AFF_BLIND)) {
    REMOVE_BIT(victim->specials.affected_by, AFF_BLIND);
    send_to_char("Your vision returns!\n\r", victim);
  }

  if (affected_by_spell(victim, SPELL_BLINDNESS)) {
    affect_from_char(victim, SPELL_BLINDNESS);    
    send_to_char("Your vision returns!\n\r", victim);
  }
  if (IS_PC(ch) && IS_PC(victim))
    GET_ALIGNMENT(ch)+=1;

}



void spell_cure_critic(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int healpoints;

  assert(victim);
if (level <0 || level >ABS_MAX_LVL)
	return;

  healpoints = dice(3,8)+3;

  if ( (healpoints + GET_HIT(victim)) > hit_limit(victim) )
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += healpoints;

  send_to_char("You feel better!\n\r", victim);

  update_pos(victim);
/*
  if (IS_PC(ch) && IS_PC(victim))
    GET_ALIGNMENT(ch)+=3;
*/
}


void spell_cure_light(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int healpoints;

  assert(victim);
if (level <0 || level >ABS_MAX_LVL)
	return;

  healpoints = dice(1,8);

  if ( (healpoints + GET_HIT(victim)) > hit_limit(victim) )
    GET_HIT(victim) = hit_limit(victim);
  else
    GET_HIT(victim) += healpoints;

  send_to_char("You feel better!\n\r", victim);

  update_pos(victim);
/*
  if (IS_PC(ch) && IS_PC(victim))
    GET_ALIGNMENT(ch)+=1;
*/
}





void spell_curse(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim || obj);
if (level <0 || level >ABS_MAX_LVL)
	return;

  if (obj) {
    SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
    SET_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);

    /* LOWER ATTACK DICE BY -1 */
    if(obj->obj_flags.type_flag == ITEM_WEAPON)
      obj->obj_flags.value[2]--;
		act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
  } else {
    if ( saves_spell(victim, SAVING_SPELL) ||
		   affected_by_spell(victim, SPELL_CURSE))
      return;

    af.type      = SPELL_CURSE;
    af.duration  = 24*7;       /* 7 Days */
    af.modifier  = -1;
    af.location  = APPLY_HITROLL;
    af.bitvector = AFF_CURSE;
    affect_to_char(victim, &af);

    af.location = APPLY_SAVING_PARA;
    af.modifier = 1; /* Make worse */
    affect_to_char(victim, &af);

    act("$n briefly reveal a red aura!", FALSE, victim, 0, 0, TO_ROOM);
    act("You feel very uncomfortable.",FALSE,victim,0,0,TO_CHAR);
    if (IS_NPC(victim) && !victim->specials.fighting)
       set_fighting(victim,ch);

    if (IS_PC(ch) && IS_PC(victim))
      GET_ALIGNMENT(ch)-=2;

   }
}



void spell_detect_evil(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);
if (level <0 || level >ABS_MAX_LVL)
	return;
  
  if ( affected_by_spell(victim, SPELL_DETECT_EVIL) )
    return;
  
  af.type      = SPELL_DETECT_EVIL;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_EVIL;

  affect_to_char(victim, &af);

  act("$n's eyes briefly glow white", FALSE, victim, 0, 0, TO_ROOM);
  send_to_char("Your eyes tingle.\n\r", victim);
}



void spell_detect_invisibility(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(ch && victim);
if (level <0 || level >ABS_MAX_LVL)
	return;

  if ( affected_by_spell(victim, SPELL_DETECT_INVISIBLE) )
		return;

  af.type      = SPELL_DETECT_INVISIBLE;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_INVISIBLE;

  affect_to_char(victim, &af);
  act("$n's eyes briefly glow yellow", FALSE, victim, 0, 0, TO_ROOM);
  send_to_char("Your eyes tingle.\n\r", victim);
}



void spell_detect_magic(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);
if (level <0 || level >ABS_MAX_LVL)
	return;

  if ( affected_by_spell(victim, SPELL_DETECT_MAGIC) )
		return;

  af.type      = SPELL_DETECT_MAGIC;
  af.duration  = level*5;
  af.modifier  = 0;
  af.location  = APPLY_NONE;
  af.bitvector = AFF_DETECT_MAGIC;

  affect_to_char(victim, &af);
  send_to_char("Your eyes tingle.\n\r", victim);
}



#if 1
void spell_detect_poison(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	assert(ch && (victim || obj));

  if (victim) {
    if (victim == ch)
      if (IS_AFFECTED(victim, AFF_POISON))
        send_to_char("You can sense poison in your blood.\n\r", ch);
      else
        send_to_char("You feel healthy.\n\r", ch);
    else
      if (IS_AFFECTED(victim, AFF_POISON)) {
        act("You sense that $E is poisoned.",FALSE,ch,0,victim,TO_CHAR);
      } else {
    act("You don't find any poisons in $E's blood.",FALSE,ch,0,victim,TO_CHAR);
      }
  } else { /* It's an object */
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      if (obj->obj_flags.value[3])
        act("Poisonous fumes are revealed.",FALSE, ch, 0, 0, TO_CHAR);
      else
        send_to_char("It looks very delicious.\n\r", ch);
    }
  }
}
#else
void spell_detect_poison(byte level, struct char_data *ch, struct char_data *victim, struct obj_data *obj)
  {
  	assert(ch && (victim || obj));
  	
  	  if (victim) {
 if (victim == ch)
  if (IS_AFFECTED(victim, AFF_POISON))
      send_to_char("You can sense poison in your blood.\n\r", ch);
     else
       send_to_char("You cannot sense any poisons in your blood.\n\r", ch);
     else
   if (IS_AFFECTED(victim, AFF_POISON)) {
	act("You sense that $E is poisoned.",FALSE,ch,0,victim,TO_CHAR);
   } else {
    act("You don't find any poisons in $E's blood.",FALSE,ch,0,victim,TO_CHAR);
   }
 } else { /* It's an object */
  if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
      (obj->obj_flags.type_flag == ITEM_FOOD)) {
 if (obj->obj_flags.value[3])
  act("Poisonous fumes are revealed.",FALSE, ch, 0, 0, TO_CHAR);
 else
  send_to_char("You cannot find any poisons in it.\n\r", ch);
   }
}                                                                                                             }
#endif



void spell_enchant_weapon(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int i;
  int count=0;
  
  assert(ch && obj);
  assert(MAX_OBJ_AFFECT >= 2);
  
  if ((GET_ITEM_TYPE(obj) == ITEM_WEAPON) &&
      !IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {
    
    for (i=0; i < MAX_OBJ_AFFECT; i++) {
      if (obj->affected[i].location == APPLY_NONE) 
	count++;
      if (obj->affected[i].location == APPLY_HITNDAM ||
          obj->affected[i].location == APPLY_HITROLL ||
          obj->affected[i].location == APPLY_DAMROLL)
	return;
    }

    if (count < 2) return;
    /*  find the slots */
    i = getFreeAffSlot(obj);
    
    SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);
    
    obj->affected[i].location = APPLY_HITROLL;
    obj->affected[i].modifier = 1;
    if (level > 20)
      obj->affected[i].modifier += 1;
    if (level > 40)
      obj->affected[i].modifier += 1;
    if (level > MAX_MORT)
      obj->affected[i].modifier += 1;
    if (level == BIG_GUY)
      obj->affected[i].modifier += 1;
    
    i = getFreeAffSlot(obj);
    
    obj->affected[i].location = APPLY_DAMROLL;		
    obj->affected[i].modifier = 1;
    if (level > 15)
      obj->affected[i].modifier += 1;
    if (level > 30)
      obj->affected[i].modifier += 1;
    if (level > MAX_MORT)
      obj->affected[i].modifier += 1;
    if (level == BIG_GUY)
      obj->affected[i].modifier += 1;
    
    if (IS_GOOD(ch)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL|ITEM_ANTI_NEUTRAL);
      act("$p glows blue.",FALSE,ch,obj,0,TO_CHAR);
    } else if (IS_EVIL(ch)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD|ITEM_ANTI_NEUTRAL);
      act("$p glows red.",FALSE,ch,obj,0,TO_CHAR);
    } else {
      act("$p glows yellow.",FALSE,ch,obj,0,TO_CHAR);
      SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD|ITEM_ANTI_EVIL);
    }
  }
}


void spell_heal(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  assert(victim);

  spell_cure_blind(level, ch, victim, obj);

  GET_HIT(victim) += 100;

  if (GET_HIT(victim) >= hit_limit(victim))
    GET_HIT(victim) = hit_limit(victim)-dice(1,4);

  update_pos( victim );

  send_to_char("A warm feeling fills your body.\n\r", victim);

  if (IS_PC(ch) && IS_PC(victim))
    GET_ALIGNMENT(ch)+=5;

}


void spell_invisibility(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

	assert((ch && obj) || victim);

  if (obj) {
    if ( !IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE) ) {
			act("$p turns invisible.",FALSE,ch,obj,0,TO_CHAR);
			act("$p turns invisible.",TRUE,ch,obj,0,TO_ROOM);
      SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
		}
  } else {              /* Then it is a PC | NPC */
		if (!affected_by_spell(victim, SPELL_INVISIBLE)) {

		  act("$n slowly fades out of existence.", TRUE, victim,0,0,TO_ROOM);
  	  send_to_char("You vanish.\n\r", victim);

	    af.type      = SPELL_INVISIBLE;
    	af.duration  = 24;
    	af.modifier  = -40;
  	  af.location  = APPLY_AC;
	    af.bitvector = AFF_INVISIBLE;
  	  affect_to_char(victim, &af);
  	}
    }
}


#if 0
void spell_locate_object(byte level, struct char_data *ch,
			 struct char_data *victim, struct obj_data *obj)
{
  struct obj_data *i;
  char name[256], buf2[256];
  char buf[MAX_STRING_LENGTH];
  int j;

	assert(ch);

  if (!obj) {
    send_to_char("Everywhere, you sense them everywhere!??\n\r",ch);
    return;
  }

  if (!obj->name || !(*obj->name)) {
    send_to_char("Which object?\n\r", ch);
    return;
  }
    

  strcpy(name, obj->name);

  j=level>>1;
  sprintf(buf, "");
  for (i = object_list; i && (j>0); i = i->next)
    if (isname(name, i->name)) {
      if(i->carried_by) {
	if (strlen(PERS(i->carried_by, ch))>0
#if ZONE_LOCATE
	&& (real_roomp(ch->in_room)->zone == 
	    real_roomp(i->in_room)->zone  ) 
#endif
	    ) {
          sprintf(buf2,"%s carried by %s.\n\r",
            i->short_description,PERS(i->carried_by,ch));
          strcat(buf, buf2);
	}
      } else if(i->equipped_by) {
	if (strlen(PERS(i->equipped_by, ch))>0 
#if ZONE_LOCATE
	&& (real_roomp(ch->in_room)->zone == 
	    real_roomp(i->in_room)->zone  ) 
#endif
	) {
          sprintf(buf2,"%s equipped by %s.\n\r",
		  i->short_description,PERS(i->equipped_by,ch));
          strcat(buf, buf2);
	}
      } else if (i->in_obj 
#if ZONE_LOCATE
	&& (real_roomp(ch->in_room)->zone == 
	    real_roomp(i->in_room)->zone  ) 
#endif
	) {
          sprintf(buf2,"%s in %s.\n\r",i->short_description,
            i->in_obj->short_description);
          strcat(buf, buf2);
      } else 
#if ZONE_LOCATE      
      if (IS_IMMORTAL(ch)	  	||
          real_roomp(ch->in_room)->zone == 
          real_roomp(i->in_room)->zone ) {
#endif                	  
          sprintf(buf2,"%s in %s.\n\r",i->short_description,
		(i->in_room == NOWHERE ? "use but uncertain." : real_roomp(i->in_room)->name));
          strcat(buf, buf2);
         j--;
#if ZONE_LOCATE
      }      
#endif   
   
    }

  page_string(ch->desc, buf, 0);

  if(j==0)
    send_to_char("You are very confused.\n\r",ch);
  if(j==level>>1)
    send_to_char("No such object.\n\r",ch);
}
#else
void spell_locate_object(byte level, struct char_data *ch,
			 struct char_data *victim, struct obj_data *obj)
{
  struct obj_data *i;
  char name[256];
  char buf[MAX_STRING_LENGTH],buf2[256];
  int j;

	assert(ch);

  if (!obj) {
    send_to_char("Everywhere, you sense them everywhere!??\n\r",ch);
    return;
  }

  if (!obj->name || !(*obj->name)) {
    send_to_char("Which object?\n\r", ch);
    return;
  }
    

  strcpy(name, obj->name);

	j=level>>2;
        if(j<2) j=2;
 
        sprintf(buf,"");
        
	for (i = object_list; i && (j>0); i = i->next)
    if (isname(name, i->name)) {
      if(i->carried_by) {
	if (strlen(PERS(i->carried_by, ch))>0) {
          sprintf(buf2,"%s carried by %s.\n\r",
            i->short_description,PERS(i->carried_by,ch));
          strcat(buf,buf2);
	}
      } else if(i->equipped_by) {
	if (strlen(PERS(i->equipped_by, ch))>0) {
          sprintf(buf2,"%s equipped by %s.\n\r",
		  i->short_description,PERS(i->equipped_by,ch));
	strcat(buf,buf2);
	}
      } else if (i->in_obj) {
          sprintf(buf2,"%s in %s.\n\r",i->short_description,
            i->in_obj->short_description);
	strcat(buf,buf2);
      } else {
          sprintf(buf2,"%s in %s.\n\r",i->short_description,
		(i->in_room == NOWHERE ? "use but uncertain." : real_roomp(i->in_room)->name));
	strcat(buf,buf2);
         j--;
      }      
    }

	page_string(ch->desc,buf,0);
	
  if(j==0)
    send_to_char("You are very confused.\n\r",ch);
  if(j==level>>1)
    send_to_char("No such object.\n\r",ch);
}


#endif


void spell_poison(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	assert(victim || obj);

  if (victim) 
  {

   if (IS_SET(victim->M_immune,IMM_POISON)) /* should fix imm_poison bug */
    {					    /* find better place later   */
     send_to_char("Forces unknown help you resist the poison!\n\r",victim);
     return;
    }
    
    if (IS_NPC(ch)) 
    {
     if (!IS_SET(ch->specials.act, ACT_DEADLY)) {
      if(!ImpSaveSpell(victim, SAVING_PARA, 0))  {
       af.type = SPELL_POISON;
       af.duration = level*2;
       af.modifier = -2;
       af.location = APPLY_STR;
       af.bitvector = AFF_POISON;
       affect_join(victim, &af, FALSE, FALSE);
       send_to_char("You feel very sick.\n\r", victim);
       if (!victim->specials.fighting)
       {
	 AddHated(victim, ch);
	 set_fighting(victim, ch);
	}  /* !v */
     } /* !save */  else 
     {
       FailPoison(victim,ch);
       return;
     } /* ! ACT_D */
     
    } /* !NPC */ else 
    {           
      if (!ImpSaveSpell(victim, SAVING_PARA, 0)) 
      {
	act("Deadly poison fills your veins.",TRUE, ch, 0, 0, TO_CHAR);
	damage(victim, victim, MAX(10, GET_HIT(victim)*2), SPELL_POISON);
	AddHated(victim, ch);
      } /* Save */ else 
      {
	FailPoison(victim,ch);
	return;
      }  /* !save */
      
    } /* end PC */
   
   } /* ! vic */ else 
   
   {  
      if(!ImpSaveSpell(victim, SAVING_PARA, 0))    
      {
       af.type = SPELL_POISON;
       af.duration = level*2;
       af.modifier = -2;
       af.location = APPLY_STR;
       af.bitvector = AFF_POISON;
       affect_join(victim, &af, FALSE, FALSE);

       send_to_char("You feel very sick suddenly.\n\r", victim);     
       if (!victim->specials.fighting && !IS_PC(victim))
       {
	 AddHated(victim, ch);
	 set_fighting(victim, ch);
	} 
      } else {
      	FailPoison(victim,ch);
      }
    }
    
  } else { /* Object poison */
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      obj->obj_flags.value[3] = 1;
    }
  }
}


void spell_protection_from_evil(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

	assert(victim);

  if (!affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL) ) {
    af.type      = SPELL_PROTECT_FROM_EVIL;
    af.duration  = 24;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_PROTECT_FROM_EVIL;
    affect_to_char(victim, &af);
    send_to_char("You have a righteous feeling!\n\r", victim);
  }
}

void spell_protection_from_evil_group(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  struct char_data *dude;

  for (dude=real_roomp(ch->in_room)->people;dude;dude=dude->next_in_room)
  if(IS_FOLLOWING(ch,dude)) {
  if (!affected_by_spell(dude, SPELL_PROTECT_FROM_EVIL) ) {
    af.type      = SPELL_PROTECT_FROM_EVIL;
    af.duration  = 24;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_PROTECT_FROM_EVIL;
    affect_to_char(dude, &af);
    send_to_char("You have a righteous feeling!\n\r", dude);
   }
  }
}


void spell_remove_curse(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
int i, found=FALSE;

  assert(ch && (victim || obj));

  if (obj) {
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
      act("$p briefly glows blue.", TRUE, ch, obj, 0, TO_CHAR);
      act("$p, held by $n, briefly glows blue.", TRUE, ch, obj, 0, TO_ROOM);
      REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
    }
  } else 
  {      /* Then it is a PC | NPC */
    if (affected_by_spell(victim, SPELL_CURSE) ) {
       act("$n briefly glows red, then blue.",FALSE,victim,0,0,TO_ROOM);
       act("You feel better.",FALSE,victim,0,0,TO_CHAR);
       affect_from_char(victim, SPELL_CURSE);
      } else { 				
    				/* uncurse items equiped by a person */
     i=0;
     do {
       if (victim->equipment[i]) 

         if (IS_SET(victim->equipment[i]->obj_flags.extra_flags, ITEM_NODROP)) {
                spell_remove_curse(level,victim,NULL,victim->equipment[i]);
                found=TRUE;
             } 
          i++;
         } while (i<MAX_WEAR && !found);
	}    
	
    if (IS_PC(ch) && IS_PC(victim))
      GET_ALIGNMENT(ch)+=2;
  }
}




void spell_remove_poison(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{

  assert(ch && (victim || obj));

  if (victim) {
    if(affected_by_spell(victim,SPELL_POISON)) {
      affect_from_char(victim,SPELL_POISON);
      act("A warm feeling runs through your body.",FALSE,victim,0,0,TO_CHAR);
      act("$N looks better.",FALSE,ch,0,victim,TO_ROOM);
    }
  } else {
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      obj->obj_flags.value[3] = 0;
      act("The $p steams briefly.",FALSE,ch,obj,0,TO_CHAR);
    }
  }
}



void spell_fireshield(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if (!affected_by_spell(victim, SPELL_FIRESHIELD) ) {

    act("$n is surrounded by a glowing red aura.",TRUE,victim,0,0,TO_ROOM);
		act("You start glowing red.",TRUE,victim,0,0,TO_CHAR);

    af.type      = SPELL_FIRESHIELD;
    af.duration  = (level<LOW_IMMORTAL) ? 3 : level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_FIRESHIELD;
    affect_to_char(victim, &af);
  }
}

void spell_sanctuary(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if ((!affected_by_spell(victim, SPELL_SANCTUARY)) && 
      (!IS_AFFECTED(victim, AFF_SANCTUARY))) {

    act("$n is surrounded by a white aura.",TRUE,victim,0,0,TO_ROOM);
		act("You start glowing.",TRUE,victim,0,0,TO_CHAR);

    af.type      = SPELL_SANCTUARY;
    af.duration  = (level<LOW_IMMORTAL) ? 3 : level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char(victim, &af);
  }
}



void spell_sleep(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

	assert(victim);

  if (IsImmune(victim, IMM_SLEEP)) {
    FailSleep(victim, ch);
    return;
  }
  if (IsResist(victim, IMM_SLEEP)) {
    if (saves_spell(victim, SAVING_SPELL)) {
       FailSleep(victim, ch);
       return;
     }
    if (saves_spell(victim, SAVING_SPELL)) {
       FailSleep(victim, ch);
       return;
     }
  } else if (!IsSusc(victim, IMM_SLEEP)) {
    if (saves_spell(victim, SAVING_SPELL)) {
       FailSleep(victim, ch);
       return;
     }    
  }

    af.type      = SPELL_SLEEP;
    af.duration  = 4+level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SLEEP;
    affect_join(victim, &af, FALSE, FALSE);

    if (GET_POS(victim)>POSITION_SLEEPING)    {
      act("You feel very sleepy ..... zzzzzz",FALSE,victim,0,0,TO_CHAR);
      act("$n go to sleep.",TRUE,victim,0,0,TO_ROOM);
	    GET_POS(victim)=POSITION_SLEEPING;
    }
}



void spell_strength(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  assert(victim);


  if (!affected_by_spell(victim,SPELL_STRENGTH)) {
     act("You feel stronger.", FALSE, victim,0,0,TO_CHAR);
     act("$n seems stronger!\n\r",
	  FALSE, victim, 0, 0, TO_ROOM);
     af.type      = SPELL_STRENGTH;
     af.duration  = 2*level;
     if (IS_NPC(victim))
        if (level >= CREATOR) {
	  af.modifier = 25 - GET_STR(victim);
        } else
        af.modifier = number(1,6);
     else {

       if (HasClass(ch, CLASS_WARRIOR) || HasClass(ch,CLASS_BARBARIAN) ) 
           af.modifier = number(1,8);
       else if (HasClass(ch, CLASS_CLERIC) ||
		HasClass(ch, CLASS_THIEF))
           af.modifier = number(1,6);
       else 
	 af.modifier = number(1,4);
     }
     af.location  = APPLY_STR;
     af.bitvector = 0;
     affect_to_char(victim, &af);
   } else {

  act("Nothing seems to happen.", FALSE, ch,0,0,TO_CHAR);

  }
}



void spell_ventriloquate(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	/* Not possible!! No argument! */
}



void spell_word_of_recall(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  extern int top_of_world;
  int location;

  void do_look(struct char_data *ch, char *argument, int cmd);

  assert(victim);

  if (IS_NPC(victim))
     return;

  /*  loc_nr = GET_HOME(ch); */

  if (victim->player.hometown) {
    location = victim->player.hometown;
  } else {
    location = 3001;
  }

  if (!real_roomp(location))    {
    send_to_char("You are completely lost.\n\r", victim);
    location = 0;
    return;
  }

if (victim->specials.fighting) {
	send_to_char("HAH, not in a fight!\n\r",ch);
	return;
}

  if (!IsOnPmp(victim->in_room)) {
    send_to_char("You can't recall!, you're on a different plane!\n\r", 
		 victim);
    return;
  }

	/* a location has been found. */

  act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, location);
  act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
  do_look(victim, "",15);

}


void spell_summon(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct char_data *tmp;
  struct room_data *rp;
  int count;

  assert(ch && victim);

  if (victim->in_room <= NOWHERE) {
    send_to_char("Couldn't find any of those.\n\r", ch);
    return;
  }

  if ((rp = real_roomp(ch->in_room)) == NULL)
    return;

  if (IS_SET(rp->room_flags, NO_SUM) || IS_SET(rp->room_flags, NO_MAGIC)) {
    send_to_char("Eldritch wizardry obstructs thee.\n\r", ch);
    return;
  }

  if (IS_SET(rp->room_flags, TUNNEL)) {
    send_to_char("There is no room in here to summon!\n\r", ch);
    return;
  }

  if ((rp->sector_type == SECT_AIR) || rp->sector_type == SECT_UNDERWATER) {
    send_to_char("Strange powers block your summons\n", ch);
    return;
  }

  if (check_peaceful(ch, "Ancient powers obstruct thy magik\n"))
    return;

  if (check_peaceful(victim, "")) {
    send_to_char("You cannot get past the magical defenses.\n\r", ch);
    return;
  }

  if (IS_SET(real_roomp(victim->in_room)->room_flags, NO_SUM)) {
    send_to_char("Ancient Magiks bar your path.\n\r", ch);
    return;
  }

  if (GetMaxLevel(victim) > LOW_IMMORTAL) {
    send_to_char("A large hand suddenly appears before you and thumps your head!\n\r", ch);
    return;
  }

  if (victim->specials.fighting) {
    send_to_char("You can't get a clear fix on them\n", ch);
    return;
  }

if (IS_SET(SystemFlags,SYS_NOSUMMON)) {
   send_to_char("A mistical fog blocks your attemps!\n",ch);
  return;
 }

  if (!IsOnPmp(victim->in_room)) {
    send_to_char("They're on an extra-dimensional plane!\n", ch);
    return;
  }

    if (CanFightEachOther(ch,victim)) 
      if (saves_spell(victim, SAVING_SPELL) ) {
          act("You failed to summon $N!",FALSE,ch,0,victim,TO_CHAR);
          act("$n tried to summon you!",FALSE,ch,0,victim,TO_VICT);          
          return;
         }
  
  if (!IS_PC(victim)) {
      count = 0;
      for (tmp=real_roomp(victim->in_room)->people; 
	   tmp; tmp = tmp->next_in_room) {
	count++;
      }
      
      if (count==0) {
	send_to_char("You failed.\n\r", ch);
	return;
      } else {
        count = number(0,count);
	
        for (tmp=real_roomp(victim->in_room)->people; 
	     count && tmp;
	     tmp = tmp->next_in_room, count--)
	  ;
	
 if ( (tmp && GET_MAX_HIT(tmp) < GET_HIT(ch) && 
 	!saves_spell(tmp,SAVING_SPELL)) ) {
	  RawSummon(tmp, ch);
        } else {
	  send_to_char("You failed\n\r", ch);
	  return;
	}
      }
    } else {
      RawSummon(victim, ch);
    }

}


void RawSummon( struct char_data *v, struct char_data *c)
{
  long   target;
  struct char_data *tmp;
  struct obj_data *o, *n;
  int    j, i;
  extern char EasySummon;
  char buf[400];  

/* this section run if the mob is above 3 levels above the caster */
/* destroys the mobs EQ */

  if (IS_NPC(v) && (!IS_SET(v->specials.act, ACT_POLYSELF)) &&
      (GetMaxLevel(v) > GetMaxLevel(c)+3)) 
{ 
    act("$N struggles, and all of $S items are destroyed!", TRUE, c, 0, v, TO_CHAR);
    /* remove objects from victim */
    for (j = 0; j < MAX_WEAR; j++) 
{
      if (v->equipment[j]) {
         o = unequip_char(v, j);
	 extract_obj(o);
       }
    }
    for (o = v->carrying; o; o = n) 
{
      n = o->next_content;
      obj_from_char(o);
      extract_obj(o);
    }
    AddHated(v, c);
  } else 
{
    if (!EasySummon) 
{
      send_to_char("A wave of nausea overcomes you.  You collapse!\n\r",c);
      WAIT_STATE(c, PULSE_VIOLENCE*6);
      GET_POS(c) = POSITION_STUNNED;
    }
  }

  act("$n disappears suddenly.",TRUE,v,0,0,TO_ROOM);
  target = c->in_room;
  char_from_room(v);
  char_to_room(v,target);
  
  act("$n arrives suddenly.",TRUE,v,0,0,TO_ROOM);
  
  sprintf(buf, "%s has summoned you!\n\r", (IS_NPC(c)?c->player.short_descr:GET_NAME(c)));
  send_to_char(buf, v);
  do_look(v,"",15);

    if (IS_NPC(v) && !(IS_SET(v->specials.act,ACT_POLYSELF)) &&
	((IS_SET(v->specials.act, ACT_AGGRESSIVE) ||
	 (IS_SET(v->specials.act, ACT_META_AGG)))))  {
	 if (CAN_SEE(v,c)) {
      		act("$n growls at you", 1, v, 0, c, TO_VICT);
      		act("$n growls at $N", 1, v, 0, c, TO_NOTVICT);
	   	hit(v, c, TYPE_UNDEFINED);
	     }
        }
  
}


void spell_charm_person(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  void add_follower(struct char_data *ch, struct char_data *leader);
  bool circle_follow(struct char_data *ch, struct char_data *victim);
  void stop_follower(struct char_data *ch);

  assert(ch && victim);

  if (victim == ch) {
    send_to_char("You like yourself even better!\n\r", ch);
    return;
  }
  
  if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
    if (circle_follow(victim, ch)) {
      send_to_char("Sorry, following in circles can not be allowed.\n\r", ch);
      return;
    }
    
    if (!IsPerson(victim)) {
      send_to_char("Umm,  that's not a person....\n\r",ch);
      return;
    }
    
    
    if (GetMaxLevel(victim) > GetMaxLevel(ch)+3) {
      FailCharm(victim, ch);
      return;
    }
    
    if (too_many_followers(ch)) {
      act("$N takes one look at the size of your posse and justs says no!",
	  TRUE, ch, ch->equipment[WIELD], victim, TO_CHAR);
      act("$N takes one look at the size of $n's posse and just says no!",
	  TRUE, ch, ch->equipment[WIELD], victim, TO_ROOM);
      return;
    }

    if (IsImmune(victim, IMM_CHARM) || (WeaponImmune(victim))) {
      FailCharm(victim,ch);
      return;
    }

    if (IsResist(victim, IMM_CHARM)) {
      if (saves_spell(victim, SAVING_PARA)) {
	FailCharm(victim,ch);
	return;
      }
      
      if (saves_spell(victim, SAVING_PARA)) {
	FailCharm(victim,ch);
	return;
      }
    } else {
      if (!IsSusc(victim, IMM_CHARM)) {
	if (saves_spell(victim, SAVING_PARA)) {
	  FailCharm(victim,ch);
	  return;
	}
      }
    }

    if (victim->master)
      stop_follower(victim);
    
    add_follower(victim, ch);
    
    af.type      = SPELL_CHARM_PERSON;
    
    if (GET_CHR(ch))
      af.duration  = follow_time(ch);
    else
      af.duration  = 24*18;

    if (IS_GOOD(victim) && IS_GOOD(ch))
      af.duration *= 2;
    if (IS_EVIL(victim) && IS_EVIL(ch))
      af.duration  += af.duration >> 1;

    
    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);
    
    act("Isn't $n just such a nice fellow?",FALSE,ch,0,victim,TO_VICT);

    if (!IS_PC(ch)) {
      REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
      SET_BIT(victim->specials.act, ACT_SENTINEL);
    }
  }
}



void spell_charm_monster(byte level, struct char_data *ch,
			 struct char_data *victim, struct obj_data *obj)
{
  char buf[MAX_INPUT_LENGTH];
  struct affected_type af;
  
  void add_follower(struct char_data *ch, struct char_data *leader);
  bool circle_follow(struct char_data *ch, struct char_data *victim);
  void stop_follower(struct char_data *ch);
  
  assert(ch && victim);

  if (victim == ch) {
    send_to_char("You like yourself even better!\n\r", ch);
    return;
  }

  if (IsVeggie(victim)) {
    send_to_char("You can't charm a plant-creature!\n\r", ch);
    return;
  }

  if (GetMaxLevel(victim) > GetMaxLevel(ch)+3) {
    FailCharm(victim, ch);
    return;
  }

  if (too_many_followers(ch)) {
    act("$N takes one look at the size of your posse and justs says no!",
	TRUE, ch, 0, victim, TO_CHAR);
    act("$N takes one look at the size of $n's posse and just says no!",
	TRUE, ch, 0, victim, TO_ROOM);
    return;
  }
  
  if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
    if (circle_follow(victim, ch)) {
      send_to_char("Sorry, following in circles can not be allowed.\n\r", ch);
      return;
    }
      if (IsImmune(victim, IMM_CHARM) || (WeaponImmune(victim))) {
          FailCharm(victim,ch);
       	  return;
      }
      if (IsResist(victim, IMM_CHARM)) {
         if (saves_spell(victim, SAVING_PARA)) {
          FailCharm(victim,ch);
       	  return;
	 }

         if (saves_spell(victim, SAVING_PARA)) {
          FailCharm(victim,ch);
       	  return;
	 }
       } else {
          if (!IsSusc(victim, IMM_CHARM)) {
	     if (saves_spell(victim, SAVING_PARA)) {
	        FailCharm(victim,ch);
		return;
	     }
	  }
       }
    
    if (victim->master)
      stop_follower(victim);
    
    add_follower(victim, ch);
    
    af.type      = SPELL_CHARM_PERSON;
    
    if (GET_CHR(ch))
      af.duration  = follow_time(ch);
    else
      af.duration  = 24*18;

    if (IS_GOOD(victim) && IS_GOOD(ch))
      af.duration *= 2;
    if (IS_EVIL(victim) && IS_EVIL(ch))
      af.duration  += af.duration >> 1;

    
    af.modifier  = 0;
    af.location  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);
    
    act("Isn't $n just such a nice fellow?",FALSE,ch,0,victim,TO_VICT);

    if (!IS_PC(ch)) {
      REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
      SET_BIT(victim->specials.act, ACT_SENTINEL);
    }

  }
}


void spell_sense_life(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

	assert(victim);

  if (!affected_by_spell(victim, SPELL_SENSE_LIFE))  {
 if (ch != victim) {
    act("$n waves $s hands around $N's eyes.", FALSE, ch, 0, victim, TO_NOTVICT);
    act("You help $N see hidden life forms.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n waves $s hands around your eyes, you feel your senses improve.", FALSE, ch, 0, victim, TO_VICT);
  } else {
    act("$n eyes flash briefly.", FALSE, ch, 0, victim, TO_NOTVICT);
    act("You feel your senses improve.", FALSE, ch, 0, victim, TO_CHAR);
  }

    af.type      = SPELL_SENSE_LIFE;
    af.duration  = 5*level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SENSE_LIFE;
    affect_to_char(victim, &af);
  } else {
   act("$N can already sense hidden life forms.",FALSE,ch,0,victim,TO_CHAR);
  }

}

void spell_globe_minor_inv(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

	assert(victim);

  if (!affected_by_spell(victim, SPELL_GLOBE_MINOR_INV)) {
 if (ch != victim) {
    act("$n summons a small globe of protection about $N", FALSE, ch, 0, victim, TO_NOTVICT);
    act("You summon a small globe of protection about $N", FALSE, ch, 0, victim, TO_CHAR);
    act("$n summons a small globe of protection about you", FALSE, ch, 0, victim, TO_VICT);
  } else {
    act("$n summons a small globe of protection about $mself", FALSE, ch, 0, victim, TO_NOTVICT);
    act("You summon a small globe of protection about yourself", FALSE, ch, 0, victim, TO_CHAR);
  }

    af.type      = SPELL_GLOBE_MINOR_INV;
    af.duration  = (level<LOW_IMMORTAL) ? level/10 : level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char(victim, &af);
  } else {
  if (ch != victim) 
   send_to_char("As you cast the spell you realize that they are already protected.\n\r",ch);  
   else
   send_to_char("As you cast the spell you remember that you are already protected.\n\r",ch);
  }
}

void spell_globe_major_inv(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

	assert(victim);

  if (!affected_by_spell(victim, SPELL_GLOBE_MAJOR_INV)) {
 if (ch != victim) {
    act("$n summons a globe of protection about $N", FALSE, ch, 0, victim, TO_NOTVICT);
    act("You summon a globe of protection about $N", FALSE, ch, 0, victim, TO_CHAR);
    act("$n summons a globe of protection about you", FALSE, ch, 0, victim, TO_VICT);
  } else {
    act("$n summons a globe of protection about $mself", FALSE, ch, 0, victim, TO_NOTVICT);
    act("You summon a globe of protection about yourself", FALSE, ch, 0, victim, TO_CHAR);
  }

    af.type      = SPELL_GLOBE_MAJOR_INV;
    af.duration  = (level<LOW_IMMORTAL) ? level/10 : level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char(victim, &af);
  } else {
  if (ch != victim) 
   send_to_char("As you cast the spell you realize that they are already protected.\n\r",ch);  
   else
   send_to_char("As you cast the spell you remember that you are already protected.\n\r",ch);
  }

}

void spell_anti_magic_shell(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

	if (!victim)
		return;

  if (!affected_by_spell(victim, SPELL_ANTI_MAGIC_SHELL)) {
    send_to_char("You errect a anti-magic globe about yourself.\n\r", ch);
    spell_dispel_magic(IMPLEMENTOR,ch,ch,0);
    
    af.type      = SPELL_ANTI_MAGIC_SHELL;
    af.duration  = (level<LOW_IMMORTAL) ? 1 : level;  				/* one tic only! */
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char(victim, &af);
  } else
    send_to_char("You are already protected by a anti-magic shell.\n\r",ch);

}


void spell_prismatic_spray(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
int dam;
struct char_data *tmp_victim;

  assert(ch);
if (level <0 || level >ABS_MAX_LVL)
	return;

/* spell effects
  red 1 40 dam 
  orange  2 80
  yellow 3 100
  green 4 poison
  blue 5 petrify
  indigo  6 feeble
  violet  7 teleport
*/   

  send_to_char("You send a rainbow of prismatic spray from you hands.\n\r",ch);
  act("$n sends a rainbow of prismatic spray from $s hands!",FALSE,ch,0,0,TO_ROOM);
  
   for (tmp_victim=real_roomp(ch->in_room)->people;
        tmp_victim ;tmp_victim=tmp_victim->next_in_room)
      if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)){
	if (!in_group(ch,tmp_victim) && !IS_IMMORTAL(tmp_victim)) {
	      switch(number(1,7)) {
	         case 1:dam=40;
	               if (!saves_spell(tmp_victim, SAVING_SPELL)) {
	                act("$N is hit by a red shaft of light!",FALSE,ch,0,tmp_victim,TO_ROOM);
	                MissileDamage(ch,tmp_victim,dam,SPELL_PRISMATIC_SPRAY);
	               }
	               break;
	         case 2:dam=80;
	               if (!saves_spell(tmp_victim, SAVING_SPELL)) {
	                act("$N is hit by a orange shaft of light!",FALSE,ch,0,tmp_victim,TO_ROOM);
     		        MissileDamage(ch,tmp_victim,dam,SPELL_PRISMATIC_SPRAY);
     		        }
	               	break;
	         case 3:dam=100;
	               if (!saves_spell(tmp_victim, SAVING_SPELL)) {
	                act("$N is hit by a yellow shaft of light!",FALSE,ch,0,tmp_victim,TO_ROOM);
	         	MissileDamage(ch,tmp_victim,dam,SPELL_PRISMATIC_SPRAY);
	         	}
	               	break;
					/* these have internal saves already */
	         case 4:act("$N is hit by a green shaft of light!",FALSE,ch,0,tmp_victim,TO_ROOM);
	         	cast_poison(level,ch,"",SPELL_TYPE_SPELL,tmp_victim,NULL); 
	         	break;
	         case 5:act("$N is hit by a blue shaft of light!",FALSE,ch,0,tmp_victim,TO_ROOM);
	         	cast_paralyze(level,ch,"",SPELL_TYPE_SPELL,tmp_victim,NULL);
	         	break;
	         case 6:act("$N is hit by a indigo shaft of light!",FALSE,ch,0,tmp_victim,TO_ROOM);
	                cast_feeblemind(level,ch,"",SPELL_TYPE_SPELL,tmp_victim,NULL);
	         	break;
	         case 7:act("$N is hit by a violet shaft of light!",FALSE,ch,0,tmp_victim,TO_ROOM);
	         	cast_teleport(level,ch,"",SPELL_TYPE_SPELL,tmp_victim,NULL);
	         	break;
	          default:	/* should never get here */
	          	dam=100;
     		        MissileDamage(ch,tmp_victim,dam,SPELL_PRISMATIC_SPRAY);
	               	break;
	        } /* end switch */
	        if (!tmp_victim->specials.fighting &&     /* make them hate the caster */
	                       ch->specials.fighting != tmp_victim) {
		     set_fighting(ch,tmp_victim);
	        }
           } else
            act("You avoid the colorful spray!\n\r",FALSE, ch, 0, tmp_victim, TO_VICT);	 
	}           

} /* end pris */

void spell_incendiary_cloud(byte level, struct char_data *ch,struct char_data *victim, struct obj_data *obj)
{
  int dam;
  struct char_data *tmp_victim, *temp;

  assert(ch);
if (level <0 || level >ABS_MAX_LVL)
	return;

  dam = dice(level,10);

  send_to_char("Billowing clouds of incendiary gases pour from your fingertips.\n\r",ch);
  act("Billowing clouds of incendiary gases pour from $n's fingertips!",FALSE,ch,0,0,TO_ROOM);
  
   for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) 
   {
      temp = tmp_victim->next;
      if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)){
	if (!in_group(ch,tmp_victim) && !IS_IMMORTAL(tmp_victim)) 
	{
	   if (!saves_spell(tmp_victim, SAVING_SPELL))	      	   {
	      heat_blind(tmp_victim);
	      MissileDamage(ch,tmp_victim,dam,SPELL_INCENDIARY_CLOUD);
	        /* damage here */
	    } else           {
	     dam >>=1;  /* half dam */
	     heat_blind(tmp_victim);
	     MissileDamage(ch,tmp_victim,dam,SPELL_INCENDIARY_CLOUD);
	  }
	 } else
           act("You avoid the smouldering gases!\n\r",FALSE, ch, 0, tmp_victim, TO_VICT);	 
	}           
    }	  /* end for */

}


void spell_comp_languages(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;
  char buf[128];

	if (!victim) 
	   return;

 if (!affected_by_spell(victim, SPELL_COMP_LANGUAGES)) {

 if (ch != victim) {
    act("$n gently touches $N's ears.", FALSE, ch, 0, victim, TO_NOTVICT);
    act("You touch $N's ears gently.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n gently touches your ears, wow you have missed so much!", FALSE, ch, 0, victim, TO_VICT);
  } else {
    act("$n touches $s own ears.", FALSE, ch, 0, victim, TO_NOTVICT);
    act("You touch your ears, wow you missed so much!", FALSE, ch, 0, victim, TO_CHAR);
  }
    af.type      = SPELL_COMP_LANGUAGES;
    af.duration  = (level<LOW_IMMORTAL) ? (int)level/2 : level;  				/* one tic only! */
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char(victim, &af);
  } 
  
    else {
  if (ch != victim) {
    sprintf(buf,"$N can already understand languages.");    
   } else {
    sprintf(buf,"You can already understand languages.");    
   }
   act(buf,FALSE,ch,0,victim,TO_CHAR);
  }

}


/* ***************************************************************************
 *                     Not cast-able spells                                  *
 * ************************************************************************* */


void sprintbit(unsigned long, char *[], char *);

void spell_identify(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  char buf[256], buf2[256];
  int i;
  bool found;
  
  struct time_info_data age(struct char_data *ch);
  
  /* Spell Names */
  extern char *spells[];
  
  /* For Objects */
  extern char *item_types[];
  extern char *extra_bits[];
  extern char *apply_types[];
  extern char *affected_bits[];
  extern char *affected_bits2[];
  extern char *immunity_names[];
  
  
  assert(ch && (obj || victim));
  
  if (obj) {
    send_to_char("You feel informed:\n\r", ch);
    
    sprintf(buf, "Object '%s', Item type: ", obj->name);
    sprinttype(GET_ITEM_TYPE(obj),item_types,buf2);
    strcat(buf,buf2); strcat(buf,"\n\r");
    send_to_char(buf, ch);
    
    if (obj->obj_flags.bitvector) {
      send_to_char("Item will give you following abilities:  ", ch);
      sprintbit((unsigned)obj->obj_flags.bitvector,affected_bits,buf);
      strcat(buf,"\n\r");
      send_to_char(buf, ch);
    }
    
    send_to_char("Item is: ", ch);
    sprintbit( (unsigned)obj->obj_flags.extra_flags,extra_bits,buf);
    strcat(buf,"\n\r");
    send_to_char(buf,ch);
    
    sprintf(buf,"Weight: %d, Value: %d, Rent cost: %d  %s\n\r",
	    obj->obj_flags.weight, obj->obj_flags.cost, obj->obj_flags.cost_per_day, obj->obj_flags.cost_per_day>LIM_ITEM_COST_MIN?"[RARE]":" ");
    send_to_char(buf, ch);

    
    switch (GET_ITEM_TYPE(obj)) {
      
    case ITEM_SCROLL : 
    case ITEM_POTION :
      sprintf(buf, "Level %d spells of:\n\r",	obj->obj_flags.value[0]);
      send_to_char(buf, ch);
      if (obj->obj_flags.value[1] >= 1) {
	sprinttype(obj->obj_flags.value[1]-1,spells,buf);
	strcat(buf,"\n\r");
	send_to_char(buf, ch);
      }
      if (obj->obj_flags.value[2] >= 1) {
	sprinttype(obj->obj_flags.value[2]-1,spells,buf);
	strcat(buf,"\n\r");
	send_to_char(buf, ch);
      }
      if (obj->obj_flags.value[3] >= 1) {
	sprinttype(obj->obj_flags.value[3]-1,spells,buf);
	strcat(buf,"\n\r");
	send_to_char(buf, ch);
      }
      break;
      
    case ITEM_WAND : 
    case ITEM_STAFF : 
      sprintf(buf, "Has %d chages, with %d charges left.\n\r",
	      obj->obj_flags.value[1],
	      obj->obj_flags.value[2]);
      send_to_char(buf, ch);
      
      sprintf(buf, "Level %d spell of:\n\r",	obj->obj_flags.value[0]);
      send_to_char(buf, ch);
      
      if (obj->obj_flags.value[3] >= 1) {
	sprinttype(obj->obj_flags.value[3]-1,spells,buf);
	strcat(buf,"\n\r");
	send_to_char(buf, ch);
      }
      break;
      
    case ITEM_WEAPON :
      sprintf(buf, "Damage Dice is '%dD%d'\n\r",
	      obj->obj_flags.value[1],
	      obj->obj_flags.value[2]);
      send_to_char(buf, ch);
      break;
      
    case ITEM_ARMOR :
      sprintf(buf, "AC-apply is %d\n\r",
	      obj->obj_flags.value[0]);
      send_to_char(buf, ch);
      break;
      
    }
    
    found = FALSE;
    
    for (i=0;i<MAX_OBJ_AFFECT;i++) {
      if ((obj->affected[i].location != APPLY_NONE) &&
	  (obj->affected[i].modifier != 0)) {
	if (!found) {
	  send_to_char("Can affect you as :\n\r", ch);
	  found = TRUE;
	}
	
	sprinttype(obj->affected[i].location,apply_types,buf2);
	sprintf(buf,"    Affects : %s By ", buf2);
	send_to_char(buf,ch);
	switch(obj->affected[i].location) {
	case APPLY_M_IMMUNE:
	case APPLY_IMMUNE:
	case APPLY_SUSC:
	   sprintbit(obj->affected[i].modifier,immunity_names,buf2);
	   strcat(buf2,"\n\r");
	   break;
	case APPLY_ATTACKS:
	   sprintf(buf2,"%f\n\r", obj->affected[i].modifier/10);
	   break;
        case APPLY_WEAPON_SPELL:
	case APPLY_EAT_SPELL:
	   sprintf(buf2,"%s\n\r", spells[obj->affected[i].modifier-1]);
	   break;
	case APPLY_SPELL:
	   sprintbit(obj->affected[i].modifier,affected_bits, buf2);
	   strcat(buf2,"\n\r");
	   break;

	 default:
	   sprintf(buf2,"%d\n\r", obj->affected[i].modifier);
	   break;
	}
	send_to_char(buf2,ch);
      }
    }
    
  } else {       /* victim */
    
    if (!IS_NPC(victim)) {
      struct time_info_data ma;

      age2(victim, &ma);
      sprintf(buf,"%d Years,  %d Months,  %d Days,  %d Hours old.\n\r",
	      ma.year, ma.month,
	      ma.day, ma.hours);
      send_to_char(buf,ch);
      
      sprintf(buf,"Height %dcm  Weight %dpounds \n\r",
	      GET_HEIGHT(victim), GET_WEIGHT(victim));
      send_to_char(buf,ch);
      
      sprintf(buf,"Armor Class %d\n\r",victim->points.armor);
      send_to_char(buf,ch);

      if (level > 30) {

	sprintf(buf,"Str %d/%d, Int %d, Wis %d, Dex %d, Con %d, Ch %d\n\r",
	GET_STR(victim), GET_ADD(victim),
	GET_INT(victim),
	GET_WIS(victim),
	GET_DEX(victim),
	GET_CON(victim),
	GET_CHR(victim));
	send_to_char(buf,ch);
      }
    } else {
      send_to_char("You learn nothing new.\n\r", ch);
    }
  }
 
if (GetMaxLevel(ch)<LOW_IMMORTAL) {
  act("You are overcome by a wave of exhaustion.",FALSE,ch,0,0,TO_CHAR); 
  act("$n slumps to the ground, exhausted.",FALSE,ch,0,0,TO_ROOM);
  WAIT_STATE(ch,PULSE_VIOLENCE*12);
  GET_POS(ch) = POSITION_STUNNED; 
 }

}





void spell_enchant_armor(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int i;
  int count=0;
  
  assert(ch && obj);
  assert(MAX_OBJ_AFFECT >= 2);
  
  if ((GET_ITEM_TYPE(obj) == ITEM_ARMOR) &&
      !IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {
    
    for (i=0; i < MAX_OBJ_AFFECT; i++) {
      if (obj->affected[i].location == APPLY_NONE) 
	count++;
      if (obj->affected[i].location == APPLY_ARMOR ||
          obj->affected[i].location == APPLY_SAVE_ALL ||
          obj->affected[i].location == APPLY_SAVING_PARA ||
          obj->affected[i].location == APPLY_SAVING_ROD ||
          obj->affected[i].location == APPLY_SAVING_PETRI ||
          obj->affected[i].location == APPLY_SAVING_BREATH ||
          obj->affected[i].location == APPLY_SAVING_SPELL ||
          obj->affected[i].location == APPLY_SAVE_ALL) {
	send_to_char("This item may not hold further enchantments.\n\r", ch);
	return;
      }
    }

    if (count < 2) {
      send_to_char("This item may not be enchanted.\n\r", ch);
      return;
    }
    /*  find the slots */
    i = getFreeAffSlot(obj);
    
    SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);
    
    obj->affected[i].location = APPLY_ARMOR;
    obj->affected[i].modifier = -1;
    if (level > 20)
      obj->affected[i].modifier -= 1;
    if (level > 40)
      obj->affected[i].modifier -= 1;
    if (level > MAX_MORT)
      obj->affected[i].modifier -= 1;
    if (level >= BIG_GUY)
      obj->affected[i].modifier -= 1;
    
    i = getFreeAffSlot(obj);
    
    obj->affected[i].location = APPLY_SAVE_ALL;		
    obj->affected[i].modifier = 0;
    if (level > 30)
      obj->affected[i].modifier -= 1;
    if (level > MAX_MORT)
      obj->affected[i].modifier -= 1;
    if (level >= BIG_GUY)
      obj->affected[i].modifier -= 1;
    
    if (IS_GOOD(ch)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL|ITEM_ANTI_NEUTRAL);
      act("$p glows blue.",FALSE,ch,obj,0,TO_CHAR);
    } else if (IS_EVIL(ch)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD|ITEM_ANTI_NEUTRAL);
      act("$p glows red.",FALSE,ch,obj,0,TO_CHAR);
    } else {
      act("$p glows yellow.",FALSE,ch,obj,0,TO_CHAR);
      SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD|ITEM_ANTI_EVIL);
    }
  }
}


/* ***************************************************************************
 *                     NPC spells..                                          *
 * ************************************************************************* */

void spell_fire_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	int hpch;
	struct obj_data *burn;

	assert(victim && ch);
if (level <0 || level >ABS_MAX_LVL)
	return;

	hpch = GET_MAX_HIT(ch);
	hpch *= level;
	hpch /= GetMaxLevel(ch);
	if(hpch<10) hpch=10;

	dam = hpch;

	if ( saves_spell(victim, SAVING_BREATH) )
		dam >>= 1;

	MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH);

	/* And now for the damage on inventory */

/*
  DamageStuff(victim, FIRE_DAMAGE);
*/

       	for (burn=victim->carrying ; 
	     burn && (burn->obj_flags.type_flag!=ITEM_SCROLL) && 
	    (burn->obj_flags.type_flag!=ITEM_WAND) &&
	    (burn->obj_flags.type_flag!=ITEM_STAFF) &&
	    (burn->obj_flags.type_flag!=ITEM_BOAT);
	     burn=burn->next_content) {
	     if (!saves_spell(victim, SAVING_BREATH) ) 	{
       		if (burn)  {
       			act("$o burns",0,victim,burn,0,TO_CHAR);
       			extract_obj(burn);
       		}
	     }
	}
}


void spell_frost_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	int hpch;
	struct obj_data *frozen;

	assert(victim && ch);
if (level <0 || level >ABS_MAX_LVL)
	return;

	hpch = GET_MAX_HIT(ch);
	hpch *= level;
	hpch /= GetMaxLevel(ch);
	if(hpch<10) hpch=10;

	dam = hpch;

	if ( saves_spell(victim, SAVING_BREATH) )
		dam >>= 1;

	MissileDamage(ch, victim, dam, SPELL_FROST_BREATH);

	/* And now for the damage on inventory */

       	for (frozen=victim->carrying ; 
       	    frozen && (frozen->obj_flags.type_flag!=ITEM_DRINKCON) && 
	    (frozen->obj_flags.type_flag!=ITEM_POTION);
	    frozen=frozen->next_content) {

       	    if (!saves_spell(victim, SAVING_BREATH) ) {
       	      if (frozen) {
		    act("$o shatters.",0,victim,frozen,0,TO_CHAR);
		    extract_obj(frozen);
	      }
	    }
	}
}


void spell_acid_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
  int hpch;
  
  int apply_ac(struct char_data *ch, int eq_pos);
  
  assert(victim && ch);
if (level <0 || level >ABS_MAX_LVL)
	return;
  
  hpch = GET_MAX_HIT(ch);
  hpch *= level;
  hpch /= GetMaxLevel(ch);
  if(hpch<10) hpch=10;
  
  dam = hpch;
  
  if ( saves_spell(victim, SAVING_BREATH) )
    dam >>= 1;
  
  MissileDamage(ch, victim, dam, SPELL_ACID_BREATH);

}


void spell_gas_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	int hpch;

	assert(victim && ch);
if (level <0 || level >ABS_MAX_LVL)
	return;

	hpch = GET_MAX_HIT(ch);
	hpch *= level;
	hpch /= GetMaxLevel(ch);
	if(hpch<10) hpch=10;

	dam = hpch;

	if ( saves_spell(victim, SAVING_BREATH) )
		dam >>= 1;

	MissileDamage(ch, victim, dam, SPELL_GAS_BREATH);


}


void spell_lightning_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	int hpch;

if (!victim || !ch) { /* 	assert(victim && ch); */
	log("!ch || !victim in breath_lightning, magic.c");
	return;
}

if (level <0 || level >ABS_MAX_LVL)
	return;

	hpch = GET_MAX_HIT(ch);
	hpch *= level;
	hpch /= GetMaxLevel(ch);
	if(hpch<10) hpch=10;

	dam = hpch;

	if ( saves_spell(victim, SAVING_BREATH) )
		dam >>= 1;

	MissileDamage(ch, victim, dam, SPELL_LIGHTNING_BREATH);


}


void spell_wizard_eye(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if (!affected_by_spell(victim, SPELL_WIZARDEYE) ) {
    act("$n summons a disembodied eye to scout ahead",TRUE,victim,0,0,TO_ROOM);
    act("You summon a disembodied eye to scout ahead.",TRUE,victim,0,0,TO_CHAR);

    af.type      = SPELL_WIZARDEYE;
    af.duration  = (level<LOW_IMMORTAL) ? 3 : level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SCRYING;
    affect_to_char(victim, &af);
  } else {
  	if (ch != victim)
  	   act("$N already uses a wizard eye.",FALSE,ch,0,victim,TO_CHAR);
  	    else
  	   act("You already have a wizard eye.",FALSE,ch,0,victim,TO_CHAR);
    }
  
}


void spell_disintergrate(byte level, struct char_data *ch,  struct char_data *victim, struct obj_data *obj)
{
int i,damage, found=FALSE;
struct obj_data *x;

if (!ch) {
    log("!ch in spell_disintergrate");
    return;
   }
   
if (!victim) {
   log("!victim in spell_disintergrate");
   return;
  }

damage = dice(level,10);
  if ( !saves_spell(victim, SAVING_SPELL) )       {

/* frag thier EQ */

       i=0;
       do {	/* could make this check the carried EQ as well... */
         if (victim->equipment[i]) 
         {
      obj=victim->equipment[i];
   if (!ItemSave(obj,SPELL_DISINTERGRATE))     {
      act("$p turns red hot, $N screams, then it disappears in a puff of smoke!", TRUE, ch, obj, victim, TO_CHAR);
   if (obj->equipped_by || obj->carried_by)      
      act("$p, held by $N, disappears in a puff of smoke!", TRUE, ch, obj, victim, TO_ROOM);
   if (obj->carried_by) {		/* remove the obj */
           obj_from_char(obj);
  	 } else 
   if (obj->equipped_by)   	 {
          obj = unequip_char(obj->equipped_by, obj->eq_pos);
  	 } else
   if (obj->in_obj) 	{
          obj_from_obj(obj);
          obj_to_room(obj,ch->in_room);
        } else
   if (obj->contains)         {
         while (obj->contains) {
         x = obj->contains;
         obj_from_obj(x);
         obj_to_room(x, ch->in_room);
       } /* end while */
      }  /* end contains */
        
    if (obj)
       extract_obj(obj);
    }  else    /* saved */     {
   if (obj) {
       act("$c0010$p resists the disintergration ray completely!", TRUE, ch, obj, victim, TO_VICT);
       act("$c0010$p carried by $N, resists $n's disintergration ray!", TRUE, ch, obj, victim, TO_ROOM);
      }
    } /* end saved obj */
    
   }
 i++;
 } while (i<MAX_WEAR);

      } else {	/* we saved ! 1/2 dam and no EQ frag */
	  damage>>=1; 
      }
   MissileDamage(ch, victim, damage, SPELL_DISINTERGRATE);      

    if (IS_PC(ch) && IS_PC(victim))
      GET_ALIGNMENT(ch)-=2;
}
