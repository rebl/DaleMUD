
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/file.h>


#include "protos.h"

/* move to protos.h later */

int SetDefaultLang(struct char_data *ch);

/* end moves */


void log_sev(char *s, int i);
void log (char *s) { log_sev(s, 1); }
extern long SystemFlags;
extern struct time_data time_info;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct index_data *mob_index, *obj_index;
extern struct chr_app_type chr_apply[];
extern struct zone_data *zone_table;
#if HASH
extern struct hash_header room_db;	                  /* In db.c */
#else
extern struct room_data *room_db[];	                  /* In db.c */
#endif
extern char *dirs[]; 
extern int  RacialMax[][MAX_CLASS];
extern int top_of_zone_table;
extern int top_of_world;
extern struct descriptor_data *descriptor_list;

/* external functions */
void stop_fighting(struct char_data *ch);
void fake_setup_dir(FILE *fl, long room, int dir);
char *fread_string(FILE *fl);
int check_falling(struct char_data *ch);
void NailThisSucker( struct char_data *ch);

/* interal move to protos.h */
int IS_UNDERGROUND(struct char_data *ch);


#if EGO
int EgoBladeSave(struct char_data *ch)
{
   int total;
   
   if (GetMaxLevel(ch) <= 10) return(FALSE);
   total = (GetMaxLevel(ch) + GET_STR(ch) + GET_CON(ch));
   if (GET_HIT(ch) == 0) return(FALSE);
   total = total - (GET_MAX_HIT(ch) / GET_HIT(ch));
   if (number(1,101) > total) {
      return(FALSE);
   } else return(TRUE);
}
#endif

int MIN(int a, int b)
{
	return a < b ? a:b;
}


int MAX(int a, int b)
{
	return a > b ? a:b;
}

int GetItemClassRestrictions(struct obj_data *obj)
{
  int total=0;

  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MAGE)) {
    total += CLASS_MAGIC_USER;
  }
  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_THIEF)) {
    total += CLASS_THIEF;
  }
  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_FIGHTER)) {
    total += CLASS_WARRIOR;
  }
  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_CLERIC)) {
    total += CLASS_CLERIC;
  }

  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_BARBARIAN)) {
    total += CLASS_BARBARIAN;
  }

  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_RANGER)) {
    total += CLASS_RANGER;
  }

  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_PALADIN)) {
    total += CLASS_PALADIN;
  }

  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_PSI)) {
    total += CLASS_PSI;
  }

  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MONK)) {
    total += CLASS_MONK;
  }

  if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_DRUID)) {
    total += CLASS_DRUID;
  }

  return(total);

}

int CAN_SEE(struct char_data *s, struct char_data *o)
{

  if (!o || s->in_room <= 0 || o->in_room <= 0){
    return(FALSE);
    }


  if (o->invis_level > GetMaxLevel(s))  
    return FALSE;                       

  if (IS_IMMORTAL(s)) 
    return(TRUE);

  if (IS_AFFECTED(s, AFF_TRUE_SIGHT))
    return(TRUE);

  if (IS_AFFECTED(s, AFF_BLIND) || IS_AFFECTED(o, AFF_HIDE))
    return(FALSE);

  if (IS_AFFECTED(o, AFF_INVISIBLE)) {
    if (IS_IMMORTAL(o))
      return(FALSE);
    if (!IS_AFFECTED(s, AFF_DETECT_INVISIBLE))
      return(FALSE);
  }

  if ((IS_DARK(s->in_room) || IS_DARK(o->in_room)) &&
        (!IS_AFFECTED(s, AFF_INFRAVISION)))
        return(FALSE);

  if (IS_AFFECTED2(o, AFF2_ANIMAL_INVIS) && IsAnimal(s))
    return(FALSE);

  return(TRUE);
}

int CAN_SEE_OBJ( struct char_data *ch, struct obj_data *obj)
{
int num=0;

  if (IS_IMMORTAL(ch))
    return(1);
  


/* changed the act.info.c, hope this works on traps INSIDE chests etc.. */
/* msw */

#if 1
if ((ITEM_TYPE(obj) == ITEM_TRAP) && (GET_TRAP_CHARGES(obj) > 0)) 
	{
	num = number(1,101);
         if (CanSeeTrap(num,ch))  {
	     return(TRUE);
	   } else return(FALSE);
        } /* not a trap */
#endif

  if (IS_AFFECTED(ch, AFF_TRUE_SIGHT))
    return(1);


  if (IS_DARK(ch->in_room) && !IS_OBJ_STAT(obj, ITEM_GLOW))
    return(0);

  if (IS_AFFECTED(ch, AFF_BLIND))
    return(0);

  if (IS_AFFECTED(ch, AFF_DETECT_INVISIBLE))
    return(1);

  if (IS_OBJ_STAT(obj, ITEM_INVISIBLE))
    return(0);

  return(1);
}

int exit_ok(struct room_direction_data	*exit, struct room_data **rpp)
{
  struct room_data	*rp;
  if (rpp==NULL)
    rpp = &rp;
  if (!exit) {
    *rpp = NULL;
    return FALSE;
  }
  *rpp = real_roomp(exit->to_room);
  return (*rpp!=NULL);
}

long MobVnum( struct char_data *c)
{
  if (IS_NPC(c)) {
    return(mob_index[c->nr].virtual);
  } else {
    return(0);
  }
}

long ObjVnum( struct obj_data *o)
{
  if (o->item_number >= 0)
     return(obj_index[o->item_number].virtual);
  else
    return(-1);
}


void Zwrite (FILE *fp, char cmd, int tf, int arg1, int arg2, int arg3, 
	     char *desc)
{
   char buf[100];

   if (*desc) {
     sprintf(buf, "%c %d %d %d %d   ; %s\n", cmd, tf, arg1, arg2, arg3, desc);
     fputs(buf, fp);
   } else {
     sprintf(buf, "%c %d %d %d %d\n", cmd, tf, arg1, arg2, arg3); 
     fputs(buf, fp);
   }
}

void RecZwriteObj(FILE *fp, struct obj_data *o)
{
   struct obj_data *t;

   if (ITEM_TYPE(o) == ITEM_CONTAINER) {
     for (t = o->contains; t; t=t->next_content) {
       Zwrite(fp, 'P', 1, ObjVnum(t), obj_index[t->item_number].number, ObjVnum(o), 
	      t->short_description);
       RecZwriteObj(fp, t);
     }
   } else {
     return;
   }
}

 
int SaveZoneFile(FILE *fp, int start_room, int end_room)
{
  struct char_data *p;
  struct obj_data *o;
  struct room_data *room;
  char cmd, c, buf[80];
  int i, j, arg1, arg2, arg3;
 
 
  for (i = start_room; i<=end_room; i++) {
    room = real_roomp(i);
    if (room) {
      /*
       *  first write out monsters
       */
      for (p = room->people; p; p = p->next_in_room) {
        if (IS_NPC(p)) {
          cmd = 'M';
          arg1 = MobVnum(p);
          arg2 = mob_index[p->nr].number;
          arg3 = i;
          Zwrite(fp, cmd, 0, arg1, arg2, arg3, p->player.short_descr);
          for (j = 0; j<MAX_WEAR; j++) {
            if (p->equipment[j]) {
              if (p->equipment[j]->item_number >= 0) {
                cmd = 'E';
                arg1 = ObjVnum(p->equipment[j]);
                arg2 = obj_index[p->equipment[j]->item_number].number;
                arg3 = j;
                strcpy(buf, p->equipment[j]->short_description);
                Zwrite(fp, cmd,1,arg1, arg2, arg3, 
                       buf);
                RecZwriteObj(fp, p->equipment[j]);
              }
            }
          }
          for (o = p->carrying; o; o=o->next_content) {
            if (o->item_number >= 0) {
              cmd = 'G';
              arg1 = ObjVnum(o);
              arg2 = obj_index[o->item_number].number;
              arg3 = 0;
              strcpy(buf, o->short_description);
              Zwrite(fp, cmd, 1, arg1, arg2, arg3, buf);
              RecZwriteObj(fp, o);
            }
          }
        }
      }
      /*
       *  write out objects in rooms
       */
      for (o = room->contents; o; o= o->next_content) {
        if (o->item_number >= 0) {
          cmd = 'O';
          arg1 = ObjVnum(o);
          arg2 = obj_index[o->item_number].number;
          arg3 = i;
          strcpy(buf, o->short_description);
          Zwrite(fp, cmd, 0, arg1, arg2, arg3, buf);
          RecZwriteObj(fp, o);
        }
      }
      /*
       *  lastly.. doors
       */
 
      for (j = 0; j < 6; j++) {
        /*
         *  if there is an door type exit, write it.
         */
        if (room->dir_option[j]) {  /* is a door */
          if (room->dir_option[j]->exit_info) {
            cmd = 'D';
            arg1 = i ;
            arg2 = j;
            arg3 = 0;
            if (IS_SET(room->dir_option[j]->exit_info, EX_CLOSED)) {
              arg3 = 1;
            }
            if (IS_SET(room->dir_option[j]->exit_info, EX_LOCKED)) {
              arg3 = 2;
            }
            Zwrite(fp, cmd, 0, arg1, arg2, arg3, room->name);
          }
        }
      }
    }
  }
  fprintf(fp,"S\n");
return 1;
}
 
 
int LoadZoneFile(FILE *fl, int zon)
{
  int cmd_no = 0, expand, tmp, cc = 22;
  char *check, buf[81];
 
  if(zone_table[zon].cmd) {
    free(zone_table[zon].cmd);
  }
 
  /* read the command table */
  cmd_no = 0;
  for (expand = 1;!feof(fl);)          {
    if (expand) {
      if (!cmd_no)
        CREATE(zone_table[zon].cmd, struct reset_com, cc);
      else
       if (cmd_no >= cc) {
         cc += 5;
         if (!(zone_table[zon].cmd =
                (struct reset_com *) realloc(zone_table[zon].cmd,
                               (cc * sizeof(struct reset_com)))))  {
                perror("reset command load");
                assert(0);
         }
       }
    }
 
    expand = 1;
 
    fscanf(fl, " "); /* skip blanks */
    fscanf(fl, "%c", &zone_table[zon].cmd[cmd_no].command);
 
    if (zone_table[zon].cmd[cmd_no].command == 'S')
      break;
 
    if (zone_table[zon].cmd[cmd_no].command == '*')   {
      expand = 0;
      fgets(buf, 80, fl); /* skip command */
      continue;
    }
 
    fscanf(fl, " %d %d %d",
             &tmp,
             &zone_table[zon].cmd[cmd_no].arg1,
             &zone_table[zon].cmd[cmd_no].arg2);
    zone_table[zon].cmd[cmd_no].if_flag=tmp;
 
    switch(zone_table[zon].cmd[cmd_no].command) {
      case 'M':
      case 'O':
      case 'C':
      case 'E':
      case 'P':
      case 'D':
        fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);
         break;
    }
    fgets(buf, 80, fl);       /* read comment */
    cmd_no++;
  }
return 1;
}

void CleanZone(int zone)
{
  struct room_data *rp;
  struct char_data *vict, *next_v;
  struct obj_data *obj, *next_o;
  int room,start,end,i;
 
  start=zone?(zone_table[zone-1].top+1):0;
  end=zone_table[zone].top;

  for(i=start;i<=end;i++) {
    rp=real_roomp(i);
    if(!rp) continue;
 
    for (vict = rp->people; vict; vict=next_v) {
      next_v=vict->next_in_room;
      if (IS_NPC(vict) && (!IS_SET(vict->specials.act, ACT_POLYSELF)))
        extract_char(vict);
      else
        send_to_char("\r\n\r\nSwirling winds of Chaos reform reality around you!\r\n\r\n",vict);
    }
    
    for (obj = rp->contents; obj; obj = next_o) {
      next_o=obj->next_content;
      extract_obj(obj);
    }
  }
}
 
 
int FindZone(int zone)
{
        int i;
  for(i=0;i<=top_of_zone_table;i++)
    if(zone_table[i].num==zone)
       break;
  if(zone_table[i].num!=zone)
    return(-1);
  else
    return i;
}
 
FILE *MakeZoneFile( struct char_data *c, int zone)
{
  char buf[256];
  FILE *fp;
 
  sprintf(buf, "zones/%d.zon", zone);
 
  if ((fp = fopen(buf, "wt")) != NULL)
    return(fp);
  else
    return(0);
 
}
 
FILE *OpenZoneFile(struct char_data *c, int zone)
{
  char buf[256];
  FILE *fp;
 
  sprintf(buf, "zones/%d.zon", zone);
 
  if ((fp = fopen(buf, "rt")) != NULL)
    return(fp);
  else
    return(0);
}

int WeaponImmune(struct char_data *ch)
{

  if (IS_SET(IMM_NONMAG, ch->M_immune) ||
      IS_SET(IMM_PLUS1, ch->M_immune) ||
      IS_SET(IMM_PLUS2, ch->M_immune) ||
      IS_SET(IMM_PLUS3, ch->M_immune) ||
      IS_SET(IMM_PLUS4, ch->M_immune))
    return(TRUE);
   return(FALSE);

}

unsigned IsImmune(struct char_data *ch, int bit)
{
  return(IS_SET(bit, ch->M_immune));
}

unsigned IsResist(struct char_data *ch, int bit)
{
  return(IS_SET(bit, ch->immune));
}

unsigned IsSusc(struct char_data *ch, int bit)
{
  return(IS_SET(bit, ch->susc));
}

/* creates a random number in interval [from;to] */
int number(int from, int to) 
{
   if (to - from + 1 )
	return((random() % (to - from + 1)) + from);
   else
       return(from);
}



/* simulates dice roll */
int dice(int number, int size) 
{
  int r;
  int sum = 0;

#if 0
	assert(size >= 0);
#else
/* instead of crashing the mud we set it to 1 */
if (size <= 0)
    size=1;
#endif

  if (size == 0) return(0);

  for (r = 1; r <= number; r++) sum += ((random() % size)+1);
  return(sum);
}


/* Causing memory leak, but is a standard c function, so commenting out. -DM


char *strdup(char *source)
{
	char *new;

	CREATE(new, char, strlen(source)+1);
	return(strcpy(new, source));
}
*/

int scan_number(char *text, int *rval)
{
  int	length;
  if (1!=sscanf(text, " %i %n", rval, &length))
    return 0;
  if (text[length] != 0)
    return 0;
  return 1;
}


/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int str_cmp(char *arg1, char *arg2)
{
  #if 1 
int chk, i;

  if ((!arg2) || (!arg1))
    return(1);

  for (i = 0; *(arg1 + i) || *(arg2 + i); i++)
    if (chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))
      if (chk < 0)
	return (-1);
      else 
	return (1);
  return(0);
#else
 return(strcmp(arg1,arg2));
#endif
}



/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int strn_cmp(char *arg1, char *arg2, int n)
{
#if 1 
  int chk, i;
  
  for (i = 0; (*(arg1 + i) || *(arg2 + i)) && (n>0); i++, n--)
    if (chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))
      if (chk < 0)
	return (-1);
      else 
	return (1);
  
  return(0);
#else
return(strncmp(arg1,arg2,n));
#endif

}



