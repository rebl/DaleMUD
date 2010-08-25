
#define MAX_BUF_LENGTH              240

#define TYPE_UNDEFINED               -1
#define SPELL_RESERVED_DBC            0  /* SKILL NUMBER ZERO */
#define SPELL_ARMOR                   1 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_TELEPORT                2 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLESS                   3 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLINDNESS               4 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BURNING_HANDS           5 /* Reserved Skill[] DO NOT CHANGE */
/* druid */
#define SPELL_CALL_LIGHTNING          6 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHARM_PERSON            7 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHILL_TOUCH             8 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CLONE                   9 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_COLOUR_SPRAY           10 /* Reserved Skill[] DO NOT CHANGE */
/*druid*/
#define SPELL_CONTROL_WEATHER        11 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_FOOD            12 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_WATER           13 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_BLIND             14 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_CRITIC            15 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_LIGHT             16 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURSE                  17 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_EVIL            18 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_INVISIBLE       19 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_MAGIC           20 /* Reserved Skill[] DO NOT CHANGE */
/* druid */
#define SPELL_DETECT_POISON          21 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_EVIL            22 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_EARTHQUAKE             23 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENCHANT_WEAPON         24 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENERGY_DRAIN           25 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FIREBALL               26 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HARM                   27 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HEAL                   28 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INVISIBLE              29 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LIGHTNING_BOLT         30 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LOCATE_OBJECT          31 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MAGIC_MISSILE          32 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_POISON                 33 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_PROTECT_FROM_EVIL      34 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_CURSE           35 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SANCTUARY              36 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SHOCKING_GRASP         37 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SLEEP                  38 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_STRENGTH               39 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SUMMON                 40 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_VENTRILOQUATE          41 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WORD_OF_RECALL         42 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_POISON          43 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SENSE_LIFE             44 /* Reserved Skill[] DO NOT CHANGE */

/* types of attacks and skills must NOT use same numbers as spells! */

#define SKILL_SNEAK                  45 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_HIDE                   46 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_STEAL                  47 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BACKSTAB               48 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_PICK_LOCK              49 /* Reserved Skill[] DO NOT CHANGE */

#define SKILL_KICK                   50 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BASH                   51 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_RESCUE                 52 /* MAXIMUM SKILL NUMBER  */

/* END OF SKILL RESERVED "NO TOUCH" NUMBERS */


/* NEW SPELLS are to be inserted here */
#define SPELL_IDENTIFY               53
#define SPELL_INFRAVISION            54
#define SPELL_CAUSE_LIGHT            55
#define SPELL_CAUSE_CRITICAL         56
#define SPELL_FLAMESTRIKE            57
#define SPELL_DISPEL_GOOD            58
#define SPELL_WEAKNESS               59
#define SPELL_DISPEL_MAGIC           60
#define SPELL_KNOCK                  61
#define SPELL_KNOW_ALIGNMENT         62
#define SPELL_ANIMATE_DEAD           63
#define SPELL_PARALYSIS              64
#define SPELL_REMOVE_PARALYSIS       65
#define SPELL_FEAR                   66
#define SPELL_ACID_BLAST             67
#define SPELL_WATER_BREATH           68
#define SPELL_FLY                    69
#define SPELL_CONE_OF_COLD           70
#define SPELL_METEOR_SWARM           71
#define SPELL_ICE_STORM              72
#define SPELL_SHIELD                 73
#define SPELL_MON_SUM_1              74  /* done */
#define SPELL_MON_SUM_2              75
#define SPELL_MON_SUM_3              76
#define SPELL_MON_SUM_4              77
#define SPELL_MON_SUM_5              78
#define SPELL_MON_SUM_6              79
#define SPELL_MON_SUM_7              80  /* done  */
#define SPELL_FIRESHIELD             81
#define SPELL_CHARM_MONSTER          82 
#define SPELL_CURE_SERIOUS           83
#define SPELL_CAUSE_SERIOUS          84
#define SPELL_REFRESH                85
#define SPELL_SECOND_WIND            86
#define SPELL_TURN                   87
#define SPELL_SUCCOR                 88
#define SPELL_LIGHT                  89
#define SPELL_CONT_LIGHT             90
#define SPELL_CALM                   91
#define SPELL_STONE_SKIN             92
#define SPELL_CONJURE_ELEMENTAL      93
#define SPELL_TRUE_SIGHT             94
#define SPELL_MINOR_CREATE           95
#define SPELL_FAERIE_FIRE            96
#define SPELL_FAERIE_FOG             97
#define SPELL_CACAODEMON             98
#define SPELL_POLY_SELF              99
#define SPELL_MANA                  100
#define SPELL_ASTRAL_WALK           101
#define SPELL_RESURRECTION          102

