
#include <stdio.h>
#include <assert.h>

#include "protos.h"

/* Extern structures */
extern struct room_data *world;
extern struct obj_data  *object_list;
extern struct char_data *character_list;


SwitchStuff( struct char_data *giver, struct char_data *taker)
{
  struct obj_data *obj, *next;
  float ratio;
  int j;
  
  /*
   *  take all the stuff from the giver, put in on the
   *  taker
   */
  
  for (j = 0; j< MAX_WEAR; j++) {
    if (giver->equipment[j]) {
      obj = unequip_char(giver, j);
      obj_to_char(obj, taker);
    }
  }
  
  for (obj = giver->carrying; obj; obj = next) {
    next = obj->next_content;
    obj_from_char(obj);
    obj_to_char(obj, taker);
  }
  
  /*
   *    gold...
   */

  GET_GOLD(taker) = GET_GOLD(giver);
  
  /*
   *   hit point ratio
   */

   ratio = (float) GET_HIT(giver) / GET_MAX_HIT(giver);
   GET_HIT(taker) = ratio * GET_MAX_HIT(taker);

   GET_HIT(taker) = MIN(GET_MAX_HIT(taker), GET_HIT(taker));

  /*
   * experience
   */

  if (!IS_IMMORTAL(taker)) {
     if (!IS_IMMORTAL(giver))
        GET_EXP(taker) = GET_EXP(giver);

     GET_EXP(taker) = MIN(GET_EXP(taker), ABS_MAX_EXP);

  }

  /*
   *  humanoid monsters can cast spells
   */

  if (IS_NPC(taker)) {
    taker->player.class = giver->player.class;
    if (!taker->skills)
      SpaceForSkills(taker);
    for (j = 0; j< MAX_SKILLS; j++) {
      taker->skills[j].learned = giver->skills[j].learned;
      taker->skills[j].flags = giver->skills[j].flags;
      taker->skills[j].special= giver->skills[j].special;
      taker->skills[j].nummem = giver->skills[j].nummem;
    }
    for (j = MAGE_LEVEL_IND;j<MAX_CLASS;j++) {
      taker->player.level[j] = giver->player.level[j];
    }
  }

  GET_MANA(taker) = GET_MANA(giver);
  GET_ALIGNMENT(taker) = GET_ALIGNMENT(giver);

}


FailCharm(struct char_data *victim, struct char_data *ch)
{
 if (OnlyClass(ch,CLASS_MAGIC_USER|CLASS_SORCERER) && (number(1,100)>50) )
    return;  /* give single classed mages a break. */
    
  if (!IS_PC(victim)) { 

    AddHated(victim, ch);

    if (!victim->specials.fighting) {
      if (GET_POS(victim) > POSITION_SLEEPING)
	set_fighting(victim, ch);
      else if (number(0,1)) {
	set_fighting(victim, ch);
      }
    }
  } else {
    send_to_char("You feel charmed, but the feeling fades.\n\r",victim);
  }
}

FailSnare(struct char_data *victim, struct char_data *ch)
{
 if (OnlyClass(ch,CLASS_MAGIC_USER|CLASS_SORCERER) && (number(1,100)>50) )
    return;  /* give single classed mages a break. */

  if (!IS_PC(victim)) {
    if (!victim->specials.fighting) {
	AddHated(victim,ch); 
	set_fighting(victim, ch);
    } 
  } else {
    send_to_char("You feel ensnared, but the feeling fades.\n\r",victim);
  }
}

FailSleep(struct char_data *victim, struct char_data *ch)
{
 if (OnlyClass(ch,CLASS_MAGIC_USER|CLASS_SORCERER) && (number(1,100)>50) )
    return;  /* give single classed mages a break. */
  
  send_to_char("You feel sleepy for a moment,but then you recover\n\r",
	       victim);
  if (!IS_PC(victim)) {
    AddHated(victim, ch);
    if ((!victim->specials.fighting) && (GET_POS(victim)>POSITION_SLEEPING))
	set_fighting(victim, ch);
  }
}


FailPara(struct char_data *victim, struct char_data *ch)
{
 if (OnlyClass(ch,CLASS_MAGIC_USER|CLASS_SORCERER) && (number(1,100)>50) )
    return;  /* give single classed mages a break. */

  send_to_char("You feel frozen for a moment,but then you recover\n\r",victim);
  if (!IS_PC(victim)) {
    AddHated(victim, ch);
    if ((!victim->specials.fighting) && (GET_POS(victim) > POSITION_SLEEPING))
	set_fighting(victim, ch);
  }
}

FailCalm(struct char_data *victim, struct char_data *ch)
{
 if (OnlyClass(ch,CLASS_MAGIC_USER|CLASS_SORCERER) && (number(1,100)>50) )
    return;  /* give single classed mages a break. */

  send_to_char("You feel happy and easygoing, but the effect soon fades.\n\r",victim);
  if (!IS_PC(victim))
    if (!victim->specials.fighting) {
      if (!number(0,2))
	set_fighting(victim, ch);
    }
}


FailPoison(struct char_data *victim, struct char_data *ch)
{
 if (OnlyClass(ch,CLASS_MAGIC_USER|CLASS_SORCERER) && (number(1,100)>50) )
    return;  /* give single classed mages a break. */

  if (!IS_PC(victim)) 
  {
    AddHated(victim, ch);
    if (!victim->specials.fighting) 
    {
      if (GET_POS(victim) > POSITION_SLEEPING)
	set_fighting(victim, ch);
      else if (number(0,1)) 
      {
	set_fighting(victim, ch);
      }
    }
  } else 
  
  {
    send_to_char("You feel sick, but the feeling fades.\n\r",victim);
  }
}