/* writes a string to the log */
void log_sev(char *str,int sev)
{
  long ct;
  char *tmstr;
  static char buf[500];
  struct descriptor_data *i;
  
  
  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  fprintf(stderr, "%s :: %s\n", tmstr, str);
  
  
  if (str)
    sprintf(buf,"/* %s */\n\r",str);
  for (i = descriptor_list; i; i = i->next)
    if ((!i->connected) && (GetMaxLevel(i->character)>=LOW_IMMORTAL) &&
	(i->character->specials.sev <= sev) &&
	(!IS_SET(i->character->specials.act, PLR_NOSHOUT)))
      SEND_TO_Q(buf, i);
}

void slog(char *str)
{
	long ct;
	char *tmstr;

	ct = time(0);
	tmstr = asctime(localtime(&ct));
	*(tmstr + strlen(tmstr) - 1) = '\0';
	fprintf(stderr, "%s :: %s\n", tmstr, str);

}
	


void sprintbit(unsigned long vektor, char *names[], char *result)
{
  long nr;
  
  *result = '\0';
  
  for(nr=0; vektor; vektor>>=1)
    {
      if (IS_SET(1, vektor))
	if (*names[nr] != '\n') {
	  strcat(result,names[nr]);
	  strcat(result," ");
	} else {
	  strcat(result,"UNDEFINED");
	  strcat(result," ");
	}
      if (*names[nr] != '\n')
	nr++;
    }
  
  if (!*result)
    strcat(result, "NOBITS");
}



void sprinttype(int type, char *names[], char *result)
{
	int nr;

	for(nr=0;(*names[nr]!='\n');nr++);
	if(type < nr)
		strcpy(result,names[type]);
	else
		strcpy(result,"UNDEFINED");
}


/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
	long secs;
	struct time_info_data now;

	secs = (long) (t2 - t1);

  now.hours = (secs/SECS_PER_REAL_HOUR) % 24;  /* 0..23 hours */
  secs -= SECS_PER_REAL_HOUR*now.hours;

  now.day = (secs/SECS_PER_REAL_DAY);          /* 0..34 days  */
  secs -= SECS_PER_REAL_DAY*now.day;

	now.month = -1;
  now.year  = -1;

	return now;
}




/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
  long secs, monthsecs, daysecs, hoursecs;
  struct time_info_data now;

/* eld (6-9-93) -- Hopefully, this will fix the negative month, day, etc.  */
/*                 problems...                                             */

  if(t2 >= t1) { 
    secs = (long) (t2 - t1);

    now.year = secs/SECS_PER_MUD_YEAR;

    monthsecs = secs % SECS_PER_MUD_YEAR;
    now.month = monthsecs/SECS_PER_MUD_MONTH;
    
    daysecs = monthsecs % SECS_PER_MUD_MONTH;
    now.day = daysecs/SECS_PER_MUD_DAY;

    hoursecs = daysecs % SECS_PER_MUD_DAY;
    now.hours = hoursecs/SECS_PER_MUD_HOUR;
  } else {
    secs = (long) (t1 - t2);

    now.year = secs/SECS_PER_MUD_YEAR;

    monthsecs = secs % SECS_PER_MUD_YEAR;
    now.month = monthsecs/SECS_PER_MUD_MONTH;
    
    daysecs = monthsecs % SECS_PER_MUD_MONTH;
    now.day = daysecs/SECS_PER_MUD_DAY;

    hoursecs = daysecs % SECS_PER_MUD_DAY;
    now.hours = hoursecs/SECS_PER_MUD_HOUR;

    if(now.hours) {
      now.hours = 24 - now.hours;
      now.day = now.day + 1;
    }
    if(now.day) {
      now.day = 35 - now.day;
      now.month = now.month + 1;
    }
    if(now.month) {
      now.month = 17 - now.month;
      now.year = now.year + 1;
    }
    if(now.year)
      now.year = -now.year;                
  }
  return(now);

}

void mud_time_passed2(time_t t2, time_t t1, struct time_info_data *t)
{
  long secs, monthsecs, daysecs, hoursecs;

/* eld (6-9-93) -- Hopefully, this will fix the negative month, day, etc.  */
/*                 problems...                                             */


  if(t2 >= t1) {
    secs = (long) (t2 - t1);
  
    t->year = secs/SECS_PER_MUD_YEAR;

    monthsecs = secs % SECS_PER_MUD_YEAR;
    t->month = monthsecs/SECS_PER_MUD_MONTH;

    daysecs = monthsecs % SECS_PER_MUD_MONTH;
    t->day = daysecs/SECS_PER_MUD_DAY;

    hoursecs = daysecs % SECS_PER_MUD_DAY;
    t->hours = hoursecs/SECS_PER_MUD_HOUR;
  }else {

    secs = (long) (t1 - t2);
  
    t->year = secs/SECS_PER_MUD_YEAR;

    monthsecs = secs % SECS_PER_MUD_YEAR;
    t->month = monthsecs/SECS_PER_MUD_MONTH;

    daysecs = monthsecs % SECS_PER_MUD_MONTH;
    t->day = daysecs/SECS_PER_MUD_DAY;

    hoursecs = daysecs % SECS_PER_MUD_DAY;
    t->hours = hoursecs/SECS_PER_MUD_HOUR;

    if(t->hours) {
      t->hours = 24 - t->hours;
      t->day = t->day + 1;
    }
    if(t->day) {
      t->day = 35 - t->day;
      t->month = t->month + 1;
    }
    if(t->month) {
      t->month = 17 - t->month;
      t->year = t->year + 1;
    }
    if(t->year)
      t->year = -t->year;                
  }
}


void age2(struct char_data *ch, struct time_info_data *g)
{

  mud_time_passed2(time(0),ch->player.time.birth, g);
  
  g->year += 17;   /* All players start at 17 */
  
}

struct time_info_data age(struct char_data *ch)
{
  struct time_info_data player_age;
  
  player_age = mud_time_passed(time(0),ch->player.time.birth);
  
  player_age.year += 17;   /* All players start at 17 */
  
  return(player_age);
}


char in_group ( struct char_data *ch1, struct char_data *ch2)
{



/* 
   possibilities ->
   1.  char is char2's master
   2.  char2 is char's master
   3.  char and char2 follow same.
   4.  char rides char2
   5.. char2 rides char

    otherwise not true.
 
*/
   if (ch1 == ch2)
      return(TRUE);

   if ((!ch1) || (!ch2))
      return(0);

   if ((!ch1->master) && (!ch2->master))
      return(0);

   if (ch1==ch2->master)
     return(1);

   if (ch1->master == ch2)
     return(1);

   if (ch1->master == ch2->master) {
     return(1);
   }

   if (MOUNTED(ch1) == ch2 || RIDDEN(ch1) == ch2)
     return(1);

   return(0);
}


/*
  more new procedures 
*/


/*
   these two procedures give the player the ability to buy 2*bread
   or put all.bread in bag, or put 2*bread in bag...
*/

char getall(char *name, char *newname)
{
   char arg[40],tmpname[80], otname[80];
   char prd;

   arg[0] = '\0';
   tmpname[0] = '\0';
   otname[0] = '\0';

   sscanf(name,"%s ",otname);   /* reads up to first space */

   if (strlen(otname)<5)
      return(FALSE);

   sscanf(otname,"%3s%c%s",arg,&prd,tmpname);

   if (prd != '.')
     return(FALSE);
   if (tmpname == NULL) 
      return(FALSE);
   if (strcmp(arg,"all"))
      return(FALSE);

   while (*name != '.')
       name++;

   name++;

   for (; *newname = *name; name++,newname++);
   /* :( this for loop is efficient yet arcane.. */
   
   return(TRUE);
}


int getabunch(char *name, char  *newname)
{
   int num=0;
   char tmpname[80];

   tmpname[0] = 0;
   sscanf(name,"%d*%s",&num,tmpname);
   if (tmpname[0] == '\0')
      return(FALSE);
   if (num < 1)
      return(FALSE);
   if (num>9)
      num = 9;

   while (*name != '*')
       name++;

   name++;

   for (; *newname = *name; name++,newname++);

   return(num);

}


int DetermineExp( struct char_data *mob, int exp_flags)
{

int base;
int phit;
int sab;
char buf[200];

   if (exp_flags > 100) { 
     sprintf(buf, "Exp flags on %s are > 100 (%d)", GET_NAME(mob), exp_flags);
     log(buf);
   }

/* 
reads in the monster, and adds the flags together 
for simplicity, 1 exceptional ability is 2 special abilities 
*/

    if (GetMaxLevel(mob) < 0)
       return(1);

    switch(GetMaxLevel(mob)) {

    case 0:   base = 5;
              phit = 1;
              sab = 10;
              break;

    case 1:   base = 10;
              phit = 1;
              sab =  15;
              break;

    case 2:   base = 20;
              phit = 2;
              sab =  20;
              break;


    case 3:   base = 35;
              phit = 3;
              sab =  25;
              break;

    case 4:   base = 60;
              phit = 4;
              sab =  30;
              break;

    case 5:   base = 90;
              phit = 5;
              sab =  40;
              break;

    case 6:   base = 150;
              phit = 6;
              sab =  75;
              break;

    case 7:   base = 225;
              phit = 8;
              sab =  125;
              break;

    case 8:   base = 600;
              phit = 12;
              sab  = 175;
              break;

    case 9:   base = 900;
              phit = 14;
              sab  = 300;
              break;

    case 10:   base = 1100;
              phit  = 15;
              sab   = 450;
              break;

    case 11:   base = 1300;
              phit  = 16;
              sab   = 700;
              break;

    case 12:   base = 1550;
              phit  = 17;
              sab   = 700;
              break;

    case 13:   base = 1800;
              phit  = 18;
              sab   = 950;
              break;

    case 14:   base = 2100;
              phit  = 19;
              sab   = 950;
              break;

    case 15:   base = 2400;
              phit  = 20;
              sab   = 1250;
              break;

    case 16:   base = 2700;
              phit  = 23;
              sab   = 1250;
              break;

    case 17:   base = 3000;
              phit  = 25;
              sab   = 1550;
              break;

    case 18:   base = 3500;
              phit  = 28;
              sab   = 1550;
              break;

    case 19:   base = 4000;
              phit  = 30;
              sab   = 2100;
              break;

    case 20:   base = 4500;
              phit  = 33;
              sab   = 2100;
              break;

    case 21:   base = 5000;
              phit  = 35;
              sab   =  2600;
              break;

    case 22:   base = 6000;
              phit  = 40;
              sab   = 3000;
              break;

    case 23:   base = 7000;
              phit  = 45;
              sab   = 3500;
              break;

    case 24:   base = 8000;
              phit  = 50;
              sab   = 4000;
              break;

    case 25:   base = 9000;
              phit  = 55;
              sab   = 4500;
              break;

    case 26:   base = 10000;
              phit  = 60;
              sab   =  5000;
              break;

    case 27:   base = 12000;
              phit  = 70;
              sab   = 6000;
              break;

    case 28:   base = 14000;
              phit  = 80;
              sab   = 7000;
              break;

    case 29:   base = 16000;
              phit  = 90;
              sab   = 8000;
              break;

    case 30:   base = 20000;
              phit  = 100;
              sab   = 10000;
              break;

    case 32:
    case 33:
    case 34:
    case 31 : 
      base = 22000;
      phit = 120;
      sab  = 12000;
      break;
      
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
      base = 32000;
      phit = 140;
      sab  = 14000;
      break;

    case 40:
    case 41:
      base = 42000;
      phit = 160;
      sab  = 16000;
      break;

    case 42:
    case 43:
      base = 52000;
      phit = 180;
      sab  = 20000;
      break;

    case 44:
    case 45:
      base = 72000;
      phit = 200;
      sab  = 24000;
      break;

    case 46:
    case 47:
      base = 92000;
      phit = 225;
      sab  = 28000;
      break;

    case 48:
    case 49:
      base = 122000;
      phit = 250;
      sab  = 32000;
      break;

    case 50:
      base = 150000;
      phit = 275;
      sab  = 36000;
      break;

    default:
      base = 200000;
      phit = 300;
      sab  = 40000;
      break;

    }

    return(base + (phit * GET_HIT(mob)) + (sab * exp_flags));


}

/*
int  DetermineExp( struct char_data *mob, int exp_flags);
char getall(char *name, char *newname);
int getabunch(char *name, char  *newname);
*/


void down_river( int pulse )
{
   struct char_data *ch, *tmp;
   struct obj_data *obj_object, *next_obj;
   int rd, or;
   char buf[80];
   struct room_data *rp;

   if (pulse < 0) 
      return;

   for (ch = character_list; ch; ch = tmp) {
        tmp = ch->next;
    if (!IS_NPC(ch)) {
     if (ch->in_room != NOWHERE) {
	if (real_roomp(ch->in_room)->sector_type == SECT_WATER_NOSWIM)
           if ((real_roomp(ch->in_room))->river_speed > 0) {
              if ((pulse % (real_roomp(ch->in_room))->river_speed)==0) {
                if (((real_roomp(ch->in_room))->river_dir<=5)&&((real_roomp(ch->in_room))->river_dir>=0)) {
 	 	   rd = (real_roomp(ch->in_room))->river_dir;
       		   for (obj_object = (real_roomp(ch->in_room))->contents;
				obj_object; obj_object = next_obj) {
		      next_obj = obj_object->next_content;
		      if ((real_roomp(ch->in_room))->dir_option[rd]) {
                          obj_from_room(obj_object);
   	                  obj_to_room(obj_object, (real_roomp(ch->in_room))->dir_option[rd]->to_room);
			}
		   }
/*
   flyers don't get moved
*/
                   if (!IS_AFFECTED(ch,AFF_FLYING) && !MOUNTED(ch)) {
		     rp = real_roomp(ch->in_room);
		     if (rp && rp->dir_option[rd] &&
			 rp->dir_option[rd]->to_room && 
			(EXIT(ch, rd)->to_room != NOWHERE)) {
      		         if (ch->specials.fighting) {
                               stop_fighting(ch);
			  }
		         if(IS_IMMORTAL(ch) && 
			    IS_SET(ch->specials.act, PLR_NOHASSLE)) {
			   send_to_char("The waters swirl beneath your feet.\n\r",ch);
			 } else {
			   sprintf(buf, "You drift %s...\n\r", dirs[rd]);
			   send_to_char(buf,ch);
			   if (RIDDEN(ch))
			     send_to_char(buf,RIDDEN(ch));
			   
			   or = ch->in_room;
			   char_from_room(ch);
			   if (RIDDEN(ch))  {
			     char_from_room(RIDDEN(ch));
			     char_to_room(RIDDEN(ch), (real_roomp(or))->dir_option[rd]->to_room);			 
			   }
			   char_to_room(ch,(real_roomp(or))->dir_option[rd]->to_room);
			   
			   do_look(ch, "\0",15);
			   if (RIDDEN(ch)) {
			     do_look(RIDDEN(ch), "\0",15);
			   }
			 }
			 if (IS_SET(RM_FLAGS(ch->in_room), DEATH) && 
			     GetMaxLevel(ch) < LOW_IMMORTAL) {
			   if (RIDDEN(ch))
			     NailThisSucker(RIDDEN(ch));
			   NailThisSucker(ch);
			 }
		       }
		    }
		 }
	      }
	   }
        }
      }
    }
 }