#define SPELL_H_FEAST               103
#define SPELL_FLY_GROUP             104
#define SPELL_DRAGON_BREATH         105
#define SPELL_WEB                   106
#define SPELL_MINOR_TRACK           107
#define SPELL_MAJOR_TRACK           108


#define SPELL_GOLEM                 109
#define SPELL_FAMILIAR              110
#define SPELL_CHANGESTAFF           111
#define SPELL_HOLY_WORD             112
#define SPELL_UNHOLY_WORD           113
#define SPELL_PWORD_KILL            114
#define SPELL_PWORD_BLIND           115

#define SPELL_CHAIN_LIGHTNING       116
#define SPELL_SCARE                 117
#define SPELL_AID                   118
#define SPELL_COMMAND               119

/* druid */
#define SPELL_CHANGE_FORM           120
#define SPELL_FEEBLEMIND            121

/* druid... */
#define SPELL_SHILLELAGH            122
#define SPELL_GOODBERRY             123
#define SPELL_FLAME_BLADE           124
#define SPELL_ANIMAL_GROWTH         125
#define SPELL_INSECT_GROWTH         126
#define SPELL_CREEPING_DEATH        127
#define SPELL_COMMUNE               128  /* whatzone*/

#define SPELL_ANIMAL_SUM_1          129
#define SPELL_ANIMAL_SUM_2          130
#define SPELL_ANIMAL_SUM_3          131

#define SPELL_FIRE_SERVANT          132
#define SPELL_EARTH_SERVANT         133
#define SPELL_WATER_SERVANT         134
#define SPELL_WIND_SERVANT          135

#define SPELL_REINCARNATE           136
#define SPELL_CHARM_VEGGIE          137
#define SPELL_VEGGIE_GROWTH         138
#define SPELL_TREE                  139

#define SPELL_ANIMATE_ROCK          140
#define SPELL_TREE_TRAVEL           141
#define SPELL_TRAVELLING            142  /* faster move outdoors */
#define SPELL_ANIMAL_FRIENDSHIP     143
#define SPELL_INVIS_TO_ANIMALS      144
#define SPELL_SLOW_POISON           145
#define SPELL_ENTANGLE              146
#define SPELL_SNARE                 147
#define SPELL_GUST_OF_WIND          148
#define SPELL_BARKSKIN              149
#define SPELL_SUNRAY                150
#define SPELL_WARP_WEAPON           151
#define SPELL_HEAT_STUFF            152
#define SPELL_FIND_TRAPS            153
#define SPELL_FIRESTORM             154

/* other */
#define SPELL_HASTE                 155
#define SPELL_SLOW                  156
#define SPELL_DUST_DEVIL            157
#define SPELL_KNOW_MONSTER          158

#define SPELL_TRANSPORT_VIA_PLANT   159
#define SPELL_SPEAK_WITH_PLANT      160
#define SPELL_SILENCE               161
#define SPELL_SENDING               162
#define SPELL_TELEPORT_WO_ERROR     163
#define SPELL_PORTAL                164
#define SPELL_DRAGON_RIDE           165
#define SPELL_MOUNT                 166
  



/* room spell like deal */

#define SPELL_BLADE_BARRIER        

/* maybe */
#define SPELL_SUMMON_OBJ            

/* add 167-169 here.......   */

#define SKILL_FIRST_AID              170
#define SKILL_SIGN                   171
#define SKILL_RIDE                   172
#define SKILL_SWITCH_OPP             173
#define SKILL_DODGE                  174
#define SKILL_REMOVE_TRAP            175
#define SKILL_RETREAT                176
#define SKILL_QUIV_PALM              177
#define SKILL_SAFE_FALL              178
#define SKILL_FEIGN_DEATH            179
#define SKILL_HUNT                   180
#define SKILL_FIND_TRAP              181
#define SKILL_SPRING_LEAP            182
#define SKILL_DISARM                 183
#define SKILL_READ_MAGIC             184
#define SKILL_EVALUATE               185
#define SKILL_SPY                    186
#define SKILL_DOORBASH               187
#define SKILL_SWIM                   188
#define SKILL_CONS_UNDEAD            189
#define SKILL_CONS_VEGGIE            190
#define SKILL_CONS_DEMON             191
#define SKILL_CONS_ANIMAL            192
#define SKILL_CONS_REPTILE           193
#define SKILL_CONS_PEOPLE            194
#define SKILL_CONS_GIANT             195
#define SKILL_CONS_OTHER             196
#define SKILL_DISGUISE               197
#define SKILL_CLIMB                  198

/* add skill 199 here */

#define SPELL_GEYSER                 200

/* 201 .. 205 */

#define SPELL_GREEN_SLIME            206


/* Add more skills/Spells here... */

