
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "protos.h"


#define DUAL_WIELD(ch) (ch->equipment[WIELD] && ch->equipment[HOLD]&&\
			ITEM_TYPE(ch->equipment[WIELD])==ITEM_WEAPON && \
			ITEM_TYPE(ch->equipment[HOLD])==ITEM_WEAPON)


/* Structures */


struct char_data *combat_list = 0;   /* head of l-list of fighting chars    */
struct char_data *missile_list = 0;   /* head of l-list of fighting chars    */
struct char_data *combat_next_dude = 0; /* Next dude global trick           */
struct char_data *missile_next_dude = 0; /* Next dude global trick           */
struct zone_data *zone_table;         /* table of reset data             */

char PeacefulWorks = 1;  /* set in @set */
char DestroyedItems;  /* set in MakeScraps */

/* External structures */
#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct obj_data  *object_list;
extern struct index_data *mob_index;
extern struct char_data *character_list;
extern struct spell_info_type spell_info[];
extern struct spell_info_type spell_info[MAX_SPL_LIST];
extern char *spells[];
extern char *ItemDamType[];
extern int ItemSaveThrows[22][5];
extern struct str_app_type str_app[];
extern int WizLock;
extern struct descriptor_data *descriptor_list;
extern struct title_type titles[MAX_CLASS][ABS_MAX_LVL];
extern struct int_app_type int_app[26];
extern struct wis_app_type wis_app[26];
extern char *room_bits[];
extern int thaco[MAX_CLASS][ABS_MAX_LVL];
int can_see_linear(struct char_data *ch, struct char_data *targ, int *rng, int *dr) ;
  
int BarbarianToHitMagicBonus ( struct char_data *ch);
int berserkthaco ( struct char_data *ch);
int berserkdambonus ( struct char_data *ch, int dam);
long ExpCaps(int group_count, long total);
long GroupLevelRatioExp(struct char_data *ch,int group_max_level,long experincepoints);

char *replace_string(char *str, char *weapon, char *weapon_s,
			        char *location_hit, char *location_hit_s);

 /* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
{
  {"hit",    "hits"},            /* TYPE_HIT      */
  {"pound",  "pounds"},          /* TYPE_BLUDGEON */
  {"pierce", "pierces"},         /* TYPE_PIERCE   */
  {"slash",  "slashes"},         /* TYPE_SLASH    */
  {"whip",   "whips"},           /* TYPE_WHIP     */
  {"claw",   "claws"},           /* TYPE_CLAW     */
  {"bite",   "bites"},           /* TYPE_BITE     */
  {"sting",  "stings"},          /* TYPE_STING    */
  {"crush",  "crushes"},         /* TYPE_CRUSH    */
  {"cleave", "cleaves"},
  {"stab",   "stabs"},
  {"smash",  "smashes"},
  {"smite",  "smites"},
  {"blast",  "blasts"},
  {"strike","strikes"}		/* type RANGE_WEAPON */
};

 /* Location of attack texts */
struct attack_hit_type location_hit_text[] =
{
  {"in $S body","body",},		/* 0 */
  {"in $S left leg",  "left leg"},	/* 1 */
  {"in $S right leg", "right leg"},	/* 2 */
  {"in $S left arm",  "left arm"},	/* 3 */      
  {"in $S right arm", "right arm"},	/* 4 */     
  {"in $S shoulder",  "shoulder"}, 	/* 5 */     
  {"in $S neck",      "neck"},     	/* 6 */     
  {"in $S left foot", "left foot"},	/* 7 */       
  {"in $S right foot","right foot"},	/* 8 */      
  {"in $S left hand", "left hand"}, 	/* 9 */       
  {"in $S right hand","right hand"},	/* 10 */
  {"in $S chest",     "chest"},		/* 11 */
  {"in $S back",      "back"},		/* 12 */
  {"in $S stomach",   "stomach"},	/* 13 */
  {"in $S head",      "head"}		/* 14 */
};




/* The Fight related routines */


void appear(struct char_data *ch)
{
  act("$n suddenly appears!", FALSE, ch,0,0,TO_ROOM);
  
  if (affected_by_spell(ch, SPELL_INVISIBLE))
    affect_from_char(ch, SPELL_INVISIBLE);

  if (affected_by_spell(ch, SPELL_INVIS_TO_ANIMALS))
    affect_from_char(ch, SPELL_INVIS_TO_ANIMALS);
  
  REMOVE_BIT(ch->specials.affected_by, AFF_INVISIBLE);
}



int LevelMod(struct char_data *ch, struct char_data *v, int exp)
{
  float ratio=0.0;
  float fexp;

  ratio = (float)GET_AVE_LEVEL(v)/GET_AVE_LEVEL(ch);

  if (ratio < 1.0)  {
    fexp = ratio*exp;
  } else {
    fexp = exp;
  }

  return((int)fexp);

}


int RatioExp( struct char_data *ch, struct char_data *victim, int total)
{
  if (!IS_SET(victim->specials.act, ACT_AGGRESSIVE) &&
      !IS_SET(victim->specials.act, ACT_META_AGG) &&
      !IS_AFFECTED(victim, AFF_CHARM))
    if (GetMaxLevel(ch) > 20)
      total = LevelMod(ch, victim, total);

  if ((IS_SET(victim->specials.act, ACT_AGGRESSIVE) ||
      IS_SET(victim->specials.act, ACT_META_AGG)) &&
      !IS_AFFECTED(victim, AFF_CHARM)) {
    /* make sure that poly mages don't abuse, by reducing their bonus */
    if (IS_NPC(ch)) {
      total *=3;
      total/=4;
    }
  }

  return(total);
}



void load_messages()
{
  FILE *f1;
  int i,type;
  struct message_type *messages;
  char chk[100];
  
  if (!(f1 = fopen(MESS_FILE, "r"))){
    perror("read messages");
    assert(0);
  }
  
  /*
    find the memset way of doing this...
    */
  
  for (i = 0; i < MAX_MESSAGES; i++)	{ 
    fight_messages[i].a_type = 0;
    fight_messages[i].number_of_attacks=0;
    fight_messages[i].msg = 0;
  }
  
  fscanf(f1, " %s \n", chk);
  
  i = 0;
  
  while(*chk == 'M')	{
    fscanf(f1," %d\n", &type);
    
    if(i>=MAX_MESSAGES){
      log("Too many combat messages.");
      exit(0);
    }
    
    CREATE(messages,struct message_type,1);
    fight_messages[i].number_of_attacks++;
    fight_messages[i].a_type=type;
    messages->next=fight_messages[i].msg;
    fight_messages[i].msg=messages;
    
    messages->die_msg.attacker_msg      = fread_string(f1);
    messages->die_msg.victim_msg        = fread_string(f1);
    messages->die_msg.room_msg          = fread_string(f1);
    messages->miss_msg.attacker_msg     = fread_string(f1);
    messages->miss_msg.victim_msg       = fread_string(f1);
    messages->miss_msg.room_msg         = fread_string(f1);
    messages->hit_msg.attacker_msg      = fread_string(f1);
    messages->hit_msg.victim_msg        = fread_string(f1);
    messages->hit_msg.room_msg          = fread_string(f1);
    messages->god_msg.attacker_msg      = fread_string(f1);
    messages->god_msg.victim_msg        = fread_string(f1);
    messages->god_msg.room_msg          = fread_string(f1);
    fscanf(f1, " %s \n", chk);
    i++;
  }
  
  fclose(f1);
}


void update_pos( struct char_data *victim )
{
  
  if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POSITION_STUNNED)) {
    return;
  } else if (GET_HIT(victim) > 0 ) {
    if (!IS_AFFECTED(victim, AFF_PARALYSIS)) {
      if (!MOUNTED(victim))
	GET_POS(victim) = POSITION_STANDING;
      else 
	GET_POS(victim) == POSITION_MOUNTED;
    } else {
      GET_POS(victim) = POSITION_STUNNED;
    }
  } else if (GET_HIT(victim) <= -11) {
    GET_POS(victim) = POSITION_DEAD;
  } else if (GET_HIT(victim) <= -6) {
    GET_POS(victim) = POSITION_MORTALLYW;
  } else if (GET_HIT(victim) <= -3) {
    GET_POS(victim) = POSITION_INCAP;
  } else {
    GET_POS(victim) = POSITION_STUNNED;
  }
}


int check_peaceful(struct char_data *ch, char *msg)
{
  struct room_data *rp;

  extern char PeacefulWorks;
  
  if (!PeacefulWorks) return(0);

if (!ch) 
  return(FALSE);
  
  rp = real_roomp(ch->in_room);
  if (rp && rp->room_flags&PEACEFUL) {
    send_to_char(msg, ch);
    return 1;
  }
  return 0;
}

/* start one char fighting another */
void set_fighting(struct char_data *ch, struct char_data *vict)
{
char buf[128];
  
  if (ch->specials.fighting) {
    log("Fighting character set to fighting another.");
    return;
  }
  
  if (vict->attackers <= 5) {
    vict->attackers+=1;
  } else {
    log("more than 6 people attacking one target");
  }
  ch->next_fighting = combat_list;
  combat_list = ch;
  
  if(IS_AFFECTED(ch,AFF_SLEEP))
    affect_from_char(ch,SPELL_SLEEP);

/* if memorizing, distirb it and remove it */

    if (affected_by_spell(ch,SKILL_MEMORIZE)) 
        affect_from_char(ch,SKILL_MEMORIZE);

/* same here */

    if (affected_by_spell(ch,SKILL_MEDITATE)) 
        affect_from_char(ch,SKILL_MEDITATE);

    
  ch->specials.fighting = vict;
  GET_POS(ch) = POSITION_FIGHTING;
  
/* player has lost link and mob is killing him, force PC to flee */
 if (IS_LINKDEAD(ch)) {
     do_flee(ch,"\0",0);
    }  

if (IS_LINKDEAD(vict)) {
     do_flee(vict,"\0",0);
    }  
  
}



/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data *ch)
{
struct char_data *tmp,tch;
  
  if (!ch->specials.fighting) {
    char buf[300];
    sprintf(buf, "%s not fighting at invocation of stop_fighting",
	    GET_NAME(ch));
    return;
  }
  
  ch->specials.fighting->attackers-=1;
  
  if (ch->specials.fighting->attackers < 0)   {
    log("too few people attacking");
    ch->specials.fighting->attackers = 0;
  }
  
  if (IS_SET(ch->specials.affected_by2,AFF2_BERSERK))    {
      REMOVE_BIT(ch->specials.affected_by2,AFF2_BERSERK);
      act("$n seems calm down!", FALSE, ch,0,0,TO_ROOM);
      act("You calm down.", FALSE,ch, 0, 0, TO_CHAR);
     }

     
  if (ch == combat_next_dude)
    combat_next_dude = ch->next_fighting;
  
  if (combat_list == ch)
    combat_list = ch->next_fighting;
  else	{
    for (tmp = combat_list; tmp && (tmp->next_fighting != ch); 
	 tmp = tmp->next_fighting);
     if (!tmp) {
      log("Char fighting not found Error (fight.c, stop_fighting)");
      abort();
    }    
    tmp->next_fighting = ch->next_fighting;
  }
  
  ch->next_fighting = 0;
  ch->specials.fighting = 0;
  if (MOUNTED(ch))
    GET_POS(ch) = POSITION_MOUNTED;
  else 
    GET_POS(ch) = POSITION_STANDING;
  update_pos(ch);
}



#define MAX_NPC_CORPSE_TIME 5
#define MAX_PC_CORPSE_TIME 10
#define SEVERED_HEAD    30      /* real number of the severed head base item */
                                /* for now I use the scraps number, should be fine */