void do_WorldSave(struct char_data *ch, char *argument, char cmd)
{
   char fn[80], temp[2048], buf[128];
   long rstart, rend, i, j, k, x, r;
   struct extra_descr_data *exptr;
   FILE *fp;
   struct room_data     *rp;
   struct room_direction_data   *rdd;
   struct descriptor_data *desc;
 
   if(!ch->desc)
     return;

   rstart = 0;
   rend = top_of_world;
 
   if((fp=fopen("tinyworld.wld.new","w")) == NULL) {
     send_to_char("Can't create .wld file\r\n",ch);
     return;
   }
 
   sprintf(buf,"%s resorts the world (The game will pause for a few moments).\r\n", ch->player.name);
   send_to_all(buf);
   
   sprintf(buf,"Saving World (%ld rooms)\n\r",(long)top_of_world);
   send_to_char(buf,ch);

   for (i=rstart;i<=rend;i++) {
 
     rp = real_roomp(i);
     if (rp==NULL)
       continue;
 
/*
   strip ^Ms from description
*/
     x = 0;
 
     if (!rp->description) 
     {
       CREATE(rp->description, char, 8);
       strcpy(rp->description, "Empty");
     }
 
     for (k = 0; k <= strlen(rp->description); k++) 
     {
       if (rp->description[k] != 13)
         temp[x++] = rp->description[k];
     }
     temp[x] = '\0';
 
     if (temp[0] == '\0') {
       strcpy(temp, "Empty");
     }
 
     fprintf(fp,"#%d\n%s~\n%s~\n",rp->number,rp->name,
                                    temp);
     if (!rp->tele_targ) {
        fprintf(fp,"%d %d %d",rp->zone, rp->room_flags, rp->sector_type);
      } else {
        if (!IS_SET(TELE_COUNT, rp->tele_mask)) {
           fprintf(fp, "%d %d -1 %d %d %d %d", rp->zone, rp->room_flags,
                rp->tele_time, rp->tele_targ, 
                rp->tele_mask, rp->sector_type);
        } else {
           fprintf(fp, "%d %d -1 %d %d %d %d %d", rp->zone, rp->room_flags,
                rp->tele_time, rp->tele_targ, 
                rp->tele_mask, rp->tele_cnt, rp->sector_type);
        } 
      }
     if (rp->sector_type == SECT_WATER_NOSWIM) {
        fprintf(fp," %d %d",rp->river_speed,rp->river_dir);
     }
 
     if (rp->room_flags & TUNNEL) {
       fprintf(fp, " %d ", (int)rp->moblim);
     }
 
     fprintf(fp,"\n");     
 
     for (j=0;j<6;j++) {
       rdd = rp->dir_option[j];
       if (rdd) {
          fprintf(fp,"D%d\n",j);
 
          if (rdd->general_description && *rdd->general_description) {
            if (strlen(rdd->general_description) > 0) {
              temp[0] = '\0';
              x = 0;
              
              for (k = 0; k <= strlen(rdd->general_description); k++) {
                if (rdd->general_description[k] != 13)
                  temp[x++] = rdd->general_description[k];
              }
              temp[x] = '\0';
 
              fprintf(fp,"%s~\n", temp);
            } else {
              fprintf(fp,"~\n");
            }
          } else {
            fprintf(fp,"~\n");
          }
 
          if (rdd->keyword) {
           if (strlen(rdd->keyword)>0)
             fprintf(fp, "%s~\n",rdd->keyword);
           else
             fprintf(fp, "~\n");
          } else {
            fprintf(fp, "~\n");
          }
 
          fprintf(fp,"%d ", rdd->exit_info);
          fprintf(fp,"%d ", rdd->key);
          fprintf(fp,"%d ", rdd->to_room);
          fprintf(fp,"%d", rdd->open_cmd);
          fprintf(fp,"\n");
       }
     }
 
/*
  extra descriptions..
*/
 
   for (exptr = rp->ex_description; exptr; exptr = exptr->next) {
     x = 0;
 
    if (exptr->description) {
      for (k = 0; k <= strlen(exptr->description); k++) {
       if (exptr->description[k] != 13)
         temp[x++] = exptr->description[k];
      }
      temp[x] = '\0';
 
     fprintf(fp,"E\n%s~\n%s~\n", exptr->keyword, temp);
    }
   }
 
   fprintf(fp,"S\n");
 
   }
   fclose(fp);
 
   sprintf(buf,"The world returns to normal as %s finishes the job.\r\n", ch->player.name);
   send_to_all(buf);
   send_to_char("\n\rDone\n\r",ch);
 
 return;
}
 

void RoomSave(struct char_data *ch, long start, long end)
{
   char fn[80], temp[2048], dots[500];
   int rstart, rend, i, j, k, x, r;
   struct extra_descr_data *exptr;
   FILE *fp;
   struct room_data     *rp;
   struct room_direction_data   *rdd;
 
   rstart = start;
   rend = end;
 
   if (((rstart <= -1) || (rend <= -1)) ||
        ((rstart > WORLD_SIZE) || (rend > WORLD_SIZE))){
    send_to_char("I don't know those room #s.  make sure they are all\n\r",ch);
    send_to_char("contiguous.\n\r",ch);
    fclose(fp);
    return;
   }
 
   send_to_char("Saving\n",ch);
   strcpy(dots, "\0");
 
   for (i=rstart;i<=rend;i++) {
 
     rp = real_roomp(i);
     if (rp==NULL)
       continue;
 
     sprintf(fn, "rooms/%d", i);
     if ((fp = fopen(fn,"w")) == NULL)
     {
       send_to_char("Can't write to disk now..try later \n\r",ch);
       return;
     }
 
     strcat(dots, ".");
 
/*
   strip ^Ms from description
*/
     x = 0;
 
     if (!rp->description) 
     {
       CREATE(rp->description, char, 8);
       strcpy(rp->description, "Empty");
     }
 
     for (k = 0; k <= strlen(rp->description); k++) 
     {
       if (rp->description[k] != 13)
         temp[x++] = rp->description[k];
     }
     temp[x] = '\0';
 
     if (temp[0] == '\0') {
       strcpy(temp, "Empty");
     }
 
     fprintf(fp,"#%d\n%s~\n%s~\n",rp->number,rp->name,
                                    temp);
     if (!rp->tele_targ) {
        fprintf(fp,"%d %d %d",rp->zone, rp->room_flags, rp->sector_type);
      } else {
        if (!IS_SET(TELE_COUNT, rp->tele_mask)) {
           fprintf(fp, "%d %d -1 %d %d %d %d", rp->zone, rp->room_flags,
                rp->tele_time, rp->tele_targ, 
                rp->tele_mask, rp->sector_type);
        } else {
           fprintf(fp, "%d %d -1 %d %d %d %d %d", rp->zone, rp->room_flags,
                rp->tele_time, rp->tele_targ, 
                rp->tele_mask, rp->tele_cnt, rp->sector_type);
        } 
      }
     if (rp->sector_type == SECT_WATER_NOSWIM) {
        fprintf(fp," %d %d",rp->river_speed,rp->river_dir);
     }
 
     if (rp->room_flags & TUNNEL) {
       fprintf(fp, " %d ", (int)rp->moblim);
     }
 
     fprintf(fp,"\n");     
 
     for (j=0;j<6;j++) {
       rdd = rp->dir_option[j];
       if (rdd) {
          fprintf(fp,"D%d\n",j);
 
          if (rdd->general_description && *rdd->general_description) {
            if (strlen(rdd->general_description) > 0) {
              temp[0] = '\0';
              x = 0;
              
              for (k = 0; k <= strlen(rdd->general_description); k++) {
                if (rdd->general_description[k] != 13)
                  temp[x++] = rdd->general_description[k];
              }
              temp[x] = '\0';
 
              fprintf(fp,"%s~\n", temp);
            } else {
              fprintf(fp,"~\n");
            }
          } else {
            fprintf(fp,"~\n");
          }
 
          if (rdd->keyword) {
           if (strlen(rdd->keyword)>0)
             fprintf(fp, "%s~\n",rdd->keyword);
           else
             fprintf(fp, "~\n");
          } else {
            fprintf(fp, "~\n");
          }
 
          fprintf(fp,"%d ", rdd->exit_info);
          fprintf(fp,"%d ", rdd->key);
          fprintf(fp,"%d ", rdd->to_room);
          fprintf(fp,"%d", rdd->open_cmd);
          fprintf(fp,"\n");
       }
     }
 
/*
  extra descriptions..
*/
 
   for (exptr = rp->ex_description; exptr; exptr = exptr->next) {
     x = 0;
 
    if (exptr->description) {
      for (k = 0; k <= strlen(exptr->description); k++) {
       if (exptr->description[k] != 13)
         temp[x++] = exptr->description[k];
      }
      temp[x] = '\0';
 
     fprintf(fp,"E\n%s~\n%s~\n", exptr->keyword, temp);
    }
   }
 
   fprintf(fp,"S\n");
   fclose(fp);
 
   }
 
   send_to_char(dots, ch);
   send_to_char("\n\rDone\n\r",ch);
}
 
 
void RoomLoad( struct char_data *ch, int start, int end)
{
  FILE *fp;
  int vnum, found = TRUE, x,r;
  char chk[50], buf[80];
  struct room_data *rp, dummy;
 
 
  send_to_char("Searching and loading rooms\n\r",ch);
 
  for(vnum=start;vnum<=end;vnum++) {
 
    sprintf(buf, "rooms/%d", vnum);
    if ((fp = fopen(buf,"r")) == NULL) {
      found = FALSE;
      send_to_char(".",ch);
      continue;
    }
	fscanf(fp,"#%*d\n");
    if ((rp=real_roomp(vnum)) == 0) {  /* empty room */
      rp = (void*)malloc(sizeof(struct room_data));
      if (rp)
        bzero(rp, sizeof(struct room_data));
#if HASH
                /* this still does not work and needs work by someone */
        room_enter(&room_db, vnum, rp);
#else
        room_enter(room_db, vnum, rp);
#endif
        send_to_char("+",ch);
    } else {
      if (rp->people) {
        act("$n reaches down and scrambles reality.", FALSE, ch, NULL,
              rp->people, TO_ROOM);
      }
      cleanout_room(rp);
      send_to_char("-",ch);
    }
 
    rp->number = vnum;
    load_one_room(fp, rp);
    fclose(fp);
  }
 
  if (!found) {
    send_to_char("\n\rThe room number(s) that you specified could not all be found.\n\r",ch);
  } else {
    send_to_char("\n\rDone.\n\r",ch);
  }
 
} 

 

void fake_setup_dir(FILE *fl, long room, int dir)
{
	int tmp;
	char *temp;

	temp = fread_string(fl); /* descr */
	if (temp)
	  free(temp);
	temp = fread_string(fl); /* key */
	if (temp)
	  free(temp);

	fscanf(fl, " %d ", &tmp); 
	fscanf(fl, " %d ", &tmp);
	fscanf(fl, " %d ", &tmp);
}


int IsHumanoid( struct char_data *ch)
{
/* these are all very arbitrary */
if (!ch)
	return(FALSE);

  switch(GET_RACE(ch))
    {
    case RACE_HUMAN:
    case RACE_GNOME:
    case RACE_DEEP_GNOME:
    case RACE_ELVEN:
    case RACE_GOLD_ELF:
    case RACE_WILD_ELF:
    case RACE_SEA_ELF:
    case RACE_DWARF:
    case RACE_DARK_DWARF:
    case RACE_HALFLING:
    case RACE_ORC:
    case RACE_LYCANTH:
    case RACE_UNDEAD:
  case RACE_UNDEAD_VAMPIRE :
  case RACE_UNDEAD_LICH    : 
  case RACE_UNDEAD_WIGHT   :
 case RACE_UNDEAD_GHAST   :
 case RACE_UNDEAD_SPECTRE :
 case RACE_UNDEAD_ZOMBIE  :
 case RACE_UNDEAD_SKELETON :
 case RACE_UNDEAD_GHOUL    :
    case RACE_GIANT:
  case RACE_GIANT_HILL   :
  case RACE_GIANT_FROST  :
  case RACE_GIANT_FIRE   :
  case RACE_GIANT_CLOUD  :
  case RACE_GIANT_STORM  :
  case RACE_GIANT_STONE  :    
    case RACE_GOBLIN:
    case RACE_DEVIL:
    case RACE_TROLL:
    case RACE_VEGMAN:
    case RACE_MFLAYER:
    case RACE_ENFAN:
    case RACE_PATRYN:
    case RACE_SARTAN:
    case RACE_ROO:
    case RACE_SMURF:
    case RACE_TROGMAN:
    case RACE_LIZARDMAN:
    case RACE_SKEXIE:
    case RACE_TYTAN:
    case RACE_DROW:
    case RACE_GOLEM:
    case RACE_DEMON:
    case RACE_DRAAGDIM:
    case RACE_ASTRAL:
    case RACE_GOD:
    case RACE_HALF_ELVEN:
    case RACE_HALF_ORC:
    case RACE_HALF_OGRE:
    case RACE_HALF_GIANT:
    case RACE_GNOLL:
      return(TRUE);
      break;

    default:
      return(FALSE);
      break;
    }

}

int IsRideable( struct char_data *ch)
{
if (!ch)
	return(FALSE);

  if (IS_NPC(ch) && !IS_PC(ch)) {
    switch(GET_RACE(ch)) {
    case RACE_HORSE:
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
      return(TRUE);
      break;
    default:
      return(FALSE);
      break;
    }
  } else return(FALSE);
}

int IsAnimal( struct char_data *ch)
{
if (!ch)
	return(FALSE);

  switch(GET_RACE(ch))
    {
    case RACE_PREDATOR:
    case RACE_FISH:
    case RACE_BIRD:
    case RACE_HERBIV:
    case RACE_REPTILE:
    case RACE_LABRAT:
    case RACE_ROO:
    case RACE_INSECT:
    case RACE_ARACHNID:
      return(TRUE);
      break;
    default:
      return(FALSE);
      break;
    }

}

int IsVeggie( struct char_data *ch)
{
if (!ch)
	return(FALSE);
	
  switch(GET_RACE(ch))
    {
    case RACE_PARASITE:
    case RACE_SLIME:
    case RACE_TREE:
    case RACE_VEGGIE:
    case RACE_VEGMAN:
      return(TRUE);
      break;
    default:
      return(FALSE);
      break;
    }

}

int IsUndead( struct char_data *ch)
{
if (!ch)
	return(FALSE);

  switch(GET_RACE(ch)) {
  case RACE_UNDEAD: 
  case RACE_GHOST:
  case RACE_UNDEAD_VAMPIRE :
  case RACE_UNDEAD_LICH    : 
  case RACE_UNDEAD_WIGHT   :
 case RACE_UNDEAD_GHAST   :
 case RACE_UNDEAD_SPECTRE :
 case RACE_UNDEAD_ZOMBIE  :
 case RACE_UNDEAD_SKELETON :
 case RACE_UNDEAD_GHOUL    :
  
    return(TRUE);
    break;
  default:
    return(FALSE);
    break;
  }

}

int IsLycanthrope( struct char_data *ch)
{
if (!ch)
	return(FALSE);
  switch (GET_RACE(ch)) {
  case RACE_LYCANTH:
    return(TRUE);
    break;
  default:
    return(FALSE);
    break;
  }

}