#define SKILL_BERSERK            207    /* msw */
#define SKILL_TAN	         208	/* msw */
#define SKILL_AVOID_BACK_ATTACK  209	/* msw */
#define SKILL_FIND_FOOD		 210	/* msw */
#define SKILL_FIND_WATER	 211    /* msw */
#define SPELL_PRAYER		 212    /* msw, not a spell but I need */
					/* a aff flag,could use PLR_*   */
#define SKILL_MEMORIZE		 213    /* msw, used for memorization stuff */					
#define SKILL_BELLOW		 214   /* msw */

/* add more spells / skills here */

#define SPELL_GLOBE_DARKNESS     215
#define SPELL_GLOBE_MINOR_INV    216
#define SPELL_GLOBE_MAJOR_INV    217
#define SPELL_PROT_ENERGY_DRAIN  218
#define SPELL_PROT_DRAGON_BREATH 219 /* used for scrolls, ALL BREATH */
#define SPELL_ANTI_MAGIC_SHELL   220

#define SKILL_DOORWAY		221 /* psi, msw */
#define SKILL_PORTAL		222 /* psi, msw */
#define SKILL_SUMMON		223 /* psi, msw */
#define SKILL_INVIS		224 /* psi, msw */
#define SKILL_CANIBALIZE	225 /* psi, msw */
#define SKILL_FLAME_SHROUD	226 /* psi, msw */
#define SKILL_AURA_SIGHT	227 /* psi, msw */
#define SKILL_GREAT_SIGHT	228 /* psi, msw */
#define SKILL_PSIONIC_BLAST	229 /* psi, msw */
#define SKILL_HYPNOSIS		230 /* psi, msw */
#define SKILL_MEDITATE		231 /* psi, msw */
#define SKILL_SCRY		232 /* psi, msw */
#define SKILL_ADRENALIZE	233 /* psi, msw */

#define SKILL_BREW		234 /* druid/mages, msw */
#define SKILL_RATION		235 /* ranger/druid, msw */

#define SKILL_HOLY_WARCRY	236  /* paladin, msw */
#define SKILL_BLESSING		237  /* paladin, msw */
#define SKILL_LAY_ON_HANDS	238  /* paladin, msw */
#define SKILL_HEROIC_RESCUE	239  /* paladin, msw */
#define SKILL_DUAL_WIELD	240  /* ranger, msw */
#define SKILL_PSI_SHIELD	241  /* psi, psw */
#define SPELL_PROT_FROM_EVIL_GROUP 242 /* msw */
#define SPELL_PRISMATIC_SPRAY	   243 /* msw */
#define SPELL_INCENDIARY_CLOUD	   244

#define SPELL_DISINTERGRATE	   245	/* not complete */

/* language skills */
#define LANG_COMMON		246
#define LANG_ELVISH		247
#define LANG_HALFLING		248
#define LANG_DWARVISH		249
#define LANG_ORCISH		250
#define LANG_GIANTISH		251
#define LANG_OGRE		252
#define LANG_GNOMISH		253
/* end languages at this time */
#define SKILL_ESP		254
#define SPELL_COMP_LANGUAGES	255
#define SPELL_PROT_FIRE		256
#define SPELL_PROT_COLD		257
#define SPELL_PROT_ENERGY	258
#define SPELL_PROT_ELEC		259
#define SPELL_ENCHANT_ARMOR	260
#define SPELL_MESSENGER		261
#define SPELL_PROT_BREATH_FIRE		262
#define SPELL_PROT_BREATH_FROST		263
#define SPELL_PROT_BREATH_ELEC		264
#define SPELL_PROT_BREATH_ACID		265
#define SPELL_PROT_BREATH_GAS		266
#define SPELL_WIZARDEYE			267
#define SKILL_MIND_BURN		268	/* flamer like Fire Starter */
#define SKILL_CLAIRVOYANCE	269	/* spy into the next room */
#define SKILL_DANGER_SENSE	270	/* sense dt's */
#define SKILL_DISINTERGRATE	271	/* not effective when used against minor or major globe */
#define SKILL_TELEKINESIS	272	/* shove mob form room, if fighting, like bash */
#define SKILL_LEVITATION	273	/* fly for psi's */
#define SKILL_CELL_ADJUSTMENT	274	/* 100 mana,stunned, get 100 HPS back */
#define SKILL_CHAMELEON		275	/* hide */
#define SKILL_PSI_STRENGTH	276	/* strength */
#define SKILL_MIND_OVER_BODY	277	/* no food/drink, rest, then 12hrs no-food/drink period */
#define SKILL_PROBABILITY_TRAVEL 278	/* astral for psi's */
#define SKILL_PSI_TELEPORT	279	/* teleport */
#define SKILL_DOMINATION	280	/* like command I think... */
#define SKILL_MIND_WIPE		281	/* feeble mind type attack */
#define SKILL_PSYCHIC_CRUSH	282	/* attack spell, massive */
#define SKILL_TOWER_IRON_WILL	283	/* immnune to some skills */
#define SKILL_MINDBLANK		284	/* immune to more skills */
#define SKILL_PSYCHIC_IMPERSONATION	285	/* disguise */
#define SKILL_ULTRA_BLAST 	286	/* area effect psionic blast type skill */
#define SKILL_INTENSIFY 	287	/* increase con/int or wis +1, -3 for attribs not chosen */
#define SKILL_SPOT		288	/* spot, for ranged weapons, archery */