void make_corpse(struct char_data *ch, int killedbytype)
{
  struct obj_data *corpse, *o, *cp;
  struct obj_data *money;       
  char buf[MAX_INPUT_LENGTH], 
       spec_desc[255]; /* used in describing the corpse */
  int r_num,i, ADeadBody=FALSE;
  
  struct obj_data *create_money( int amount );

  CREATE(corpse, struct obj_data, 1);
  clear_object(corpse);
  
  corpse->item_number = NOWHERE;
  corpse->in_room = NOWHERE;
  
  if (!IS_NPC(ch) || (!IsUndead(ch))) {
/* this is so we drop a severed head at the corpse, just for visual */
if ((GET_HIT(ch) < -50) && (killedbytype == TYPE_SLASH || 
                            killedbytype == TYPE_CLEAVE)) {
    if ((r_num = real_object(SEVERED_HEAD)) >= 0) {
     cp = read_object(r_num, REAL);
     sprintf(buf,"head severed %s",corpse->name);
if (cp->name)
	free(cp->name);
     cp->name=strdup(buf);
     sprintf(buf,"the severed head of %s",
       (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
if (cp->short_description)
	free(cp->short_description);       
     cp->short_description=strdup(buf);
if (cp->action_description)
	free(cp->action_description);     
     cp->action_description=strdup(buf);
     sprintf(buf,"%s is lying on the ground.",buf);
if (cp->description)
	free(cp->description);     
     cp->description=strdup(buf);

  cp->obj_flags.type_flag = ITEM_CONTAINER;
  cp->obj_flags.wear_flags = ITEM_TAKE;
  cp->obj_flags.value[0] = 0; /* You can't store stuff in a corpse */
  cp->affected[0].modifier=GET_RACE(ch);     /* race of corpse NOT USED HERE*/
  cp->affected[1].modifier=GetMaxLevel(ch);  /* level of corpse NOT USED HERE*/
  cp->obj_flags.value[3] = 1; /* corpse identifyer */
  if (IS_NPC(ch))
    cp->obj_flags.timer = MAX_NPC_CORPSE_TIME+2;
  else
    cp->obj_flags.timer = MAX_PC_CORPSE_TIME+3;
   
     obj_to_room(cp,ch->in_room);
  } /* we got the numerb of the item... */
} 

                /* so we can have some description on the corpse */
                /* msw                                          */
        switch(killedbytype) {
case    SPELL_COLOUR_SPRAY:sprintf(spec_desc,"rainbow coloured corpse of %s is",
                         (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
                        break;
case    SPELL_ACID_BLAST: sprintf(spec_desc,"dissolving remains of %s are",
                         (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
                        break;
case    SPELL_FIRESHIELD:
case    SPELL_FIREBALL:sprintf(spec_desc,"smoldering remains of %s are",
                         (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
                        break;
case    SPELL_CHAIN_LIGHTNING:
case    SPELL_LIGHTNING_BOLT:sprintf(spec_desc,"charred remains of %s are",
                         (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
                        break;
case    SKILL_PSIONIC_BLAST:sprintf(spec_desc,"jelly-fied remains of %s are",
                         (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
                        break;
        default:sprintf(spec_desc,"corpse of %s is",
                         (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));         
                break;
        } /* end switch */



    sprintf(buf, "corpse %s",spec_desc);
	if (corpse->name)
	    free(corpse->name);
    corpse->name = strdup(buf);
  if (IS_AFFECTED(ch,AFF_FLYING)) 
    sprintf(buf, "The %s floating in the air here.", spec_desc);
      else
    sprintf(buf, "The %s lying here.", spec_desc);
if (corpse->description)
	free(corpse->description);
    corpse->description = strdup(buf);
    
    sprintf(buf, "the corpse of %s",  /* for the dissolve message */
           (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));       
if (corpse->short_description)
	free(corpse->short_description);
    corpse->short_description = strdup(buf);
    
    ADeadBody = TRUE;
    
  } else 
  if (IsUndead(ch)) {
if (corpse->name)
    free(corpse->name);
if (corpse->description)
	free(corpse->description);
if (corpse->short_description)
	free(corpse->short_description);
    corpse->name = strdup("dust pile bones");
    corpse->description = strdup("A pile of dust and bones is here.");
    corpse->short_description = strdup("a pile of dust and bones");        
  } 
  

  corpse->contains = ch->carrying;
  if(GET_GOLD(ch)>0) {
    money = create_money(GET_GOLD(ch));
    GET_GOLD(ch)=0;
    obj_to_obj(money,corpse);
  }
  
  corpse->obj_flags.type_flag = ITEM_CONTAINER;
  corpse->obj_flags.wear_flags = ITEM_TAKE;
  corpse->obj_flags.value[0] = 0; /* You can't store stuff in a corpse */

  corpse->affected[0].modifier=GET_RACE(ch); /* race of corpse */
  corpse->affected[1].modifier=GetMaxLevel(ch);  /* level of corpse */

  corpse->obj_flags.value[3] = 1; /* corpse identifyer */
  if (ADeadBody) {
    corpse->obj_flags.weight = GET_WEIGHT(ch)+IS_CARRYING_W(ch);
  } else {
    corpse->obj_flags.weight = 1+IS_CARRYING_W(ch);
  }
  corpse->obj_flags.cost_per_day = 100000;
  if (IS_NPC(ch))
    corpse->obj_flags.timer = MAX_NPC_CORPSE_TIME;
  else
    corpse->obj_flags.timer = MAX_PC_CORPSE_TIME;
  
  for (i=0; i<MAX_WEAR; i++)
    if (ch->equipment[i])
      obj_to_obj(unequip_char(ch, i), corpse);
  
  ch->carrying = 0;
  IS_CARRYING_N(ch) = 0;
  IS_CARRYING_W(ch) = 0;
  
  if (IS_NPC(ch)) {
    corpse->char_vnum = mob_index[ch->nr].virtual;
    corpse->char_f_pos = 0;
  } else {
    if (ch->desc) {
      corpse->char_f_pos = ch->desc->pos;
      corpse->char_vnum = 0;
    } else {
      corpse->char_f_pos = 0;
      corpse->char_vnum = 100;
    }
  }


  corpse->carried_by = 0;
  corpse->equipped_by = 0;
  
  corpse->next = object_list;
  object_list = corpse;
  
  for(o = corpse->contains; o; o = o->next_content)
    o->in_obj = corpse;
  
  object_list_new_owner(corpse, 0);
  
  obj_to_room(corpse, ch->in_room);


/* this must be set before dispel_magic, because if they */
/* are flying and in a fly zone then the mud will crash  */
  if (GET_POS(ch) != POSITION_DEAD) 
      GET_POS(ch) = POSITION_DEAD;


  /*
    remove spells
    */
         RemAllAffects(ch);     /* new msw, 8/7/94 */
         
/*   spell_dispel_magic(IMPLEMENTOR,ch,ch,0); */

 check_falling_obj(corpse, ch->in_room); /* hmm */

}

void change_alignment(struct char_data *ch, struct char_data *victim)
{
  int change, diff, d2;

  if (IS_NPC(ch)) return;

  if (IS_GOOD(ch) && (IS_GOOD(victim))) {
    change = (GET_ALIGNMENT(victim)  / 200) * (MAX(1,GetMaxLevel(victim) - GetMaxLevel(ch))); 
  } else if (IS_EVIL(ch) && (IS_GOOD(victim))) {
    change = (GET_ALIGNMENT(victim) / 30) * (MAX(1, GetMaxLevel(victim) - GetMaxLevel(ch)));
  } else if (IS_EVIL(victim) && (IS_GOOD(ch))) {
    change = (GET_ALIGNMENT(victim) / 30) * (MAX(1, GetMaxLevel(victim) - GetMaxLevel(ch)));
  } else if (IS_EVIL(ch) && (IS_EVIL(victim))) {
    change = ((GET_ALIGNMENT(victim) / 200)+1) * (MAX(1, GetMaxLevel(victim) - GetMaxLevel(ch)));
  } else {
    change = ((GET_ALIGNMENT(victim) / 200)+1) * (MAX(1, GetMaxLevel(victim) - GetMaxLevel(ch)));
  }

  if (change == 0) {
    if (GET_ALIGNMENT(victim) > 0) 
      change = 1;
    else if (GET_ALIGNMENT(victim) < 0)
      change = -1;
  }

  if (HasClass(ch, CLASS_DRUID) && (GetMaxLevel(ch)<LOW_IMMORTAL)) 
  {
    diff = 0 - GET_ALIGNMENT(ch);
    d2 = 0 - (GET_ALIGNMENT(ch)-change);
    if (diff < 0) diff = -diff;
    if (d2 < 0) d2 = -d2;
    if (d2 > diff) 
    {
      send_to_char("Beware, you are straying from the path\n\r", ch);
      if (d2 > 150) 
      {
	send_to_char("Your lack of faith is disturbing\n\r", ch);
	if (d2 > 275) 
	{
	  send_to_char("If you do not mend your ways soon, you will be punished\n\r", ch);
	  if (d2 > 425) 
	  {
	    send_to_char("Your unfaithfullness demands punishment!\n\r", ch);
	    drop_level(ch, CLASS_DRUID,FALSE);
	  }
	}
      }
    }
    
  }  

  if (HasClass(ch, CLASS_PALADIN) && (GetMaxLevel(ch)<LOW_IMMORTAL))   {
    diff = GET_ALIGNMENT(ch);
    d2 = (GET_ALIGNMENT(ch)-change);
    if (diff < 0) diff = -diff;
    if (d2 < 0) d2 = -d2;
    if (d2 < diff)     {
      send_to_char("Beware, you are straying from the path\n\r", ch);
      if (d2 < 950)       {
	send_to_char("Your lack of faith is disturbing\n\r", ch);
	if (d2 < 550) 	{
	  send_to_char("If you do not mend your ways soon, you will be punished\n\r", ch);
	  if (d2 < 350) 	  {
	    send_to_char("Your unfaithfullness demands punishment!\n\r", ch);
	    drop_level(ch, CLASS_PALADIN,FALSE);
	  }
	}
      }
    }
  }  

  if (HasClass(ch, CLASS_RANGER) && (GetMaxLevel(ch)<LOW_IMMORTAL))   {
    diff = GET_ALIGNMENT(ch);
    d2 = (GET_ALIGNMENT(ch)-change);
    if (diff < 0) diff = -diff;
    if (d2 < 0) d2 = -d2;
    if (d2 < diff)     {
      send_to_char("Beware, you are straying from the path\n\r", ch);
      if (d2 < 500)       {
	send_to_char("Your lack of faith is disturbing\n\r", ch);
	if (d2 < 0) 	{
	  send_to_char("If you do not mend your ways soon, you will be punished\n\r", ch);
	  if (d2 < -350) 	  {
	    send_to_char("Your unfaithfullness demands punishment!\n\r", ch);
	    drop_level(ch, CLASS_RANGER,FALSE);
	  }
	}
      }
    }
  }  


  GET_ALIGNMENT(ch) -= change;

  if (HasClass(ch, CLASS_DRUID)  && (GetMaxLevel(ch)<LOW_IMMORTAL) ) {
    if (GET_ALIGNMENT(ch) > 600 || GET_ALIGNMENT(ch) < -600) {
      send_to_char("Eldath, patron of druids, has excommunicated you for your heresies\n\r", ch);
      send_to_char("You are forever more a mere cleric\n\r", ch);
      REMOVE_BIT(ch->player.class, CLASS_DRUID);
      if (!HasClass(ch, CLASS_CLERIC)) {
	GET_LEVEL(ch, CLERIC_LEVEL_IND) = GET_LEVEL(ch, DRUID_LEVEL_IND);
      }
      GET_LEVEL(ch, DRUID_LEVEL_IND) = 0;
      SET_BIT(ch->player.class, CLASS_CLERIC);
    }
  }


  if (HasClass(ch, CLASS_PALADIN)  && (GetMaxLevel(ch)<LOW_IMMORTAL) ) {
    if (GET_ALIGNMENT(ch) < 350) {
      send_to_char("Torm, patron of paladins, has excommunicated you for your heresies\n\r", ch);
      send_to_char("You are forever more a mere warrior!\n\r", ch);
      REMOVE_BIT(ch->player.class, CLASS_PALADIN);
      if (!HasClass(ch, CLASS_WARRIOR)) {
	GET_LEVEL(ch, WARRIOR_LEVEL_IND) = GET_LEVEL(ch, PALADIN_LEVEL_IND);
      }
      GET_LEVEL(ch, PALADIN_LEVEL_IND) = 0;
      SET_BIT(ch->player.class, CLASS_WARRIOR);
    }
  }

  if (HasClass(ch, CLASS_RANGER)  && (GetMaxLevel(ch)<LOW_IMMORTAL) ) {
    if (GET_ALIGNMENT(ch) < -350) {
      send_to_char("Eldath, patron of rangers and druids, has excommunicated you for your heresies\n\r", ch);
      send_to_char("You are forever more a mere warrior!\n\r", ch);
      REMOVE_BIT(ch->player.class, CLASS_RANGER);
      if (!HasClass(ch, CLASS_WARRIOR)) {
	GET_LEVEL(ch, WARRIOR_LEVEL_IND) = GET_LEVEL(ch, RANGER_LEVEL_IND);
      }
      GET_LEVEL(ch, RANGER_LEVEL_IND) = 0;
      SET_BIT(ch->player.class, CLASS_WARRIOR);
    }
  }


  GET_ALIGNMENT(ch) = MAX(GET_ALIGNMENT(ch), -1000);
  GET_ALIGNMENT(ch) = MIN(GET_ALIGNMENT(ch), 1000);

}

void death_cry(struct char_data *ch)
{
  int door, was_in;
  
  if (ch->in_room == -1)
    return;
  
  act("$c0005Your blood freezes as you hear $c0015$n's$c0005 death cry.", FALSE, ch,0,0,TO_ROOM);
  was_in = ch->in_room;
  
  for (door = 0; door <= 5; door++) {
    if (CAN_GO(ch, door))	{
      ch->in_room = (real_roomp(was_in))->dir_option[door]->to_room;
      act("$c0005Your blood freezes as you hear someones death cry.",FALSE,ch,0,0,TO_ROOM);
      ch->in_room = was_in;
    }
  }
}



void raw_kill(struct char_data *ch,int killedbytype)
{
#if 0
 struct char_data *tmp, *tch;
 char buf[256];
#endif
  
if((IS_MOB(ch)) && (!IS_SET(ch->specials.act,ACT_POLYSELF))&&(mob_index[ch->nr].func))
  (*mob_index[ch->nr].func)(ch, 0, "", ch, EVENT_DEATH);

/* tell mob to hate killer next load here, or near here */

#if 0
/* this seems to cause a infinate loop, figure out why */

tmp = ch->specials.fighting;
if (is_murdervict(ch) && 
   (IS_PC(tmp) || IS_SET(tmp->specials.act,ACT_POLYSELF)) && ch != tmp) {
for (tch=real_roomp(ch->in_room)->people;tch;tch = tch->next_in_room) {
 if (ch != tch && GET_POS(tch) > POSITION_SLEEPING && 
     IS_NPC(tch) && CAN_SEE(tmp, tch))     {
      if (IS_GOOD(tch) || IS_NEUTRAL(tch))     {
          sprintf(buf, "Setting MURDER bit on %s for killing %s.",
                  GET_NAME(tmp),GET_NAME(ch));
          log(buf);
          SET_BIT(tmp->player.user_flags,MURDER_1);
	  act("$n points at $N and screams 'MURDERER!",TRUE,tch,0,tmp,TO_ROOM);
         } /* good/neut */
        } /* npc */
       } /* for */
    } /* start murder stuff */

#endif


  if (ch->specials.fighting)
    stop_fighting(ch);
  
  death_cry(ch);
  
  /*
    give them some food and water so they don't whine.
    */
  if (GetMaxLevel(ch)<LOW_IMMORTAL)
      GET_COND(ch,THIRST)=20;
  if (GetMaxLevel(ch)<LOW_IMMORTAL)
      GET_COND(ch,FULL)=20;
  
/* remove berserk after they flee/die... */

  if (IS_SET(ch->specials.affected_by2,AFF2_BERSERK))  {
    REMOVE_BIT(ch->specials.affected_by2,AFF2_BERSERK);
    }  
    
  /*
   *   return them from polymorph
   */

  make_corpse(ch,killedbytype);
  zero_rent(ch);
  extract_char(ch);
}



void die(struct char_data *ch,int killedbytype)
{
  struct char_data *pers;
  int i,tmp;
  char buf[80];
  int fraction;

  /* need at least 1/fraction worth of exp for the minimum needed for */
  /* the pc's current level, or else you lose a level.  If all three  */
  /* classes are lacking in exp, you lose one level in each class. */

  fraction = 16;
  if (IS_NPC(ch) && (IS_SET(ch->specials.act, ACT_POLYSELF))) {
    /*
     *   take char from storage, to room
     */
    if (ch->desc) {
       pers = ch->desc->original;
       char_from_room(pers);
       char_to_room(pers, ch->in_room);
       SwitchStuff(ch, pers);
       extract_char(ch);
       ch = pers;
     } else {
       /* we don't know who the original is.  Gets away with it, i guess*/
     }
  }

#if LEVEL_LOSS

  for(i=0;i<MAX_CLASS;i++) {
    if (GET_LEVEL(ch,i) > 1) {
      if (GET_LEVEL(ch,i) >= LOW_IMMORTAL) break;
      if (GET_EXP(ch) < (titles[i][GET_LEVEL(ch, i)].exp/fraction)) {
        tmp = (ch->points.max_hit)/GetMaxLevel(ch);
        ch->points.max_hit -= tmp;
        GET_LEVEL(ch, i) -= 1;
        ch->specials.spells_to_learn -= MAX(1, MAX(2, wis_app[GET_RWIS(ch)].bonus)/HowManyClasses(ch));
        send_to_char("\n\rInsufficient experience has cost you a level.\n\r",
                     ch);
      }
    }
  }
#endif

#if NEWEXP
  if (GetMaxLevel(ch) > 15)
    gain_exp(ch, -GET_EXP(ch)/2);
  else if (GetMaxLevel(ch) > 10)
    gain_exp(ch, -GET_EXP(ch)/3);
  else if (GetMaxLevel(ch) > 5)
    gain_exp(ch, -GET_EXP(ch)/4);
  else
    gain_exp(ch, -GET_EXP(ch)/5);
#else
  gain_exp(ch, -GET_EXP(ch)/2);
#endif
#if LEVEL_LOSS

  /* warn people if their next death will result in a level loss */
  for(i=0;i<MAX_CLASS;i++) {
    if (GET_LEVEL(ch,i) > 1) {
      if (GET_EXP(ch) < (titles[i][GET_LEVEL(ch, i)].exp/fraction)) {
        send_to_char("\n\r\n\rWARNING WARNING WARNING WARNING WARNING WARNING\n\r",
                     ch);
        send_to_char("Your next death will result in the loss of a level,\n\r",
                     ch);
        sprintf(buf,"unless you get at least %d more exp points.\n\r",
                (titles[i][GET_LEVEL(ch, i)].exp/fraction) - GET_EXP(ch));
        send_to_char(buf,ch);
      }
    }
  }
#endif

  /*
   **      Set the talk[2] to be TRUE, i.e. DEAD
   */
  ch->player.talks[2] = 1;  /* char is dead */

  DeleteHatreds(ch);
  DeleteFears(ch);
  raw_kill(ch,killedbytype);
}

#define EXP_CAP		150000 
#define EXP_CAP_1	250000 
#define EXP_CAP_2	350000 
#define EXP_CAP_3 	450000 
#define EXP_CAP_4 	500000 
#define EXP_CAP_5	550000 
#define EXP_CAP_6 	600000 
#define EXP_CAP_7 	700000 
#define EXP_CAP_8	800000 
#define EXP_CAP_OTHER	1000000 
long ExpCaps(int group_count, long total) 
{
	
   if (group_count >=1)	
	switch(group_count) {
		case 1: if (total>EXP_CAP_1)
			  total=EXP_CAP_1;
			  break;
		case 2:if (total>EXP_CAP_2)
			  total=EXP_CAP_2;
			  break;
		case 3:if (total>EXP_CAP_3)
			  total=EXP_CAP_3;
			  break;
		case 4:if (total>EXP_CAP_4)
			  total=EXP_CAP_4;
			  break;
		case 5:if (total>EXP_CAP_5)
			  total=EXP_CAP_5;
			  break;
		case 6:if (total>EXP_CAP_6)
			  total=EXP_CAP_6;
			  break;
		case 7:if (total>EXP_CAP_7)
			  total=EXP_CAP_7;
			  break;
		case 8:if (total>EXP_CAP_8)
			  total=EXP_CAP_8;
			  break;
		default:
		if (total>EXP_CAP_OTHER)
			total=EXP_CAP_OTHER;
		break;
	} /* end switch */
	else  {
	 if (total>EXP_CAP)	/* not grouped, so limit max exp gained so */
	    total = EXP_CAP;	/* grouping will be used more and benifical */
	}

  return(total);
}

long GroupLevelRatioExp(struct char_data *ch,int group_max_level,long experincepoints)
{
  unsigned int diff=0;
	diff=abs(group_max_level-GetMaxLevel(ch));

	if (diff) {
		/* More than 10 levels difference, then we knock down */
		/* the ratio of EXP he gets, keeping high level people */
		/* from getting newbies up to fast... */
	if (diff >=10) 			
	experincepoints=experincepoints/2;	 else	
	 if (diff >= 20)
	     experincepoints=experincepoints/3;	 else
	 if (diff >= 30)
	     experincepoints=experincepoints/4;	 else
	 if (diff >= 40)
	     experincepoints=experincepoints/5;	 else
	 if (diff >= 49)
	     experincepoints=experincepoints/6;	 
	}
                                                                         
 return(experincepoints);
}

void group_gain(struct char_data *ch,struct char_data *victim) {
  char buf[256];
  int no_members, share;
  struct char_data *k;
  struct follow_type *f;
  int total, pc, group_count=0,
  	group_max_level=1; /* the highest level number the group has */
  
  if (!(k=ch->master))
    k = ch;
  
  /* can't get exp for killing players */
  
  if (!IS_NPC(victim)) {
    return;
  }
  
  if (IS_AFFECTED(k, AFF_GROUP) &&
      (k->in_room == ch->in_room))
    no_members = GET_AVE_LEVEL(k);
  else
    no_members = 0;

  pc = FALSE;

   group_max_level=GetMaxLevel(k);  
   
  for (f=k->followers; f; f=f->next)
    if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	(f->follower->in_room == ch->in_room)) {
      	no_members+=GET_AVE_LEVEL(f->follower);
      if (IS_PC(f->follower))	
          pc++;
	if (IS_PC(f->follower) || IS_SET(f->follower->specials.act,ACT_POLYSELF) 
		&& f->follower->in_room==k->in_room) {
		if (group_max_level<GetMaxLevel(f->follower))
		    group_max_level=GetMaxLevel(f->follower);
		 group_count++;
		}
    }

  if (pc > 10)
    pc = 10;

  if (no_members >= 1)
    share = (GET_EXP(victim)/no_members);
  else
    share = 0;
    
  if (IS_AFFECTED(k, AFF_GROUP) &&
      (k->in_room == ch->in_room)) {

      total = share*GET_AVE_LEVEL(k);

      if (pc) {
	total *= (100 + (3*pc));
	total /= 100;
      }

	RatioExp(k, victim, total);
	total= GroupLevelRatioExp(k,group_max_level,total);
	total= ExpCaps(group_count,total);	/* figure EXP MAXES */
	
      sprintf(buf,"You receive your share of %d experience.", total);
      act(buf, FALSE, k, 0, 0, TO_CHAR);
      gain_exp(k,total);
      change_alignment(k, victim);
  }
  
  for (f=k->followers; f; f=f->next) {
    if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	(f->follower->in_room == ch->in_room)) {

        total = share*GET_AVE_LEVEL(f->follower);

	if (IS_PC(f->follower)) {
	  total *= (100 + (1*pc));
	  total /= 100;
	} else 
	  total /= 2;


	if (IS_PC(f->follower)) {
	  total = RatioExp(f->follower, victim, total);
	total= GroupLevelRatioExp(f->follower,group_max_level,total);
	  total= ExpCaps(group_count,total);	/* figure EXP MAXES */
	  sprintf(buf,"You receive your share of %d experience.", total);
	  act(buf, FALSE, f->follower,0,0,TO_CHAR);
	  gain_exp(f->follower,  total);
	
	  change_alignment(f->follower, victim);
	} else {
	  if (f->follower->master && IS_AFFECTED(f->follower, AFF_CHARM)) {
	    total = RatioExp(f->follower->master, victim, total);
	total= GroupLevelRatioExp(f->follower,group_max_level,total);

            total= ExpCaps(group_count,total);	/* figure EXP MAXES */
	    if (f->follower->master->in_room ==
		f->follower->in_room) {
	      sprintf(buf,"You receive $N's share of %d experience.", total);
	      act(buf, FALSE, f->follower->master,0,f->follower,TO_CHAR);
	      gain_exp(f->follower->master,  total);
	      change_alignment(f->follower, victim);
	    }
	  } else {
	    total = RatioExp(f->follower, victim, total);
	total= GroupLevelRatioExp(f->follower,group_max_level,total);

	    total= ExpCaps(group_count,total);	/* figure EXP MAXES */
	    sprintf(buf,"You receive your share of %d experience.", total);
	    act(buf, FALSE, f->follower,0,0,TO_CHAR);
	    gain_exp(f->follower,  total);
	    
	    change_alignment(f->follower, victim);
	  }
	}
    }
  }
}

char *replace_string(char *str, char *weapon, char *weapon_s,
			        char *location_hit, char *location_hit_s)
{
  static char buf[256];
  char *cp;
  
  cp = buf;
  
  for (; *str; str++) {
    if (*str == '#') {
      switch(*(++str)) {
      case 'W' : 
	for (; *weapon; *(cp++) = *(weapon++));
	break;
      case 'w' : 
	for (; *weapon_s; *(cp++) = *(weapon_s++));
	break;

	/* added this to show where the person was hit */
      case 'L' : 
	for (; *location_hit; *(cp++) = *(location_hit++));
	break;
      case 'l' : 
	for (; *location_hit_s; *(cp++) = *(location_hit_s++));
	break;

	default :
	  *(cp++) = '#';
	break;
      }
    } else {
      *(cp++) = *str;
    }
    
    *cp = 0;
  } /* For */
  
  return(buf);
}




void dam_message(int dam, struct char_data *ch, struct char_data *victim,
                 int w_type)
{
  struct obj_data *wield;
  char *buf;
  int snum,hitloc;
  
  static struct dam_weapon_type {
    char *to_room;
    char *to_char;
    char *to_victim;
  } dam_weapons[] = {
    
    {
      "$n misses $N.",                           /*    0    */
      "You miss $N.",
      "$n misses you." 
    }, 
    
    { 
      "$n bruises $N with $s #w #l.",                       /*  1.. 2  */
      "You bruise $N as you #w $M #l.",
      "$n bruises you as $e #W your #L." 
    }, 
    
    {
      "$n barely #W $N #l.",                                   /*  3.. 4  */
      "You barely #w $N #l.",
      "$n barely #W your #L."
    }, 
    
    {
      "$n #W $N #l.",                                          /*  5.. 6  */
      "You #w $N #l.",
      "$n #W your #L."
    }, 
    
    {
      "$n #W $N hard #l.",                                     /*  7..10  */
      "You #w $N hard #l.",
      "$n #W you hard on your #L."
    }, 
    
    {
      "$n #W $N very hard #l.",                                /* 11..14  */
      "You #w $N very hard #l.",
      "$n #W you very hard on you #L."
    }, 
    
    {
      "$n #W $N extremely well #l.",                          /* 15..20  */
      "You #w $N extremely well #l.",
      "$n #W you extremely well on your #L."
    }, 
    
    {
      "$n massacres $N with $s #w #l.",     /* > 20    */
      "You massacre $N with your #w #l.",
      "$n massacres you with $s #w on your #L."
    },
   
    {
     "$n devastates $N with $s #w #l.",
     "You devastate $N with your #w #l.",
     "$n devastates you with $s #w on your #L."
    }
    
  };

  
  w_type -= TYPE_HIT;   /* Change to base of table with text */


  wield = ch->equipment[WIELD];
  
  if (dam <= 0) {
    snum = 0;
  } else if (dam <= 2) {
    snum = 1;
  } else if (dam <= 4) {
    snum = 2;
  } else if (dam <= 10) {
    snum = 3;
  } else if (dam <= 15) {
    snum = 4;
  } else if (dam <= 25) {
    snum = 5;
  } else if (dam <= 35) {
    snum = 6;
  } else if (dam <= 45) {
    snum = 7;
  } else {
    snum = 8;
  }

/* generate random hit location */
   hitloc=number(0,14);

/* make body/chest hits happen more often than the others */
  if (hitloc !=0 && hitloc != 11 && hitloc != 13)
      hitloc=number(0,14);
      
/* make sure the mob has this body part first! */  
	if (!HasHands(victim))
             hitloc=0;   /* if not then just make it a body hit hitloc=0 */


  buf = replace_string(dam_weapons[snum].to_room, attack_hit_text[w_type].plural, attack_hit_text[w_type].singular,
	  location_hit_text[hitloc].plural,   location_hit_text[hitloc].singular);
  act(buf, FALSE, ch, wield, victim, TO_NOTVICT);

  buf = replace_string(dam_weapons[snum].to_char, attack_hit_text[w_type].plural, attack_hit_text[w_type].singular,
  	  location_hit_text[hitloc].plural,   location_hit_text[hitloc].singular);
  act(buf, FALSE, ch, wield, victim, TO_CHAR);

  buf = replace_string(dam_weapons[snum].to_victim, attack_hit_text[w_type].plural, attack_hit_text[w_type].singular,
	  location_hit_text[hitloc].plural,   location_hit_text[hitloc].singular);
  act(buf, FALSE, ch, wield, victim, TO_VICT);

}

int DamCheckDeny(struct char_data *ch, struct char_data *victim, int type)
{
  struct room_data *rp;
  char buf[MAX_INPUT_LENGTH];

/*  assert(GET_POS(victim) > POSITION_DEAD);  */
 
if (!GET_POS(victim) > POSITION_DEAD) {
  log("!GET_POS(victim) > POSITION_DEAD in fight.c");
  return(TRUE);
}
  
  rp = real_roomp(ch->in_room);
  if (rp && (rp->room_flags&PEACEFUL) && type!=SPELL_POISON && 
      type!=SPELL_HEAT_STUFF && type != TYPE_SUFFERING) {
    sprintf(buf, "damage(,,,%d) called in PEACEFUL room", type);
    log(buf);
    return(TRUE); /* true, they are denied from fighting */
  }
  return(FALSE);

}

int DamDetailsOk( struct char_data *ch, struct char_data *v, int dam, int type)
{

  if (dam < 0) return(FALSE);

/* we check this already I think, be sure to keep an eye out. msw */
  if ((type != TYPE_RANGE_WEAPON) && (ch->in_room != v->in_room)) 
      return(FALSE);
     
  if ((ch == v) && 
      ((type != SPELL_POISON) && (type != SPELL_HEAT_STUFF) && (type !=TYPE_SUFFERING))) return(FALSE);

  if (MOUNTED(ch)) {
    if (MOUNTED(ch) == v) {
      FallOffMount(ch, v);
      Dismount(ch, MOUNTED(ch), POSITION_SITTING);
    }
  }

  return(TRUE);

}


int SetCharFighting(struct char_data *ch, struct char_data *v)
{
  if (GET_POS(ch) > POSITION_STUNNED) {	
    if (!(ch->specials.fighting)) {
       set_fighting(ch, v);
       GET_POS(ch) = POSITION_FIGHTING;
    } else {
       return(FALSE);
    }
  }
  return(TRUE);

}


int SetVictFighting(struct char_data *ch, struct char_data *v)
{

  if ((v != ch) && (GET_POS(v) > POSITION_STUNNED) && 
     (!(v->specials.fighting))) {
     if (ch->attackers < 6) {
        set_fighting(v, ch);
        GET_POS(v) = POSITION_FIGHTING;
      }
  } else {
      return(FALSE);	
  }
  return(TRUE);
}

int DamageTrivia(struct char_data *ch, struct char_data *v, int dam, int type)
{

char buf[255];

  if (v->master == ch)
    stop_follower(v);
  
  if (IS_AFFECTED(ch, AFF_INVISIBLE) || IS_AFFECTED2(ch, AFF2_ANIMAL_INVIS))
    appear(ch);

  if (IS_AFFECTED(ch, AFF_SNEAK)) {
    affect_from_char(ch, SKILL_SNEAK);
  }
  if (IS_AFFECTED(ch, AFF_HIDE)){
    REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
  }
  
  if (IS_AFFECTED(v, AFF_SANCTUARY)) {
      dam = MAX((int)(dam/2), 0);  /* Max 1/2 damage when sanct'd */
  }
 
  if (IS_SET(ch->specials.affected_by2,AFF2_BERSERK) 
     && type >= TYPE_HIT) /*chec to see if berserked and using a weapon */
   {
     dam = berserkdambonus(ch,dam);     /* More damage if berserked */
   }
   
  dam = PreProcDam(v,type,dam);

	
#if PREVENT_PKILL

 if ( (IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF)) &&
      (IS_PC(v) || IS_SET(v->specials.act,ACT_POLYSELF)) &&
      (ch != v) &&
       !CanFightEachOther(ch,v)       ) {
	act("Your attack seems usless against $N!",FALSE,ch,0,v,TO_CHAR);
	act("The attack from $n is futile!",FALSE,ch,0,v,TO_VICT);
	dam=-1;	
    }
#endif
	/* shield makes you immune to magic missle! */
	
if (affected_by_spell(v,SPELL_SHIELD) && type == SPELL_MAGIC_MISSILE) {
  act("$n's magic missle is deflected by $N's shield!", FALSE, ch, 0, v, TO_NOTVICT);
  act("$N's shield deflects your magic missle!", FALSE, ch, 0, v, TO_CHAR);
  act("Your shell deflects $n's magic missle!", FALSE, ch, 0, v, TO_VICT);
  dam=-1;
}

if (affected_by_spell(v,SKILL_TOWER_IRON_WILL) && 
    (type == SKILL_PSIONIC_BLAST) ) {
    act("$n's psionic attack is ignored by $N!", FALSE, ch, 0, v, TO_NOTVICT);
    act("$N's psionic protections shield against your attack!", FALSE, ch, 0, v, TO_CHAR);
    act("Your psionic protections protection you against $n's attack!", FALSE, ch, 0, v, TO_VICT);
    dam=-1;
    }
    
		/* we check for prot from breath weapons here */
		
if (type >= FIRST_BREATH_WEAPON && type <= LAST_BREATH_WEAPON) {
   int right_protection=FALSE;
   
if (affected_by_spell(v,SPELL_PROT_DRAGON_BREATH)) {	/* immune to all breath */
  	right_protection = TRUE;
  	} else
	if (affected_by_spell(v,SPELL_PROT_BREATH_FIRE) && 
	    type == SPELL_FIRE_BREATH) {
  	right_protection = TRUE;	    
      		} else
	if (affected_by_spell(v,SPELL_PROT_BREATH_GAS) && 
	    type == SPELL_GAS_BREATH) {
  	right_protection = TRUE;	    
      		} else
	if (affected_by_spell(v,SPELL_PROT_BREATH_FROST) && 
	    type == SPELL_FROST_BREATH) {
  	right_protection = TRUE;	    
      		} else
	if (affected_by_spell(v,SPELL_PROT_BREATH_ACID) && 
	    type == SPELL_ACID_BREATH) {
  	right_protection = TRUE;	    
      		} else
	if (affected_by_spell(v,SPELL_PROT_BREATH_ELEC) && 
	    type == SPELL_LIGHTNING_BREATH) {
  	right_protection = TRUE;	    
	}

if (right_protection) {
	 act("$N smiles as some of the breath is turned aside by $S protective globe!", FALSE, ch, 0, v, TO_NOTVICT);
   	 act("$N's protective globe deflects a bit of your breath!", FALSE, ch, 0, v, TO_CHAR);
   	 act("Your globe deflects the some of the breath weapon from $n!", FALSE, ch, 0, v, TO_VICT);
         dam=(int)dam/4;  /* 1/4 half damage */
    }
    
} /* else non-breath type hit/spell */
	else {

if (affected_by_spell(v,SPELL_ANTI_MAGIC_SHELL) && IsMagicSpell(type)) {
   sprintf(buf,"$N snickers as the %s from $n fizzles on $S anti-magic globe!",spells[type-1]);
   act(buf, FALSE, ch, 0, v, TO_NOTVICT);
   sprintf(buf,"$N's globes deflects your %s",spells[type-1]); 
   act(buf, FALSE, ch, 0, v, TO_CHAR);
   sprintf(buf,"Your globe deflects the %s from $n!",spells[type-1]);
   act(buf, FALSE, ch, 0, v, TO_VICT);
   dam = -1;
} else
 /* minor globe check here immune to level 1-5 and below magic user spells */
 if (affected_by_spell(v,SPELL_GLOBE_MINOR_INV) && 
    type < TYPE_HIT && spell_info[type].min_level_magic < 6) {
   sprintf(buf,"$N snickers as the %s from $n fizzles on $S globe!",spells[type-1]);
   act(buf, FALSE, ch, 0, v, TO_NOTVICT);
   sprintf(buf,"$N's globes deflects your %s",spells[type-1]); 
   act(buf, FALSE, ch, 0, v, TO_CHAR);
   sprintf(buf,"Your globe deflects the %s from $n!",spells[type-1]);
   act(buf, FALSE, ch, 0, v, TO_VICT);
   dam = -1;
 }

 /* major globe immune to level 5-10 magic user spells       */
 if (affected_by_spell(v,SPELL_GLOBE_MAJOR_INV) && type < TYPE_HIT && 
     spell_info[type].min_level_magic < 11 && 
     spell_info[type].min_level_magic > 5 ) {
 
   sprintf(buf,"$N laughs as the %s from $n bounces off $S globe!",spells[type-1]);
   act(buf, FALSE, ch, 0, v, TO_NOTVICT);
   sprintf(buf,"$N's globes completely deflects your %s",spells[type-1]); 
   act(buf, FALSE, ch, 0, v, TO_CHAR);
   sprintf(buf,"Your globe completely deflects the %s from $n!",spells[type-1]);
   act(buf, FALSE, ch, 0, v, TO_VICT);
   dam = -1;
 }
} /* was not breath spell */
  
  if (dam > -1) {
    dam = WeaponCheck(ch, v, type, dam);

    DamageStuff(v, type, dam);
  
    dam=MAX(dam,0);

  
    /*
     *  check if this hit will send the target over the edge to -hits
     */
    if (GET_HIT(v)-dam < 1) {
      if (IS_AFFECTED(v, AFF_LIFE_PROT)) {
	BreakLifeSaverObj(v);
	dam = 0;
	REMOVE_BIT(ch->specials.affected_by, AFF_LIFE_PROT);
      }
    }

    if (MOUNTED(v)) {
      if (!RideCheck(v, -(dam/2))) {
	FallOffMount(v, MOUNTED(v));
	WAIT_STATE(v, PULSE_VIOLENCE*2);
	Dismount(v, MOUNTED(v), POSITION_SITTING);
      }
    } else if (RIDDEN(v)) {
      if (!RideCheck(RIDDEN(v), -dam)) {
	FallOffMount(RIDDEN(v), v);
	WAIT_STATE(RIDDEN(v), PULSE_VIOLENCE*2);
	Dismount(RIDDEN(v), v, POSITION_SITTING);
      }
    }
  }

  return(dam);
}

int DoDamage(struct char_data *ch, struct char_data *v, int dam, int type)
{

  if (dam >= 0) {
    GET_HIT(v)-=dam;  

    if (type >= TYPE_HIT)    {
      if (IS_AFFECTED(v, AFF_FIRESHIELD) &&
         !IS_AFFECTED(ch, AFF_FIRESHIELD))      {
	if (damage(v, ch, dam, SPELL_FIREBALL)) 	{
	  if (GET_POS(ch) == POSITION_DEAD)
	    return(TRUE);
	}
      }
    }
    
    update_pos(v);
  } else   {
  }

  return(FALSE);
}


int DamageMessages( struct char_data *ch, struct char_data *v, int dam,
		    int attacktype)
{
  int nr, max_hit, i, j;
  struct message_type *messages;
  char buf[500];

  if (attacktype == SKILL_KICK) return; /* filter out kicks,
					  hard coded in do_kick */
  else 
  
  if ((attacktype >= TYPE_HIT) && (attacktype <= TYPE_RANGE_WEAPON)) 
  {
    dam_message(dam, ch, v, attacktype);
		/* do not wanna frag the bow, frag the arrow instead! */
    if (ch->equipment[WIELD] && attacktype != TYPE_RANGE_WEAPON)     {
      BrittleCheck(ch,v, dam); 
    }
  } else {
    
    for(i = 0; i < MAX_MESSAGES; i++) {
      if (fight_messages[i].a_type == attacktype) 
      {
	nr=dice(1,fight_messages[i].number_of_attacks);

	for(j=1,messages=fight_messages[i].msg;(j<nr)&&(messages);j++)
	  messages=messages->next;
	
	if (!IS_NPC(v) && (GetMaxLevel(v) > MAX_MORT))
	{
	  act(messages->god_msg.attacker_msg, 
	      FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
	  act(messages->god_msg.victim_msg, 
	      FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
	  act(messages->god_msg.room_msg, 
	      FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT);
	} else if (dam > 0) 
	{
	  if (GET_POS(v) == POSITION_DEAD) 
	  {
	    act(messages->die_msg.attacker_msg, 
		FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
	    act(messages->die_msg.victim_msg, 
		FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
	    act(messages->die_msg.room_msg, 
		FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT);
	  } else 
	  {
	    act(messages->hit_msg.attacker_msg, 
		FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
	    act(messages->hit_msg.victim_msg, 
		FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
	    act(messages->hit_msg.room_msg, 
		FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT);
	  }
       } /* dam >0 */
	  else if (dam == 0) 
	{
	  act(messages->miss_msg.attacker_msg, 
	      FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
	  act(messages->miss_msg.victim_msg, 
	      FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
	  act(messages->miss_msg.room_msg, 
	      FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT);
	} /* dam == 0 */
      } /* fight messages == atk type */
      
    }
  }
  switch (GET_POS(v)) {
  case POSITION_MORTALLYW:
    act("$n is mortally wounded, and will die soon, if not aided.", 
	TRUE, v, 0, 0, TO_ROOM);
    act("You are mortally wounded, and will die soon, if not aided.", 
	FALSE, v, 0, 0, TO_CHAR);
    break;
  case POSITION_INCAP:
    act("$n is incapacitated and will slowly die, if not aided.", 
	TRUE, v, 0, 0, TO_ROOM);
    act("You are incapacitated and you will slowly die, if not aided.", 
	FALSE, v, 0, 0, TO_CHAR);
    break;
  case POSITION_STUNNED:
    act("$n is stunned, but will probably regain consciousness again.", 
	TRUE, v, 0, 0, TO_ROOM);
    act("You're stunned, but you will probably regain consciousness again.", 
	FALSE, v, 0, 0, TO_CHAR);
    break;
  case POSITION_DEAD:
    act("$c0015$n is dead! $c0011R.I.P.", TRUE, v, 0, 0, TO_ROOM);
    act("$c0009You are dead!  Sorry...", FALSE, v, 0, 0, TO_CHAR);
    break;
    
  default:  /* >= POSITION SLEEPING */
    
    max_hit=hit_limit(v);
    
    if (dam > (max_hit/5)) {
      act("That Really $c0010HURT!$c0007",FALSE, v, 0, 0, TO_CHAR);  
    }
    if (GET_HIT(v) < (max_hit/5) && GET_HIT(v) > 0) {
      act("You wish that your wounds would stop $c0010BLEEDING$c0007 so much!",
	  FALSE,v,0,0,TO_CHAR);
      
      if (IS_NPC(v) && (IS_SET(v->specials.act, ACT_WIMPY))) {
	strcpy(buf, "flee");
	command_interpreter(v, buf);
      } else if (!IS_NPC(v)) {
	if (IS_SET(v->specials.act, PLR_WIMPY)) {
	  strcpy(buf, "flee");
	  command_interpreter(v, buf);
	}

      }
    }
    if (MOUNTED(v)) {
      /* chance they fall off */
      RideCheck(v, -dam/2);
    }
    if (RIDDEN(v)) {
      /* chance the rider falls off */
      RideCheck(RIDDEN(v), -dam);
    }
    break;
  }
}


int DamageEpilog(struct char_data *ch, struct char_data *victim, int killedbytype)
{
  int exp;
  char buf[256];
  struct room_data *rp;

  extern char DestroyedItems;


  
if (IS_LINKDEAD(victim)) {
     if (GET_POS(victim) != POSITION_DEAD) {
	do_flee(victim,"\0",0);
	return(FALSE);
     } else {
	die(victim,killedbytype);
        return(FALSE);
       }
  }

  if (!AWAKE(victim))
    if (victim->specials.fighting)
      stop_fighting(victim);
  
  if (GET_POS(victim) == POSITION_DEAD) {

    /*
      special for no-death rooms
      */
    rp = real_roomp(victim->in_room);
    if (rp && IS_SET(rp->room_flags, NO_DEATH)) {
      GET_HIT(victim) = 1;
      GET_POS(victim) = POSITION_STANDING;
      strcpy(buf, "flee");
      command_interpreter(victim, buf);
      return(FALSE);
    }

    if (ch->specials.fighting == victim)
      stop_fighting(ch);
    if (IS_NPC(victim) && !IS_SET(victim->specials.act, ACT_POLYSELF)) {
      if (IS_AFFECTED(ch, AFF_GROUP)) {
	group_gain(ch, victim);
      } else {
	/* Calculate level-difference bonus */
	exp = GET_EXP(victim);
	
	exp = MAX(exp, 1);

	if (!IS_PC(victim)) {
	  
	exp = ExpCaps(0,exp);	/* bug fix for non_grouped peoples */

	  gain_exp(ch, exp);
	}
	change_alignment(ch, victim);
      }
    }
    if (IS_PC(victim)) {
      if (victim->in_room > -1) {
	if (IS_NPC(ch)&&!IS_SET(ch->specials.act, ACT_POLYSELF)) {
	/* killed by npc */

  if (IS_MURDER(victim)) {
      REMOVE_BIT(victim->player.user_flags,MURDER_1);
     }

/* same here, with stole */
  if (IS_STEALER(victim)) {
      REMOVE_BIT(victim->player.user_flags,STOLE_1);
     }
	   sprintf(buf, "%s killed by %s at %s\n\r",
		GET_NAME(victim), ch->player.short_descr,
		(real_roomp(victim->in_room))->name);
	
	/* global death messages */
#if 1
	send_to_all(buf);
#endif	

	} else {
		/* killed by PC */
if (!IS_PC(victim) && !IS_SET(victim->specials.act,ACT_POLYSELF))
      if (victim != ch && !IS_IMMORTAL(victim)) {
           SET_BIT(ch->player.user_flags,MURDER_1);
          sprintf(buf, "Setting MURDER bit on %s for killing %s.",
                  GET_NAME(ch),GET_NAME(victim));
          log(buf);

	         }
	   if ((IS_GOOD(ch) && !IS_EVIL(victim))  ||
	       IS_EVIL(ch) && IS_NEUTRAL(victim)) {
	     sprintf(buf, "%s killed by %s at %s -- <Player kill, Illegal>",
		     GET_NAME(victim), ch->player.name, 
		     (real_roomp(victim->in_room))->name);
	   } else {
	     sprintf(buf, "%s killed by %s at %s",
		     GET_NAME(victim), GET_NAME(ch),
		     (real_roomp(victim->in_room))->name);
	   }

	}
      } else {
	sprintf(buf, "%s killed by %s at Nowhere.",
		GET_NAME(victim),
		(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
      }
      log_sev(buf, 6);
    }
    die(victim,killedbytype);
    /*
     *  if the victim is dead, return TRUE.
     */
    victim = 0;
    return(TRUE);
  } else {
    if (DestroyedItems) {
      if (check_falling(victim)) /* 0 = ok, 1 = dead */
	return(TRUE);
      DestroyedItems = 0;
    }
    return(FALSE);
  }


}

int MissileDamage(struct char_data *ch, struct char_data *victim,
	          int dam, int attacktype)
{

   if (DamCheckDeny(ch, victim, attacktype))
     return(FALSE);

   dam = SkipImmortals(victim, dam, attacktype);

   if (!DamDetailsOk(ch, victim, dam, attacktype))
     return(FALSE);

   SetVictFighting(ch, victim);
/*
  make the ch hate the loser who used a missile attack on them.
*/
   if (!IS_PC(victim)) {
     if (!Hates(victim, ch)) {
       AddHated(victim, ch);
     }
   }
   dam = DamageTrivia(ch, victim, dam, attacktype);

   if (DoDamage(ch, victim, dam, attacktype))
     return(TRUE);

   DamageMessages(ch, victim, dam, attacktype);

   if (DamageEpilog(ch, victim,attacktype)) return(TRUE);

   return(FALSE);  /* not dead */

}

int damage(struct char_data *ch, struct char_data *victim,
	          int dam, int attacktype)
{

   if (DamCheckDeny(ch, victim, attacktype))
     return(FALSE);

   dam = SkipImmortals(victim, dam,attacktype);

   if (!DamDetailsOk(ch, victim, dam, attacktype))
     return(FALSE);

if (attacktype != TYPE_RANGE_WEAPON) { /*this ain't smart, pc's wielding bows? */
     SetVictFighting(ch, victim);
     SetCharFighting(ch, victim);
   }

   dam = DamageTrivia(ch, victim, dam, attacktype);

   if (DoDamage(ch, victim, dam, attacktype))
     return(TRUE);

   DamageMessages(ch, victim, dam, attacktype);

	/* testing !!!! */
if (GET_HIT(ch)<=0 || GET_HIT(victim)<=0) {
	char buff[1024];
	sprintf(buff,"ch=%s with hp=%d, vict=%s with hp=%d attacktype=%d dam=%d",
		GET_NAME(ch),GET_HIT(ch),GET_NAME(victim),GET_HIT(victim),
		attacktype,dam);
	slog(buff);
	}

   if (DamageEpilog(ch, victim,attacktype)) return(TRUE);

   return(FALSE);  /* not dead */
}



int GetWeaponType(struct char_data *ch, struct obj_data **wielded) 
{
	char buf[255];
	  int w_type;

  if (ch->equipment[WIELD] &&
        (ch->equipment[WIELD]->obj_flags.type_flag == ITEM_WEAPON) ) {

    *wielded = ch->equipment[WIELD];
    w_type = Getw_type(*wielded);

  }	else {
    if (IS_NPC(ch) && (ch->specials.attack_type >= TYPE_HIT))
      w_type = ch->specials.attack_type;
    else
      w_type = TYPE_HIT;

    *wielded = 0;  /* no weapon */

  }
  return(w_type);

}

int Getw_type(struct obj_data *wielded) 
{
  int w_type;
  
  
  switch (wielded->obj_flags.value[3]) {
    case 0  : w_type = TYPE_SMITE; break;
    case 1  : w_type = TYPE_STAB;  break;
    case 2  : w_type = TYPE_WHIP; break;
    case 3  : w_type = TYPE_SLASH; break;
    case 4  : w_type = TYPE_SMASH; break;
    case 5  : w_type = TYPE_CLEAVE; break;
    case 6  : w_type = TYPE_CRUSH; break;
    case 7  : w_type = TYPE_BLUDGEON; break;
    case 8  : w_type = TYPE_CLAW; break;
    case 9  : w_type = TYPE_BITE; break;
    case 10 : w_type = TYPE_STING; break;
    case 11 : w_type = TYPE_PIERCE; break;
    case 12 : w_type = TYPE_BLAST; break;
    case 13 : w_type = TYPE_RANGE_WEAPON; break;
    default : w_type = TYPE_HIT; break;
  }
  return(w_type);
}

int HitCheckDeny(struct char_data *ch, struct char_data *victim, int type, 
			int DistanceWeapon)
{
  struct room_data *rp;
  char buf[256];
  extern char PeacefulWorks;

  rp = real_roomp(ch->in_room);
  if (rp && rp->room_flags&PEACEFUL && PeacefulWorks) {
    sprintf(buf, "hit() called in PEACEFUL room");
    log(buf);
    stop_fighting(ch);
    return(TRUE);
  }
  

#if PREVENT_PKILL
		/* this should help stop pkills */
		
 if ( (IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF)) &&
      (IS_PC(victim) || IS_SET(victim->specials.act,ACT_POLYSELF)) &&
      (ch != victim) &&
	!CanFightEachOther(ch,victim)       ) {
     char buf[255];
	sprintf(buf,"%s was found fighing %s!",GET_NAME(ch),GET_NAME(victim));
     log(buf);
     act("You get a errie feeling you should not be doing this, you FLEE!",FALSE,ch,0,victim,TO_CHAR);
     act("$n seems about to attack you, then looks very scared!",FALSE,ch,0,victim,TO_VICT);
     do_flee(ch,"",0);
    }
#endif
    
    if ((ch->in_room != victim->in_room) && !DistanceWeapon) {
        sprintf(buf, "NOT in same room when fighting : %s, %s", ch->player.name, victim->player.name);
        log(buf);
        stop_fighting(ch);
       return(TRUE);
     }

  if (GET_MOVE(ch) < -10) {
    send_to_char("You're too exhausted to fight\n\r",ch);
    stop_fighting(ch);
    return(TRUE);
  }

  
  if (victim->attackers >= 6 && ch->specials.fighting != victim) {
    send_to_char("You can't attack them,  no room!\n\r", ch);
    return(TRUE);
  }

/*
   if the character is already fighting several opponents, and he wants
   to hit someone who is not currently attacking him, then deny them.
   if he is already attacking that person, he can continue, even if they
   stop fighting him.
*/  
  if ((ch->attackers >= 6) && (victim->specials.fighting != ch) &&
      ch->specials.fighting != victim) {
    send_to_char("There are too many other people in the way.\n\r", ch);
    return(TRUE);
  }

#if 0
	/* forces mob/pc to flee if person fighting cuts link */
  if (!IS_PC(ch)) {
    if (ch->specials.fighting && IS_PC(ch->specials.fighting) &&
	!ch->specials.fighting->desc) {
      do_flee(ch,"\0",0);
      return(TRUE);
    }
  }
#endif

	/* force link dead persons to flee from all battles */
	if (IS_LINKDEAD(victim) && (victim->specials.fighting)) {
	     do_flee(victim,"",0);
	    }

	if (IS_LINKDEAD(ch) && (ch->specials.fighting)) {
	     do_flee(ch,"",0);
	    }
	/* end link dead flees */    

#if 0

if (IS_LINKDEAD(ch)) {
    return(TRUE);
  }

#endif

  if (victim == ch) {
    if (Hates(ch,victim)) {
      RemHated(ch, victim);
    }
    return(TRUE);
  }

  if (GET_POS(victim) == POSITION_DEAD)
    return(TRUE);

  if (MOUNTED(ch)) {
    if (!RideCheck(ch, -5)) {
      FallOffMount(ch, MOUNTED(ch));
      Dismount(ch, MOUNTED(ch), POSITION_SITTING);
      return(TRUE);
    }
  } else {
    if (RIDDEN(ch)) {
      if (!RideCheck(RIDDEN(ch),-10)) {
	FallOffMount(RIDDEN(ch), ch);
	Dismount(RIDDEN(ch), ch, POSITION_SITTING);
	return(TRUE);
      }
    }
  }
  

  return(FALSE);

}

int CalcThaco(struct char_data *ch)
{  
  int calc_thaco;
  extern struct str_app_type str_app[];


  
  /* Calculate the raw armor including magic armor */
  /* The lower AC, the better                      */
  
  if (!IS_NPC(ch))
    calc_thaco = thaco[BestFightingClass(ch)][GET_LEVEL(ch, BestFightingClass(ch))];
  else
    /* THAC0 for monsters is set in the HitRoll */
    calc_thaco = 20;
  
    /*  Drow are -4 to hit during daylight or lighted rooms. */
  if (!IS_DARK(ch->in_room) && GET_RACE(ch) == RACE_DROW && IS_PC(ch)
      && !affected_by_spell(ch,SPELL_GLOBE_DARKNESS) && !IS_UNDERGROUND(ch)) { 
       calc_thaco +=4;
      }

  if (IS_SET(ch->specials.affected_by2,AFF2_BERSERK))  {
     calc_thaco +=berserkthaco(ch);
    }

 if (IS_AFFECTED(ch,SPELL_PROTECT_FROM_EVIL)) { /* get +1 to hit evil */
    if (ch->specials.fighting) {
	if (IS_EVIL(ch->specials.fighting)) calc_thaco -=1;    
    }
  }/* end prot evil check */
			
			/* you get -4 to hit a mob if your evil and he has */
			/* prot from evil */
if (ch->specials.fighting) {
 if (IS_AFFECTED(ch->specials.fighting,SPELL_PROTECT_FROM_EVIL)) {
	calc_thaco+=4;
  }
} /* end OTHER end prot evil check */
  
  calc_thaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;
  calc_thaco -= GET_HITROLL(ch);
  calc_thaco += GET_COND(ch, DRUNK)/5;
  return(calc_thaco);
}

int HitOrMiss(struct char_data *ch, struct char_data *victim, int calc_thaco) 
{
  int diceroll, victim_ac;

  extern struct dex_app_type dex_app[];

  diceroll = number(1,20);
  
  victim_ac  = GET_AC(victim)/10;
  
  if (!AWAKE(victim))
    victim_ac -= dex_app[GET_DEX(victim)].defensive;
  
  victim_ac = MAX(-10, victim_ac);  /* -10 is lowest */
  
  if ((diceroll < 20) && AWAKE(victim) &&
      ((diceroll==1) || ((calc_thaco-diceroll) > victim_ac))) {
    return(FALSE);
  } else {
    return(TRUE);
  }
}

int MissVictim(struct char_data *ch, struct char_data *v, int type, int w_type,
	       int (*dam_func)())
{
  struct obj_data *o;

  if (type <= 0) type = w_type;
  if (dam_func == MissileDamage) {
    if (ch->equipment[WIELD]) {
      o = unequip_char(ch, WIELD);
      if (o) {
	act("$p falls to the ground harmlessly", FALSE, ch, o, 0, TO_CHAR);
	act("$p falls to the ground harmlessly", FALSE, ch, o, 0, TO_ROOM);
	obj_to_room(o, ch->in_room);
      }
    }
  }
  (*dam_func)(ch, v, 0, w_type);
}


int GetWeaponDam(struct char_data *ch, struct char_data *v, 
		 struct obj_data *wielded)
{
   int dam, j;
   struct obj_data *obj;
   extern struct str_app_type str_app[];

    
    dam  = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
    dam += GET_DAMROLL(ch);
    
    if (!wielded) {
      if (IS_NPC(ch) || HasClass(ch, CLASS_MONK ))
	dam += dice(ch->specials.damnodice, ch->specials.damsizedice);
      else
	dam += number(0,2);  /* Max. 2 dam with bare hands */
    } else { 
      if (wielded->obj_flags.value[2] > 0) 
      {
#if 0 
	if (wielded->obj_flags.value[0] == 2) 
	{
 
 /* I do not know why they wanted to do this...  */
 /* halves the dam dice on a value[0] ==2, which */ 
 /* should be blank by what the docs say...	 */
 /* I disabled it msw 				 */
 
	  if (GET_HEIGHT(v) < 250) 
	  {
	    dam += dice(wielded->obj_flags.value[1],wielded->obj_flags.value[2]/2);
	    send_to_char("Your weapon seems to be less effective against non-giant-sized opponents\n\r", ch);
	  }/* height */ else 
	  {
	    dam += dice(wielded->obj_flags.value[1],wielded->obj_flags.value[2]);
	  } /* ! height */
	} /* not v[0] = 2 */ 
	else 
	{
	  dam += dice(wielded->obj_flags.value[1],wielded->obj_flags.value[2]);
	}
#else
        dam += dice(wielded->obj_flags.value[1],wielded->obj_flags.value[2]);	
#endif
        
      }  /* !v[2]>0 */ else 
      {
        act("$p snaps into pieces!", TRUE, ch, wielded, 0, TO_CHAR);
	act("$p snaps into pieces!", TRUE, ch, wielded, 0, TO_ROOM);
	if ((obj = unequip_char(ch, WIELD))!=NULL) {
	  MakeScrap(ch,v, obj);
	  dam += 1;
	}
      }
/* aarcerak bug fix..get_str(ch) can't be used because of additional str */
      if (wielded->obj_flags.weight > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) {
	if (ch->equipment[HOLD]) {
	  /*
	    its too heavy to wield properly
	    */
	  dam /= 2;
	}
      }

      /*
	check for the various APPLY_RACE_SLAYER and APPLY_ALIGN_SLAYR
	 here.
       */


      for(j=0; j<MAX_OBJ_AFFECT; j++)       {
	if (wielded->affected[j].location == APPLY_RACE_SLAYER)         {
	  if (wielded->affected[j].modifier == GET_RACE(v)) {
	     dam *= 2;
	    }
	}
	
	if (wielded->affected[j].location ==  APPLY_ALIGN_SLAYER) {
	  if (wielded->affected[j].modifier >=2)
	    if (IS_GOOD(v))  {
	        dam *= 2;
	       }
	  else if (wielded->affected[j].modifier == 0)
	    if (IS_EVIL(v)) {
	        dam *= 2;
	       }
	  else if (wielded->affected[j].modifier ==1)
	    if (!IS_GOOD(v) && !IS_EVIL(v)) {
	        dam *=2;
	       }
	      
	}
      }
    }
    
    if (GET_POS(v) < POSITION_FIGHTING)
      dam *= 1+(POSITION_FIGHTING-GET_POS(v))/3;
    /* Position  sitting  x 1.33 */
    /* Position  resting  x 1.66 */
    /* Position  sleeping x 2.00 */
    /* Position  stunned  x 2.33 */
    /* Position  incap    x 2.66 */
    /* Position  mortally x 3.00 */
    
    if (GET_POS(v) <= POSITION_DEAD)
      return(0);
    
    dam = MAX(1, dam);  /* Not less than 0 damage */
 
    return(dam);
    
}

int LoreBackstabBonus(struct char_data *ch, struct char_data *v)
{
  int mult = 0;
  int learn=0;

  if (IsAnimal(v) && ch->skills[SKILL_CONS_ANIMAL].learned) {
    learn = ch->skills[SKILL_CONS_ANIMAL].learned;
  }
  if (IsVeggie(v) && ch->skills[SKILL_CONS_VEGGIE].learned) {
    learn = MAX(learn, ch->skills[SKILL_CONS_VEGGIE].learned);
  }
  if (IsDiabolic(v) && ch->skills[SKILL_CONS_DEMON].learned) {
    learn = MAX(learn, ch->skills[SKILL_CONS_DEMON].learned);
  }
  if (IsReptile(v) && ch->skills[SKILL_CONS_REPTILE].learned) {
    learn = MAX(learn, ch->skills[SKILL_CONS_REPTILE].learned);
  }
  if (IsUndead(v) && ch->skills[SKILL_CONS_UNDEAD].learned) {
    learn = MAX(learn, ch->skills[SKILL_CONS_UNDEAD].learned);
  }  
  if (IsGiantish(v)&& ch->skills[SKILL_CONS_GIANT].learned) {
    learn = MAX(learn, ch->skills[SKILL_CONS_GIANT].learned);
  }
  if (IsPerson(v) && ch->skills[SKILL_CONS_PEOPLE].learned) {
    learn = MAX(learn, ch->skills[SKILL_CONS_PEOPLE].learned);
  }
  if (IsOther(v)&& ch->skills[SKILL_CONS_OTHER].learned) {
    learn = MAX(learn, ch->skills[SKILL_CONS_OTHER].learned/2);
  }

  if (learn > 40)  
    mult += 1;
  if (learn > 74)
    mult += 1;

  if (mult > 0) 
    send_to_char("Your lore aids your attack!\n\r", ch);

  return(mult);
}

int HitVictim(struct char_data *ch, struct char_data *v, int dam, 
		   int type, int w_type, int (*dam_func)())
{
  extern byte backstab_mult[];
  int dead;

    if (type == SKILL_BACKSTAB) {
      int tmp;
      if (GET_LEVEL(ch, THIEF_LEVEL_IND)) {
	tmp = backstab_mult[GET_LEVEL(ch, THIEF_LEVEL_IND)];
	tmp += LoreBackstabBonus(ch, v);
      } else {
	tmp = backstab_mult[GetMaxLevel(ch)];
      }

      dam *= tmp;
      dead = (*dam_func)(ch, v, dam, type);

    } else {
/*
  reduce damage for dodge skill:
*/

      if (v->skills && v->skills[SKILL_DODGE].learned) {
	if (number(1,101) <= v->skills[SKILL_DODGE].learned) {
	  dam -= number(1,3);
	  if (HasClass(v, CLASS_MONK)) {
	    MonkDodge(ch, v, &dam);
	  }
	}
      }
       dead = (*dam_func)(ch, v, dam, w_type);
    }
      
      /*
       *  if the victim survives, lets hit him with a 
       *  weapon spell
       */
      
    if (!dead) {
	WeaponSpell(ch,v,0,w_type);
    
    }
}


void root_hit(struct char_data *ch, struct char_data *victim, int type, 
	      int (*dam_func)(), int DistanceWeapon)
{
  int w_type, thaco, dam;
  struct obj_data *wielded=0;  /* this is rather important. */

  if (HitCheckDeny(ch, victim, type, DistanceWeapon)) 
     return;

  GET_MOVE(ch) -= 1;

  w_type = GetWeaponType(ch, &wielded);
  if (w_type == TYPE_HIT)
    w_type = GetFormType(ch);  /* races have different types of attack */

  thaco = CalcThaco(ch);

  if (HitOrMiss(ch, victim, thaco)) {
    if ((dam = GetWeaponDam(ch, victim, wielded)) > 0) {
       HitVictim(ch, victim, dam, type, w_type, dam_func);
    } else {
       MissVictim(ch, victim, type, w_type, dam_func);
    }
  } else {
    MissVictim(ch, victim, type, w_type, dam_func);
  }

}

void MissileHit(struct char_data *ch, struct char_data *victim, int type)
{
  root_hit(ch, victim, type, MissileDamage,TRUE); 
}

void hit(struct char_data *ch, struct char_data *victim, int type)
{
  root_hit(ch, victim, type, damage,FALSE); 
}





/* control the fights going on */
void perform_violence(int pulse)
{
  struct char_data *ch, *vict;
  struct obj_data *tmp,*tmp2;
  int i,max,tdir,cmv,max_cmv,caught,rng,tdr,t,found;
  float x;
  int perc;
  
  for (ch = combat_list; ch; ch=combat_next_dude)	{
    struct room_data *rp;
    
    combat_next_dude = ch->next_fighting;

		
    
    rp = real_roomp(ch->in_room);

/*    assert(ch->specials.fighting); */
if (!ch->specials.fighting) {	/* rather this than assert, msw 8/31/94*/
	log("!ch->specials.fighting in perform violence fight.c");
	return;
	} else
    
    if (rp && rp->room_flags&PEACEFUL) {
      char	buf[MAX_INPUT_LENGTH];
      sprintf(buf,"perform_violence() found %s fighting in a PEACEFUL room.",
	      ch->player.name);
      stop_fighting(ch);
      log(buf);
    } else if (ch == ch->specials.fighting) {
      stop_fighting(ch);
    } else {

      
      if (IS_NPC(ch)) {
	struct char_data *rec;
	DevelopHatred(ch, ch->specials.fighting);
	rec = ch->specials.fighting;
	if (!IS_PC(ch->specials.fighting)) {
	  while (rec->master) {
	    if (rec->master->in_room == ch->in_room) {
	      AddHated(ch, rec->master);
	      rec = rec->master;
	    } else {
	      break;
	    }
	  }
	}
      }
      
      if (AWAKE(ch) && (ch->in_room==ch->specials.fighting->in_room) &&
	  (!IS_AFFECTED(ch, AFF_PARALYSIS))) {
	
	if (!IS_NPC(ch)) {

	  /* set x = # of attacks */
	  x = ch->mult_att;

	  /* if dude is a monk, and is wielding something */

	  if (HasClass(ch, CLASS_MONK)) {
	    if (ch->equipment[WIELD]) {
	      /* set it to one, they only get one attack */
	      x = 1.000;
	    }
	  }

	  if (MOUNTED(ch)) {
	      x /= 2.0;
	  }

#if 0	  
	  /* heavy woundage = fewer attacks */
	  x -= WoundWearyness(ch);
#endif
	  
	  /* work through all of their attacks, until there is not
	     a full attack left */
	  
	  tmp = 0;
	  tmp2 = 0;
	  
	  if (DUAL_WIELD(ch)) {
	    tmp = unequip_char(ch, HOLD);
	  }
	  
	  /* have to check for monks holding things. */
	  if(ch->equipment[HOLD] && !(ch->equipment[WIELD]) &&
	     ITEM_TYPE(ch->equipment[HOLD])==ITEM_WEAPON && 
	     HasClass(ch, CLASS_MONK)) {
	    tmp2 = unequip_char(ch, HOLD);
	  }
	  
	  while (x > 0.999) {
	    if (ch->specials.fighting)
	      hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
	    else {
	      x = 0.0;
	      break;
	    }
	    x -= 1.0;
	  }
#if 0	  
	  if(GET_RACE(ch) == RACE_MFLAYER && ch->specials.fighting)
	    MindflayerAttack(ch, ch->specials.fighting);
#endif	
	  if (x > .01) {
#if 0	
    /* check to see if the chance to make the last attack
       is successful 	       */
	    perc = number(1,100);
	    if (perc <= (int)(x*100.0)) {
	      if (ch->specials.fighting)
		hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
	    }
#endif	    
          	/* lets give them the hit */
	      if (ch->specials.fighting)
		hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
	  }

	  if (tmp)
	    equip_char(ch, tmp, HOLD);
	  if(tmp2)
	    equip_char(ch, tmp2, HOLD);


#if 1
	  
	  /* check for the second attack */
	  if (DUAL_WIELD(ch) && ch->skills) {
	    struct obj_data *weapon;
	    int perc;
	    /* check the skill */
	    if ((perc = number(1,101)) < 
		ch->skills[SKILL_DUAL_WIELD].learned){
	    /* if a success, remove the weapon in the wielded hand,
	       place the weapon in the off hand in the wielded hand.
	     */
	      weapon = unequip_char(ch, WIELD);
	      tmp = unequip_char(ch, HOLD);
	      equip_char(ch, tmp, WIELD);
	      /* adjust to_hit based on dex */
	      if (ch->specials.fighting) {
		hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
	      }
	      /* get rid of the to_hit adjustment */
	      /* put the weapons back, checking for destroyed items */
	      if (ch->equipment[WIELD]) {
		tmp = unequip_char(ch, WIELD);
		equip_char(ch, tmp, HOLD);
		equip_char(ch, weapon, WIELD);
	      }
	    } else {
    if (!HasClass(ch,CLASS_RANGER) || number(1,20) > GET_DEX(ch)) {
		tmp = unequip_char(ch, HOLD);
		obj_to_room(tmp, ch->in_room);
		act("You fumble and drop $p", 0, ch, tmp, tmp, TO_CHAR);
		act("$n fumbles and drops $p", 0, ch, tmp, tmp, TO_ROOM);
		if (number(1,20) > GET_DEX(ch)) {
		  tmp = unequip_char(ch, WIELD);
		  obj_to_room(tmp, ch->in_room);
		  act("and you fumble and drop $p too!", 
		      0, ch, tmp, tmp, TO_CHAR);
		  act("and then fumbles and drops $p as well!", 
		      0, ch, tmp, tmp, TO_ROOM);
      if (HasClass(ch,CLASS_RANGER)) {
          LearnFromMistake(ch,SKILL_DUAL_WIELD,FALSE,95);
          }
		      
		}
	      }
	    }
	  }
#endif
	} else {
	  x = ch->mult_att;
	  
	  while (x > 0.999) {
	    if (ch->specials.fighting)
	      hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
	    else {
	      if ((vict = FindAHatee(ch)) != NULL) {
		if (vict->attackers < 6)
		  hit(ch, vict, TYPE_UNDEFINED);
	      } else if ((vict = FindAnAttacker(ch)) != NULL) {
		if (vict->attackers < 6)
		  hit(ch, vict, TYPE_UNDEFINED);
	      }
	    }
	    x -= 1.0;
	  }
#if 0
	  if(GET_RACE(ch) == RACE_MFLAYER && ch->specials.fighting)
	    MindflayerAttack(ch, ch->specials.fighting);
#endif
	  if (x > .01) 
	  {
	    /* check to see if the chance to make the last attack
	       is successful 
	       */
	    perc = number(1,100);
	    if (perc <= (int)(x*100.0)) 
	   {
	      if (ch->specials.fighting) 
	      {
		hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
	      } else 
	    {
		if ((vict = FindAHatee(ch)) != NULL) 
               {
		  if (vict->attackers < 6)
		    hit(ch, vict, TYPE_UNDEFINED);
	       } else if ((vict = FindAnAttacker(ch)) != NULL) 
	       {
		  if (vict->attackers < 6)
		    hit(ch, vict, TYPE_UNDEFINED);
	       }
	    } /* was not fighting */
	   } /* made percent check */
	   
	  }
	}
      } else { /* Not in same room or not awake */
	stop_fighting(ch);
      }
    }
  }
	  /* charging loop */
  for (ch=character_list;ch;ch=ch->next) {
                        /* If charging deal with that */
      if (ch->specials.charging) {
         caught = 0;
         max_cmv = 2;
         cmv = 0;
         while ((cmv<max_cmv)&&(caught==0)) {
            if (ch->in_room==ch->specials.charging->in_room) {
               caught = 1;
            } else {
               /* Continue in a straight line */
               if (clearpath(ch, ch->in_room, ch->specials.charge_dir)) {
                  do_move(ch,"\0",ch->specials.charge_dir+1); 
                  cmv++;
               } else {
                  caught = 2;
               }
            }
         }
         switch (caught) {
            case 1 : /* Caught him */
               act("$n sees $N, and attacks!",TRUE,ch,0,ch->specials.charging,TO_NOTVICT);
               act("$n sees you, and attacks!",TRUE,ch,0,ch->specials.charging,TO_VICT);
               act("You see $N and attack!",TRUE,ch,0,ch->specials.charging,TO_CHAR);
               hit(ch,ch->specials.charging,TYPE_UNDEFINED);
               ch->specials.charging = NULL;
               break;
            case 2 : /* End of line and didn't catch him */
               tdir = can_see_linear(ch,ch->specials.charging,&rng,&tdr);
               if (tdir>-1) {
                  ch->specials.charge_dir = tdr; 
               } else {
                  ch->specials.charging = NULL;
                  act("$n looks around, and sighs dejectedly.",FALSE,ch,0,0,TO_ROOM);
               }
               break;
            default : /* Still charging */
               break;
         } 
     }
  }

	  /* end charge */
  
}



struct char_data *FindVictim( struct char_data *ch)
{
  struct char_data *tmp_ch;
  unsigned char found=FALSE;
  unsigned short ftot=0,ttot=0,ctot=0,ntot=0,mtot=0, ktot=0, dtot=0;
  unsigned short total;
  unsigned short fjump=0,njump=0,cjump=0,mjump=0,tjump=0,kjump=0,djump=0;
  
  if (ch->in_room < 0) return(0);
  
  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;
       tmp_ch=tmp_ch->next_in_room) {
    if ((CAN_SEE(ch,tmp_ch))&&(!IS_SET(tmp_ch->specials.act,PLR_NOHASSLE))&&
	(!IS_AFFECTED(tmp_ch, AFF_SNEAK)) && (ch!=tmp_ch)) {
      if (!IS_SET(ch->specials.act, ACT_WIMPY) || !AWAKE(tmp_ch)) {

	if ((tmp_ch->specials.zone != ch->specials.zone &&
	    !strchr(zone_table[ch->specials.zone].races, GET_RACE(tmp_ch))) ||
	    IS_SET(tmp_ch->specials.act, ACT_ANNOYING)) 
	    
	    {
  if (!in_group(ch, tmp_ch)) {
	    found = TRUE;  /* a potential victim has been found */ 
    if (!IS_NPC(tmp_ch)) {

if (affected_by_spell(tmp_ch,SKILL_DISGUISE) ||
    affected_by_spell(tmp_ch,SKILL_PSYCHIC_IMPERSONATION)) {
	if (number(1,101) > 50) /* 50/50 chance to not attack disguised person */
	    return(NULL);
    }
   if(HasClass(tmp_ch, CLASS_WARRIOR|CLASS_BARBARIAN|CLASS_PALADIN|CLASS_RANGER))
		ftot++;
	      else if (HasClass(tmp_ch, CLASS_CLERIC))
		ctot++;
	      else if (HasClass(tmp_ch,CLASS_MAGIC_USER)
	            || HasClass(tmp_ch,CLASS_SORCERER))
		mtot++;
	      else if (HasClass(tmp_ch, CLASS_THIEF|CLASS_PSI))
		ttot++;
	      else if (HasClass(tmp_ch, CLASS_DRUID)) 
		dtot++;
	      else if (HasClass(tmp_ch, CLASS_MONK)) 
		ktot++;
	    } else {
	      ntot++;
	    }
	  }
	}
      }
    }
  }
  
  /* if no legal enemies have been found, return 0 */
  
  if (!found) {
    return(0);
  }
  
  /* 
    give higher priority to fighters, clerics, thieves,magic users if int <= 12
    give higher priority to fighters, clerics, magic users thieves is inv > 12
    give higher priority to magic users, fighters, clerics, thieves if int > 15
    */
  
  /*
    choose a target  
    */
  
  if (ch->abilities.intel <= 3) {
    fjump=2; cjump=2; tjump=2; njump=2; mjump=2; kjump = 2; djump = 0;
  } else if (ch->abilities.intel <= 9) {
    fjump=4; cjump=3;tjump=2;njump=2;mjump=1; kjump = 2; djump =2;
  } else if (ch->abilities.intel <= 12) {
    fjump=3; cjump=3;tjump=2;njump=2;mjump=2; kjump = 3; djump = 2;
  } else if (ch->abilities.intel <= 15) {
    fjump=3; cjump=3;tjump=2;njump=2;mjump=3; kjump = 2; djump = 2;
  } else {  
    fjump=3;cjump=3;tjump=2;njump=1;mjump=3; kjump = 3; djump = 2;
  }
  
  total = (fjump*ftot)+(cjump*ctot)+(tjump*ttot)+(njump*ntot)+(mjump*mtot)+
    (djump*dtot)+(kjump*ktot);
  
  total = (int) number(1,(int)total);
  
  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;
       tmp_ch=tmp_ch->next_in_room) {
    if ((CAN_SEE(ch,tmp_ch))&&(!IS_SET(tmp_ch->specials.act,PLR_NOHASSLE))&&
	(!IS_AFFECTED(tmp_ch, AFF_SNEAK)) && (ch != tmp_ch)) {
      if (!IS_SET(ch->specials.act, ACT_WIMPY) || !AWAKE(tmp_ch)) {
	if ((tmp_ch->specials.zone != ch->specials.zone &&
	    !strchr(zone_table[ch->specials.zone].races, GET_RACE(tmp_ch))) ||
	    IS_SET(tmp_ch->specials.act, ACT_ANNOYING)) {
	  if (!in_group(ch, tmp_ch)) {
    if (IS_NPC(tmp_ch)) {
      total -= njump;
    } else 
  if (HasClass(tmp_ch,CLASS_WARRIOR|CLASS_BARBARIAN|CLASS_PALADIN|CLASS_RANGER )) {
      total -= fjump;
	    } else if (HasClass(tmp_ch,CLASS_CLERIC)) {
	      total -= cjump;
	    } else if (HasClass(tmp_ch,CLASS_MAGIC_USER)
	            || HasClass(tmp_ch,CLASS_SORCERER))	     {
	      total -= mjump;
	    } else if (HasClass(tmp_ch, CLASS_THIEF|CLASS_PSI)) {
	      total -= tjump;
	    } else if (HasClass(tmp_ch, CLASS_DRUID)) {
	      total -= djump;
	    } else if (HasClass(tmp_ch, CLASS_MONK)) {
	      total -= kjump;
	    }
	    if (total <= 0)
	      return(tmp_ch);
	  }
	}
      }
    }
  }
  
  if (ch->specials.fighting)
    return(ch->specials.fighting);
  
  return(0);
}

struct char_data *FindAnyVictim( struct char_data *ch)
{
  struct char_data *tmp_ch;
  unsigned char found=FALSE;
  unsigned short ftot=0,ttot=0,ctot=0,ntot=0,mtot=0, ktot=0, dtot=0;
  unsigned short total;
  unsigned short fjump=0,njump=0,cjump=0,mjump=0,tjump=0,kjump=0,djump=0;
  
  if (ch->in_room < 0) return(0);
  
  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;tmp_ch=tmp_ch->next_in_room) {
    if ((CAN_SEE(ch,tmp_ch))&&(!IS_SET(tmp_ch->specials.act,PLR_NOHASSLE))) {
      if (!(IS_AFFECTED(ch, AFF_CHARM)) || (ch->master != tmp_ch)) {
	if (!SameRace(ch, tmp_ch) || (!IS_NPC(tmp_ch))) {
	  found = TRUE;  /* a potential victim has been found */ 
  if (!IS_NPC(tmp_ch)) {
    if(HasClass(tmp_ch, CLASS_WARRIOR|CLASS_BARBARIAN|CLASS_RANGER|CLASS_PALADIN ))
	      ftot++;
	    else if (HasClass(tmp_ch, CLASS_CLERIC))
	      ctot++;
	    else if (HasClass(tmp_ch,CLASS_MAGIC_USER)
                  || HasClass(tmp_ch,CLASS_SORCERER))	    
	      mtot++;
	    else if (HasClass(tmp_ch, CLASS_THIEF|CLASS_PSI))
	      ttot++;
	    else if (HasClass(tmp_ch, CLASS_DRUID)) 
	      dtot++;
	    else if (HasClass(tmp_ch, CLASS_MONK)) 
	      ktot++;
	  } else {
	    ntot++;
	  }
	}
      }
    }
  }
  
  /* if no legal enemies have been found, return 0 */
  
  if (!found) {
    return(0);
  }
  
  /* 
    give higher priority to fighters, clerics, thieves, magic users if int <= 12
    give higher priority to fighters, clerics, magic users thieves is inv > 12
    give higher priority to magic users, fighters, clerics, thieves if int > 15
    */
  
  /*
    choose a target  
    */
  
  if (ch->abilities.intel <= 3) {
    fjump=2; cjump=2; tjump=2; njump=2; mjump=2; kjump = 2; djump = 0;
  } else if (ch->abilities.intel <= 9) {
    fjump=4; cjump=3;tjump=2;njump=2;mjump=1; kjump = 2; djump =2;
  } else if (ch->abilities.intel <= 12) {
    fjump=3; cjump=3;tjump=2;njump=2;mjump=2; kjump = 3; djump = 2;
  } else if (ch->abilities.intel <= 15) {
    fjump=3; cjump=3;tjump=2;njump=2;mjump=3; kjump = 2; djump = 2;
  } else {  
    fjump=3;cjump=3;tjump=2;njump=1;mjump=3; kjump = 3; djump = 2;
  }
  
  total = (fjump*ftot)+(cjump*ctot)+(tjump*ttot)+(njump*ntot)+(mjump*mtot)+
    (djump*dtot)+(kjump*ktot);
  
  total = number(1,(int)total);
  
  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;tmp_ch=tmp_ch->next_in_room) {
    if ((CAN_SEE(ch,tmp_ch))&&(!IS_SET(tmp_ch->specials.act,PLR_NOHASSLE))) {
      if (!SameRace(tmp_ch, ch) || (!IS_NPC(tmp_ch))) {
	if (IS_NPC(tmp_ch)) {
	  total -= njump;
} else
 if (HasClass(tmp_ch,CLASS_WARRIOR|CLASS_BARBARIAN|CLASS_PALADIN|CLASS_RANGER)){
	  total -= fjump;
	} else if (HasClass(tmp_ch,CLASS_CLERIC)) {
	  total -= cjump;
	} else if (HasClass(tmp_ch,CLASS_MAGIC_USER)
                  || HasClass(tmp_ch,CLASS_SORCERER))	 {
	  total -= mjump;
	} else if (HasClass(tmp_ch, CLASS_THIEF|CLASS_PSI)) {
	  total -= tjump;
	} else if (HasClass(tmp_ch, CLASS_DRUID)) {
	  total -= djump;
	} else if (HasClass(tmp_ch, CLASS_MONK)) {
	  total -= kjump;
	}
	if (total <= 0)
	  return(tmp_ch);
      }
    }      
  }
  
  if (ch->specials.fighting)
    return(ch->specials.fighting);
  
  return(0);
  
}

int BreakLifeSaverObj( struct char_data *ch)
{

      int found=FALSE, i, j;
      char buf[200];
      struct obj_data *o;

      /*
       *  check eq for object with the effect
       */
      for (i = 0; i< MAX_WEAR && !found; i++) {
	if (ch->equipment[i]) {
	  o = ch->equipment[i];
          for (j=0; j<MAX_OBJ_AFFECT; j++) {
            if (o->affected[j].location == APPLY_SPELL) {
              if (IS_SET(o->affected[j].modifier,AFF_LIFE_PROT)) {
		 found = i;		 
	      }
	    }
	  }
	}
      }
      if (found) {

	/*
         *  break the object.
         */

	 sprintf(buf,"%s shatters with a blinding flash of light!\n\r", 
		 ch->equipment[found]->name);
	 send_to_char(buf, ch);
	 if ((o = unequip_char(ch, found)) != NULL) {
	   MakeScrap(ch,NULL, o);
	 }
      }

}

int BrittleCheck(struct char_data *ch, struct char_data *v, int dam)
{
  char buf[200];
  struct obj_data *obj;

  if (dam <= 0)
    return(FALSE);

  if (ch->equipment[WIELD]) {
    if (IS_OBJ_STAT(ch->equipment[WIELD], ITEM_BRITTLE)) {
       if ((obj = unequip_char(ch,WIELD))!=NULL) {
	 sprintf(buf, "%s shatters.\n\r", obj->short_description);
	 send_to_char(buf, ch);
	 MakeScrap(ch,v, obj);
         return(TRUE);
       }
    }
  }
}

int PreProcDam(struct char_data *ch, int type, int dam)
{
  
  unsigned Our_Bit;
  
  /*
    long, intricate list, with the various bits and the various spells and
    such determined
    */
  
  switch (type) {
  case SPELL_FIREBALL:
  case SPELL_BURNING_HANDS:
  case SPELL_FLAMESTRIKE:
  case SPELL_FIRE_BREATH:
  case SPELL_HEAT_STUFF:
  case SPELL_FIRESTORM:
  case SPELL_INCENDIARY_CLOUD:
  case SKILL_MIND_BURN:
    Our_Bit = IMM_FIRE;
    break;
    
  case SPELL_SHOCKING_GRASP:
  case SPELL_LIGHTNING_BOLT:
  case SPELL_CALL_LIGHTNING:
  case SPELL_LIGHTNING_BREATH:
  case SPELL_CHAIN_LIGHTNING:
    Our_Bit = IMM_ELEC;
    break;

  case SPELL_CHILL_TOUCH:		     
  case SPELL_CONE_OF_COLD:		     
  case SPELL_ICE_STORM:			     
  case SPELL_FROST_BREATH:
    Our_Bit = IMM_COLD;
    break;
    
  case SPELL_MAGIC_MISSILE:
  case SPELL_COLOUR_SPRAY:
  case SPELL_GAS_BREATH:
  case SPELL_METEOR_SWARM:
  case SPELL_SUNRAY:
  case SPELL_DISINTERGRATE:
  
    Our_Bit = IMM_ENERGY;
    break;
    
  case SPELL_ENERGY_DRAIN:
    Our_Bit = IMM_DRAIN;
    break;
    
  case SPELL_ACID_BREATH:
  case SPELL_ACID_BLAST:
    Our_Bit = IMM_ACID;
    break;

  case SKILL_BACKSTAB:
  case TYPE_PIERCE:
  case TYPE_STING:
  case TYPE_STAB:
  case TYPE_RANGE_WEAPON:
    Our_Bit = IMM_PIERCE;
    break;

  case TYPE_SLASH:
  case TYPE_WHIP:
  case TYPE_CLEAVE:
  case TYPE_CLAW:
    Our_Bit = IMM_SLASH;
    break;

  case TYPE_BLUDGEON:
  case TYPE_HIT:
  case SKILL_KICK:
  case TYPE_CRUSH:
  case TYPE_BITE:
  case TYPE_SMASH:
  case TYPE_SMITE:
  case TYPE_BLAST:
    Our_Bit = IMM_BLUNT;
    break;

  case SPELL_POISON:
    Our_Bit = IMM_POISON;
    break;

  default:
    return(dam);
    break;
  }
  
  if (IS_SET(ch->susc, Our_Bit))
    dam <<= 1;
  
  if (IS_SET(ch->immune, Our_Bit))
    dam >>= 1;
  
  if (IS_SET(ch->M_immune, Our_Bit))
    dam = -1;
  
  return(dam);
}


int DamageOneItem( struct char_data *ch, int dam_type, struct obj_data *obj)
{
  int num;
  char buf[256];
  
  num = DamagedByAttack(obj, dam_type);
  if (num != 0) {
    sprintf(buf, "%s is %s.\n\r",obj->short_description, 
	    ItemDamType[dam_type-1]);
    send_to_char(buf,ch);
    if (num == -1) {  /* destroy object*/
      return(TRUE);

    } else {   /* "damage item"  (armor), (weapon) */
      if (DamageItem(ch, obj, num)) {
	return(TRUE);
      }
    }
  }
  return(FALSE);
  
}


void MakeScrap( struct char_data *ch,struct char_data *v, struct obj_data *obj)
{
  char buf[200];
  struct obj_data *t, *x;

  extern char DestroyedItems;
  
  act("$p falls to the ground in scraps.", TRUE, ch, obj, 0, TO_CHAR);
  act("$p falls to the ground in scraps.", TRUE, ch, obj, 0, TO_ROOM);
  
  t = read_object(30, VIRTUAL);
  
  sprintf(buf, "Scraps from %s lie in a pile here.", 
	  obj->short_description);
if (t->description)
  free(t->description);
  t->description = (char *)strdup(buf);
  if (obj->carried_by) {
     obj_from_char(obj);
  } else if (obj->equipped_by) {
     obj = unequip_char(ch, obj->eq_pos);
  }

if (v) {
#if 0
  if  (v->in_room != ch->in_room)	/* for shooting missles */
       obj_to_room(t,v->in_room); else
#endif       
       obj_to_room(t, ch->in_room);
   } else 
     obj_to_room(t, ch->in_room);
   
  t->obj_flags.value[0] = 20;

  while (obj->contains) {
    x = obj->contains;
    obj_from_obj(x);
    obj_to_room(x, ch->in_room);
  }
    

  check_falling_obj(t, ch->in_room);

  extract_obj(obj);

  DestroyedItems = 1;
  
}

void DamageAllStuff( struct char_data *ch, int dam_type)
{
  int j;
  struct obj_data *obj, *next;
  
  /* this procedure takes all of the items in equipment and inventory
     and damages the ones that should be damaged */
  
  /* equipment */
  
  for (j = 0; j < MAX_WEAR; j++) {
    if (ch->equipment[j] && ch->equipment[j]->item_number>=0) {
      obj = ch->equipment[j];
      if (DamageOneItem(ch, dam_type, obj)) { /* TRUE == destroyed */
	if ((obj = unequip_char(ch,j))!=NULL) {
	  MakeScrap(ch,NULL, obj);
	} else {
	  log("hmm, really wierd in DamageAllStuff!");
	}
      }
    }
  }
  
  /* inventory */
  
  obj = ch->carrying;
  while (obj) {
    next = obj->next_content;
    if (obj->item_number >= 0) {
      if (DamageOneItem(ch, dam_type, obj)) {
	MakeScrap(ch,NULL, obj);
      }
    }
    obj = next;
  }
  
}


int DamageItem(struct char_data *ch, struct obj_data *o, int num)
{
  /*  damage weaons or armor */
  
  if (ITEM_TYPE(o) == ITEM_ARMOR) {
    o->obj_flags.value[0] -= num;
    if (o->obj_flags.value[0] < 0) {
      return(TRUE);
    }    
  } else if (ITEM_TYPE(o) == ITEM_WEAPON) {
    o->obj_flags.value[2] -= num;
    if (o->obj_flags.value[2] <= 0) {
      return(TRUE);
    }
  }
  return(FALSE);
}

int ItemSave( struct obj_data *i, int dam_type) 
{
  int num, j;

 /* obj fails save automatically it brittle */ 
if (IS_OBJ_STAT(i,ITEM_BRITTLE)) {
    return(FALSE);
  }
  
/* this is to keep immune objects from getting dammaged */
 if (IS_OBJ_STAT(i, ITEM_IMMUNE)) {
  return(TRUE);
}

/* this is to give resistant magic items a better chance to save */
 if (IS_OBJ_STAT(i, ITEM_RESISTANT)) {
   if (number(1,100)>=50) return(TRUE);
}

  num = number(1,20);
  if (num <= 1) return(FALSE);
  if (num >= 20) return(TRUE);
  
  for(j=0; j<MAX_OBJ_AFFECT; j++)
    if ((i->affected[j].location == APPLY_SAVING_SPELL) || 
	(i->affected[j].location == APPLY_SAVE_ALL)) {
      num -= i->affected[j].modifier;
    }
  if (i->affected[j].location != APPLY_NONE) {
    num += 1;
  }
  if (i->affected[j].location == APPLY_HITROLL) {
    num += i->affected[j].modifier;
  }
  
  if (ITEM_TYPE(i) != ITEM_ARMOR)
    num += 1;
  
  if (num <= 1) return(FALSE);
  if (num >= 20) return(TRUE);
  
  if (num >= ItemSaveThrows[(int)GET_ITEM_TYPE(i)-1][dam_type-1]) {
    return(TRUE);
  } else {
    return(FALSE);
  }
}



int DamagedByAttack( struct obj_data *i, int dam_type)
{
  int num = 0;
  
  if ((ITEM_TYPE(i) == ITEM_ARMOR) || (ITEM_TYPE(i) == ITEM_WEAPON)){
    while (!ItemSave(i,dam_type)) {
      num+=1;
      if (num > 75)
	return(num);  /* so anything with over 75 ac points will not be
			 destroyed */
    }
    return(num);
  } else {
    if (ItemSave(i, dam_type)) {
      return(0);
    } else {
      return(-1);
    }
  }
}

int WeaponCheck(struct char_data *ch, struct char_data *v, int type, int dam)
{
  int Immunity, total, j;
  
  Immunity = -1;
  if (IS_SET(v->M_immune, IMM_NONMAG)) {
    Immunity = 0;
  }
  if (IS_SET(v->M_immune, IMM_PLUS1)) {
    Immunity = 1;
  }
  if (IS_SET(v->M_immune, IMM_PLUS2)) {
    Immunity = 2;
  }
  if (IS_SET(v->M_immune, IMM_PLUS3)) {
    Immunity = 3;
  }
  if (IS_SET(v->M_immune, IMM_PLUS4)) {
    Immunity = 4;
  }
  
  if (Immunity < 0)
    return(dam);
  
  if ((type < TYPE_HIT) || (type > TYPE_RANGE_WEAPON))  
  {
    return(dam);
  } 
  else 
  {
    if (type == TYPE_HIT || IS_NPC(ch)) 
    {
  if (GetMaxLevel(ch) > ((Immunity+1)*(Immunity+1))+6 || 
     (HasClass(ch,CLASS_BARBARIAN)  && BarbarianToHitMagicBonus(ch) >= Immunity))
      {
	return(dam);
      }
        else 
      {
	act("$N ignores your puny attack", FALSE, ch, 0, v, TO_CHAR);
	return(0);
      } /* was not TYPE_HIT or NPC */
      
    } else 
    {
      total = 0;
   if (!ch->equipment[WIELD])
	return(0);
	
      for(j=0; j<MAX_OBJ_AFFECT; j++)
	if ((ch->equipment[WIELD]->affected[j].location == APPLY_HITROLL) ||
	    (ch->equipment[WIELD]->affected[j].location == APPLY_HITNDAM)) 
        {
	  total += ch->equipment[WIELD]->affected[j].modifier;
	}

  if (HasClass(ch,CLASS_BARBARIAN) && BarbarianToHitMagicBonus(ch) > total)
     {
      total = BarbarianToHitMagicBonus(ch);
     }
     
      if (total > Immunity) 
      {
	return(dam);
      }
       else 
      {
	act("$N ignores your puny weapon", FALSE, ch, 0, v, TO_CHAR);
	return(0);
      }     
    }
  }
}


int DamageStuff(struct char_data *v, int type, int dam)
{
  int num, dam_type;
  struct obj_data *obj;
  
/* add a check for anti-magic shell or some other item protection */
/* spell right here I would think */


  if (type >= TYPE_HIT && type <= TYPE_RANGE_WEAPON) {
    num = number(3,17);  /* wear_neck through hold */
    if (v->equipment[num]) {
      if ((type == TYPE_BLUDGEON && dam > 10) ||
	  (type == TYPE_CRUSH && dam > 5) ||
	  (type == TYPE_SMASH && dam > 10) ||
	  (type == TYPE_BITE && dam > 15) ||
	  (type == TYPE_CLAW && dam > 20) ||
	  (type == TYPE_SLASH && dam > 30) ||
	  (type == TYPE_SMITE && dam > 10) ||
	  (type == TYPE_HIT && dam > 20)) {
	if (DamageOneItem(v, BLOW_DAMAGE, v->equipment[num])) {
	  if ((obj = unequip_char(v,num))!=NULL) {
	    MakeScrap(v,NULL, obj);
	  }
	}
      }
    }
  } else {
    dam_type = GetItemDamageType(type);
    if (dam_type) {
      num = number(1,50); /* as this number increases or decreases
			     the chance of item damage decreases
			     or increases */
      if (dam >= num)
	DamageAllStuff(v, dam_type);
    }
  }
  
}


int GetItemDamageType( int type)
{
  
  switch(type) {
  case SPELL_FIREBALL:
  case SPELL_FLAMESTRIKE:
  case SPELL_FIRE_BREATH:
  case SPELL_INCENDIARY_CLOUD:
  case SKILL_MIND_BURN:
    return(FIRE_DAMAGE);
    break;
    
  case SPELL_LIGHTNING_BOLT:
  case SPELL_CALL_LIGHTNING:
  case SPELL_LIGHTNING_BREATH:
    return(ELEC_DAMAGE);
    break;
    
  case SPELL_CONE_OF_COLD:
  case SPELL_ICE_STORM:
  case SPELL_FROST_BREATH:
    return(COLD_DAMAGE);
    break;
    
  case SPELL_COLOUR_SPRAY:
  case SPELL_METEOR_SWARM:
  case SPELL_GAS_BREATH:
  case SPELL_DISINTERGRATE:
    return(BLOW_DAMAGE);
    break;
    
  case SPELL_ACID_BREATH:
  case SPELL_ACID_BLAST:
    return(ACID_DAMAGE);
  default:
    return(0);
    break;  
  }
  
}

int SkipImmortals(struct char_data *v, int amnt,int attacktype)
{
  /* You can't damage an immortal! */
  
  if ((GetMaxLevel(v)>MAX_MORT) && !IS_NPC(v)) 
    amnt = 0;
  
  /* special type of monster */		
  if (IS_NPC(v) && (IS_SET(v->specials.act, ACT_IMMORTAL))) {
    amnt = -1;
  }

#if 1
  if (IS_PC(v) && IS_LINKDEAD(v) && 
     (attacktype == TYPE_SUFFERING ||
       attacktype == SPELL_POISON  ||
         attacktype == SPELL_HEAT_STUFF) ) {  /* link dead pc, no damage */
    amnt = -1;
  }
#endif

  return(amnt);
  
}

#if 0
int WeaponSpell( struct char_data *c, struct char_data *v, int type)
{
  int j, num;
  
  if ((c->in_room == v->in_room) && (GET_POS(v) != POSITION_DEAD)) {
    if ((c->equipment[WIELD]) && ((type >= TYPE_BLUDGEON) &&
				  (type <= TYPE_SMITE))) {
      for(j=0; j<MAX_OBJ_AFFECT; j++) {
	if (c->equipment[WIELD]->affected[j].location ==
	    APPLY_WEAPON_SPELL) {
	  num = c->equipment[WIELD]->affected[j].modifier;
	  ((*spell_info[num].spell_pointer)
	   (6, c, "", SPELL_TYPE_WAND, v, 0));
	}
      }
    }
  }
}
#else
int WeaponSpell( struct char_data *c, struct char_data *v, 
				struct obj_data *obj, int type)
{
  int j, num;
  
  if ( (c->in_room == v->in_room) && (GET_POS(v) != POSITION_DEAD) ||
       (GET_POS(v) !=POSITION_DEAD && type == TYPE_RANGE_WEAPON) ) 
  {
    if ((c->equipment[WIELD]) && 
    (((type >= TYPE_BLUDGEON) && (type <= TYPE_SMITE))) ||
      (type == TYPE_RANGE_WEAPON && obj)) 				  
      {
       struct obj_data *weapon;

       if (type == TYPE_RANGE_WEAPON)
        weapon=obj; else
        weapon=c->equipment[WIELD];
        
      for(j=0; j<MAX_OBJ_AFFECT; j++)       {
	if (weapon->affected[j].location == APPLY_WEAPON_SPELL) 	{
	  num = weapon->affected[j].modifier;
          if(num<=0) 
            num=1;
	  ((*spell_info[num].spell_pointer)
	   (6, c, "", SPELL_TYPE_WAND, v, 0));
	} /* was weapon spell */
      } /* MAX_OBJ for */
    } /* type check */
  } /* in same room */
  
}

#endif

struct char_data *FindAnAttacker(struct char_data *ch) 
{
  struct char_data *tmp_ch;
  unsigned char found=FALSE;
  unsigned short ftot=0,ttot=0,ctot=0,ntot=0,mtot=0, ktot=0, dtot=0;
  unsigned short total;
  unsigned short fjump=0,njump=0,cjump=0,mjump=0,tjump=0,kjump=0,djump=0;
  
  if (ch->in_room < 0) return(0);
  
  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;
       tmp_ch=tmp_ch->next_in_room) {
       if (ch!=tmp_ch) {
	  if (tmp_ch->specials.fighting == ch) {
	      found = TRUE;  /* a potential victim has been found */ 
	      if (!IS_NPC(tmp_ch)) {
	if(HasClass(tmp_ch, CLASS_WARRIOR|CLASS_BARBARIAN|CLASS_PALADIN|CLASS_RANGER ))
		  ftot++;
		else if (HasClass(tmp_ch, CLASS_CLERIC))
		  ctot++;
		else if (HasClass(tmp_ch,CLASS_MAGIC_USER)
                      || HasClass(tmp_ch,CLASS_SORCERER))		
		  mtot++;
		else if (HasClass(tmp_ch, CLASS_THIEF|CLASS_PSI))
		  ttot++;
		else if (HasClass(tmp_ch, CLASS_DRUID)) 
		  dtot++;
		else if (HasClass(tmp_ch, CLASS_MONK)) 
		  ktot++;
	      } else {
		ntot++;
	      }
	    }
	}
     }
  
  /* if no legal enemies have been found, return 0 */
  
  if (!found) {
    return(0);
  }
  
  /* 
    give higher priority to fighters, clerics, thieves, magic users if int <= 12
    give higher priority to fighters, clerics, magic users thieves is inv > 12
    give higher priority to magic users, fighters, clerics, thieves if int > 15
    */
  
  /*
    choose a target  
    */
  
  if (ch->abilities.intel <= 3) {
    fjump=2; cjump=2; tjump=2; njump=2; mjump=2; kjump = 2; djump = 0;
  } else if (ch->abilities.intel <= 9) {
    fjump=4; cjump=3;tjump=2;njump=2;mjump=1; kjump = 2; djump =2;
  } else if (ch->abilities.intel <= 12) {
    fjump=3; cjump=3;tjump=2;njump=2;mjump=2; kjump = 3; djump = 2;
  } else if (ch->abilities.intel <= 15) {
    fjump=3; cjump=3;tjump=2;njump=2;mjump=3; kjump = 2; djump = 2;
  } else {  
    fjump=3;cjump=3;tjump=2;njump=1;mjump=3; kjump = 3; djump = 2;
  }
  
  total = (fjump*ftot)+(cjump*ctot)+(tjump*ttot)+(njump*ntot)+(mjump*mtot)+
    (djump*dtot)+(kjump*ktot);
  
  total = number(1,(int)total);
  
  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;
       tmp_ch=tmp_ch->next_in_room) {
	    if (tmp_ch->specials.fighting == ch) {
	      if (IS_NPC(tmp_ch)) {
		total -= njump;
      } else if (HasClass(tmp_ch,CLASS_WARRIOR|CLASS_BARBARIAN|CLASS_PALADIN|CLASS_RANGER )){
		total -= fjump;
	      } else if (HasClass(tmp_ch,CLASS_CLERIC)) {
		total -= cjump;
	      } else if (HasClass(tmp_ch,CLASS_MAGIC_USER)
                      || HasClass(tmp_ch,CLASS_SORCERER))	       {
		total -= mjump;
	      } else if (HasClass(tmp_ch, CLASS_THIEF|CLASS_PSI)) {
		total -= tjump;
	      } else if (HasClass(tmp_ch, CLASS_DRUID)) {
		total -= djump;
	      } else if (HasClass(tmp_ch, CLASS_MONK)) {
		total -= kjump;
	      }
	      if (total <= 0)
		return(tmp_ch);
	    }
	  }
  
  if (ch->specials.fighting)
    return(ch->specials.fighting);
  
  return(0);
}

void shoot( struct char_data *ch, struct char_data *victim)
{
#if 0
  struct obj_data *bow, *arrow;
  int oldth,oldtd;
  int tohit=0, todam=0;
  
  /*
  **  check for bow and arrow.
  */


  bow = ch->equipment[HOLD];
  arrow = ch->equipment[WIELD];

			/* this is checked in do_shoot now */
  if (!bow) {
    send_to_char("You need a missile weapon (like a bow)\n\r", ch);
    return;
  } else 
  if (!arrow) {
    send_to_char("You need a projectile to shoot!\n\r", ch);
  } else if (!bow && !arrow) {
    send_to_char("You need a bow-like item, and a projectile to shoot!\n\r",ch);
  } else {
    /*
    **  for bows:  value[0] = arror type
    **             value[1] = type 0=short,1=med,2=longranged 
    **             value[2] = + to hit
    **             value[3] = + to damage
    */


    if (bow->obj_flags.value[0] != arrow->obj_flags.value[0]) {
      send_to_char("That projectile does not fit in that projector.\n\r", ch);
      return;
    }
	    
    /*
    **  check for bonuses on the bow.
    */
    tohit = bow->obj_flags.value[2];
    todam = bow->obj_flags.value[3];
    
    /*
    **   temporarily remove other stuff and add bow bonuses.
    */
oldth=GET_HITROLL(ch);
oldtd=GET_DAMROLL(ch);
			/* figure range mods for this weapon */
	if (victim->in_room != ch->in_room)		
		switch(bow->obj_flags.value[1]) {
			case 0:tohit -=4;  /* short range weapon -4 to hit */
				break;
			case 1:tohit -=3;  /* med range weapon -3 to hit */
				break;
			case 2:tohit -=2;  /* long range weapon -2 to hit */
				break;
			default:tohit-=1;  /* unknown, default to -1 tohit */
				break;
			} /* end switch */
/* set tohit and dam to bows only, lets not use cumalitive of what */
/* they already have */

GET_HITROLL(ch)=tohit;
GET_DAMROLL(ch)=todam;

    act("$n shoots $p at $N!", FALSE, ch, arrow, victim, TO_NOTVICT);
    act("$n launches $p at you", FALSE, ch, arrow, victim, TO_VICT);
    act("You shoot at $N with $p", FALSE, ch, arrow, victim, TO_CHAR);

    /*
    **   fire the weapon.
    */
    MissileHit(ch, victim, TYPE_UNDEFINED);

    GET_HITROLL(ch)=oldth;
    GET_DAMROLL(ch)=oldtd;

  }

 slog("end shoot, fight.c");

#endif
}

struct char_data *FindMetaVictim( struct char_data *ch)
{
  struct char_data *tmp_ch;
  unsigned char found=FALSE;
  unsigned short total=0;

  
  if (ch->in_room < 0) return(0);
  
  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;tmp_ch=tmp_ch->next_in_room) {
    if ((CAN_SEE(ch,tmp_ch))&&(!IS_SET(tmp_ch->specials.act,PLR_NOHASSLE))) {
      if (!(IS_AFFECTED(ch, AFF_CHARM)) || (ch->master != tmp_ch)) {
	if (!SameRace(ch, tmp_ch)) {
	   found = TRUE;
	   total++;
	}
      }
    }
  }
  
  /* if no legal enemies have been found, return 0 */
  
  if (!found) {
    return(0);
  }
  
  total = number(1,(int)total);
  
  for (tmp_ch=(real_roomp(ch->in_room))->people;tmp_ch;tmp_ch=tmp_ch->next_in_room) {
    if ((CAN_SEE(ch,tmp_ch))&&(!IS_SET(tmp_ch->specials.act,PLR_NOHASSLE))) {
      if (!SameRace(tmp_ch, ch)){
	total--;
	if (total == 0)
	  return(tmp_ch);
      }
    }
  }
  
  if (ch->specials.fighting)
    return(ch->specials.fighting);
  
  return(0);
  
}


/*
  returns, extracts, switches etc.. anyone.
*/
void NailThisSucker( struct char_data *ch)
{

  struct char_data *pers;
  long room_num;
  char buf[256];
  struct room_data *rp;
  struct obj_data *obj, *next_o;
  
  rp = real_roomp(ch->in_room);
  room_num=ch->in_room;
  
  death_cry(ch);
    
  if (IS_NPC(ch) && (IS_SET(ch->specials.act, ACT_POLYSELF))) {
    /*
     *   take char from storage, to room     
     */
    pers = ch->desc->original;
    char_from_room(pers);
    char_to_room(pers, ch->in_room);
    SwitchStuff(ch, pers);
    extract_char(ch);
    ch = pers;
  }
  zero_rent(ch);
  extract_char(ch);

		/* delete EQ dropped by them if room was a DT */
if (IS_SET(rp->room_flags,DEATH)) {
      sprintf(buf,"%s hit a DeathTrap in room %s[%ld]\r\n",GET_NAME(ch),\
	real_roomp(room_num)->name,room_num);
      log(buf);
      for (obj = real_roomp(room_num)->contents;obj; obj = next_o) {
	next_o = obj->next_content;
	extract_obj(obj);
      }  /* end DT for */
    }
    
}


int GetFormType(struct char_data *ch)
{
  int num;

  num = number(1,100);
  switch(GET_RACE(ch)) {
  case RACE_REPTILE:
    if (num <= 50) {
      return(TYPE_CLAW);
    } else {
      return(TYPE_BITE);
    }
    break;
  case RACE_LYCANTH:
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
 case RACE_DRAGON_BRASS  :  
  case RACE_PREDATOR:
  case RACE_LABRAT:
    if (num <= 33) {
      return(TYPE_BITE);
    } else {
      return(TYPE_CLAW);
    }
    break;
  case RACE_INSECT:
    if (num <= 50) {
      return(TYPE_BITE);
    } else {
      return(TYPE_STING);
    }
    break;
  case RACE_ARACHNID:
  case RACE_DINOSAUR:
  case RACE_FISH:
  case RACE_SNAKE:
    return(TYPE_BITE);
    break;
  case RACE_BIRD:
  case RACE_SKEXIE:
    return(TYPE_CLAW);
    break;
  case RACE_GIANT:
  case RACE_GIANT_HILL   :
  case RACE_GIANT_FROST  :
  case RACE_GIANT_FIRE   :
  case RACE_GIANT_CLOUD  :
  case RACE_GIANT_STORM  :
    case RACE_GIANT_STONE  :
  case RACE_GOLEM:
    return(TYPE_BLUDGEON);
    break;
  case RACE_DEMON:
  case RACE_DEVIL:
  case RACE_TROLL:
  case RACE_TROGMAN:
  case RACE_LIZARDMAN:
    return(TYPE_CLAW);
    break;
  case RACE_TREE:
    return(TYPE_SMITE);
    break;
  case RACE_MFLAYER:
    if (num <= 60) {
      return(TYPE_WHIP);
    } else if (num < 80) {
      return(TYPE_BITE);
    } else {
      return(TYPE_HIT);
    }
    break;
  case RACE_PRIMATE:
    if (num <= 70) {
      return(TYPE_HIT);
    } else {
      return(TYPE_BITE);
    }
    break;
  case RACE_TYTAN:
    return(TYPE_BLAST);
    break;
  default:
    return(TYPE_HIT);
  }
}

int MonkDodge( struct char_data *ch, struct char_data *v, int *dam)
{
  if (number(1, 20000) < v->skills[SKILL_DODGE].learned*
                         GET_LEVEL(v , MONK_LEVEL_IND)) { 
    *dam = 0;
    act("You dodge the attack", FALSE, ch, 0, v, TO_VICT);
    act("$N dodges the attack", FALSE, ch, 0, v, TO_CHAR);
    act("$N dodges $n's attack", FALSE, ch, 0, v, TO_NOTVICT);
  } else {
    *dam -= GET_LEVEL(ch, MONK_LEVEL_IND)/10;
  }

  return(0);
}

int BarbarianToHitMagicBonus ( struct char_data *ch)
{
 int bonus=0;
 
 if (GetMaxLevel(ch) <=7)
   bonus = 1;
    else
 if (GetMaxLevel(ch) <= 8)  
    bonus = 2;
    else
 if (GetMaxLevel(ch) <= 20)   
    bonus = 3;
    else
 if (GetMaxLevel(ch) <= 28)   
    bonus = 4;
    else
 if (GetMaxLevel(ch) <= 35)   
    bonus = 5;
    else
if (GetMaxLevel(ch) >35)
    bonus = 5;
    
 return(bonus);
}

int berserkthaco ( struct char_data *ch)
{
  if (GetMaxLevel(ch) <= 10) /* -5 to hit when berserked */
   return(5);
  if (GetMaxLevel(ch) <= 25) /* -3 */
   return(3);
  if (GetMaxLevel(ch) <= 40) /* -2 */
   return(2);
   return(2);
}

int berserkdambonus ( struct char_data *ch, int dam)
{
  if (GetMaxLevel(ch) <= 10)     /* 1.33 dam when berserked */
   return((int)dam*1.33); 
    else
  if (GetMaxLevel(ch) <= 25)     /* 1.5 */
   return((int)dam*1.5); 
    else
  if (GetMaxLevel(ch) <= 40)     /* 1.7 */
   return((int)dam*1.7); 
    else
   return((int)dam*1.7);
   
}


int range_hit(struct char_data *ch, struct char_data *targ, int rng, struct
   obj_data *missile, int tdir, int max_rng) {
   /* Returns 1 on a hit, 0 otherwise */
   /* Does the roll, damage, messages, and everything */
 
   int calc_thaco, i, dam = 0, diceroll, victim_ac;
   char *dir_name[] = {
       "the north",
       "the east",
       "the south",
       "the west",
       "above",
       "below"};
   int opdir[] = {2, 3, 0, 1, 5, 4}, rmod, cdir, rang, cdr;
   char buf[MAX_STRING_LENGTH];
   extern struct dex_app_type dex_app[];

   if (!IS_NPC(ch)) {
      calc_thaco=20;
      for(i=1;i<5;i++) {
         if (thaco[i-1][GetMaxLevel(ch)]<calc_thaco) {
            calc_thaco=thaco[i-1][GetMaxLevel(ch)];
         }
      }
   } else {
      /* THAC0 for monsters is set in the HitRoll */
      calc_thaco = 20;
   }

   calc_thaco -= GET_HITROLL(ch);
   rmod = 20*rng/max_rng;
   calc_thaco += rmod;
   if (GET_POS(targ)==POSITION_SITTING) {
      calc_thaco += 7;
   }
   if (GET_POS(targ)==POSITION_RESTING) {
      calc_thaco += 10;
   }

   diceroll = number(1,20);

   victim_ac  = GET_AC(targ)/10;

   if (AWAKE(targ))
      victim_ac += dex_app[GET_DEX(targ)].defensive;

   victim_ac = MAX(-10, victim_ac);  /* -10 is lowest */

   if ((diceroll < 20) &&
       ((diceroll==1) || ((calc_thaco-diceroll) > victim_ac))) {
      /* Missed! */
      if (rng>0) {
         sprintf(buf,"$p from %s narrowly misses $n!",dir_name[opdir[tdir]]);
         act(buf,FALSE,targ,missile,0,TO_ROOM);
         act("$p misses $N.",TRUE,ch,missile,targ,TO_CHAR);
      } else {
          act("$p narrowly misses $n!",FALSE,targ,missile,0,TO_ROOM);
      }
      if (AWAKE(targ)) {
         if (rng>0) {
            sprintf(buf,"$p whizzes past from %s, narrowly missing you!",dir_name[opdir[tdir]]);
            act(buf,TRUE,targ,missile,0,TO_CHAR);
         } else {
            act("$n fires $p at you, narrowly missing!",TRUE,ch,missile,targ,TO_VICT);
         }
         if (IS_NPC(targ)) {
            if (rng==0) {
               hit(targ,ch,TYPE_UNDEFINED);
            } else {

#if 1
            cdir = can_see_linear(targ,ch,&rang,&cdr);
            if (!(targ->specials.charging)) {
             if (number(1,10)<4) {
              if (cdir!=-1) {
               if (GET_POS(targ)==POSITION_STANDING) { 
               /* Ain't gonna take any more of this missile crap! */
               AddHated(targ,ch);
               act("$n roars angrily and charges!",TRUE,targ,0,0,TO_ROOM);
               targ->specials.charging = ch;
               targ->specials.charge_dir = cdr;
               }
              }
             }
            }
#endif            
            }
         }
      }
      return 0;
   } else {
      dam += dice(missile->obj_flags.value[1],missile->obj_flags.value[2]);
      dam = MAX(1, dam);  /* Not less than 0 damage */
      AddHated(targ,ch);
      sprintf(buf,"$p from %s hits $n!",dir_name[opdir[tdir]]);
      act("$p hits $N!",TRUE,ch,missile,targ,TO_CHAR);
      act(buf,TRUE,targ,missile,0,TO_ROOM);
      sprintf(buf,"$p from %s hits you!",dir_name[opdir[tdir]]);
      act(buf,TRUE,targ,missile,0,TO_CHAR);
      damage(ch, targ, dam, TYPE_RANGE_WEAPON);
      
      
      if ((!targ)||(GET_HIT(targ)<1)) return 1;
      else {
        if(ch->in_room != targ->in_room) {
	  WeaponSpell(ch,targ,missile,TYPE_RANGE_WEAPON);
	}
      }

#if 1
      if ((GET_POS(targ)!=POSITION_FIGHTING)&&(GET_POS(targ)>POSITION_STUNNED)) 
      {
         if ((IS_NPC(targ))&&(!targ->specials.charging)) 
         {
            GET_POS(targ) = POSITION_STANDING;
            if (rng==0) 
            {
               hit(targ,ch,TYPE_UNDEFINED);
            } else 
            {
               cdir = can_see_linear(targ,ch,&rang,&cdr);
               if (cdir!=-1) 
               {
                  act("$n roars angrily and charges!",TRUE,targ,0,0,TO_ROOM);
                  targ->specials.charging = ch;
                  targ->specials.charge_dir = cdr;
               }
            }
         }
      }
#endif      
      return 1;
   }
}