int IsDiabolic( struct char_data *ch)
{

if (!ch)
	return(FALSE);
  switch(GET_RACE(ch))
    {
    case RACE_DEMON:
    case RACE_DEVIL:
      return(TRUE);
      break;
    default:
      return(FALSE);
      break;
    }

}

int IsReptile( struct char_data *ch)
{
if (!ch)
	return(FALSE);
  switch(GET_RACE(ch)) {
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
 case RACE_DRAGON_BRASS  : 
    case RACE_DINOSAUR:
    case RACE_SNAKE:
    case RACE_TROGMAN:
    case RACE_LIZARDMAN:
    case RACE_SKEXIE:
      return(TRUE);
      break;
    default:
      return(FALSE);
      break;
    }
}

int HasHands( struct char_data *ch)
{
if (!ch)
	return(FALSE);

  if (IsHumanoid(ch))
    return(TRUE);
  if (IsUndead(ch)) 
    return(TRUE);
  if (IsLycanthrope(ch)) 
    return(TRUE);
  if (IsDiabolic(ch))
    return(TRUE);
  if (GET_RACE(ch) == RACE_GOLEM || GET_RACE(ch) == RACE_SPECIAL)
    return(TRUE);
  return(FALSE);
}

int IsPerson( struct char_data *ch)
{
if (!ch)
	return(FALSE);


  switch(GET_RACE(ch))
    {
    case RACE_HUMAN:
    case RACE_ELVEN:
    case RACE_DROW:
    case RACE_DWARF:
    case RACE_DARK_DWARF:
    case RACE_HALFLING:
    case RACE_GNOME:
    case RACE_DEEP_GNOME:
    case RACE_GOLD_ELF:
    case RACE_WILD_ELF:
    case RACE_SEA_ELF:
    case RACE_GOBLIN:
    case RACE_ORC:
      return(TRUE);
      break;

    default:
      return(FALSE);
      break;

    }
}

int IsGiantish( struct char_data *ch)
{

if (!ch)
	return(FALSE);

  switch(GET_RACE(ch)) {
  case RACE_ENFAN:

  case RACE_GOBLIN:  /* giantish for con's only... */
  case RACE_ORC:
  
  case RACE_GIANT:
  case RACE_GIANT_HILL   :
  case RACE_GIANT_FROST  :
  case RACE_GIANT_FIRE   :
  case RACE_GIANT_CLOUD  :
  case RACE_GIANT_STORM  :
  case RACE_GIANT_STONE  :  
  case RACE_TYTAN:
  case RACE_TROLL:
  case RACE_DRAAGDIM:

  case RACE_HALF_ORC:
  case RACE_HALF_OGRE:
  case RACE_HALF_GIANT:
    return(TRUE);
  default:
    return(FALSE);
    break;      
  }
}

int IsSmall( struct char_data *ch)
{

if (!ch)
	return(FALSE);

  switch(GET_RACE(ch)) {
  case RACE_SMURF:
  case RACE_GNOME:
  case RACE_HALFLING:
  case RACE_GOBLIN:
  case RACE_ENFAN:
  case RACE_DEEP_GNOME:
    return(TRUE);
  default:
    return(FALSE);
  }
}

int IsGiant ( struct char_data *ch)
{

if (!ch)
	return(FALSE);

  switch(GET_RACE(ch)) {
  case RACE_GIANT:
  case RACE_GIANT_HILL   :
  case RACE_GIANT_FROST  :
  case RACE_GIANT_FIRE   :
  case RACE_GIANT_CLOUD  :
  case RACE_GIANT_STORM  :
  case RACE_GIANT_STONE  :
  case RACE_HALF_GIANT	:
  case RACE_TYTAN:
  case RACE_GOD:
    return(TRUE);
  default:
    return(FALSE);
  }
}

int IsExtraPlanar( struct char_data *ch)
{

if (!ch)
	return(FALSE);

  switch(GET_RACE(ch)) {
  case RACE_DEMON:
  case RACE_DEVIL:
  case RACE_PLANAR:
  case RACE_ELEMENT:
  case RACE_ASTRAL:
  case RACE_GOD:
    return(TRUE);
    break;
  default:
    return(FALSE);
    break;
  }
}
int IsOther( struct char_data *ch)
{

if (!ch)
	return(FALSE);

  switch(GET_RACE(ch)) {
  case RACE_MFLAYER:
  case RACE_SPECIAL:
  case RACE_GOLEM:
  case RACE_ELEMENT:
  case RACE_PLANAR:
  case RACE_LYCANTH:
    return(TRUE);
  default:
    return(FALSE);
    break;
  }
}

int IsGodly( struct char_data *ch)
{

if (!ch)
	return(FALSE);

  if (GET_RACE(ch) == RACE_GOD) return(TRUE);
  if (GET_RACE(ch) == RACE_DEMON || GET_RACE(ch) == RACE_DEVIL)
    if (GetMaxLevel(ch) >= 45)
      return(TRUE);

}

/*
int IsUndead( struct char_data *ch)
int IsUndead( struct char_data *ch)
int IsUndead( struct char_data *ch)
int IsUndead( struct char_data *ch)

*/

int IsDragon( struct char_data *ch)
{

if (!ch)
	return(FALSE);

  switch(GET_RACE(ch))
   {
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
      return(TRUE);
      break;
  default:
   return(FALSE);
   break;
  }
}

void SetHunting( struct char_data *ch, struct char_data *tch)
{
   int persist, dist;
   char buf[256];

#if NOTRACK
return;
#endif

if (!ch || !tch) {
	log("!ch || !tch in SetHunting");
	return;
}

	
 
   persist =  GetMaxLevel(ch);
   persist *= (int) GET_ALIGNMENT(ch) / 100;

   if (persist < 0)
     persist = -persist;

   dist = GET_ALIGNMENT(tch) - GET_ALIGNMENT(ch);
   dist = (dist > 0) ? dist : -dist;
   if (Hates(ch, tch))
       dist *=2;

   SET_BIT(ch->specials.act, ACT_HUNTING);
   ch->specials.hunting = tch;
   ch->hunt_dist = dist;
   ch->persist = persist;
   ch->old_room = ch->in_room;
#if 0
    if (GetMaxLevel(tch) >= IMMORTAL) {
        sprintf(buf, ">>%s is hunting you from %s\n\r", 
       	   (ch->player.short_descr[0]?ch->player.short_descr:"(null)"),
       	   (real_roomp(ch->in_room)->name[0]?real_roomp(ch->in_room)->name:"(null)"));
        send_to_char(buf, tch);
    }
#endif

}


void CallForGuard
  ( struct char_data *ch, struct char_data *vict, int lev, int area)
{
  struct char_data *i;
  int type1, type2;
  
  switch(area) {
  case MIDGAARD:
    type1 = 3060;
    type2 = 3069;
    break;
  case NEWTHALOS:
    type1 = 3661;
    type2 = 3682;
    break;
  case TROGCAVES:
    type1 = 21114;
    type2 = 21118;
    break;
  case OUTPOST:
    type1 = 21138;
    type2 = 21139;
    break;

  case PRYDAIN:
    type1 = 6606;
    type2 = 6614;
    break;

 default:
    type1 = 3060;
    type2 = 3069;
    break;
  }


  if (lev == 0) lev = 3;

  for (i = character_list; i && lev>0; i = i->next) {
      if (IS_NPC(i) && (i != ch)) {
	 if (!i->specials.fighting) {
	    if (mob_index[i->nr].virtual == type1) {
	       if (number(1,6) == 1) {
	         if (!IS_SET(i->specials.act, ACT_HUNTING)) {
                   if (vict) {
		      SetHunting(i, vict);
                      lev--;
		    }
	         }
	       }
	    } else if (mob_index[i->nr].virtual == type2) {
	       if (number(1,6) == 1) {
	          if (!IS_SET(i->specials.act, ACT_HUNTING)) {
		    if (vict) {
		      SetHunting(i, vict);
	              lev-=2;
		    }
		  }
		}
	    }
	  }
       }
    }
}

void StandUp (struct char_data *ch)
{
   if ((GET_POS(ch)<POSITION_STANDING) && 
       (GET_POS(ch)>POSITION_STUNNED)) {
       if (ch->points.hit > (ch->points.max_hit / 2))
         act("$n quickly stands up.", 1, ch,0,0,TO_ROOM);
       else if (ch->points.hit > (ch->points.max_hit / 6))
         act("$n slowly stands up.", 1, ch,0,0,TO_ROOM);
       else 
         act("$n gets to $s feet very slowly.", 1, ch,0,0,TO_ROOM);
       GET_POS(ch)=POSITION_STANDING;
   }
}


void MakeNiftyAttack( struct char_data *ch)
{
  int num;


  if (!ch->skills) {
    SpaceForSkills(ch);
    return;
    }

if (!ch || !ch->skills)
	{
		log("!or !ch-skills in MakeNiftyAttack() in utility.c");
		return;
	}
  if (!ch->specials.fighting) 
     return;

  num = number(1,4);
  if (num <= 2) {
      if (!ch->skills[SKILL_BASH].learned)
         ch->skills[SKILL_BASH].learned = 10 + GetMaxLevel(ch)*4;
      do_bash(ch, GET_NAME(ch->specials.fighting), 0);
  } else if (num == 3) {
     if (ch->equipment[WIELD]) {
         if (!ch->skills[SKILL_DISARM].learned)
            ch->skills[SKILL_DISARM].learned = 10 + GetMaxLevel(ch)*4;
         do_disarm(ch, GET_NAME(ch->specials.fighting), 0);
     } else {
       if (!ch->skills[SKILL_KICK].learned)
         ch->skills[SKILL_KICK].learned = 10 + GetMaxLevel(ch)*4;
       do_kick(ch, GET_NAME(ch->specials.fighting), 0);
     }
  } else {
      if (!ch->skills[SKILL_KICK].learned)
         ch->skills[SKILL_KICK].learned = 10 + GetMaxLevel(ch)*4;
      do_kick(ch, GET_NAME(ch->specials.fighting), 0);
   }

}


void FighterMove( struct char_data *ch)
{
  struct char_data *friend;

  if (!ch->skills) {
    SET_BIT(ch->player.class, CLASS_WARRIOR);
    SpaceForSkills(ch);
  }

  if (ch->specials.fighting && ch->specials.fighting->specials.fighting != 0) {
    friend = ch->specials.fighting->specials.fighting;
    if (friend == ch) {
      MakeNiftyAttack(ch);
    } else {
      /* rescue on a 1 or 2, other on a 3 or 4 */
      if (GET_RACE(friend) == (GET_RACE(ch))) {
	 if (GET_HIT(friend) < GET_HIT(ch)) { 
	   if (!ch->skills[SKILL_RESCUE].learned)
	     ch->skills[SKILL_RESCUE].learned = GetMaxLevel(ch)*3+30;
	   do_rescue(ch, GET_NAME(friend), 0);
	 } else {
	   MakeNiftyAttack(ch);
	 }
       } else {
	   MakeNiftyAttack(ch);
       }
    }
  } else {
    return;
  }

}



void MonkMove( struct char_data *ch)
{

  if (!ch->skills) {
    SpaceForSkills(ch);
    ch->skills[SKILL_DODGE].learned = GetMaxLevel(ch)+50;
    SET_BIT(ch->player.class, CLASS_MONK);
  }

  if (!ch->specials.fighting) return;

  if (GET_POS(ch) < POSITION_FIGHTING) {
    if (!ch->skills[SKILL_SPRING_LEAP].learned) 
      ch->skills[SKILL_SPRING_LEAP].learned = (GetMaxLevel(ch)*3)/2+25;
    do_springleap(ch, GET_NAME(ch->specials.fighting), 0);
    return;
  } else {
    char buf[100];

    /* Commented out as a temporary fix to monks fleeing challenges. */
    /* Was easier than rooting around in the spec_proc for the monk */
    /* challenge for it, which is proobably what should be done. */
    /* jdb - was commented back in with the change to use 
       command_interpreter */

    if (GET_HIT(ch) < GET_MAX_HIT(ch)/20) {
      if (!ch->skills[SKILL_RETREAT].learned)  
	ch->skills[SKILL_RETREAT].learned = GetMaxLevel(ch)*2+10;
      strcpy(buf, "flee");
      command_interpreter(ch, buf);
      return;
    } else {

      if (GetMaxLevel(ch)>30 && !number(0,4)) {
	if (GetMaxLevel(ch->specials.fighting) <= GetMaxLevel(ch)) {
	  if (GET_MAX_HIT(ch->specials.fighting) < 2*GET_MAX_HIT(ch)) {
	    if ((!affected_by_spell(ch->specials.fighting, SKILL_QUIV_PALM)) &&
		(!affected_by_spell(ch, SKILL_QUIV_PALM)) && 
		ch->in_room == 551) {
	      if(ch->specials.fighting->skills[SKILL_QUIV_PALM].learned &&
		 ch->in_room == 551) {
		if (!ch->skills[SKILL_QUIV_PALM].learned && ch->in_room == 551)
		  ch->skills[SKILL_QUIV_PALM].learned = GetMaxLevel(ch)*2-5;
		do_quivering_palm(ch, GET_NAME(ch->specials.fighting), 0);
		return;
	      }
	    }
	  }
	}
      }
      if (ch->specials.fighting->equipment[WIELD]) {
	if (!ch->skills[SKILL_DISARM].learned)
	  ch->skills[SKILL_DISARM].learned = (GetMaxLevel(ch)*3)/2+25;
	do_disarm(ch, GET_NAME(ch->specials.fighting), 0);
	return;
      }
      if (!ch->skills[SKILL_KICK].learned)
	ch->skills[SKILL_KICK].learned = (GetMaxLevel(ch)*3)/2+25;
      do_kick(ch, GET_NAME(ch->specials.fighting), 0);
    }
  }
}

void DevelopHatred( struct char_data *ch, struct char_data *v)
{
   int diff, patience, var;

   if (Hates(ch, v))
     return;

  if (ch == v)
    return;

  diff = GET_ALIGNMENT(ch) - GET_ALIGNMENT(v);
  if (diff < 0) diff = -diff;
  
  diff /= 20;

  if (GET_MAX_HIT(ch)) {
     patience = (int) 100 * (float) (GET_HIT(ch) / GET_MAX_HIT(ch));
  } else {
     patience = 10;
  }

  var = number(1,40) - 20;

  if (patience+var < diff)
     AddHated(ch, v);

}

int HasObject( struct char_data *ch, int ob_num)
{
int j, found;
struct obj_data *i;

/*
   equipment too
*/

found = 0;

        for (j=0; j<MAX_WEAR; j++)
     	   if (ch->equipment[j])
       	     found += RecCompObjNum(ch->equipment[j], ob_num);

      if (found > 0)
	return(TRUE);

  /* carrying  */
       	for (i = ch->carrying; i; i = i->next_content)
       	  found += RecCompObjNum(i, ob_num);

     if (found > 0)
       return(TRUE);
     else
       return(FALSE);
}


int room_of_object(struct obj_data *obj)
{
  if (obj->in_room != NOWHERE)
    return obj->in_room;
  else if (obj->carried_by)
    return obj->carried_by->in_room;
  else if (obj->equipped_by)
    return obj->equipped_by->in_room;
  else if (obj->in_obj)
    return room_of_object(obj->in_obj);
  else
    return NOWHERE;
}