#define MAX_EXIST_SPELL         288   /* max number of skills/spells */

#define FIRST_BREATH_WEAPON	     306 /*	201 */
#define SPELL_FIRE_BREATH            306 /*	201 */
#define SPELL_GAS_BREATH             307 /*	202 */
#define SPELL_FROST_BREATH           308 /*	203 */
#define SPELL_ACID_BREATH            309 /*	204 */
#define SPELL_LIGHTNING_BREATH       310 /*	205 */
#define LAST_BREATH_WEAPON	     310 /*	205 */

/* NOTE!!!!!!!!!!!!!!!
   all spells MUST be before these types.   Otherwise, certain aspects of
   fireshield, sanct, etc, will not work!
   */


#define TYPE_HIT                     311
#define TYPE_BLUDGEON                312
#define TYPE_PIERCE                  313
#define TYPE_SLASH                   314
#define TYPE_WHIP                    315  /* EXAMPLE */
#define TYPE_CLAW                    316  /* NO MESSAGES WRITTEN YET! */
#define TYPE_BITE                    317  /* NO MESSAGES WRITTEN YET! */
#define TYPE_STING                   318  /* NO MESSAGES WRITTEN YET! */
#define TYPE_CRUSH                   319  /* NO MESSAGES WRITTEN YET! */
#define TYPE_CLEAVE                  320
#define TYPE_STAB                    321
#define TYPE_SMASH                   322
#define TYPE_SMITE                   323
#define TYPE_BLAST                   324
#define TYPE_RANGE_WEAPON	     325

	/* This one needs to be after all real damage skills, might wanna */
	/* alter messages if you move it */
#define TYPE_SUFFERING               326
                                   /* MAX is MAX_SKILLS = 350 */

/* More anything but spells and weapontypes can be insterted here! */
#define MAX_TYPES 70
#define MAX_SPL_LIST	326

#define SAVING_PARA   0
#define SAVING_ROD    1
#define SAVING_PETRI  2
#define SAVING_BREATH 3
#define SAVING_SPELL  4


#define TAR_IGNORE	 (1<< 0)
#define TAR_CHAR_ROOM	 (1<< 1)
#define TAR_CHAR_WORLD	 (1<< 2)
#define TAR_FIGHT_SELF	 (1<< 3)
#define TAR_FIGHT_VICT	 (1<< 4)
#define TAR_SELF_ONLY	 (1<< 5) /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_SELF_NONO	 (1<< 6) /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_OBJ_INV	 (1<< 7)
#define TAR_OBJ_ROOM	 (1<< 8)
#define TAR_OBJ_WORLD	 (1<< 9)
#define TAR_OBJ_EQUIP	 (1<<10)
#define TAR_NAME	 (1<<11)
#define TAR_VIOLENT	 (1<<12)
#define TAR_ROOM         (1<<13)  /* spells which target the room  */

struct spell_info_type
{
	void (*spell_pointer) 
               (byte level, struct char_data *ch, char *arg, int type,
	        struct char_data *tar_ch, struct obj_data *tar_obj);
	byte minimum_position;  /* Position for caster 			*/
	ubyte min_usesmana;     /* Amount of mana used by a spell	 */
	byte beats;             /* Heartbeats until ready for next */

	byte min_level_cleric;  /* Level required for cleric       */
	byte min_level_magic;   /* Level required for magic user   */
	byte min_level_druid;   /* Level required for druids       */
	byte min_level_sorcerer;
	byte min_level_paladin;
	byte min_level_ranger;
	byte min_level_psi;
	sh_int targets;         /* See below for use with TAR_XXX  */
        sh_int spellfail;       /* modifier for spell failure      */
};

/* Possible Targets:

   bit 0 : IGNORE TARGET
   bit 1 : PC/NPC in room
   bit 2 : PC/NPC in world
   bit 3 : Object held
   bit 4 : Object in inventory
   bit 5 : Object in room
   bit 6 : Object in world
   bit 7 : If fighting, and no argument, select tar_char as self
   bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
   bit 9 : If no argument, select self, if argument check that it IS self.

*/

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4


/* Attacktypes with grammar */

struct attack_hit_type {
  char *singular;
  char *plural;
};