struct char_data *char_holding(struct obj_data *obj)
{
  if (obj->in_room != NOWHERE)
    return NULL;
  else if (obj->carried_by)
    return obj->carried_by;
  else if (obj->equipped_by)
    return obj->equipped_by;
  else if (obj->in_obj)
    return char_holding(obj->in_obj);
  else
    return NULL;
}


int RecCompObjNum( struct obj_data *o, int obj_num)
{

int total=0;
struct obj_data *i;

  if (obj_index[o->item_number].virtual == obj_num)
    total = 1;

  if (ITEM_TYPE(o) == ITEM_CONTAINER) {
    for (i = o->contains; i; i = i->next_content)
      total += RecCompObjNum( i, obj_num);
  }
  return(total);

}

void RestoreChar(struct char_data *ch)
{

  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch); 
  if (GetMaxLevel(ch) < LOW_IMMORTAL) {
    GET_COND(ch,THIRST) = 24;
    GET_COND(ch,FULL) = 24;
  } else {
    GET_COND(ch,THIRST) = -1;
    GET_COND(ch,FULL) = -1;
  }

}


void RemAllAffects( struct char_data *ch)
{
  spell_dispel_magic(IMPLEMENTOR,ch,ch,0);

}

int CheckForBlockedMove
  (struct char_data *ch, int cmd, char *arg, int room, int dir, int class)
{
  
  char buf[256], buf2[256];
  
  if (cmd>6 || cmd<1)
    return(FALSE);
  
  strcpy(buf,  "The guard humiliates you, and block your way.\n\r");
  strcpy(buf2, "The guard humiliates $n, and blocks $s way.");
  
  if ((IS_NPC(ch) && (IS_POLICE(ch))) || (GetMaxLevel(ch) >= DEMIGOD) ||
      (IS_AFFECTED(ch, AFF_SNEAK)))
    return(FALSE);


  if ((ch->in_room == room) && (cmd == dir+1)) {
    if (!HasClass(ch,class))  {
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      send_to_char(buf, ch);
      return TRUE;
    }  
  }
  return FALSE;

}


void TeleportPulseStuff(int pulse)
{
  
  /*
    check_mobile_activity(pulse);
    Teleport(pulse);
    */
  
  register struct char_data *ch;
  struct char_data *next, *tmp, *bk, *n2;
  int tick, tm;
  struct room_data *rp, *dest;
  struct obj_data *obj_object, *temp_obj;
  
  tm = pulse % PULSE_MOBILE;    /* this is dependent on P_M = 3*P_T */
  
  if (tm == 0) {
    tick = 0;
  } else if (tm == PULSE_TELEPORT) {
    tick = 1;
  } else if (tm == PULSE_TELEPORT*2) {
    tick = 2;
  }
  
  for (ch = character_list; ch; ch = next) {
    next = ch->next;
    if (IS_MOB(ch)) {
      if (ch->specials.tick == tick && !ch->specials.fighting) {
	mobile_activity(ch);
      }
    } else if (IS_PC(ch)) {
      rp = real_roomp(ch->in_room);
      if (rp &&
	  (rp)->tele_targ > 0 &&
	  rp->tele_targ != rp->number &&
	  (rp)->tele_time > 0 &&
	  (pulse % (rp)->tele_time)==0) {
	
	dest = real_roomp(rp->tele_targ);
	if (!dest) {
	  log("invalid tele_targ");
	  continue;
	}
	
	obj_object = (rp)->contents;
	while (obj_object) {
	  temp_obj = obj_object->next_content;
	  obj_from_room(obj_object);
	  obj_to_room(obj_object, (rp)->tele_targ);
	  obj_object = temp_obj;
	}
	
	bk = 0;

	while(rp->people/* should never fail */) {
	  
	  tmp = rp->people;   /* work through the list of people */
	  if (!tmp) break;

	  if (tmp == bk) break;

	  bk = tmp;

	  char_from_room(tmp); /* the list of people in the room has changed */
	  char_to_room(tmp, rp->tele_targ);

	  if (IS_SET(TELE_LOOK, rp->tele_mask) && IS_PC(tmp)) {
	    do_look(tmp, "\0",15);
	  }

       	  if (IS_SET(dest->room_flags, DEATH) && (!IS_IMMORTAL(tmp))) {
	    if (tmp == next) 
	      next = tmp->next;
	    NailThisSucker(tmp);
	    continue;
	  }
	  if (dest->sector_type == SECT_AIR) {
	    n2 = tmp->next;
	    if (check_falling(tmp)) {	      
	      if (tmp == next)
		next = n2;
	    }
	  }
	}

	if (IS_SET(TELE_COUNT, rp->tele_mask)) {
	  rp->tele_time = 0;   /* reset it for next count */
	}
	if (IS_SET(TELE_RANDOM, rp->tele_mask)) {
	  rp->tele_time = number(1,10)*100;
	}
      }
    }
  }
}

void RiverPulseStuff(int pulse)
{
  /*
    down_river(pulse);
    MakeSound();
    */
  struct descriptor_data *i;
  register struct char_data *ch;
  struct char_data *tmp;
  register struct obj_data *obj_object;
  struct obj_data *next_obj;
  int rd, or;
  char buf[80], buffer[100];
  struct room_data *rp;
  
  if (pulse < 0) 
    return;

  for (i = descriptor_list; i; i=i->next) {
    if (!i->connected) {
      ch = i->character;
      
      if (IS_PC(ch) || RIDDEN(ch)) {
	if (ch->in_room != NOWHERE) {
	  if ((real_roomp(ch->in_room)->sector_type == SECT_WATER_NOSWIM) ||
	      (real_roomp(ch->in_room)->sector_type == SECT_UNDERWATER))
	    if ((real_roomp(ch->in_room))->river_speed > 0) {
	    if ((pulse % (real_roomp(ch->in_room))->river_speed)==0) {
	      if (((real_roomp(ch->in_room))->river_dir<=5)&&
		  ((real_roomp(ch->in_room))->river_dir>=0)) {
		rd = (real_roomp(ch->in_room))->river_dir;
		for (obj_object = (real_roomp(ch->in_room))->contents;
		     obj_object; obj_object = next_obj) {
		  next_obj = obj_object->next_content;
		  if ((real_roomp(ch->in_room))->dir_option[rd]) {
		    obj_from_room(obj_object);
		    obj_to_room(obj_object, 
				(real_roomp(ch->in_room))->dir_option[rd]->to_room);
		  }
		}
		/*
		  flyers don't get moved
		  */
		if(IS_IMMORTAL(ch) &&
		   IS_SET(ch->specials.act, PLR_NOHASSLE)) {
		  send_to_char("The waters swirl and eddy about you.\n\r",ch);
		} else {
		  if(!IS_AFFECTED(ch,AFF_FLYING) ||
		     (real_roomp(ch->in_room)->sector_type == SECT_UNDERWATER)) {
		    if (!MOUNTED(ch)) {
		      rp = real_roomp(ch->in_room);
		      if (rp && rp->dir_option[rd] &&
			  rp->dir_option[rd]->to_room && 
			  (EXIT(ch, rd)->to_room != NOWHERE)) {
			if (ch->specials.fighting) {
			  stop_fighting(ch);
			}
			sprintf(buf, "You drift %s...\n\r", dirs[rd]);
			send_to_char(buf,ch);
			if (RIDDEN(ch))
			  send_to_char(buf,RIDDEN(ch));
			or = ch->in_room;
			char_from_room(ch);
			if (RIDDEN(ch))  {
			  char_from_room(RIDDEN(ch));
			  char_to_room(RIDDEN(ch),
				       (real_roomp(or))->dir_option[rd]->to_room);
			}
			char_to_room(ch,
				     (real_roomp(or))->dir_option[rd]->to_room);
			do_look(ch, "\0",15);
			if (RIDDEN(ch)) {
			  do_look(RIDDEN(ch), "\0",15);
			}
			
			
			if (IS_SET(RM_FLAGS(ch->in_room), DEATH) && 
			    GetMaxLevel(ch) < LOW_IMMORTAL) {
			  NailThisSucker(ch);
			  if (RIDDEN(ch))
			    NailThisSucker(RIDDEN(ch));
			}
		      }
		    }
		  }
		}		/* end of else for is_immort() */
	      }
	    }
	  }
	}
      }
    }
  }

  if (!number(0,4)) {
    for (ch = character_list; ch; ch = tmp) {
      tmp = ch->next;
      
      /*
       *   mobiles
       */	
      if (!IS_PC(ch) && (ch->player.sounds) && (number(0,5)==0)) {
	if (ch->specials.default_pos > POSITION_SLEEPING) {
	  if (GET_POS(ch) > POSITION_SLEEPING) {
	    /*
	     *  Make the sound;
	     */
	    MakeNoise(ch->in_room, ch->player.sounds, 
		      ch->player.distant_snds);
	  } else if (GET_POS(ch) == POSITION_SLEEPING) {
	    /*
	     * snore 
	     */	 
	    sprintf(buffer, "%s snores loudly.\n\r", 
		    ch->player.short_descr);
	    MakeNoise(ch->in_room, buffer, 
		      "You hear a loud snore nearby.\n\r");
	  }
	} else if (GET_POS(ch) == ch->specials.default_pos) {
	  /*
	   * Make the sound
	   */       
	  MakeNoise(ch->in_room, ch->player.sounds, ch->player.distant_snds);
	}
      }
    }      
  }
}

/*
**  Apply soundproof is for ch making noise
*/
int apply_soundproof(struct char_data *ch)
{
  struct room_data *rp;

  if (IS_AFFECTED(ch, AFF_SILENCE)) {
    send_to_char("You are silenced, you can't make a sound!\n\r", ch);
    return(TRUE);
  }

  rp = real_roomp(ch->in_room);

  if (!rp) return(FALSE);  

  if (IS_SET(rp->room_flags, SILENCE)) {
     send_to_char("You are in a silence zone, you can't make a sound!\n\r",ch);
     return(TRUE);   /* for shouts, emotes, etc */
  }

  if (rp->sector_type == SECT_UNDERWATER) {
    send_to_char("Speak underwater, are you mad????\n\r", ch);
    return(TRUE);
  }
  return(FALSE);

}

/*
**  check_soundproof is for others making noise
*/
int check_soundproof(struct char_data *ch)
{
  struct room_data *rp;

  if (IS_AFFECTED(ch, AFF_SILENCE)) {
    return(TRUE);
  }

  rp = real_roomp(ch->in_room);

  if (!rp) return(FALSE);  

  if (IS_SET(rp->room_flags, SILENCE)) {
     return(TRUE);   /* for shouts, emotes, etc */
  }
  if (rp->sector_type == SECT_UNDERWATER)
    return(TRUE);

  return(FALSE);
}

int MobCountInRoom( struct char_data *list)
{
  int i;
  struct char_data *tmp;

  for (i=0, tmp = list; tmp; tmp = tmp->next_in_room, i++)
    ;

  return(i);

}

void *Mymalloc( long size)
{
  if (size < 1) {
    fprintf(stderr, "attempt to malloc negative memory - %d\n", size);
    assert(0);
  }
  return(malloc(size));
}

int SpaceForSkills(struct char_data *ch)
{

  /*
    create space for the skills for some mobile or character.
  */


  ch->skills = (struct char_skill_data *)malloc(MAX_SKILLS*sizeof(struct char_skill_data));

  if (ch->skills == 0)
    assert(0);

}

int CountLims(struct obj_data *obj)
{
  int total=0;

  if (!obj)
    return(0);

  if (obj->contains)
    total += CountLims(obj->contains);
  if (obj->next_content)
    total += CountLims(obj->next_content);
  if (obj->obj_flags.cost_per_day > LIM_ITEM_COST_MIN)
    total+=1;
  return(total);
}


char *lower(char *s)
{
  static char c[1000];
  static char *p;
  int i=0;
  
  strcpy(c, s);

  while (c[i]) {
    if (c[i] < 'a' && c[i] >= 'A' && c[i] <= 'Z')
      c[i] = (char)(int)c[i]+32;
    i++;
  }
  p = c;
  return(p);
}

int getFreeAffSlot( struct obj_data *obj)
{
  int i;

  for (i=0; i < MAX_OBJ_AFFECT; i++)
    if (obj->affected[i].location == APPLY_NONE)
      return(i);

  assert(0);
}

void SetRacialStuff( struct char_data *mob)
{

  switch(GET_RACE(mob)) {
  case RACE_BIRD:
    SET_BIT(mob->specials.affected_by, AFF_FLYING);    
    break;
  case RACE_FISH:
    SET_BIT(mob->specials.affected_by, AFF_WATERBREATH);
    break;
  case RACE_ELVEN:
  case RACE_DROW:
  case RACE_GOLD_ELF:
  case RACE_WILD_ELF:
  case RACE_SEA_ELF:
	SET_BIT(mob->specials.affected_by,AFF_INFRAVISION);
	SET_BIT(mob->immune, IMM_CHARM);
	break;	
  case RACE_DWARF:
  case RACE_DARK_DWARF:
  case RACE_DEEP_GNOME:
  case RACE_GNOME:
  case RACE_MFLAYER:
  case RACE_TROLL:
  case RACE_ORC:
  case RACE_GOBLIN:
  case RACE_HALFLING:
  case RACE_GNOLL:
    SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
    break;
  case RACE_INSECT:
  case RACE_ARACHNID:
    if (IS_PC(mob)) {
      GET_STR(mob) = 18;
      GET_ADD(mob) = 100;
    }
    break;
  case RACE_LYCANTH:
    SET_BIT(mob->M_immune, IMM_NONMAG);
    break;
  case RACE_PREDATOR:
    if (mob->skills)
      mob->skills[SKILL_HUNT].learned = 100;
    break;

  case RACE_GIANT_FROST  :
    SET_BIT(mob->M_immune, IMM_COLD);
    SET_BIT(mob->susc,IMM_FIRE);
    break;
  case RACE_GIANT_FIRE   :
    SET_BIT(mob->M_immune, IMM_FIRE);
    SET_BIT(mob->susc,IMM_COLD);    
    break;
  case RACE_GIANT_CLOUD  :
    SET_BIT(mob->M_immune, IMM_SLEEP);  /* should be gas... but no IMM_GAS */
    SET_BIT(mob->susc,IMM_ACID);    
    break;
  case RACE_GIANT_STORM  :
    SET_BIT(mob->M_immune, IMM_ELEC);
    break;

  case RACE_GIANT_STONE  :
    SET_BIT(mob->M_immune, IMM_PIERCE);
    break;
  case RACE_UNDEAD:
  case RACE_UNDEAD_VAMPIRE : 
  case RACE_UNDEAD_LICH    : 
  case RACE_UNDEAD_WIGHT   :
  case RACE_UNDEAD_GHAST   :
  case RACE_UNDEAD_GHOUL   :
  case RACE_UNDEAD_SPECTRE :   
  case RACE_UNDEAD_ZOMBIE  :
  case RACE_UNDEAD_SKELETON : 
   SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
   SET_BIT(mob->M_immune,IMM_POISON+IMM_DRAIN+IMM_SLEEP+IMM_HOLD+IMM_CHARM);
   break;

 case RACE_DRAGON_RED    : SET_BIT(mob->M_immune,IMM_FIRE);
 			   SET_BIT(mob->susc,IMM_COLD);
 			   break;
 case RACE_DRAGON_BLACK  : SET_BIT(mob->M_immune,IMM_ACID);
 			   break;

 case RACE_DRAGON_GREEN  : SET_BIT(mob->M_immune,IMM_SLEEP);
 			   break;

 case RACE_DRAGON_WHITE  : SET_BIT(mob->M_immune,IMM_COLD);
 			   SET_BIT(mob->susc,IMM_FIRE);
 			   break;
 case RACE_DRAGON_BLUE   : SET_BIT(mob->M_immune,IMM_ELEC);
 			   break;
 case RACE_DRAGON_SILVER : SET_BIT(mob->M_immune,IMM_ENERGY);
 			   break;
 case RACE_DRAGON_GOLD   : SET_BIT(mob->M_immune,IMM_SLEEP+IMM_ENERGY);
 			   break;
 case RACE_DRAGON_BRONZE : SET_BIT(mob->M_immune,IMM_COLD+IMM_ACID);
 			   break;
 case RACE_DRAGON_COPPER : SET_BIT(mob->M_immune,IMM_FIRE);
 			   break;
 case RACE_DRAGON_BRASS  : SET_BIT(mob->M_immune,IMM_ELEC);
 			   break;
 
case RACE_HALF_ELVEN:
case RACE_HALF_OGRE:
case RACE_HALF_ORC:
case RACE_HALF_GIANT:
   SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
   break;

  default:
    break;
  }

   /* height and weight      / Hatred Foes! / */
 if (IS_NPC(mob))
{ 
 switch(GET_RACE(mob)) 
 {
  case RACE_HUMAN:break;

  case RACE_ELVEN:
  case RACE_GOLD_ELF:
  case RACE_WILD_ELF:
  case RACE_SEA_ELF:
   AddHatred(mob,OP_RACE,RACE_ORC);
   break;
  case RACE_GNOME:break;

  case RACE_DWARF:
      AddHatred(mob,OP_RACE,RACE_GOBLIN);
      AddHatred(mob,OP_RACE,RACE_ORC);
      break;
  case RACE_HALFLING:break;

  case RACE_LYCANTH:
   AddHatred(mob,OP_RACE,RACE_HUMAN);
   break;
  
  case RACE_UNDEAD:break;
  case RACE_DARK_DWARF:		  /* these guys hate good people */
  case RACE_UNDEAD_VAMPIRE :
  case RACE_UNDEAD_LICH    : 
  case RACE_UNDEAD_WIGHT   :
  case RACE_UNDEAD_GHAST   :
  case RACE_UNDEAD_GHOUL   :
  case RACE_UNDEAD_SPECTRE :   AddHatred(mob,OP_GOOD,1000);
  				break;
  case RACE_UNDEAD_ZOMBIE  :break;
  case RACE_UNDEAD_SKELETON :break;

  

  case RACE_VEGMAN:break;
  case RACE_MFLAYER:break;

  case RACE_DROW:
      AddHatred(mob,OP_RACE,RACE_ELVEN);
      break;
  case RACE_SKEXIE:break;
  case RACE_TROGMAN:break;
  case RACE_LIZARDMAN:break;
  case RACE_SARTAN:break;
  case RACE_PATRYN:break;
  case RACE_DRAAGDIM:break;
  case RACE_ASTRAL: break;

  case RACE_HORSE:
    mob->player.weight = 400;
    mob->player.height = 175;
    break;

  case RACE_ORC:
    AddHatred(mob,OP_GOOD,1000);
    AddHatred(mob,OP_RACE,RACE_ELVEN);
    mob->player.weight = 150;
    mob->player.height = 140;
    break;

  case RACE_SMURF:
    mob->player.weight = 5;
    mob->player.height = 10;
    break;

  case RACE_GOBLIN:
  case RACE_GNOLL:
    AddHatred(mob,OP_GOOD,1000);
    AddHatred(mob,OP_RACE,RACE_DWARF);
    break;

  case RACE_ENFAN:
    mob->player.weight = 120;
    mob->player.height = 100;
    break;

  case RACE_LABRAT:    break;
  case RACE_INSECT:    break;
  case RACE_ARACHNID:    break;
  case RACE_REPTILE:    break;
  case RACE_DINOSAUR: break;
  case RACE_FISH: break;
  case RACE_PREDATOR: break;
  case RACE_SNAKE: break;
  case RACE_HERBIV: break;
  case RACE_VEGGIE: break;
  case RACE_ELEMENT: break;
  case RACE_PRIMATE: break;

  case RACE_GOLEM:
    mob->player.weight = 10+GetMaxLevel(mob)*GetMaxLevel(mob)*2;
    mob->player.height = 20+MIN(mob->player.weight,600);
    break;

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
    mob->player.weight = MAX(60, GetMaxLevel(mob)*GetMaxLevel(mob)*2);
    mob->player.height = 100+ MIN(mob->player.weight, 500);
    break;

  case RACE_BIRD: 
  case RACE_PARASITE: 
  case RACE_SLIME: 
    mob->player.weight = GetMaxLevel(mob)*(GetMaxLevel(mob)/5);
    mob->player.height = 10*GetMaxLevel(mob);
    break;
  

  case RACE_GHOST:
    mob->player.weight = GetMaxLevel(mob)*(GetMaxLevel(mob)/5);
    mob->player.height = 10*GetMaxLevel(mob);
    break;
  case RACE_TROLL: 
    AddHatred(mob,OP_GOOD,1000);
    mob->player.height = 200+GetMaxLevel(mob)*15;
    mob->player.weight = (int)mob->player.height*1.5;
    break;

  case RACE_GIANT:
  case RACE_GIANT_HILL   :
  case RACE_GIANT_FROST  :
  case RACE_GIANT_FIRE   :
  case RACE_GIANT_CLOUD  :
  case RACE_GIANT_STORM  :
    mob->player.height = 200+GetMaxLevel(mob)*15;
    mob->player.weight = (int)mob->player.height*1.5;
    AddHatred(mob,OP_RACE,RACE_DWARF);
    break;

  case RACE_DEVIL: 
  case RACE_DEMON: 
    AddHatred(mob,OP_GOOD,1000);
    mob->player.height = 200+GetMaxLevel(mob)*15;
    mob->player.weight = (int)mob->player.height*1.5;
    break;


   case RACE_PLANAR:    
    mob->player.height = 200+GetMaxLevel(mob)*15;
    mob->player.weight = (int)mob->player.height*1.5;
    break;

  case RACE_GOD: break;
  case RACE_TREE: break;
  case RACE_TYTAN: 
    mob->player.weight = MAX(500, GetMaxLevel(mob)*GetMaxLevel(mob)*10);
    mob->player.height = GetMaxLevel(mob)/2*100;
    break;

 case RACE_HALF_ELVEN:
 case RACE_HALF_OGRE:
 case RACE_HALF_ORC:
 case RACE_HALF_GIANT:    
			break;
  } /* end switch */
} /* if NPC */  

}

int check_nomagic(struct char_data *ch, char *msg_ch, char *msg_rm)
{
  struct room_data *rp;

  rp = real_roomp(ch->in_room);
  if (rp && rp->room_flags&NO_MAGIC) {
    if (msg_ch)
      send_to_char(msg_ch, ch);
    if (msg_rm)
      act(msg_rm, FALSE, ch, 0, 0, TO_ROOM);
    return 1;
  }
  return 0;
}

int NumCharmedFollowersInRoom(struct char_data *ch)
{
  struct char_data *t;
  long count = 0;
  struct room_data *rp;

  rp = real_roomp(ch->in_room);
  if (rp) {
    count=0;
    for (t = rp->people;t; t= t->next_in_room) {
      if (IS_AFFECTED(t, AFF_CHARM) && (t->master == ch))
	count++;
    }
    return(count);
  } else return(0);

  return(0);
}


struct char_data *FindMobDiffZoneSameRace(struct char_data *ch)
{
  int num;
  struct char_data *t;
  struct room_data *rp1,*rp2;

  num = number(1,100);

  for (t=character_list;t;t=t->next, num--) {
    if (GET_RACE(t) == GET_RACE(ch) && IS_NPC(t) && !IS_PC(t) && num==0) {
      rp1 = real_roomp(ch->in_room);
      rp2 = real_roomp(t->in_room);
      if (rp1->zone != rp2->zone)
	return(t);
    }
  }
  return(0);
}

int NoSummon(struct char_data *ch)
{
  struct room_data *rp;

  rp = real_roomp(ch->in_room);
  if (!rp) return(TRUE);

  if (IS_SET(rp->room_flags, NO_SUM)) {
    send_to_char("Cryptic powers block your summons.\n\r", ch);
    return(TRUE);
  }

  if (IS_SET(rp->room_flags, TUNNEL)) {
    send_to_char("Strange forces collide in your brain,\n\r", ch);
    send_to_char("Laws of nature twist, and dissapate before\n\r", ch);
    send_to_char("your eyes, strange ideas wrestle with green furry\n\r", ch);
    send_to_char("things, which are crawling up your super-ego...\n\r", ch);
    send_to_char("  You lose a sanity point.\n\r\n\r", ch);
    send_to_char("  OOPS!  Sorry, wronge Genre.  :-) \n\r", ch);
    return(TRUE);
  }

  return(FALSE);
}

#if 0
int GetNewRace(struct char_file_u *s)
{
  int ok, newrace, i;

  while (1) {
    newrace = number(1,MAX_RACE);
    if (newrace == RACE_UNDEAD)
      continue;
    else {
      ok = TRUE;
      for (i=0;i<MAX_CLASS;i++) {
	if (RacialMax[newrace][i] <= 0) {
	  ok = FALSE;
	  break;
	}
      }
      if (ok) {
	return(newrace);
      }
    }
  }
}
#else
int GetNewRace(struct char_file_u *s)
{
 int return_race,try_again=TRUE;
 
 while (try_again) {
 
   return_race=number(1,MAX_RACE);
   
  switch(return_race) { 
		/* we allow these races to be used in reincarnations */
	case RACE_HUMAN     :
	case RACE_ELVEN     :
	case RACE_DWARF     :
	case RACE_HALFLING  :
	case RACE_GNOME     :
	case RACE_ORC      :
	case RACE_DROW     :
	case RACE_MFLAYER  :
	case RACE_DARK_DWARF: 
	case RACE_DEEP_GNOME :
	case RACE_GNOLL	:
	case RACE_GOLD_ELF	:
	case RACE_WILD_ELF	:
	case RACE_SEA_ELF	:
	case RACE_LIZARDMAN :
	case RACE_HALF_ELVEN :  
	case RACE_HALF_OGRE   : 
	case RACE_HALF_ORC     :
	case RACE_HALF_GIANT   :
	case RACE_GIANT_HILL   :
	case RACE_GIANT_FROST  :
	case RACE_GIANT_FIRE   :
	case RACE_GIANT_CLOUD  :
	case RACE_GIANT_STORM  :
	case RACE_ROO      :
	case RACE_PRIMATE  :
	case RACE_GOBLIN   :
	case RACE_TROLL    :
			try_again=FALSE;
			break;
				/* not a valid race, try again */
	default:try_again=TRUE;
		break;
	}  /* end switch */
   }	 /* end while */
   
   return(return_race);
}
#endif

int GetApprox(int num, int perc)
{
  /* perc = 0 - 100 */
  int adj, r;
  float fnum, fadj;

  adj = 100 - perc;
  if (adj < 0) adj = 0;
  adj *=2;  /* percentage of play (+- x%) */

  r = number(1,adj);

  perc += r;

  fnum = (float)num;
  fadj = (float)perc*2;
  fnum *= (float)(fadj/(200.0));

  num = (int)fnum;

  return(num);
}

int MountEgoCheck(struct char_data *ch, struct char_data *horse)
{
  int ride_ego, drag_ego, align, check;

  if (IsDragon(horse)) {
    if (ch->skills) {
      drag_ego = GetMaxLevel(horse)*2;
      if (IS_SET(horse->specials.act, ACT_AGGRESSIVE) ||
	  IS_SET(horse->specials.act, ACT_META_AGG)) {
	drag_ego += GetMaxLevel(horse);
      }
      ride_ego = ch->skills[SKILL_RIDE].learned/10 +
	GetMaxLevel(ch)/2;
      if (IS_AFFECTED(ch, AFF_DRAGON_RIDE)) {
	ride_ego += ((GET_INT(ch) + GET_WIS(ch))/2);
      }
      align = GET_ALIGNMENT(ch) - GET_ALIGNMENT(horse);
      if (align < 0) align = -align;
      align/=100;
      align -= 5;
      drag_ego += align;
      if (GET_HIT(horse) > 0)
	drag_ego -= GET_MAX_HIT(horse)/GET_HIT(horse);
      else 
	    drag_ego = 0;
      if (GET_HIT(ch) > 0)
	ride_ego -= GET_MAX_HIT(ch)/GET_HIT(ch);
      else 
	ride_ego = 0;
      
      check = drag_ego+number(1,10)-(ride_ego+number(1,10));
      return(check);
      
    } else {
      return(-GetMaxLevel(horse));
    }
  } else {
    if (!ch->skills) return(-GetMaxLevel(horse));

    drag_ego = GetMaxLevel(horse);

    if (drag_ego > 15) 
      drag_ego *= 2;

    ride_ego = ch->skills[SKILL_RIDE].learned/10 +
      GetMaxLevel(ch);

    if (IS_AFFECTED(ch, AFF_DRAGON_RIDE)) {
      ride_ego += (GET_INT(ch) + GET_WIS(ch));
    }
    check = drag_ego+number(1,5)-(ride_ego+number(1,10));
    return(check);
  }    
} 

int RideCheck( struct char_data *ch, int mod)
{
  if (ch->skills) {
    if (!IS_AFFECTED(ch, AFF_DRAGON_RIDE)) {
      if (number(1,90) > ch->skills[SKILL_RIDE].learned+mod) {
	if (number(1,91-mod) > ch->skills[SKILL_RIDE].learned/2) {
	  if (ch->skills[SKILL_RIDE].learned < 90) {
	    send_to_char("You learn from your mistake\n\r", ch);
	    ch->skills[SKILL_RIDE].learned+=2;
	  }
	}
	return(FALSE);
      }
      return(TRUE);
    } else {
      if (number(1,90) > (ch->skills[SKILL_RIDE].learned+
			  GET_LEVEL(ch, BestMagicClass(ch))+mod))
	return(FALSE);
    }
  } else {
    return(FALSE);
  }
}

void FallOffMount(struct char_data *ch, struct char_data *h)
{
  act("$n loses control and falls off of $N", FALSE, ch, 0, h, TO_NOTVICT);
  act("$n loses control and falls off of you", FALSE, ch, 0, h, TO_VICT);
  act("You lose control and fall off of $N", FALSE, ch, 0, h, TO_CHAR);

}

int EqWBits(struct char_data *ch, int bits)
{
  int i;

  for (i=0;i< MAX_WEAR;i++){
    if (ch->equipment[i] && 
        IS_SET(ch->equipment[i]->obj_flags.extra_flags, bits))
      return(TRUE);
  }
  return(FALSE);
}

int InvWBits(struct char_data *ch, int bits)
{
  struct obj_data *o;

  for (o = ch->carrying;o;o=o->next_content) {
    if (IS_SET(o->obj_flags.extra_flags, bits))
      return(TRUE);
  }
  return(FALSE);
}

int HasWBits(struct char_data *ch, int bits)
{
  if (EqWBits(ch, bits))
    return(TRUE);
  if (InvWBits(ch, bits))
    return(TRUE);
  return(FALSE);
}

int LearnFromMistake(struct char_data *ch, int sknum, int silent, int max)
{
char buf[128];

  if (!ch->skills) return(0);
    
  if (!IS_SET(ch->skills[sknum].flags, SKILL_KNOWN)) 
    return(0);

  if (ch->skills[sknum].learned < max && ch->skills[sknum].learned > 0)   {
    if (number(1, 101) > ch->skills[sknum].learned/2)     {
      if (!silent)
	send_to_char("You learn from your mistake\n\r", ch);
      ch->skills[sknum].learned+=1;
      if (ch->skills[sknum].learned >= max)
	if (!silent)
	  send_to_char("You are now learned in this skill!\n\r", ch);
    }
  }
}

/* if (!IsOnPmp(roomnumber)) then they are on another plane! */
int IsOnPmp(int room_nr)
{
  extern struct zone_data *zone_table;

  if (real_roomp(room_nr)) {
    if (!IS_SET(zone_table[real_roomp(room_nr)->zone].reset_mode, ZONE_ASTRAL))
      return(TRUE);
    return(FALSE);
  } else {
    return(FALSE);
  }

}


int GetSumRaceMaxLevInRoom( struct char_data *ch)
{
  struct room_data *rp;
  struct char_data *i;
  int sum=0;

  rp = real_roomp(ch->in_room);

  if (!rp) return(0);

  for (i = rp->people; i; i=i->next_in_room) {
    if (GET_RACE(i) == GET_RACE(ch)) {
      sum += GetMaxLevel(i);
    }
  }
  return(sum);
}

int too_many_followers(struct char_data *ch) 
{
  struct follow_type *k;
  int max_followers,actual_fol;
  char buf[80];

  max_followers = (int) chr_apply[GET_CHR(ch)].num_fol;
  
  for(k=ch->followers,actual_fol=0; k; k=k->next) 
    if (IS_AFFECTED(k->follower, AFF_CHARM)) 
      actual_fol++;

  if(actual_fol < max_followers)
    return FALSE;
}

int follow_time(struct char_data *ch)
{
  int fol_time=0;
  fol_time= (int) (24*GET_CHR(ch)/11);
  return fol_time;
}

int ItemAlignClash(struct char_data *ch, struct obj_data *obj)
{
  if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
    return(TRUE);
  }
  return(FALSE);
}

int ItemEgoClash(struct char_data *ch, struct obj_data *obj, int bon)
{

#ifndef EGO
  return(0);
#else

  int obj_ego, p_ego, tmp;

  obj_ego = obj->obj_flags.cost_per_day;

 if (strstr(obj->name,"scroll") ||  strstr(obj->name,"potion") ||
     strstr(obj->name,"bag") ||     
     strstr(obj->name,"key"   ) ) {
      return(0);
     }
     
  if (obj_ego >= MAX(LIM_ITEM_COST_MIN,14000) || obj_ego < 0) {
    
    if (obj_ego < 0)
      obj_ego = 50000;

    obj_ego /= 666;

/*    
  alignment stuff
    */    

    if (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) || 
	(IS_OBJ_STAT(obj, ITEM_ANTI_EVIL))) {
      if (IS_NEUTRAL(ch))
	obj_ego += obj_ego/4;
    }

    if (IS_PC(ch)) {
      p_ego = GetMaxLevel(ch)+HowManyClasses(ch);

      if (p_ego > 40) {
	p_ego *= (p_ego-39);
      } else if (p_ego > 20) {
	p_ego += (p_ego -20);
      }
      
    } else {
      p_ego = 10000;
    }

    tmp = GET_INT(ch)+GET_WIS(ch)+GET_CHR(ch);
    tmp /= 3;


    tmp *= GET_HIT(ch);
    tmp /= GET_MAX_HIT(ch);


    return((p_ego + tmp + bon + number(1,6))-(obj_ego+number(1,6)));
  }

  return(1);

#endif
}

void IncrementZoneNr(int nr)
{
  struct char_data *c;
  extern struct char_data *character_list;

  if (nr > top_of_zone_table)
    return;

  if (nr >= 0) {
    for (c = character_list;c;c=c->next) {
      if (c->specials.zone >= nr)
	c->specials.zone++;
    }
  } else {
    for (c = character_list;c;c=c->next) {
      if (c->specials.zone >= nr)
	c->specials.zone--;
    }    
  }
}

int IsDarkOutside(struct room_data *rp)
{
  extern int gLightLevel;

  if (gLightLevel >= 4)
    return(0);

  if (IS_SET(rp->room_flags, INDOORS) || IS_SET(rp->room_flags, DEATH))
    return(0);

  if (rp->sector_type == SECT_FOREST) {
    if (gLightLevel <= 1)
      return(1);
  } else {
    if (gLightLevel == 0)
      return(0);
  }
}

int anti_barbarian_stuff(struct obj_data *obj_object)
{
if ((IS_OBJ_STAT(obj_object,ITEM_GLOW))  || (IS_OBJ_STAT(obj_object,ITEM_HUM)) ||
   (IS_OBJ_STAT(obj_object,ITEM_MAGIC)) || (IS_OBJ_STAT(obj_object,ITEM_BLESS)) ||	      
   (IS_OBJ_STAT(obj_object,ITEM_NODROP)) )
    return(TRUE); else
    return(FALSE);
}

int CheckGetBarbarianOK(struct char_data *ch,struct obj_data *obj_object)
{
#ifndef BARB_GET_DISABLE
 if (GET_LEVEL(ch,BARBARIAN_LEVEL_IND) !=0 && 
       (anti_barbarian_stuff(obj_object)) && (GetMaxLevel(ch)<LOW_IMMORTAL)  )
       {
	act("$n you sense magic on $p and drop it.",FALSE,ch,obj_object,0,TO_CHAR);	        
        act("$n shakes $s head and refuses to take $p.", FALSE, ch, obj_object, 0, TO_ROOM);	        
        return(FALSE);
       }
#endif
  return(TRUE);
}

int CheckGiveBarbarianOK(struct char_data *ch,struct char_data *vict,
						struct obj_data *obj)
{
  char buf[MAX_STRING_LENGTH];

#ifndef BARB_GET_DISABLE 
 if (GET_LEVEL(vict,BARBARIAN_LEVEL_IND) !=0 && 
         anti_barbarian_stuff(obj) && GetMaxLevel(vict)<LOW_IMMORTAL  )
       {
	 if (GET_POS(vict)<=POSITION_SLEEPING)  {
	   sprintf(buf,"You think it best to not give this item to %s.\n\r",GET_NAME(vict));
 	   return(FALSE);
	  } else  {
	    sprintf(buf,"%s senses magic on the object and refuses it!\n\r",GET_NAME(vict));
	   }            
        send_to_char(buf,ch);
	return(FALSE);
       } 
#endif
 return(TRUE);
}

int CheckEgo(struct char_data *ch, struct obj_data *obj) 
{
 int j=0;

#if DISABLE_EGO
return(TRUE);
#else
if (!obj || !ch) {
	log("!obj || !ch in CheckEgo, utility.c");
	return(FALSE);
	}
	
  /*
    if the item is limited, check its ego.
    use some funky function to determine if pc's ego is higher than objs'
    ego.. if it is, proceed.. otherwise, deny.
    */
  j = ItemEgoClash(ch, obj, 0);
  if ((j < -5) && (GetMaxLevel(ch) < IMPLEMENTOR))
  {
    act("$p almost seems to say 'You're much too puny to use me, twerp!'",0,
	ch, obj, 0, TO_CHAR); 

 if (obj->in_obj==NULL) {
     act("$p falls to the floor",0,ch, obj, 0, TO_CHAR); 
     act("$p removes itself from $n, and falls to the floor",0,ch, obj, 0, TO_ROOM); 
    }
    return(FALSE);
  } else 
  if ((j < 0) && (GetMaxLevel(ch) < IMPLEMENTOR))  {
    act("$p almost seems to say 'You're pretty puny.  I don't want to be seen with you!\n", 0, ch, obj, 0, TO_CHAR);

if (obj->in_obj==NULL) { 
    act("$p falls to the floor",0,ch, obj, 0, TO_CHAR); 
    act("$p removes itself from $n, and falls to the floor",0,ch, obj, 0, TO_ROOM); 
 }
    return(FALSE);
  } else 
    return(TRUE);
  
  return(FALSE);
#endif
}

int CheckGetEgo(struct char_data *ch, struct obj_data *obj) 
{
  return(TRUE);
}

int CheckEgoGive(struct char_data *ch,struct char_data *vict,
					 struct obj_data *obj) 
{
 int j=0;

#if DISABLE_EGO
return(TRUE);
#else
  /*
    if the item is limited, check its ego.
    use some funky function to determine if pc's ego is higher than objs'
    ego.. if it is, proceed.. otherwise, deny.
    */
  j = ItemEgoClash(vict, obj, 0);
  if ((j < -5) && (GetMaxLevel(vict) < IMPLEMENTOR))
  {
if (AWAKE(vict)) {
      act("$p almost seems to say 'You're much too puny to use me, twerp!'",0,
 	vict, obj, 0, TO_CHAR); 
     act("$p refuses to leave $N's hands",0,vict, obj, ch, TO_CHAR); 
    }
    act("$p refuses to be given to $n by $N ",0,vict, obj, ch, TO_ROOM); 
    return(FALSE);
  } else 
  if ((j < 0) && (GetMaxLevel(vict) < IMPLEMENTOR))  {
  if (AWAKE(vict)) {
    act("$p almost seems to say 'You're pretty puny.  I don't want to be seen with you!\n", 0, vict, obj, 0, TO_CHAR);
    act("$p refuses to leave $N's hands",0,vict, obj, ch, TO_CHAR); 
    }
    act("$p refuses to be given to $n by $N.",0,vict, obj, ch ,TO_ROOM); 
    return(FALSE);
  } else 
    return(TRUE);
  
  return(FALSE);
#endif
}





int IsSpecialized(int sk_num)
{
 return(IS_SET(sk_num,SKILL_SPECIALIZED));
}

				 /* this persons max specialized skills */
int HowManySpecials(struct char_data *ch)
{
int i,ii=0;

for (i=0;i<MAX_SPL_LIST;i++)
 if (IsSpecialized(ch->skills[i].special)) ii++;
 return(ii);
}

int MAX_SPECIALS(struct char_data *ch)
{
 return(GET_INT(ch));              
}

int CanSeeTrap(int num,struct char_data *ch)
{
if (HasClass(ch,CLASS_THIEF)) {
 return((affected_by_spell(ch,SPELL_FIND_TRAPS)) ||
        (ch->skills && num < (ch->skills[SKILL_FIND_TRAP].learned)
         && !MOUNTED(ch)));
 } 
if (HasClass(ch,CLASS_RANGER) && OUTSIDE(ch)) {
   return((affected_by_spell(ch,SPELL_FIND_TRAPS)) ||
        (ch->skills && num < (ch->skills[SKILL_FIND_TRAP].learned)
         && !MOUNTED(ch)));
  }

if (affected_by_spell(ch,SPELL_FIND_TRAPS) && !MOUNTED(ch))
    return(TRUE);
    
 return(FALSE);
}

/* this is where we figure the max limited items the char may rent with    */
/* for his/her current level, pc's wanted it this way. MAX_LIM_ITEMS is in */
/* structs.h 								   */
int MaxLimited(int lev)
{
 if (lev <= 10) {
   return(2);
  } else 
 if (lev <= 15) {
   return(5);
  } else 
 if (lev <= 20) {
   return(7);
  } else
 if (lev <= 25) {
   return(9);
  } else
 if (lev <= 30) {
   return(13);
  } else
 if (lev <= 35) {
   return(15);
  } else
 if (lev <= 40) {
   return(16);
  } else
 if (lev <= 45) {
   return(17);
  } else
 return(MAX_LIM_ITEMS);    
}

int SiteLock (char *site)
{
#if SITELOCK

int i,length;
extern numberhosts;
extern char hostlist[MAX_BAN_HOSTS][30];

 length = strlen(site);

for (i=0;i<numberhosts;i++) {
  if (strncmp(hostlist[i],site,length)==0)
	return(TRUE);
   }
  return(FALSE);
#else
 return(FALSE);  
#endif
}

int MaxDexForRace(struct char_data *ch)
{
	switch(GET_RACE(ch)) {
	case RACE_ELVEN:
	case RACE_WILD_ELF:
	case RACE_GOLD_ELF:
	case RACE_HALFLING:
			return(19);
			break;
	case RACE_DROW:
			return(20);
			break;
	case RACE_DWARF:
	case RACE_HALF_OGRE:
			return(17);
			break;
	case RACE_HALF_GIANT:
			return(16);
			break;						
	default:return(18);
		break;		
	}/* end switch */
}

int MaxIntForRace(struct char_data *ch)
{
	switch(GET_RACE(ch)) {
	case RACE_GOLD_ELF:
	case RACE_GNOME :return(19);
			break;
	case RACE_HALF_GIANT:
	case RACE_HALF_OGRE:
		return(17);
		break;

	default:return(18);
			break;
	}/* end switch */
}

int MaxWisForRace(struct char_data *ch)
{
	switch(GET_RACE(ch)) {
	case RACE_GOLD_ELF:
	case RACE_WILD_ELF:
	case RACE_GNOME:
		return(17);
		break;
	case RACE_HALF_GIANT:
		return(17);
		break;		
	default:return(18);
		break;
	}/* end switch */
}

int MaxConForRace(struct char_data *ch)
{
	switch(GET_RACE(ch)) {
	 case RACE_HALF_ORC:
	 case RACE_DWARF:
	 case RACE_HALF_OGRE:return(19);
	 			break;
	 case RACE_ELVEN:
	 case RACE_GOLD_ELF:
	 case RACE_SEA_ELF:
	 case RACE_DROW:
	 		return(17);
	 		break;	 			
	 default:return(18);
	 	break;
	}/* end switch */
}

int MaxChrForRace(struct char_data *ch)
{
	switch(GET_RACE(ch)) {
	case RACE_HALF_ORC:
	case RACE_ORC:
	case RACE_DROW:
	case RACE_DWARF:
	case RACE_DARK_DWARF:
		return(17);
		break;
	default:return(18);
		break;
	}/* end switch */
}

int MaxStrForRace(struct char_data *ch)
{

if (GetMaxLevel(ch) >= SILLYLORD)
	return(25);

	switch(GET_RACE(ch)) {
	case RACE_TROLL:
	case RACE_HALF_GIANT:return(19);
				break;
				
	case RACE_HALFLING:
	case RACE_GOBLIN:
			return(17);
			break;				
	default:return(18);
		break;
	}/* end switch */
}


int IS_MURDER(struct char_data *ch)
{
#if 0
char buf[256];
 if (IS_PC(ch) && IS_SET(ch->player.user_flags,MURDER_1) && !IS_IMMORTAL(ch)) {
 sprintf(buf,"%s has the MURDER set.",GET_NAME(ch));
 log(buf); 
 return (TRUE); 
} 
#endif
  return(FALSE);
}

int IS_STEALER(struct char_data *ch)
{
#if 0
char buf[256];

 if (IS_PC(ch) && IS_SET(ch->player.user_flags,STOLE_1) && !IS_IMMORTAL(ch)) {
 sprintf(buf,"%s has STOLE set.",GET_NAME(ch));
 log(buf); 
 return(TRUE); 
}
#endif
  return(FALSE);
}

int MEMORIZED(struct char_data *ch, int spl) 
{
  if (ch->skills[spl].nummem >0)
     return(TRUE); else

   if (ch->skills[spl].nummem < 0) 
     ch->skills[spl].nummem = 0;
 return(FALSE);
}

int FORGET(struct char_data *ch, int spl) 
{
 if (ch->skills[spl].nummem)
     ch->skills[spl].nummem -=1;
}

/* return the amount max a person can memorize a single spell */
int MaxCanMemorize(struct char_data *ch, int spell)
{
int  BONUS;  /* use this later to figure item bonuses or something */

if (OnlyClass(ch,CLASS_SORCERER))
	BONUS=2; else
	BONUS=0;	/* multies get less... */

if (GET_INT(ch) > 18)
    BONUS +=(GET_INT(ch)-18);  /* +1 spell per intel over 18 */
    
if (GET_LEVEL(ch,SORCERER_LEVEL_IND) < 4) {
   return(3+BONUS);
  } else
if (GET_LEVEL(ch,SORCERER_LEVEL_IND) < 11) {
   return(3+BONUS);
  } else
if (GET_LEVEL(ch,SORCERER_LEVEL_IND) < 18) {
  return(3+BONUS);
  } else
if (GET_LEVEL(ch,SORCERER_LEVEL_IND) < 21) {
  return(4+BONUS);
  } else
if (GET_LEVEL(ch,SORCERER_LEVEL_IND) < 26) {
  return(4+BONUS);
  } else
if (GET_LEVEL(ch,SORCERER_LEVEL_IND) < 21) {
  return(5+BONUS);
  } else
if (GET_LEVEL(ch,SORCERER_LEVEL_IND) < 23) {
  return(5+BONUS);
  } else
if (GET_LEVEL(ch,SORCERER_LEVEL_IND) < 26) {
  return(6+BONUS);
  } else
if (GET_LEVEL(ch,SORCERER_LEVEL_IND) < 28) {
  return(6+BONUS);
  } else
if (GET_LEVEL(ch,SORCERER_LEVEL_IND) < 31) {
  return(7+BONUS);
  } else
if (GET_LEVEL(ch,SORCERER_LEVEL_IND) < 34) {
  return(7+BONUS);
  } else
if (GET_LEVEL(ch,SORCERER_LEVEL_IND) < 36) {
  return(7+BONUS);
  } else
if (GET_LEVEL(ch,SORCERER_LEVEL_IND) < 41) {
  return(8+BONUS);
  } else
if (GET_LEVEL(ch,SORCERER_LEVEL_IND) < 46) {
  return(9+BONUS);
  } else
if (GET_LEVEL(ch,SORCERER_LEVEL_IND) < 51) {
  return(10+BONUS);
  } else
   /* should never get here, cept for immos */
   return((int)GetMaxLevel(ch)/10);
}




int IS_LINKDEAD(struct char_data *ch)
{
 if (IS_PC(ch) && !ch->desc)
     return(TRUE);

if (IS_SET(ch->specials.act,ACT_POLYSELF) && !ch->desc)
      return(TRUE);
      
   return(FALSE);
}

int IS_UNDERGROUND(struct char_data *ch)
{
 struct room_data *rp;
 extern struct zone_data *zone_table;

  if ((rp = real_roomp(ch->in_room))!=NULL) {
   if (IS_SET(zone_table[rp->zone].reset_mode, ZONE_UNDER_GROUND)) 
   return(TRUE);
  }
 
return(FALSE);
}

int SetDefaultLang(struct char_data *ch)
{
int i;
		
switch (GET_RACE(ch)) {
	case RACE_ELVEN:
	case RACE_GOLD_ELF:
	case RACE_WILD_ELF:
	case RACE_SEA_ELF:
	case RACE_DROW:	i = LANG_ELVISH;
			break;
	case RACE_TROLL:
	case RACE_HALF_GIANT:i= LANG_GIANTISH;
				break;
	case RACE_HALF_OGRE:i=LANG_OGRE;
				break;
	case RACE_HALFLING:i=LANG_HALFLING;
				break;
	case RACE_DWARF:i=LANG_DWARVISH;
			break;
	case RACE_DEEP_GNOME:
	case RACE_GNOME:i=LANG_GNOMISH;
			break;
	default:i= LANG_COMMON;
		break;
	} /* end switch */
	ch->skills[i].learned = 95;
	SET_BIT(ch->skills[i].flags,SKILL_KNOWN);
	
	/* end default langauges sets */
}

int IsMagicSpell(int spell_num) 
{
int tmp=FALSE;

	/* using non magic items, since everything else is almost magic */
	/* lot smaller switch this way */
switch(spell_num) {
	case SKILL_BACKSTAB:
	case SKILL_SNEAK:
	case SKILL_HIDE:
	case SKILL_PICK_LOCK:
	case SKILL_KICK:
	case SKILL_BASH:
	case SKILL_RESCUE:
	case SKILL_FIRST_AID:
	case SKILL_SIGN:
	case SKILL_RIDE:
	case SKILL_SWITCH_OPP:
	case SKILL_DODGE:
	case SKILL_REMOVE_TRAP:
	case SKILL_RETREAT:
	case SKILL_QUIV_PALM:
	case SKILL_SAFE_FALL:
	case SKILL_FEIGN_DEATH:
	case SKILL_HUNT:
	case SKILL_FIND_TRAP:
	case SKILL_SPRING_LEAP:
	case SKILL_DISARM:
	case SKILL_EVALUATE:
	case SKILL_SPY:
	case SKILL_DOORBASH:
	case SKILL_SWIM:
	case SKILL_CONS_UNDEAD:
	case SKILL_CONS_VEGGIE:
	case SKILL_CONS_DEMON:
	case SKILL_CONS_ANIMAL:
	case SKILL_CONS_REPTILE:
	case SKILL_CONS_PEOPLE:
	case SKILL_CONS_GIANT:
	case SKILL_CONS_OTHER:
	case SKILL_DISGUISE:
	case SKILL_CLIMB:
	case SKILL_BERSERK:
	case SKILL_TAN:
	case SKILL_AVOID_BACK_ATTACK:
	case SKILL_FIND_FOOD:
	case SKILL_FIND_WATER:
	case SPELL_PRAYER:
	case SKILL_MEMORIZE:
	case SKILL_BELLOW:
	case SKILL_DOORWAY:
	case SKILL_PORTAL:
	case SKILL_SUMMON:
	case SKILL_INVIS:
	case SKILL_CANIBALIZE:
	case SKILL_FLAME_SHROUD:
	case SKILL_AURA_SIGHT:
	case SKILL_GREAT_SIGHT:
	case SKILL_PSIONIC_BLAST:
	case SKILL_HYPNOSIS:
	case SKILL_MEDITATE:
	case SKILL_SCRY:
	case SKILL_ADRENALIZE:
	case SKILL_RATION:
	case SKILL_HOLY_WARCRY:
	case SKILL_HEROIC_RESCUE:
	case SKILL_DUAL_WIELD:
	case SKILL_PSI_SHIELD:
	case LANG_COMMON:
	case LANG_ELVISH:
	case LANG_HALFLING:
	case LANG_DWARVISH:
	case LANG_ORCISH:
	case LANG_GIANTISH:
	case LANG_OGRE:
	case LANG_GNOMISH:
	case SKILL_ESP:				/* end skills */

	case TYPE_HIT:				/* weapon types here */
	case TYPE_BLUDGEON:
	case TYPE_PIERCE:
	case TYPE_SLASH:
	case TYPE_WHIP:
	case TYPE_CLAW:
	case TYPE_BITE:
	case TYPE_STING:
	case TYPE_CRUSH:
	case TYPE_CLEAVE:
	case TYPE_STAB:
	case TYPE_SMASH:
	case TYPE_SMITE:
	case TYPE_BLAST:
	case TYPE_SUFFERING:
	case TYPE_RANGE_WEAPON:
		tmp = FALSE;	/* these are NOT magical! */
		break;
		
	default :tmp = TRUE; /* default to IS MAGIC */
		 break;
  } /* end switch */
  
 return(tmp);
}


int exist(char *s)
{
  int f;
  
    f=open(s,O_RDONLY);
	close(f);
	
        if (f>0)
        return(TRUE);
              else
        return(FALSE);
}
                  

	/* Good side just means they are of the first races and on the */
	/* general good side of life, it does NOT refer to alignment */
	/* only good side people can kill bad side people PC's */
int IsGoodSide(struct char_data *ch)
{
  
  switch(GET_RACE(ch)) {

	case RACE_HUMAN     :
	case	 RACE_ELVEN     :
	case RACE_GOLD_ELF:
	case RACE_WILD_ELF:
	case RACE_SEA_ELF:
	case	 RACE_DWARF     :
	case	 RACE_HALFLING  :
	case	 RACE_GNOME     :
	case	 RACE_HALF_ELVEN  :
	case	 RACE_HALF_OGRE   : 
	case	 RACE_HALF_ORC    : 
	case	 RACE_HALF_GIANT  : 

	case 	RACE_DROW:
				return(TRUE);	 
     } /* */
    
  return(FALSE);
}

	/* this just means the PC is a troll/orc or the like not releated to */
	/* to alignment what-so-ever */
	/* only bad side people can hit and kill good side people PC's */
int IsBadSide(struct char_data *ch)
{
	
	switch(GET_RACE(ch))  {
	 case RACE_GOBLIN:
	 case RACE_DARK_DWARF:
	 case RACE_ORC:
	 case RACE_TROLL:
	 case RACE_MFLAYER:
 			return(TRUE);
	} /* */
	
  return(FALSE);
}

/* good side can fight bad side people pc to pc fighting only, not used */
/* for NPC fighting to pc fighting */
int CanFightEachOther(struct char_data *ch,struct char_data *ch2)
{

 if (IS_NPC(ch) && !IS_SET(ch->specials.act,ACT_POLYSELF) ||
    (IS_NPC(ch2) && !IS_SET(ch2->specials.act,ACT_POLYSELF)))
     return(TRUE);

if (IS_SET(SystemFlags,SYS_NOKILL))
    return(FALSE);
         
     if (IsGoodSide(ch) && IsGoodSide(ch2) ||
         IsBadSide(ch)  && IsBadSide (ch2)) 
         return(FALSE); else 
if (IS_SET(ch->player.user_flags,RACE_WAR) &&
    IS_SET(ch2->player.user_flags,RACE_WAR))  {
        return(TRUE);
    }
 
  return(FALSE);
}


int fighting_in_room(int room_n)
{
 struct char_data *ch;
 struct room_data *r;
                 
  r=real_roomp(room_n);
    
    if(!r) 
     return FALSE;
  
  for(ch=r->people;ch;ch=ch->next_in_room) 
    if(ch->specials.fighting) 
       return TRUE;
  
  return FALSE;
}
                                        
                                        

int str_cmp2(char *arg1, char *arg2)
{
  int chk, i;
  
  if (((!arg2) || (!arg1)) || (strlen(arg1)==0))
   return(1);

  for (i = 0; i<strlen(arg1); i++)
   if (chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))
	if (chk < 0)
  return (-1);
	else
  return (1);
  return(0);
}

int make_exit_ok(struct char_data *ch, struct room_data **rpp, int dir) 
{
 long l;
 int rdir;
 char buf[255];
 struct zone_data *zd,*tmpz;
 struct room_data *newrp;
   
 if (GetMaxLevel(ch) < LOW_IMMORTAL || !rpp || !ch->desc ||
 	!IS_SET(ch->player.user_flags,FAST_AREA_EDIT))
	 return(FALSE);

 if(GetMaxLevel(ch) < GOD &&  (*rpp)->zone != GET_ZONE(ch))  {
    send_to_char("Sorry, you are not authorized to edit this zone. Get one assigned to you.\n\r", ch);
    return(FALSE);
  }
/* check for first room avalible in this zone */
  zd = zone_table+(*rpp)->zone;
  tmpz = zone_table+(*rpp)->zone-1;
l=tmpz->top+1;
newrp=real_roomp(l);
for (l=tmpz->top+1,newrp=real_roomp(l);
     l<zd->top && newrp;l++,newrp=real_roomp(l))

if (newrp || l >zd->top) {
	send_to_char("No more empty rooms!\n\r",ch);
	return(FALSE);
	}
	
/* create new room  */
	sprintf(buf,"%ld %ld",l,l);
	do_create(ch,buf,0);

	/* check and make sure it loaded alright... */
	newrp=real_roomp(l);
	if (!newrp) {
		send_to_char("Could not create the next room!\n\r",ch);
		return(FALSE);
		}
		
/* 	set room flags and types to current rooms settings */
	newrp->zone= (*rpp)->zone;
	newrp->continent = (*rpp)->continent;
	newrp->sector_type = (*rpp)->sector_type;
	newrp->river_dir  = (*rpp)->river_dir;
	newrp->river_speed= (*rpp)->river_speed;
	newrp->tele_time = (*rpp)->tele_time;
	newrp->tele_targ = (*rpp)->tele_targ;
	newrp->tele_mask = (*rpp)->tele_mask;
	newrp->tele_cnt	 = (*rpp)->tele_cnt;
	newrp->moblim= (*rpp)->moblim;
	newrp->room_flags= (*rpp)->room_flags;

/* 	figure out number for reverse direction of dir */
	switch(dir-1) 
		{
			case NORTH:	rdir= SOUTH;
				break;
			case SOUTH:	rdir= NORTH;
				break;
			case EAST:	rdir= WEST;
				break;
			case WEST:	rdir=EAST;
				break;
			case UP	:	rdir=DOWN;
				break;
			case DOWN:	rdir=UP;
				break;
 		       default: 
 		           rdir=NORTH;
				break;
		} /* end switch */
		
/* 	make the exit to return in current room in new room */
	sprintf(buf,"exit %d 0 -1 %ld",rdir,(*rpp)->number);
	do_edit(ch,buf,0);

/*	use dir number for direction in this one */
/*	make exit in current room to new room */
	sprintf(buf,"exit %d 0 -1 %ld",dir-1,newrp->number);
	do_edit(ch,buf,0);


	
/* return(TRUE); */

 return(FALSE);
}


/* Used to search for corrupted memory, call before and after suspect functions */
void memory_check(char *p)
{
#if DEBUG
  if(!malloc_verify()) {	/* some systems might not have this lib */
    fprintf(stderr,"memory_check failed: %s!\r\n", p);
    fflush(stderr);
    abort();
  }
#endif  
}

void dlog(char *s)
{
#if DEBUG_LOG
	slog(s);
#endif
}
