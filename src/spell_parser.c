

/*
*** DaleMUD, 
*/



#include <stdio.h>
#include <assert.h>

#include "protos.h"


#define MANA_MU 1
#define MANA_CL 1




/* 100 is the MAX_MANA for a character */
#define USE_MANA(ch, sn) \
  MAX((int)spell_info[sn].min_usesmana,100/MAX(2,(2+GET_LEVEL(ch, BestMagicClass(ch))-SPELL_LEVEL(ch,sn))))

/* Global data */

extern struct room_data *world;
extern struct char_data *character_list;
extern char *spell_wear_off_msg[];
extern char *spell_wear_off_soon_msg[];
extern char *spell_wear_off_room_msg[];
extern char *spell_wear_off_soon_room_msg[];
extern struct obj_data *object_list;
extern struct index_data *obj_index;

/*  internal procedures */
void SpellWearOffSoon(int s, struct char_data *ch);
void check_drowning( struct char_data *ch);
int check_falling( struct char_data *ch);
void check_decharm( struct char_data *ch);



/* Extern procedures */

char *strdup(char *str);

void NailThisSucker( struct char_data *ch);
void do_look( struct char_data *ch, char *arg, int cmd);
void DamageAllStuff( struct char_data *ch, int dam_type);


/* Extern procedures */

/* psi stuff */
void mind_use_burn( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void mind_use_teleport( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void mind_use_probability_travel( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void mind_use_danger_sense( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void mind_use_clairvoyance( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void mind_use_disintergrate( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void mind_use_telekinesis( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void mind_use_levitation( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void mind_use_cell_adjustment( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void mind_use_chameleon( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void mind_use_psi_strength( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void mind_use_mind_over_body( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void mind_use_domination( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void mind_use_mind_wipe( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void mind_use_psychic_crush( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void mind_use_tower_iron_will( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void mind_use_mindblank( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void mind_use_psychic_impersonation( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void mind_use_ultra_blast( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void mind_use_intensify( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
/* end psi mind stuff */


void cast_animate_dead( byte level, struct char_data *ch, char *arg, int type,
		       struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_conjure_elemental( byte level, struct char_data *ch, char *arg, 
			    int type, struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_acid_blast( byte level, struct char_data *ch, char *arg, int type,
		     struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_armor( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_teleport( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_bless( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_blindness( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_burning_hands( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_incendiary_cloud( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void cast_prismatic_spray(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void cast_call_lightning( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_charm_person( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_charm_monster( byte level, struct char_data *ch, char *arg, int si, 
			struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cacaodemon( byte level, struct char_data *ch, char *arg, int si, 
			struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_chill_touch( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_shocking_grasp( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_clone( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_colour_spray( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_control_weather( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_food( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_water( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_blind( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_critic( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cause_critic( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_light( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cause_light( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_curse( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cont_light( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, 
		     struct obj_data *tar_obj);
void cast_calm( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, 
	       struct obj_data *tar_obj);
void cast_detect_evil( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_invisibility( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_magic( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_poison( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dispel_evil( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dispel_good( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dispel_magic( byte level, struct char_data *ch, char *arg, int type,
		       struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_earthquake( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_enchant_weapon( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_enchant_armor( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_energy_drain( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_fear( byte level, struct char_data *ch, char *arg, int type,
	       struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_fireball( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_flamestrike( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_flying( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_flying( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_harm( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_heal( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_infravision( byte level, struct char_data *ch, char *arg, int type, struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_invisibility( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cone_of_cold( byte level, struct char_data *ch, char *arg, int type,
		       struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_ice_storm( byte level, struct char_data *ch, char *arg, int type,
		    struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_knock( byte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_know_alignment(byte level, struct char_data *ch, char *arg, int type,
			 struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_true_seeing(byte level, struct char_data *ch, char *arg, int type,
			 struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_minor_creation(byte level, struct char_data *ch, char *arg, int type,
			 struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_faerie_fire(byte level, struct char_data *ch, char *arg, int type,
			 struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_faerie_fog(byte level, struct char_data *ch, char *arg, int type,
			 struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_heroes_feast(byte level, struct char_data *ch, char *arg, int type,
			 struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_fly_group(byte level, struct char_data *ch, char *arg, int type,
			 struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_web(byte level, struct char_data *ch, char *arg, int type,
			 struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_minor_track(byte level, struct char_data *ch, char *arg, int type,
			 struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_major_track(byte level, struct char_data *ch, char *arg, int type,
			 struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_mana(byte level, struct char_data *ch, char *arg, int type,
			 struct char_data *tar_ch, struct obj_data *tar_obj );



void cast_lightning_bolt( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_light( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, 
		struct obj_data *tar_obj);
void cast_locate_object( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_magic_missile( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_mon_sum1( byte level, struct char_data *ch, char *arg, int si, 
		   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mon_sum2( byte level, struct char_data *ch, char *arg, int si, 
		   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mon_sum3( byte level, struct char_data *ch, char *arg, int si, 
		   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mon_sum4( byte level, struct char_data *ch, char *arg, int si, 
		   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mon_sum5( byte level, struct char_data *ch, char *arg, int si, 
		   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mon_sum6( byte level, struct char_data *ch, char *arg, int si, 
		   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mon_sum7( byte level, struct char_data *ch, char *arg, int si, 
		   struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_meteor_swarm( byte level, struct char_data *ch, char *arg, int si, 
		       struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_poly_self( byte level, struct char_data *ch, char *arg, int si, 
		 struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_change_form( byte level, struct char_data *ch, char *arg, int si, 
		 struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_poison( byte level, struct char_data *ch, char *arg, int si, 
		 struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_protection_from_evil( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_protection_from_evil_group( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj );

void cast_remove_curse( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sanctuary( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sleep( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_strength( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_stone_skin( byte level, struct char_data *ch, char *arg, int si, 
		     struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_summon( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_ventriloquate( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_word_of_recall( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_water_breath( byte level, struct char_data *ch, char *arg, int si, 
		       struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_remove_poison( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_remove_paralysis( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_weakness( byte level, struct char_data *ch, char *arg, int type,
		   struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_sense_life( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_identify( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_paralyze( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_dragon_breath( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *potion);


void cast_fireshield( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_serious( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cause_serious( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_refresh( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_second_wind( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_shield( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_turn( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_succor( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_astral_walk( byte level, struct char_data *ch, char *arg, int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_resurrection( byte level, struct char_data *ch, char *arg, int si,
struct char_data *tar_ch, struct obj_data *tar_obj);


void cast_tree_travel( byte level, struct char_data *ch, char *arg, int type,
                       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_haste( byte level, struct char_data *ch, char *arg, int type,
                       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_slow( byte level, struct char_data *ch, char *arg, int type,
                       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_speak_with_plants( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);


void cast_transport_via_plant( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);




void cast_changestaff( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_holyword( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_unholyword( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_aid( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_golem( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_familiar( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_pword_kill( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_pword_blind( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_command( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_scare( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_chain_lightn( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_reincarnate( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_feeblemind( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_shillelagh( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_goodberry( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_flame_blade( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_animal_growth( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_insect_growth( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_creeping_death( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_commune( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);


void cast_animal_summon_1( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_animal_summon_2( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_animal_summon_3( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_fire_servant( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_earth_servant( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_water_servant( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_wind_servant( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);


void cast_charm_veggie( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_veggie_growth( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_tree( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_animate_rock( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_travelling( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_animal_friendship( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_invis_to_animals( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_slow_poison( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_entangle( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_snare( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_gust_of_wind( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_barkskin( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_sunray( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_heat_stuff( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_warp_weapon( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_find_traps( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_firestorm( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dust_devil( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_know_monster( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_silence( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);


void cast_sending( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_messenger( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);

void cast_teleport_wo_error( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_portal( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dragon_ride( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mount( byte level, struct char_data *ch, char *arg, 
		int type,  struct char_data *tar_ch, struct obj_data *tar_obj);

/* new ones msw */
void cast_globe_darkness( byte level, struct char_data *ch, char *arg, 
     int type, struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_globe_minor_inv( byte level, struct char_data *ch, char *arg, 
     int type, struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_globe_major_inv( byte level, struct char_data *ch, char *arg, 
     int type, struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_prot_energy_drain( byte level, struct char_data *ch, char *arg, 
     int type, struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_prot_dragon_breath( byte level, struct char_data *ch, char *arg, 
     int type, struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_prot_dragon_breath_fire( byte level, struct char_data *ch, char *arg, 
     int type, struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_prot_dragon_breath_frost( byte level, struct char_data *ch, char *arg, 
     int type, struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_prot_dragon_breath_elec( byte level, struct char_data *ch, char *arg, 
     int type, struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_prot_dragon_breath_acid( byte level, struct char_data *ch, char *arg, 
     int type, struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_prot_dragon_breath_gas( byte level, struct char_data *ch, char *arg, 
     int type, struct char_data *tar_ch, struct obj_data *tar_obj );

void cast_anti_magic_shell( byte level, struct char_data *ch, char *arg, 
     int type, struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_comp_languages( byte level, struct char_data *ch, char *arg, int type,
		     struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_prot_fire( byte level, struct char_data *ch, char *arg, int type,
		     struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_prot_cold( byte level, struct char_data *ch, char *arg, int type,
		     struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_prot_energy( byte level, struct char_data *ch, char *arg, int type,
		     struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_prot_elec( byte level, struct char_data *ch, char *arg, int type,
		     struct char_data *tar_ch, struct obj_data *tar_obj );

void cast_disintergrate( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
void cast_wizard_eye( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *victim, struct obj_data *tar_obj );
  







struct spell_info_type spell_info[MAX_SPL_LIST];

char *spells[]=
{
   "armor",               /* 1 */
   "teleport",
   "bless",
   "blindness",
   "burning hands",
   "call lightning",
   "charm person",
   "chill touch",
   "clone",
   "colour spray",
   "control weather",     /* 11 */
   "create food",
   "create water",
   "cure blind",
   "cure critic",
   "cure light",
   "curse",
   "detect evil",
   "detect invisibility",
   "detect magic",
   "detect poison",       /* 21 */
   "dispel evil",
   "earthquake",
   "enchant weapon",
   "energy drain",
   "fireball",
   "harm",
   "heal",
   "invisibility",
   "lightning bolt",
   "locate object",      /* 31 */
   "magic missile",
   "poison",
   "protection from evil",
   "remove curse",
   "sanctuary",
   "shocking grasp",
   "sleep",
   "strength",
   "summon",
   "ventriloquate",      /* 41 */
   "word of recall",
   "remove poison",
   "sense life",         /* 44 */

   /* RESERVED SKILLS */
   "sneak",        /* 45 */
   "hide",
   "steal",
   "backstab",
   "pick",
   "kick",         /* 50 */
   "bash",
   "rescue",
   /* NON-CASTABLE SPELLS (Scrolls/potions/wands/staffs) */

   "identify",           /* 53 */
   "infravision",        
   "cause light",        
   "cause critical",
   "flamestrike",
   "dispel good",      
   "weakness",
   "dispel magic",
   "knock",
   "know alignment",
   "animate dead",
   "paralyze",
   "remove paralysis",
   "fear",
   "acid blast",  /* 67 */
   "water breath",
   "fly",
   "cone of cold",   /* 70 */
   "meteor swarm",
   "ice storm",
   "shield",
   "monsum one",
   "monsum two",
   "monsum three",
   "monsum four",
   "monsum five",
   "monsum six",
   "monsum seven",  /* 80 */
   "fireshield",
   "charm monster",
   "cure serious",
   "cause serious",
   "refresh",
   "second wind",
   "turn",
   "succor",
   "create light",
   "continual light",	/* 90 */
   "calm",
   "stone skin",
   "conjure elemental",
   "true sight",
   "minor creation",
   "faerie fire",
   "faerie fog",
   "cacaodemon",
   "polymorph self",
   "mana",	/* 100 */
   "astral walk",
   "resurrection",
   "heroes feast",  /* 103 */
   "group fly",
   "breath",
   "web",
   "minor track",
   "major track",
   "golem",
   "find familiar",	/* 110 */
   "changestaff",
   "holy word",
   "unholy word",
   "power word kill",
   "power word blind",
   "chain lightning",
   "scare",
   "aid",
   "command",
   "change form",	/* 120 */
   "feeblemind",
   "shillelagh",
   "goodberry",
   "flame blade",
   "animal growth",
   "insect growth",
   "creeping death",
   "commune",
   "animal summon one",
   "animal summon two",	/* 130 */
   "animal summon three",
   "fire servant",
   "earth servant",
   "water servant",
   "wind servant",
   "reincarnate",
   "charm vegetable",
   "vegetable growth",
   "tree",
   "animate rock",	/* 140 */
   "tree travel",
   "travelling",
   "animal friendship",
   "invis to animals",
   "slow poison",
   "entangle",
   "snare",
   "gust of wind",
   "barkskin",
   "sunray",	/* 150 */
   "warp weapon",
   "heat stuff",
   "find traps",
   "firestorm",
   "haste",
   "slowness",
   "dust devil",
   "know monster",
   "transport via plant",
   "speak with plants",	/* 160 */
   "silence",
   "sending",
   "teleport without error",
   "portal",
   "dragon ride",
   "mount",
   "****",
   "****",
   "****",
   "first aid",	/* 170 */
   "sign language",
   "riding",
   "switch opponents",
   "dodge",
   "remove trap",
   "retreat",
   "quivering palm",
   "safe fall",
   "feign death",
   "hunt",	/* 180 */
   "find trap",
   "spring leap",
   "disarm",
   "read magic",
   "evauluate",
   "spy",
   "doorbash",
   "swim",
   "necromancy",
   "vegetable lore",	/* 190 */
   "demonology",
   "animal lore",
   "reptile lore",
   "people lore",
   "giant lore",
   "other lore",	/* 196 */
   "disguise",
   "climb",
   "***",
   "***",	/* 200 */
   "***",
   "***",
   "***",
   "***",
   "***",
   "***",
   "berserk",
   "tan",
   "avoid back attack",
   "find food",    /* 210 */
   "find water",   /* 211 */
   "pray",    	   /* spell Prayer, 212 */
   "memorizing",
   "bellow",
   "darkness",
   "minor invulnerability",
   "major invulnerability",
   "protection from drain",
   "protection from breath",
   "anti magic shell",	 /* anti magic shell */
   "doorway",
   "psi portal",
   "psi summon",
   "psi invisibility",
   "canibalize",
   "flame shroud",
   "aura sight",
   "great sight",
   "psionic blast",
   "hypnosis",
   "meditate",
   "scry",
   "adrenalize",
   "brew",
   "ration",
   "warcry",
   "blessing",
   "lay on hands",
   "heroic rescue",
   "dual wield",
   "psi shield",
   "protection from evil group",
   "prismatic spray",
   "incendiary cloud",
   "disintergrate",
   "language common",
   "language elvish",
   "language halfling",
   "language dwarvish",
   "language orcish",
   "language giantish",
   "language ogre",
   "language gnomish",
   "esp",
   "comprehend languages",
   "protection from fire",
   "protection from cold",
   "protection from energy",
   "protection from electricity",
   "enchant armor",
   "messenger",
   "protection fire breath",
   "protection frost breath",
   "protection electric breath",
   "protection acid breath",
   "protection gas breath",
   "wizardeye",
   "mind burn",
   "clairvoyance",
   "psionic danger sense",
   "psionic disintergrate",
   "telekinesis",
   "levitation",
   "cell adjustment",
   "chameleon",
   "psionic strength",
   "mind over body",
   "probability travel",
   "psionic teleport",
   "domination",
   "mind wipe",
   "psychic crush",
   "tower of iron will",
   "mindblank",
   "psychic impersonation",
   "ultra blast",
   "intensify",
   "spot",
   "\n"
};


const byte saving_throws[MAX_CLASS][5][ABS_MAX_LVL] = {
{
/* mage */
  {16,14,14,14,14,14,13,13,13,13,13,11,11,11,11,11,10,10,10,10,10, 8, 6, 4, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0},
  {13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 2, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0},
  {15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 4, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0},
  {17,15,15,15,15,15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 5, 3, 3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0},
  {14,12,12,12,12,12,10,10,10,10,10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0}
}, 
/* cleric */
{
  {11,10,10,10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0},
  {16,14,14,14,13,13,13,11,11,11,10,10,10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0},
  {15,13,13,13,12,12,12,10,10,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {18,16,16,16,15,15,15,13,13,13,12,12,12,11,11,11,10,10,10, 8, 8, 7, 6, 5, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {17,15,15,15,14,14,14,12,12,12,11,11,11,10,10,10, 9, 9, 9, 7, 7, 6, 5, 4, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
}, 
/* warrior */
{
  {15,13,13,13,13,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 7, 6, 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {16,14,14,14,14,12,12,12,12,10,10,10,10, 8, 8, 8, 8, 6, 6, 6, 6, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {14,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 5, 3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {18,16,16,16,16,15,15,15,15,14,14,14,14,13,13,13,13,12,12,12,12,11, 9, 5, 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {17,15,15,15,15,13,13,13,13,11,11,11,11, 9, 9, 9, 9, 7, 7, 7, 7, 5, 3, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
}, 
/* thief */
{
  {16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {18,16,16,15,15,13,13,12,12,10,10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {17,15,15,14,14,12,12,11,11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {20,17,17,16,16,13,13,12,12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {19,17,17,16,16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
}, /* druid */
{
  {11,10,10,10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0},
  {16,14,14,14,13,13,13,11,11,11,10,10,10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0},
  {15,13,13,13,12,12,12,10,10,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {18,16,16,16,15,15,15,13,13,13,12,12,12,11,11,11,10,10,10, 8, 8, 7, 6, 5, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {17,15,15,15,14,14,14,12,12,12,11,11,11,10,10,10, 9, 9, 9, 7, 7, 6, 5, 4, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
}, 
{ /* monk */
  {16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {18,16,16,15,15,13,13,12,12,10,10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {17,15,15,14,14,12,12,11,11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {20,17,17,16,16,13,13,12,12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {19,17,17,16,16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
},
/* barbarian */
{
  {15,13,13,13,13,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 7, 6, 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {16,14,14,14,14,12,12,12,12,10,10,10,10, 8, 8, 8, 8, 6, 6, 6, 6, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {14,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 5, 3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {18,16,16,16,16,15,15,15,15,14,14,14,14,13,13,13,13,12,12,12,12,11, 9, 5, 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {17,15,15,15,15,13,13,13,13,11,11,11,11, 9, 9, 9, 9, 7, 7, 7, 7, 5, 3, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
},
		 /* sorcerer */
{
  {16,14,14,14,14,14,13,13,13,13,13,11,11,11,11,11,10,10,10,10,10, 8, 6, 4, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0},
  {13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 2, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0},
  {15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 4, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0},
  {17,15,15,15,15,15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 5, 3, 3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0},
  {14,12,12,12,12,12,10,10,10,10,10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0}
},
/* paladin (same as cleric) */
{
  {11,10,10,10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0},
  {16,14,14,14,13,13,13,11,11,11,10,10,10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0},
  {15,13,13,13,12,12,12,10,10,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {18,16,16,16,15,15,15,13,13,13,12,12,12,11,11,11,10,10,10, 8, 8, 7, 6, 5, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {17,15,15,15,14,14,14,12,12,12,11,11,11,10,10,10, 9, 9, 9, 7, 7, 6, 5, 4, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
}, 
 /* ranger (same as druid) */
{
  {11,10,10,10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0},
  {16,14,14,14,13,13,13,11,11,11,10,10,10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0,0},
  {15,13,13,13,12,12,12,10,10,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {18,16,16,16,15,15,15,13,13,13,12,12,12,11,11,11,10,10,10, 8, 8, 7, 6, 5, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {17,15,15,15,14,14,14,12,12,12,11,11,11,10,10,10, 9, 9, 9, 7, 7, 6, 5, 4, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
},
/* psi */
{
  {16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {18,16,16,15,15,13,13,12,12,10,10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {17,15,15,14,14,12,12,11,11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {20,17,17,16,16,13,13,12,12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {19,17,17,16,16,14,14,13,13,11,11,10,10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
}
};


void spello(int nr, byte beat, byte pos, 
	    byte mlev, byte clev, byte dlev, 
	    byte slev, byte plev, byte rlev, byte ilev,
	    ubyte mana, sh_int tar, void *func, sh_int sf)
{
  spell_info[nr].spell_pointer      = func;
  spell_info[nr].beats 		    = beat;
  spell_info[nr].minimum_position   = pos;
  spell_info[nr].min_level_cleric   = clev;
  spell_info[nr].min_level_magic    = mlev;
  spell_info[nr].min_level_druid    = dlev;
  spell_info[nr].min_level_sorcerer = slev;  
  spell_info[nr].min_level_paladin  = plev;  
  spell_info[nr].min_level_ranger   = rlev;  
  spell_info[nr].min_level_psi      = ilev;  
  spell_info[nr].min_usesmana 	    = mana;  
  spell_info[nr].targets            = tar;
  spell_info[nr].spellfail 	    = sf;
}


int SPELL_LEVEL(struct char_data *ch, int sn) 
{
  int min;

  min = ABS_MAX_LVL;

  if (HasClass(ch, CLASS_MAGIC_USER))
    min = MIN(min, spell_info[sn].min_level_magic);

  if (HasClass(ch, CLASS_SORCERER  ))
    min = MIN(min, spell_info[sn].min_level_sorcerer);


  if (HasClass(ch, CLASS_CLERIC))
    min = MIN(min, spell_info[sn].min_level_cleric);

  if (HasClass(ch, CLASS_PALADIN))
    min = MIN(min, spell_info[sn].min_level_paladin);

  if (HasClass(ch, CLASS_RANGER))
    min = MIN(min, spell_info[sn].min_level_ranger);

  if (HasClass(ch, CLASS_PSI))
    min = MIN(min, spell_info[sn].min_level_psi);

  if (HasClass(ch, CLASS_DRUID))
    min = MIN(min, spell_info[sn].min_level_druid);

  return(min);

#if 0
  if ((HasClass(ch, CLASS_MAGIC_USER)) &&
      (HasClass(ch, CLASS_CLERIC))) {
   return(MIN(spell_info[sn].min_level_magic,spell_info[sn].min_level_cleric));
  } else if (HasClass(ch, CLASS_MAGIC_USER)) {
    return(spell_info[sn].min_level_magic);
  } else {
    return(spell_info[sn].min_level_cleric);
  }
#endif

}

#if 1
void affect_update( int pulse )
{
  register struct affected_type *af, *next_af_dude;
  register struct char_data *i;
  register struct obj_data *j;
  struct obj_data *next_thing;
  struct char_data  *next_char;
  struct room_data *rp;
  int dead=FALSE, room, k;
  
  extern struct time_info_data time_info;
  
  
  void update_char_objects( struct char_data *ch ); /* handler.c */
  void do_save(struct char_data *ch, char *arg, int cmd); /* act.other.c */

  
  for (i = character_list; i; i = next_char) {
    next_char = i->next;
    /*
     *  check the effects on the char
     */
    dead=FALSE;
    for (af = i->affected; af&&!dead; af = next_af_dude) 
{
      next_af_dude = af->next;
	if (af->duration >= 1) 
	{
		af->duration--;
	if (af->duration == 1) 
		SpellWearOffSoon(af->type, i);
       }/* >=1 end */
        else 
  {
	/* It must be a spell */	/* /----- was FIRST_BREATH_WEAPON */
	if ((af->type > 0) && (af->type < MAX_EXIST_SPELL)) 
     { 
	  if (!af->next || (af->next->type != af->type) ||
	      (af->next->duration > 0)) 
      {
            SpellWearOff(af->type, i);
	    check_memorize(i,af);
#if 0
	if (!affected_by_spell(i,SPELL_CHARM_PERSON)) /* added to fix bug */
	/* ro does not remove it here! */	
	    affect_remove(i, af);			/* msw 12/16/94 */
#endif							/* check_check_decharm */	    
      }/* end duration > 0 || */
    } /* was not a spell */ 
  else 
if (af->type>=FIRST_BREATH_WEAPON && af->type <=LAST_BREATH_WEAPON ) 
{
	 extern funcp bweapons[];
 bweapons[af->type-FIRST_BREATH_WEAPON](-af->modifier/2, i, "",SPELL_TYPE_SPELL, i, 0);
	  if (!i->affected) 
	  {
	    /* oops, you're dead :) */
	    dead = TRUE;
	    break;
	  } /* end dead */
	/* ro does not do it here either*/
	/*  affect_remove(i, af);  */
} /* end was breath */
			/* ro does it here! */
	  affect_remove(i, af);  
      } /* end on else of duration >= 1 */

 } /* end on for loop */
    if (!dead) {

      if (GET_POS(i) >= POSITION_STUNNED && (i->desc || !IS_PC(i))) {
	/* note - because of poison, this one has to be in the 
	   opposite order of the others.  The logic:

	   hit_gain() modifies the characters hps if they are poisoned.
	   but if they were in the opposite order,
	   the total would be: hps before poison + gain.  But of course,
           the hps after poison are lower, but No one cares!
	   and that is why the gain is added to the hits, not vice versa
	   */
        GET_HIT(i)  = MIN(hit_gain(i) + GET_HIT(i),  hit_limit(i));
        GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
        GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
#if 0
if (i->pc)
   GET_DIMD(i) +=2;
#endif   
        if (GET_POS(i) == POSITION_STUNNED) 
	  update_pos( i );
      } else if (GET_POS(i) == POSITION_INCAP) {
        /* do nothing */  damage(i, i, 0, TYPE_SUFFERING);
      } else if (!IS_NPC(i) && (GET_POS(i) == POSITION_MORTALLYW)) {
	damage(i, i, 1, TYPE_SUFFERING);
      }

      if (IS_PC(i))	{
        update_char_objects(i);
        if (GetMaxLevel(i) < DEMIGOD && i->in_room != 3 && i->in_room != 2)
	  check_idling(i);
        rp = real_roomp(i->in_room);
	if (rp) {
	  if (rp->sector_type == SECT_WATER_SWIM ||
	      rp->sector_type == SECT_WATER_NOSWIM) {
	    if(GET_RACE(i) == RACE_HALFLING) {
	      gain_condition(i,FULL,-2);
	      gain_condition(i,DRUNK,-2);
	    } else if(GET_RACE(i) == RACE_HALF_OGRE) {
	      gain_condition(i,FULL,-3);
	      gain_condition(i,DRUNK,-2);
	    } else {
	      gain_condition(i,FULL,-1);
	      gain_condition(i,DRUNK,-1);
	    }
	  } else if (rp->sector_type == SECT_DESERT) {
	    if(GET_RACE(i) == RACE_HALFLING) {
	      gain_condition(i,FULL,-2);
	      gain_condition(i,DRUNK,-3);
	      gain_condition(i,THIRST,-3);
	    } else if(GET_RACE(i) == RACE_HALF_OGRE) {
              gain_condition(i,FULL,-3);
              gain_condition(i,DRUNK,-2);
	      gain_condition(i,THIRST,-3);
            } else {
	      gain_condition(i,FULL,-1);
	      gain_condition(i,DRUNK,-2);
	      gain_condition(i,THIRST,-2);
	    }
	  } else if (rp->sector_type == SECT_MOUNTAIN ||
		     rp->sector_type == SECT_HILLS) {
            if(GET_RACE(i) == RACE_HALFLING) {
	      gain_condition(i,FULL,-3);
	      gain_condition(i,DRUNK,-3);
	    } else if(GET_RACE(i) == RACE_HALF_OGRE) {
              gain_condition(i,FULL,-3);
              gain_condition(i,DRUNK,-3);
            } else {
	      gain_condition(i,FULL,-2);
	      gain_condition(i,DRUNK,-2);
	    }
	  } else {
            if(GET_RACE(i) == RACE_HALFLING) {
	      gain_condition(i,FULL,-2);
	      gain_condition(i,DRUNK,-2);
	      gain_condition(i,THIRST,-2);
	    } else if(GET_RACE(i) == RACE_HALF_OGRE) {
              gain_condition(i,FULL,-3);
              gain_condition(i,DRUNK,-3);
              gain_condition(i,THIRST,-3);
            } else {
	      gain_condition(i,FULL,-1);
	      gain_condition(i,DRUNK,-1);
	      gain_condition(i,THIRST,-1);
	    }
	  }
	}
        if (i->specials.tick == time_info.hours) {/* works for 24, change for
						     anything else        */
	  if (!IS_IMMORTAL(i) && i->in_room != 3)  /* the special case for room
						      3 is a hack to keep
						      link dead people who
						      have no stuff from
						      being saved without
						      stuff...  */
  	     do_save(i,"",0);
        }
      }
      check_nature(i);  /* check falling, check drowning, etc */
    }
  }
  
  /*
   *  update the objects
   */
  for(j = object_list; j ; j = next_thing){
    next_thing = j->next; /* Next in object list */
    
    /* If this is a corpse */
    if ( (GET_ITEM_TYPE(j) == ITEM_CONTAINER) && 
	(j->obj_flags.value[3]) ) {
      /* timer count down */
      if (j->obj_flags.timer > 0) j->obj_flags.timer--;
      
      if (!j->obj_flags.timer) {
	if (j->carried_by)
      act("$p biodegrades in your hands. Everything in it falls to the floor", 
	      FALSE, j->carried_by, j, 0, TO_CHAR);
	else if ((j->in_room != NOWHERE) && 
		 (real_roomp(j->in_room)->people)){
	  act("$p dissolves into a fertile soil.",
	      TRUE, real_roomp(j->in_room)->people, j, 0, TO_ROOM);
	  act("$p dissolves into a fertile soil.",
	      TRUE, real_roomp(j->in_room)->people, j, 0, TO_CHAR);
	}	
	ObjFromCorpse(j);
      }
    } else {
      
      /*
       *  Sound objects
       */
      if (ITEM_TYPE(j) == ITEM_AUDIO) {
	if (((j->obj_flags.value[0]) && 
	     (pulse % j->obj_flags.value[0])==0) ||
	    (!number(0,5))) {
	  if (j->carried_by) {
	    room = j->carried_by->in_room;
	  } else if (j->equipped_by) {
	    room = j->equipped_by->in_room;
	  } else if (j->in_room != NOWHERE) {
	    room = j->in_room;
	  } else {
	    room = RecGetObjRoom(j);
	  }
	  /*
	   *  broadcast to room
	   */
	  
	  if (j->action_description) {	  
	    MakeNoise(room, j->action_description, j->action_description);
	  }
	}
      } else {
	if(obj_index[j->item_number].func && j->item_number>=0) {
	  (*obj_index[j->item_number].func)(0, 0, 0, j, PULSE_TICK);
	}
      }
    }    
  }
}  
#else
	/* testing affect_update() */
void affect_update( int pulse )
{
  struct descriptor_data *d;
  static struct affected_type *af, *next_af_dude;
  register struct char_data *i;
  register struct obj_data *j;
  struct obj_data *next_thing;
  struct char_data  *next_char;
  struct room_data *rp;
  int dead=FALSE, room, y, x, bottom=1, top=1, time_until_backup=1;
  long time_until_reboot;
  char buf[400];
  struct char_data *vict, *next_v;
  struct obj_data *obj, *next_o;

  for (i = character_list; i; i = next_char) {
    next_char = i->next;
    /*
     *  check the effects on the char
     */
    dead=FALSE;
    for (af = i->affected; af&&!dead; af = next_af_dude) {
      next_af_dude = af->next;
      if (af->duration >= 1)
	af->duration--;
      else {
	/* It must be a spell */
	if ((af->type > 0) && (af->type <= 120)) { /* urg.. a constant */
	  if (!af->next || (af->next->type != af->type) ||
	      (af->next->duration > 0)) {
	    if (*spell_wear_off_msg[af->type]) {
	      send_to_char(spell_wear_off_msg[af->type], i);
	      send_to_char("\n\r", i);
	    }
	    
	  }
	} else if (af->type>=FIRST_BREATH_WEAPON &&
		   af->type <=LAST_BREATH_WEAPON ) {
	  extern funcp bweapons[];
	  bweapons[af->type-FIRST_BREATH_WEAPON](-af->modifier/2, i, "",
						 SPELL_TYPE_SPELL, i, 0);
	  if (!i->affected) {
	    /* oops, you're dead :) */
	    dead = TRUE;
	    break;
	  }
	}
	
	affect_remove(i, af);
      }
    }
    if (!dead) {
      
      if (GET_POS(i) >= POSITION_STUNNED) {
        GET_HIT(i)  = MIN(GET_HIT(i)  + hit_gain(i),  hit_limit(i));
        GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
        GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
        if (GET_POS(i) == POSITION_STUNNED) 
	  update_pos( i );
      } else if (GET_POS(i) == POSITION_INCAP) {
        /* do nothing */  damage(i, i, 0, TYPE_SUFFERING);
			} else if (!IS_NPC(i) && (GET_POS(i) == POSITION_MORTALLYW)) {
			  damage(i, i, 1, TYPE_SUFFERING);
			}
      if (IS_PC(i))	{
        update_char_objects(i);
        if (GetMaxLevel(i) < DEMIGOD)
	  check_idling(i);
        rp = real_roomp(i->in_room);
	if (rp) {
	  if (rp->sector_type == SECT_WATER_SWIM ||
		   rp->sector_type == SECT_WATER_NOSWIM) {
             gain_condition(i,FULL,-1);
             gain_condition(i,DRUNK,-1);
	   } else if (rp->sector_type == SECT_DESERT) {
             gain_condition(i,FULL,-1);
             gain_condition(i,DRUNK,-2);
             gain_condition(i,THIRST,-2);
	   } else if (rp->sector_type == SECT_MOUNTAIN ||
		      rp->sector_type == SECT_HILLS) {
             gain_condition(i,FULL,-2);
             gain_condition(i,DRUNK,-2);
	   } else {
             gain_condition(i,FULL,-1);
             gain_condition(i,DRUNK,-1);
             gain_condition(i,THIRST,-1);
	   }
	}

	        
      }
    }
  }
  
  /*
   *  update the objects
   */
  
  for(j = object_list; j ; j = next_thing){
    next_thing = j->next; /* Next in object list */
    
    /* If this is a corpse */
    if ( (GET_ITEM_TYPE(j) == ITEM_CONTAINER) && 
	(j->obj_flags.value[3]) ) {
      /* timer count down */
      if (j->obj_flags.timer > 0) j->obj_flags.timer--;
      
      if (!j->obj_flags.timer) {
	if (j->carried_by)
	  act("$p biodegrades in your hands.", 
	      FALSE, j->carried_by, j, 0, TO_CHAR);
	else if ((j->in_room != NOWHERE) && 
		 (real_roomp(j->in_room)->people)){
	  act("$p dissolves into a fertile soil.",
	      TRUE, real_roomp(j->in_room)->people, j, 0, TO_ROOM);
	  act("$p dissolves into a fertile soil.",
	      TRUE, real_roomp(j->in_room)->people, j, 0, TO_CHAR);
	}	
	ObjFromCorpse(j);
      }
    } else {
      
      /*
       *  Sound objects
       */
      if (ITEM_TYPE(j) == ITEM_AUDIO) {
	if (((j->obj_flags.value[0]) && 
	     (pulse % j->obj_flags.value[0])==0) ||
	    (!number(0,5))) {
	  if (j->carried_by) {
	    room = j->carried_by->in_room;
	  } else if (j->equipped_by) {
	    room = j->equipped_by->in_room;
	  } else if (j->in_room != NOWHERE) {
	    room = j->in_room;
	  } else {
	    room = RecGetObjRoom(j);
	  }
	  /*
	   *  broadcast to room
	   */
	  
	  if (j->action_description) {	  
	    MakeNoise(room, j->action_description, j->action_description);
	  }
	}
      }
    }    
  }
}  
	
#endif



/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow(struct char_data *ch, struct char_data *victim)
{
  struct char_data *k;

  for(k=victim; k; k=k->master) {
    if (k == ch)
      return(TRUE);
  }
  
  return(FALSE);
}



/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower(struct char_data *ch)
{
  struct follow_type *j, *k;

  if (!ch->master) return;

  if (IS_AFFECTED(ch, AFF_CHARM)) {
    act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, 
	TO_NOTVICT);
    act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);

    if (affected_by_spell(ch, SPELL_CHARM_PERSON))
      affect_from_char(ch, SPELL_CHARM_PERSON);

  } else {
    act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
    if (!IS_SET(ch->specials.act,PLR_STEALTH)) {
      act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
      act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
    }
  }
  
  if (ch->master->followers->follower == ch) { /* Head of follower-list? */
    k = ch->master->followers;
    ch->master->followers = k->next;
if (k)
    free(k);
  } else { /* locate follower who is not head of list */
    
    for(k = ch->master->followers; k->next && k->next->follower!=ch; 
	k=k->next)  
      ;
    
    if (k->next) {
      j = k->next;
      k->next = j->next;
if (j)
      free(j);
    } else {
      assert(FALSE);
    }
  }
  
  ch->master = 0;
  REMOVE_BIT(ch->specials.affected_by, AFF_CHARM | AFF_GROUP);
}



/* Called when a character that follows/is followed dies */
void die_follower(struct char_data *ch)
{
  struct follow_type *j, *k;
  
  if (ch->master)
    stop_follower(ch);
  
  for (k=ch->followers; k; k=j) {
    j = k->next;
    stop_follower(k->follower);
  }
}



/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data *ch, struct char_data *leader)
{
  struct follow_type *k;
 char buf[200];  
 
#if 0
  assert(!ch->master);
#else
			/* instead of crashing the mud we try this */
if (ch->master) {
 act("$n cannot follow you for some reason.",TRUE,ch,0,leader,TO_CHAR);
 act("You cannot follow $N for some reason.",TRUE,ch,0,leader,TO_CHAR);
 sprintf(buf,"%s cannot follow %s for some reason",GET_NAME(ch),GET_NAME(leader));
 log(buf);
 return;
}
#endif
  
  ch->master = leader;
  
  CREATE(k, struct follow_type, 1);
  
  k->follower = ch;
  k->next = leader->followers;
  leader->followers = k;
  
  
  act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
  if (!IS_SET(ch->specials.act, PLR_STEALTH)) {
    act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
    act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
  }
}


struct syllable {
  char org[10];
  char new[10];
};

struct syllable syls[] = {
  { " ", " " },
  { "ar", "abra"   },
  { "au", "kada"    },
  { "bless", "fido" },
  { "blind", "nose" },
  { "bur", "mosa" },
  { "cu", "judi" },
  { "ca", "jedi" },
  { "de", "oculo"},
  { "en", "unso" },
  { "light", "dies" },
  { "lo", "hi" },
  { "mor", "zak" },
  { "move", "sido" },
  { "ness", "lacri" },
  { "ning", "illa" },
  { "per", "duda" },
  { "ra", "gru"   },
  { "re", "candus" },
  { "son", "sabru" },
  { "se",  "or"},
  { "tect", "infra" },
  { "tri", "cula" },
  { "ven", "nofo" },
  {"a", "a"},{"b","b"},{"c","q"},{"d","e"},{"e","z"},{"f","y"},{"g","o"},
  {"h", "p"},{"i","u"},{"j","y"},{"k","t"},{"l","r"},{"m","w"},{"n","i"},
  {"o", "a"},{"p","s"},{"q","d"},{"r","f"},{"s","g"},{"t","h"},{"u","j"},
  {"v", "z"},{"w","x"},{"x","n"},{"y","l"},{"z","k"}, {"",""}
};

say_spell( struct char_data *ch, int si )
{
  char buf[MAX_STRING_LENGTH], splwd[MAX_BUF_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  
  int j, offs;
  struct char_data *temp_char;
  extern struct syllable syls[];
  
  
  
  strcpy(buf, "");
  strcpy(splwd, spells[si-1]);
  
  offs = 0;
  
  while(*(splwd+offs)) {
    for(j=0; *(syls[j].org); j++)
      if (strncmp(syls[j].org, splwd+offs, strlen(syls[j].org))==0) {
	strcat(buf, syls[j].new);
	if (strlen(syls[j].org))
	  offs+=strlen(syls[j].org);
	else
	  ++offs;
      }
  }
  
  
  

sprintf(buf2,"$n utters the words, '%s'", buf);
sprintf(buf, "$n utters the words, '%s'", spells[si-1]);
  
  for(temp_char = real_roomp(ch->in_room)->people;
      temp_char;
      temp_char = temp_char->next_in_room)
    if(temp_char != ch) {
      if (GET_RACE(ch) == GET_RACE(temp_char))
	act(buf, FALSE, ch, 0, temp_char, TO_VICT);
      else
	act(buf2, FALSE, ch, 0, temp_char, TO_VICT);
      
    }
}



bool saves_spell(struct char_data *ch, sh_int save_type)
{
  int save;
  
  /* Negative apply_saving_throw makes saving throw better! */
  
  save = ch->specials.apply_saving_throw[save_type];
  
  if (!IS_NPC(ch)) {
    
    save += saving_throws[BestMagicClass(ch)][save_type][GET_LEVEL(ch,BestMagicClass(ch))];
    if (GetMaxLevel(ch) > MAX_MORT)
      return(TRUE);
  }

  if (GET_RACE(ch) == RACE_GOD)   /* gods always save */
    return(1);
  
  return(MAX(1,save) < number(1,20));
}

bool ImpSaveSpell(struct char_data *ch, sh_int save_type, int mod)
{
  int save;
  
  /* Positive mod is better for save */
  
  /* Negative apply_saving_throw makes saving throw better! */
  
  save = ch->specials.apply_saving_throw[save_type] - mod;
  
  if (!IS_NPC(ch)) {
    
    save += saving_throws[BestMagicClass(ch)][save_type]
      [GET_LEVEL(ch,BestMagicClass(ch))];
    if (GetMaxLevel(ch) >= LOW_IMMORTAL)
      return(TRUE);
  }
  
  return(MAX(1,save) < number(1,20));
}



char *skip_spaces(char *string)
{
  for(;*string && (*string)==' ';string++);

  return(string);
}



/* Assumes that *argument does start with first letter of chopped string */
void do_cast(struct char_data *ch, char *argument, int cmd)
{
  char buf[254];
  struct obj_data *tar_obj;
  struct char_data *tar_char;
  char name[MAX_INPUT_LENGTH];
  char ori_argument[256];     /* make a copy of argument for log */ 
  int qend, spl, i;
#if 0
  int  mlev, clev, dlev, minl;
#endif
  bool target_ok;
  
#if 0

/* disabled, trying out new style of casting by npc's. Use the same */
/* rules as the PC use! magic_user_imp 				    */

  if (IS_NPC(ch) && (!IS_SET(ch->specials.act, ACT_POLYSELF)))
    return;

#endif    



  if (!IsHumanoid(ch)) {
    send_to_char("Sorry, you don't have the right form for that.\n\r",ch);
    return;
  }
  
  if (!IS_IMMORTAL(ch)) {
    if (BestMagicClass(ch) == WARRIOR_LEVEL_IND) {
      send_to_char("Think you had better stick to fighting...\n\r", ch);
      return;
    } else if (BestMagicClass(ch) == THIEF_LEVEL_IND) {
      send_to_char("Think you should stick to stealing...\n\r", ch);
      return;
    } else if (BestMagicClass(ch) == MONK_LEVEL_IND) {
      send_to_char("Think you should stick to meditating...\n\r", ch);
      return;
    }
  }

 
  if (cmd!=370 && apply_soundproof(ch))
     return;

  argument = skip_spaces(argument);

/* did not want to invoke strcpy.. so added this loop to copy string [polax] */
  for(i=0;argument[i] && (i < 255);i++)  /* enforce bound check on i */
     ori_argument[i] = argument[i];
  ori_argument[i] = '\0';  /* ensure proper null termination */
/* modification end */
  
  /* If there is no chars in argument */
  if (!(*argument)) {
    send_to_char("Cast which what where?\n\r", ch);
    return;
  }
  
  if (*argument != '\'') {

    sprintf(buf,argument);
    log(buf);
    
    send_to_char("Magic must always be enclosed by the holy magic symbols : '\n\r",ch);
    return;
  }
  
  /* Locate the last quote && lowercase the magic words (if any) */
  
  for (qend=1; *(argument+qend) && (*(argument+qend) != '\'') ; qend++)
    *(argument+qend) = LOWER(*(argument+qend));
  
  if (*(argument+qend) != '\'') {
    send_to_char("Magic must always be enclosed by the holy magic symbols : '\n\r",ch);
    return;
  }
  
  spl = old_search_block(argument, 1, qend-1,spells, 0);
  
  if (!spl) {
    send_to_char("Nothing seems to happen ! WOW! \n\r",ch);
    return;
  }


	/* mobs do not get  skills so we just check it for PC's */
	
  if (!ch->skills)
   if (IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF)) {
     send_to_char("You do not have skills!\n\r",ch);
     return;
    }


 if (cmd != 370 && OnlyClass(ch, CLASS_PSI)) {
 	send_to_char("Use your mind!\n\r",ch);
 	return;
 	}

if ((cmd == 84 || cmd == 370) && OnlyClass(ch,CLASS_SORCERER)) {
	send_to_char("You must use recall.\n\r",ch);
	return;
	}

#if 1
/* this should make sorcerer learned spells be forced to be recalled */
if ((cmd == 84 || cmd == 370) && HasClass(ch,CLASS_SORCERER) && 
    !IS_IMMORTAL(ch) && IS_SET(ch->skills[spl].flags,SKILL_KNOWN_SORCERER) ) {
   send_to_char("You must use recall for this spell.\n\r",ch); 
   return;
  }
#endif

  if ((spl > 0) && (spl < MAX_SKILLS) && spell_info[spl].spell_pointer) {
    if (GET_POS(ch) < spell_info[spl].minimum_position) {
      switch(GET_POS(ch)) {
      case POSITION_SLEEPING :
	send_to_char("You dream about great magical powers.\n\r", ch);
	break;
      case POSITION_RESTING :
	send_to_char("You can't concentrate enough while resting.\n\r",ch);
	break;
      case POSITION_SITTING :
	send_to_char("You can't do this sitting!\n\r", ch);
	break;
      case POSITION_FIGHTING :
	send_to_char("Impossible! You can't concentrate enough!.\n\r", ch);
	break;
      default:
	send_to_char("It seems like you're in pretty bad shape!\n\r",ch);
	break;
      } /* Switch */
    }	else {
      
      if (!IS_IMMORTAL(ch)) {

	if ((spell_info[spl].min_level_magic > 
	     GET_LEVEL(ch,MAGE_LEVEL_IND)) &&
	     
           (spell_info[spl].min_level_sorcerer > 
	     GET_LEVEL(ch,SORCERER_LEVEL_IND)) &&	     
	     
	    (spell_info[spl].min_level_cleric >
	     GET_LEVEL(ch,CLERIC_LEVEL_IND)) &&
	     
	    (spell_info[spl].min_level_paladin >
	     GET_LEVEL(ch,PALADIN_LEVEL_IND)) &&
	     
	    (spell_info[spl].min_level_ranger >
	     GET_LEVEL(ch,RANGER_LEVEL_IND)) &&	     

	    (spell_info[spl].min_level_psi >
	     GET_LEVEL(ch,PSI_LEVEL_IND)) &&	     

	    (spell_info[spl].min_level_druid >
	     GET_LEVEL(ch, DRUID_LEVEL_IND))) {
	  send_to_char("Sorry, you can't do that.\n\r", ch);
	  return;
	}
      }
      argument+=qend+1;	/* Point to the last ' */
      for(;*argument == ' '; argument++);
      
      /* **************** Locate targets **************** */
      
      target_ok = FALSE;
      tar_char = 0;
      tar_obj = 0;
      
      if (IS_SET(spell_info[spl].targets, TAR_VIOLENT) &&
	  check_peaceful(ch, 
	  "This seems to be an dead magic zone!\n\r"))
	return;

   /* for seeing what the other guys are doing test */
   
if (IS_IMMORTAL(ch) && IS_PC(ch) && GetMaxLevel(ch)<59) {
     sprintf(buf,"%s cast %s",GET_NAME(ch),ori_argument);  /* changed argument */
     log(buf);
  }

      if (!IS_SET(spell_info[spl].targets, TAR_IGNORE)) {
	
	argument = one_argument(argument, name);
	
	if (*name) {
	  if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM)) {
	    if (tar_char = get_char_room_vis(ch, name)) {
	      if (tar_char == ch || tar_char == ch->specials.fighting ||
		  tar_char->attackers < 6 || 
		  tar_char->specials.fighting == ch)
		target_ok = TRUE;
	      else {
		send_to_char("Too much fighting, you can't get a clear shot.\n\r", ch);
		target_ok = FALSE;
	      }
	    } else {
	      target_ok = FALSE;
	    }
	  }
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
	    if (tar_char = get_char_vis(ch, name))
	      target_ok = TRUE;
	  
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
	    if (tar_obj = get_obj_in_list_vis(ch, name, ch->carrying))
	      target_ok = TRUE;
	  
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
	    if (tar_obj = get_obj_in_list_vis(ch, name, real_roomp(ch->in_room)->contents))
	      target_ok = TRUE;
	  
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
	    if (tar_obj = get_obj_vis(ch, name))
	      target_ok = TRUE;
	  
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP)) {
	    for(i=0; i<MAX_WEAR && !target_ok; i++)
	      if (ch->equipment[i] && str_cmp(name, ch->equipment[i]->name) == 0) {
		tar_obj = ch->equipment[i];
		target_ok = TRUE;
	      }
	  }
	  
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY))
	    if (str_cmp(GET_NAME(ch), name) == 0) {
	      tar_char = ch;
	      target_ok = TRUE;
	    }
	  
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_NAME)) {
	    tar_obj = (void*)name;
	    target_ok = TRUE;
	  }
	  
	  if (tar_char) {
	    if (IS_NPC(tar_char)) 
	      if (IS_SET(tar_char->specials.act, ACT_IMMORTAL)) {
		send_to_char("You can't cast magic on that!",ch);
		return;
	      }
	  }
	  
	  
	} else { /* No argument was typed */
	  
	  if (IS_SET(spell_info[spl].targets, TAR_FIGHT_SELF))
	    if (ch->specials.fighting) {
	      tar_char = ch;
	      target_ok = TRUE;
	    }
	  
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_FIGHT_VICT))
	    if (ch->specials.fighting) {
	      /* WARNING, MAKE INTO POINTER */
	      tar_char = ch->specials.fighting;
	      target_ok = TRUE;
	    }
	  
	  if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
	    tar_char = ch;
	    target_ok = TRUE;
	  }
	  
	}
	
      } else {
	target_ok = TRUE; /* No target, is a good target */
      }
      
      if (!target_ok) {
	if (*name) {
	  if (IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
	    send_to_char("Nobody playing by that name.\n\r", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
	    send_to_char("Nobody here by that name.\n\r", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
	    send_to_char("You are not carrying anything like that.\n\r", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
	    send_to_char("Nothing here by that name.\n\r", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
	    send_to_char("Nothing at all by that name.\n\r", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP))
	    send_to_char("You are not wearing anything like that.\n\r", ch);
	  else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
	    send_to_char("Nothing at all by that name.\n\r", ch);
	  
	} else { /* Nothing was given as argument */
	  if (spell_info[spl].targets < TAR_OBJ_INV)
	    send_to_char("Who should the spell be cast upon?\n\r", ch);
	  else
	    send_to_char("What should the spell be cast upon?\n\r", ch);
	}
	return;
      } else { /* TARGET IS OK */
	if ((tar_char == ch) && IS_SET(spell_info[spl].targets, TAR_SELF_NONO)) {
	  send_to_char("You can not cast this spell upon yourself.\n\r", ch);
	  return;
	}
	else if ((tar_char != ch) && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
	  send_to_char("You can only cast this spell upon yourself.\n\r", ch);
	  return;
	} else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tar_char)) {
	  send_to_char("You are afraid that it could harm your master.\n\r", ch);
	  return;
	}
      }

  
	

      if (cmd == 283) 
      { /* recall */
	if (!MEMORIZED(ch, spl)) 
	{
	  send_to_char("You don't have that spell memorized!\n\r", ch);
	  return;
	}
      } else {      
	if (GetMaxLevel(ch) < LOW_IMMORTAL) {
	  if (GET_MANA(ch) < (unsigned int)USE_MANA(ch, (int)spl) || 
	      GET_MANA(ch) <=0) {
	    send_to_char("You can't summon enough energy!\n\r", ch);
	    return;
	  }
	}
      }

      if (spl != SPELL_VENTRILOQUATE && cmd != 370)  /* :-) */
	say_spell(ch, spl);		/* psi's do not utter! */
      
      WAIT_STATE(ch, spell_info[spl].beats);
      
      if ((spell_info[spl].spell_pointer == 0) && spl>0)
	send_to_char("Sorry, this magic has not yet been implemented :(\n\r", 
		     ch);
      else {
	int max, cost;

	max = ch->specials.spellfail;
	max += GET_COND(ch, DRUNK)*10; /* 0 - 240 */

/* check EQ and alter spellfail accordingly */
 if (!OnlyClass(ch,CLASS_MAGIC_USER&CLASS_SORCERER&CLASS_CLERIC&CLASS_DRUID)) {
	switch(BestMagicClass(ch)) {
		case	MAGE_LEVEL_IND:if (EqWBits(ch,ITEM_ANTI_MAGE))
					max+=10; /* 20% harder to cast spells */
					break;		
		case	SORCERER_LEVEL_IND:if (EqWBits(ch,ITEM_ANTI_MAGE))
					max+=10; /* 20% harder to cast spells */
					break;		
		case	CLERIC_LEVEL_IND:if (EqWBits(ch,ITEM_ANTI_CLERIC))
					max+=10; /* 20% harder to cast spells */
					break;		
		case	DRUID_LEVEL_IND:if (EqWBits(ch,ITEM_ANTI_DRUID))
					max+=10; /* 20% harder to cast spells */
					break;		
		case	PALADIN_LEVEL_IND:if (EqWBits(ch,ITEM_ANTI_PALADIN))
					max+=10; /* 20% harder to cast spells */
					break; 
		case 	PSI_LEVEL_IND:if (EqWBits(ch,ITEM_ANTI_PSI))
					max+=10; /* 20% harder to cast spells */
					break;		
		case 	RANGER_LEVEL_IND:if (EqWBits(ch,ITEM_ANTI_RANGER))
					max+=10; /* 20% harder to cast spells */
					break;		
		default:if (EqWBits(ch,ITEM_ANTI_MAGE))
			max+=10; /* 20% harder to cast spells */
			break;
		} /* end switch */

  }
/* end EQ check				    */
	if (ch->attackers > 0)
	  max += spell_info[spl].spellfail;
	else if (ch->specials.fighting)
	  max += spell_info[spl].spellfail/2;

if (number(1,max) > ch->skills[spl].learned && 
     !IsSpecialized(ch->skills[spl].special))      {
	  send_to_char("You lost your concentration!\n\r", ch); 
	  cost = (int)USE_MANA(ch, (int)spl);
       if (cmd == 283)  {
       if (number(1,101) >=ch->skills[spl].learned)/* give chance to forgett */
              FORGET(ch, spl); 
        } else
	      GET_MANA(ch) -= (cost>>1); 
	      
	  LearnFromMistake(ch, spl, 0, 95);
	  return;
     }
     
        if (tar_char) {

	 if (affected_by_spell(tar_char,SPELL_ANTI_MAGIC_SHELL) &&
	/* psi shit ain't magic */
	     spell_info[spl].min_level_psi == 0) {
	    act("Your magic fizzles against $N's anti-magic shell!",FALSE,ch,0,tar_char,TO_CHAR);
	    act("$n wastes a spell on $N's anti-magic shell!",FALSE,ch,0,tar_char,TO_ROOM);
	    act("$n casts a spell and growls as it fizzles against your anti-magic shell!",FALSE,ch,0,tar_char,TO_VICT);
	    return;
	   }

	  if (GET_POS(tar_char) == POSITION_DEAD) {
	    send_to_char("The magic fizzles against the dead body.\n", ch);
	    return;
	  }
	}

if (affected_by_spell(ch,SPELL_ANTI_MAGIC_SHELL) &&
	/* psi shit ain't magic */
	     spell_info[spl].min_level_psi == 0) {
	act("Your magic fizzles against your anti-magic shell!",FALSE,ch,0,0,TO_CHAR);
	act("$n tries to cast a spell within a anti-magic shell, muhahaha!",FALSE,ch,0,0,TO_ROOM);
	return;
	}

	if (check_nomagic(ch, 
	     "Your skill appears useless in this magic dead zone.",
	    "$n's spell dissolves like so much wet toilet paper"))
	  return;

#if 0
	mlev = spell_info[spl].min_level_magic;
	clev = spell_info[spl].min_level_cleric;
	dlev = spell_info[spl].min_level_druid;

	minl = 0;
	if (HasClass(ch, CLASS_MAGIC_USER)) {
	  if (!EqWBits(ch, ITEM_ANTI_MAGE))
	    minl = 
	}
#endif
	send_to_char("Ok.\n\r",ch);
	((*spell_info[spl].spell_pointer) (GET_LEVEL(ch, BestMagicClass(ch)), ch, argument, SPELL_TYPE_SPELL, tar_char, tar_obj));
	cost = (int)USE_MANA(ch, (int)spl);
	if (cmd == 283) /* recall */ {
	  FORGET(ch, spl);
	} else {
	  GET_MANA(ch) -= cost;
	}
      }
      
    }	/* if GET_POS < min_pos */
    
    return;
  }
  
  switch (number(1,5)){
  case 1: send_to_char("Bylle Grylle Grop Gryf???\n\r", ch); break;
  case 2: send_to_char("Olle Bolle Snop Snyf?\n\r",ch); break;
  case 3: send_to_char("Olle Grylle Bolle Bylle?!?\n\r",ch); break;
  case 4: send_to_char("Gryffe Olle Gnyffe Snop???\n\r",ch); break;
  default: send_to_char("Bolle Snylle Gryf Bylle?!!?\n\r",ch); break;
  }
}


void assign_spell_pointers()
{
  int i;
  
					/* make sure defaults are 0! msw */
  for(i=0; i<MAX_SPL_LIST; i++) {
     spell_info[i].spell_pointer = 0;
     spell_info[i].min_level_cleric   = 0;
     spell_info[i].min_level_magic    = 0;
     spell_info[i].min_level_druid    = 0;
     spell_info[i].min_level_sorcerer = 0;
     spell_info[i].min_level_paladin  = 0;
     spell_info[i].min_level_ranger   = 0;
     spell_info[i].min_level_psi      = 0;
    }
  
  
  /* From spells1.c */

  /* mage only 	    */   
  
spello(32,12,POSITION_FIGHTING, 1, LOW_IMMORTAL, LOW_IMMORTAL,  1,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  15,TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_magic_missile,10);  

  spello( 8,12,POSITION_FIGHTING, 4, LOW_IMMORTAL,  LOW_IMMORTAL, 
  4,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL, 
  15,TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_chill_touch,10);
  
  spello( 5,24,POSITION_FIGHTING, 6, LOW_IMMORTAL,  LOW_IMMORTAL, 6,
  LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 15, TAR_IGNORE | TAR_VIOLENT,
  cast_burning_hands,5);
  
  spello(37,12,POSITION_FIGHTING, 2, LOW_IMMORTAL,  LOW_IMMORTAL, 
  2,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL, 
  15,	 TAR_CHAR_ROOM|TAR_FIGHT_VICT|TAR_VIOLENT, cast_shocking_grasp,0);

  
  spello( 30, 24, POSITION_FIGHTING, 
  10,  LOW_IMMORTAL,  LOW_IMMORTAL, 
  10,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL, 
  15,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,cast_lightning_bolt,20);

  spello(10,24,POSITION_FIGHTING, 14,LOW_IMMORTAL, LOW_IMMORTAL, 
  14,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL, 
  20, 	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_colour_spray,40);

  spello(26,36,POSITION_FIGHTING, 25, LOW_IMMORTAL,  LOW_IMMORTAL, 
  25,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL, 
  50,	 TAR_IGNORE | TAR_VIOLENT, cast_fireball,25);

/* cleric */
  spello(23,24,POSITION_FIGHTING, LOW_IMMORTAL, 15, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL, 
  15, TAR_IGNORE | TAR_VIOLENT, cast_earthquake,30);
    
  spello(22,24,POSITION_FIGHTING, LOW_IMMORTAL, 20,  LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL, 
  15,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_dispel_evil, 30);
  
  
  spello( 6,36,POSITION_FIGHTING, LOW_IMMORTAL, 45, 18, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL, 
  20,	 TAR_CHAR_ROOM|TAR_FIGHT_VICT|TAR_VIOLENT, cast_call_lightning, 50);
  
  spello(25,36,POSITION_FIGHTING, 22, LOW_IMMORTAL, LOW_IMMORTAL, 
  22,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL, 
  35, 	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_energy_drain, 100);
  
  spello(27,36,POSITION_FIGHTING, LOW_IMMORTAL, 25,  LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL, 
  35,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_harm, 510);
  
  
  /* Spells2.c */
  
  spello( 1,12,POSITION_STANDING, 
   5,  2, LOW_IMMORTAL,
   5,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL, 
   5,	 TAR_CHAR_ROOM, cast_armor, 0);
  
  spello( 2,12,POSITION_STANDING, 17, LOW_IMMORTAL,  LOW_IMMORTAL, 
  17,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL, 
  33,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_teleport, 60);
  
  spello( 3,12,POSITION_STANDING,LOW_IMMORTAL,  1,  LOW_IMMORTAL, 
   17,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL, 
  5,	 TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_CHAR_ROOM, cast_bless, 0);
  
  spello( 4,24,POSITION_FIGHTING, 12,  14,  LOW_IMMORTAL, 
   12,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL, 
  15,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_blindness, 60);
  
  spello(7,12,POSITION_STANDING, 4, LOW_IMMORTAL,  IMMORTAL, 
   4,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL, 
  10,	 TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT,
	 cast_charm_person, 0);
  
  /* */	spello( 9,12,POSITION_STANDING, BIG_GUY+1, BIG_GUY+1, BIG_GUY+1,	
   BIG_GUY+1,  BIG_GUY+1,  BIG_GUY+1,  BIG_GUY+1, 
  40, TAR_CHAR_ROOM, cast_clone, 0);
  
  spello(11,36,POSITION_STANDING, LOW_IMMORTAL, 26, 15, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  25,	 TAR_IGNORE, cast_control_weather,0);
  
  spello(12,12,POSITION_STANDING,LOW_IMMORTAL,  5, IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  5,	 TAR_IGNORE, cast_create_food,0); 
  
  spello(13,12,POSITION_STANDING,LOW_IMMORTAL,  2, IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  5,	 TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_create_water,0);
  
  spello(14,12,POSITION_STANDING,LOW_IMMORTAL,  6, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  5,	 TAR_CHAR_ROOM, cast_cure_blind,0);
  
  spello(15,24,POSITION_FIGHTING,LOW_IMMORTAL,  10, 13, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  11,	 TAR_CHAR_ROOM, cast_cure_critic, 30);
  
  spello(16,12,POSITION_FIGHTING,LOW_IMMORTAL,  1, 2, 
  LOW_IMMORTAL,  9,  LOW_IMMORTAL,  LOW_IMMORTAL,
  5,	 TAR_CHAR_ROOM, cast_cure_light, 10);
  
  spello(17,24,POSITION_STANDING,13, 12, LOW_IMMORTAL, 
  13,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  20,	 TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_FIGHT_VICT | TAR_VIOLENT, cast_curse, 5);
  
  spello(18,12,POSITION_STANDING, LOW_IMMORTAL, 1, 6, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  5,	 TAR_CHAR_ROOM, cast_detect_evil,0);
  
  spello(19,12,POSITION_STANDING, 2,  5, 7, 
  2,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  5,	 TAR_CHAR_ROOM, cast_detect_invisibility,0);
  
  spello(20,12,POSITION_STANDING, 1,  3, 5, 
  1,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  5,	 TAR_CHAR_ROOM, cast_detect_magic,0);
  
  spello(21,12,POSITION_STANDING,LOW_IMMORTAL,  LOW_IMMORTAL, 1, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  3,  LOW_IMMORTAL,
  5,	 TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_detect_poison,0);
  
  spello(24,48,POSITION_STANDING,14, LOW_IMMORTAL, LOW_IMMORTAL, 
  14,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  100,	 TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_enchant_weapon,0);
  
  spello(28,12,POSITION_FIGHTING,LOW_IMMORTAL, 25, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  50,	 TAR_CHAR_ROOM, cast_heal,310);
  
  spello(29,12,POSITION_STANDING, 4, LOW_IMMORTAL, LOW_IMMORTAL, 
  4,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  5,	 TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP, cast_invisibility, 0);
  
  spello(31,12,POSITION_STANDING, LOW_IMMORTAL, 11, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  20,	 TAR_OBJ_WORLD, cast_locate_object, 0);
  
  spello(33,24,POSITION_FIGHTING,LOW_IMMORTAL,  13, 8, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  10,	 TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_FIGHT_VICT | TAR_VIOLENT, cast_poison, 60);
  
  spello(34,12,POSITION_STANDING,LOW_IMMORTAL,  7, LOW_IMMORTAL, 
  LOW_IMMORTAL,  20,  25,  LOW_IMMORTAL,
  5,	 TAR_CHAR_ROOM, cast_protection_from_evil, 0);
  
  spello(35,12,POSITION_STANDING,LOW_IMMORTAL, 8, LOW_IMMORTAL, 
   LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  5,	 TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM, cast_remove_curse, 0);
  
  spello(36,36,POSITION_STANDING, LOW_IMMORTAL, 26, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  50,	 TAR_CHAR_ROOM, cast_sanctuary, 0);
  
  spello(38,24,POSITION_STANDING, 3, LOW_IMMORTAL, LOW_IMMORTAL, 
  3,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  15,	 TAR_CHAR_ROOM | TAR_VIOLENT |TAR_FIGHT_VICT, cast_sleep, 0);
  
  spello(39,12,POSITION_STANDING, 6, LOW_IMMORTAL, LOW_IMMORTAL, 
  6,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  10,	 TAR_CHAR_ROOM, cast_strength, 0);
  
  spello(40,36,POSITION_STANDING, 27,  19, LOW_IMMORTAL, 
  27,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  20,	 TAR_CHAR_WORLD | TAR_VIOLENT, cast_summon, 0);
  
  /* */	
  spello(41,12,POSITION_STANDING, 1, LOW_IMMORTAL, LOW_IMMORTAL, 
  1,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  5,	 TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_SELF_NONO, cast_ventriloquate, 0);
  
  spello(42,12,POSITION_STANDING,LOW_IMMORTAL, 15, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  5,	 TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_word_of_recall, 0);
  
  spello(43,12,POSITION_STANDING,LOW_IMMORTAL,  17, 8,
  LOW_IMMORTAL,  40,  20,  LOW_IMMORTAL,
   5,	 TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, cast_remove_poison,0);
  
  spello(44,12,POSITION_STANDING,LOW_IMMORTAL,  4, LOW_IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
   5,	 TAR_CHAR_ROOM, cast_sense_life,0);
  
  spello(47,0,POSITION_STANDING,BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,
 BIG_GUY+1,  BIG_GUY+1,  BIG_GUY+1,  BIG_GUY+1, 
  200,	 TAR_IGNORE, 0, 0);
  
  spello(48,0,POSITION_STANDING,BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,
 BIG_GUY+1,  BIG_GUY+1,  BIG_GUY+1,  BIG_GUY+1, 
  200,	 TAR_IGNORE, 0, 0);
  
  spello(49,0,POSITION_STANDING,BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,
 BIG_GUY+1,  BIG_GUY+1,  BIG_GUY+1,  BIG_GUY+1, 
  200,	 TAR_IGNORE, 0, 0);
  
  spello(50,0,POSITION_STANDING,BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,
 BIG_GUY+1,  BIG_GUY+1,  BIG_GUY+1,  BIG_GUY+1, 
  200,	 TAR_IGNORE, 0, 0);
  
  
  spello(52,0,POSITION_STANDING,BIG_GUY+1,BIG_GUY+1,BIG_GUY+1, 
 BIG_GUY+1,  BIG_GUY+1,  BIG_GUY+1,  BIG_GUY+1, 
  200,	 TAR_IGNORE, 0, 0);
  
  spello(53,1,POSITION_STANDING,45,IMMORTAL, IMMORTAL, 
  45,  IMMORTAL,  IMMORTAL,  IMMORTAL,
  100, 	 TAR_OBJ_ROOM | TAR_OBJ_INV, cast_identify, 0);
  
  spello(54,12,POSITION_STANDING, 8, LOW_IMMORTAL, 5,  
  8,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  7,	 TAR_CHAR_ROOM, cast_infravision, 0); 
  
  spello(55,12,POSITION_FIGHTING, LOW_IMMORTAL,  1, 2,  
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  8,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_cause_light, 10);
  
  spello(56,24,POSITION_FIGHTING, LOW_IMMORTAL,  10,  13, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  11,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT , cast_cause_critic, 20);
  
  spello(57,36,POSITION_FIGHTING, LOW_IMMORTAL , 15, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  15,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_flamestrike, 50);
  
  spello(58,36,POSITION_FIGHTING, LOW_IMMORTAL, 20, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  15,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_dispel_good, 50);
  
  spello(59,12,POSITION_FIGHTING, 6, LOW_IMMORTAL, LOW_IMMORTAL, 
  6,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  10,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_weakness, 30);
  
  spello(60,12,POSITION_FIGHTING, 9, 10, 9, 
  9,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  15,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_OBJ_ROOM | TAR_OBJ_INV |
	 TAR_VIOLENT , cast_dispel_magic, 50);
  
  spello(61,12,POSITION_STANDING, 3, LOW_IMMORTAL, LOW_IMMORTAL, 
  3,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  10,	 TAR_IGNORE, cast_knock, 0);
  
  spello(62,12,POSITION_STANDING, 7, 4, 2, 
  7,  5,  LOW_IMMORTAL,  LOW_IMMORTAL,
  10,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_know_alignment, 0);
  
  spello(63,12,POSITION_STANDING, 16, 8, LOW_IMMORTAL,
   16,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
   5,	 TAR_OBJ_ROOM, cast_animate_dead, 0);
  
  spello(64,36,POSITION_FIGHTING, 20, LOW_IMMORTAL, LOW_IMMORTAL, 
  20,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  15,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_paralyze, 90);
  
  spello(65,12,POSITION_FIGHTING, LOW_IMMORTAL, 6, 9 , 
  LOW_IMMORTAL,  15,  LOW_IMMORTAL,  LOW_IMMORTAL,
  10,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_remove_paralysis, 30);
  
  spello( 66, 12, POSITION_FIGHTING, 8, LOW_IMMORTAL, LOW_IMMORTAL, 
  8,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  15,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_fear, 40);
  
  spello(67,24,POSITION_FIGHTING, 6, LOW_IMMORTAL, LOW_IMMORTAL, 
  6,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  15,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_acid_blast, 20);
  
  spello(68,12,POSITION_STANDING, 9, LOW_IMMORTAL, 6, 
  9,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  15,	 TAR_CHAR_ROOM, cast_water_breath, 0);
  
  spello(69,12,POSITION_STANDING, 11, 22, 14, 
  11,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  15,	 TAR_CHAR_ROOM, cast_flying, 0);
  
  spello(70,24,POSITION_FIGHTING, 17, LOW_IMMORTAL, LOW_IMMORTAL, 
  17,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  15,	 TAR_IGNORE | TAR_VIOLENT,  cast_cone_of_cold, 40);
  
  spello(71,24,POSITION_FIGHTING, 36, LOW_IMMORTAL, LOW_IMMORTAL, 
  36,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  35,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_meteor_swarm, 80);
  
  spello(72,12,POSITION_FIGHTING, 11, LOW_IMMORTAL, LOW_IMMORTAL, 
  11,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  15,	 TAR_IGNORE | TAR_VIOLENT,  cast_ice_storm, 20);
  
  spello(73,24,POSITION_FIGHTING, 1, LOW_IMMORTAL, LOW_IMMORTAL, 
  1,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  1,	 TAR_CHAR_ROOM, cast_shield, 15);
  
  spello(74,24,POSITION_STANDING, 4, LOW_IMMORTAL, LOW_IMMORTAL, 
  4,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  10,	 TAR_IGNORE, cast_mon_sum1,0);
  
  spello(75,24,POSITION_STANDING, 7, LOW_IMMORTAL, LOW_IMMORTAL, 
  7,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  12,	 TAR_IGNORE, cast_mon_sum2,0);
  
  spello(76,24,POSITION_STANDING, 9, LOW_IMMORTAL, LOW_IMMORTAL, 
  9,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  15,	 TAR_IGNORE, cast_mon_sum3,0);
  
  spello(77,24,POSITION_STANDING, 12, LOW_IMMORTAL, LOW_IMMORTAL, 
  12,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  17,	 TAR_IGNORE, cast_mon_sum4,0);
  
  spello(78,24,POSITION_STANDING, 15, LOW_IMMORTAL, LOW_IMMORTAL, 
  15,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  20,	 TAR_IGNORE, cast_mon_sum5,0);
  
  spello(79,24,POSITION_STANDING, 18, LOW_IMMORTAL, LOW_IMMORTAL, 
  18,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  22,	 TAR_IGNORE, cast_mon_sum6,0);
  
  spello(80,24,POSITION_STANDING, 22, LOW_IMMORTAL, LOW_IMMORTAL, 
  22,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  25,	 TAR_IGNORE, cast_mon_sum7,0);
  
  spello(81,24,POSITION_STANDING, 40, 45, 48, 
  40,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  40,	 TAR_SELF_ONLY | TAR_CHAR_ROOM, cast_fireshield,0);
  
  spello(82,12,POSITION_STANDING, 10, LOW_IMMORTAL, 12, 
  10,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  5,	 TAR_CHAR_ROOM | TAR_VIOLENT, cast_charm_monster,0);
  
  spello(83,12,POSITION_FIGHTING, LOW_IMMORTAL, 7, 8, 
  45, 50,  LOW_IMMORTAL,  LOW_IMMORTAL,
  9,	 TAR_CHAR_ROOM, cast_cure_serious, 14);
  
  spello(84,12,POSITION_FIGHTING, LOW_IMMORTAL, 7, 8,
  45,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  9,	 TAR_CHAR_ROOM | TAR_VIOLENT, cast_cause_serious, 14);
  
  spello(85,12,POSITION_STANDING, 6, 3, 4, 
  6,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  5,	 TAR_CHAR_ROOM, cast_refresh,0);
  
  spello(86,12,POSITION_FIGHTING, 20, 9, 14, 
  20,  35,  30,  LOW_IMMORTAL,
  5,	 TAR_CHAR_ROOM, cast_second_wind,25);
  
  spello(87,12,POSITION_STANDING, LOW_IMMORTAL, 1, 12, 
  LOW_IMMORTAL,  10,  LOW_IMMORTAL,  LOW_IMMORTAL,
  5,	 TAR_CHAR_ROOM, cast_turn,0);
  
  spello(88,24,POSITION_STANDING, 24, 23, LOW_IMMORTAL, 
  24,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  15,	 TAR_IGNORE, cast_succor,0);
  
  spello(89,12,POSITION_STANDING, 1, 2, 3,  
  1,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  5,	 TAR_IGNORE, cast_light,0);
  
  spello(90,24,POSITION_STANDING, 10, 26, 16, 
  10,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  10,	 TAR_IGNORE, cast_cont_light,0);
  
  spello(91,24,POSITION_STANDING, 4, 2, LOW_IMMORTAL, 
  4,  18,  LOW_IMMORTAL,  LOW_IMMORTAL,
  15,	 TAR_CHAR_ROOM, cast_calm,0);
  
  spello(92,24,POSITION_STANDING, 26, LOW_IMMORTAL, LOW_IMMORTAL, 
  26,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  20,	 TAR_SELF_ONLY, cast_stone_skin,0);
  
  spello(93,24,POSITION_STANDING, 16, 13, 11, 
  16,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  30,	 TAR_IGNORE, cast_conjure_elemental,0);
  
  spello(94,24,POSITION_STANDING, LOW_IMMORTAL, 19, 24, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  20,	 TAR_CHAR_ROOM, cast_true_seeing,0);
  
  spello(95,24,POSITION_STANDING, 8, LOW_IMMORTAL, LOW_IMMORTAL, 
  8,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  30,	 TAR_IGNORE, cast_minor_creation,0);
  
  spello(96,12,POSITION_STANDING, 5, 4, 1, 
  5,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  10,	 TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT, cast_faerie_fire,0);
  
  spello(97,24,POSITION_STANDING, 16, 11, 10, 
  16,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  20,	 TAR_IGNORE, cast_faerie_fog,0);
  
  spello(98,24,POSITION_STANDING, 30, 29, LOW_IMMORTAL, 
  30,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  50,	 TAR_IGNORE, cast_cacaodemon,0);
  

  spello(99,12,POSITION_STANDING, 9, LOW_IMMORTAL, LOW_IMMORTAL, 
  9,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  10,	 TAR_IGNORE, cast_poly_self,0);
    
  spello(100,12,POSITION_FIGHTING, BIG_GUY+1, BIG_GUY+1, BIG_GUY+1,
   BIG_GUY+1,  BIG_GUY+1,  BIG_GUY+1,  BIG_GUY+1, 
  200, TAR_IGNORE, cast_mana,0);
  
  spello( 101,12,POSITION_STANDING, LOW_IMMORTAL, 30, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  30,	 TAR_CHAR_WORLD, cast_astral_walk,0);
  
  spello( 102,36,POSITION_STANDING, LOW_IMMORTAL, 36, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
 33,	 TAR_OBJ_ROOM, cast_resurrection,0);

  spello( 103,12,POSITION_STANDING, LOW_IMMORTAL, 24, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  40,	 TAR_IGNORE, cast_heroes_feast,0);

  spello( 104,12,POSITION_STANDING, 24, LOW_IMMORTAL, 22, 
  24,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  30,	 TAR_IGNORE, cast_fly_group,0);

  spello(105, 250, POSITION_STANDING, BIG_GUY+1, BIG_GUY+1,BIG_GUY+1, 
  BIG_GUY+1,  BIG_GUY+1,  BIG_GUY+1,  BIG_GUY+1, 
  200, TAR_IGNORE | TAR_VIOLENT, cast_dragon_breath,0);
 

  spello( 106,12,POSITION_FIGHTING, 11, LOW_IMMORTAL, LOW_IMMORTAL, 
  11,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  3,	 TAR_CHAR_ROOM, cast_web,60);

  spello( 107,12,POSITION_STANDING, 12, LOW_IMMORTAL, 7, 
  12,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  10,	 TAR_CHAR_ROOM, cast_minor_track,0);

  spello( 108,12,POSITION_STANDING, 20, LOW_IMMORTAL, 17, 
  20,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  20,	 TAR_CHAR_ROOM, cast_major_track,0);

/* new stuff */

  spello(109,24,POSITION_STANDING, IMMORTAL, 15, IMMORTAL, 
  IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  30,	 TAR_IGNORE, cast_golem,0);

  spello(110,24,POSITION_STANDING, 2, IMMORTAL, IMMORTAL, 
  2,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  30,	 TAR_IGNORE, cast_familiar,0);

  spello(111,24,POSITION_STANDING, LOW_IMMORTAL, IMMORTAL, 30, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  30,	 TAR_IGNORE, cast_changestaff,0);

  spello(112,24,POSITION_FIGHTING, IMMORTAL, IMMORTAL, IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  30,	 TAR_IGNORE | TAR_VIOLENT,  cast_holyword,80);

  spello(113,24,POSITION_FIGHTING, IMMORTAL, IMMORTAL, IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  30,	 TAR_IGNORE | TAR_VIOLENT, cast_unholyword,80);

  spello(114,24,POSITION_FIGHTING, 23, IMMORTAL, IMMORTAL, 
  23,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  10,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_pword_kill,40);

  spello(115,24,POSITION_FIGHTING, 16, IMMORTAL, IMMORTAL, 
  16,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  10,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_pword_blind,20);

  spello(116,24,POSITION_FIGHTING, 25, IMMORTAL, 25, 
  25,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  30,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_chain_lightn,80);

  spello(117,24,POSITION_FIGHTING, 4, IMMORTAL, IMMORTAL, 
  4,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  3,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_scare,10);

  spello(118,24,POSITION_FIGHTING, LOW_IMMORTAL, 4, IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
   5,	 TAR_CHAR_ROOM, cast_aid,10);

  spello(119,24,POSITION_FIGHTING, LOW_IMMORTAL, 1, IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  3,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_command,30);

  spello(120, 12, POSITION_STANDING, IMMORTAL, IMMORTAL, 12, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  20,	 TAR_IGNORE, cast_change_form, 0);

  spello(121,24,POSITION_FIGHTING, 34, IMMORTAL, IMMORTAL, 
  34,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  30,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT |TAR_VIOLENT,cast_feeblemind,70);

  spello(122,12,POSITION_STANDING, LOW_IMMORTAL, IMMORTAL, 3, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  10,	 TAR_OBJ_INV, cast_shillelagh, 0);

  spello(123,12,POSITION_STANDING, LOW_IMMORTAL, IMMORTAL, 4, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  10,	 TAR_IGNORE ,cast_goodberry,0);

  spello(124,12,POSITION_STANDING, LOW_IMMORTAL, IMMORTAL, 7, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  10,	 TAR_IGNORE,cast_flame_blade,0);

  spello(125,24,POSITION_FIGHTING, LOW_IMMORTAL, IMMORTAL, 35, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  20,	 TAR_CHAR_ROOM | TAR_SELF_NONO,cast_animal_growth,40);

  spello(126,24,POSITION_FIGHTING, LOW_IMMORTAL, IMMORTAL, 33, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  20,	 TAR_CHAR_ROOM | TAR_SELF_NONO,cast_insect_growth,40);

  spello(127,36,POSITION_STANDING, LOW_IMMORTAL, IMMORTAL, 45, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  50,	 TAR_IGNORE,cast_creeping_death,0);


  spello(128,24,POSITION_STANDING, LOW_IMMORTAL, IMMORTAL, 20, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  20,  LOW_IMMORTAL,
  10,	 TAR_IGNORE,cast_commune,0);

  spello(129,24,POSITION_STANDING, IMMORTAL, LOW_IMMORTAL, 15, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  20,  LOW_IMMORTAL,
  15,	 TAR_IGNORE, cast_animal_summon_1,0);
  spello(130,24,POSITION_STANDING, IMMORTAL, LOW_IMMORTAL, 20, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  25,  LOW_IMMORTAL,
  20,	 TAR_IGNORE, cast_animal_summon_2,0);
  spello(131,24,POSITION_STANDING, IMMORTAL, LOW_IMMORTAL, 25, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  30,  LOW_IMMORTAL,
  25,	 TAR_IGNORE, cast_animal_summon_3,0);


  spello(132,24,POSITION_STANDING, IMMORTAL, IMMORTAL, 35, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  30,	 TAR_IGNORE, cast_fire_servant,0);
  spello(133,24,POSITION_STANDING, IMMORTAL, IMMORTAL, 36, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  30,	 TAR_IGNORE, cast_earth_servant,0);
  spello(134,24,POSITION_STANDING, IMMORTAL, IMMORTAL, 37, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  30,	 TAR_IGNORE, cast_water_servant,0);
  spello(135,24,POSITION_STANDING, IMMORTAL, IMMORTAL, 38, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  30,	 TAR_IGNORE, cast_wind_servant,0);

  spello(136,36,POSITION_STANDING, LOW_IMMORTAL, IMMORTAL, 39, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  50,	 TAR_OBJ_ROOM, cast_reincarnate,0);


  spello(137,12,POSITION_STANDING, IMMORTAL, LOW_IMMORTAL, 17, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  5,	 TAR_CHAR_ROOM | TAR_VIOLENT, cast_charm_veggie,0);

  spello(138,24,POSITION_FIGHTING, LOW_IMMORTAL, IMMORTAL, 20, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  20,	 TAR_CHAR_ROOM | TAR_SELF_NONO,cast_veggie_growth,40);

  spello(139,24,POSITION_STANDING, LOW_IMMORTAL, IMMORTAL, 15, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  30,	 TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_tree,0);


  spello(140,24,POSITION_STANDING, 
  LOW_IMMORTAL, IMMORTAL, 31, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  15,	 TAR_OBJ_INV | TAR_OBJ_ROOM, cast_animate_rock,0);

  spello( 141, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 8, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  2,	 TAR_SELF_ONLY, cast_tree_travel, 0);

  spello( 142, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 10, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  15,  LOW_IMMORTAL,
  2,	 TAR_SELF_ONLY, cast_travelling, 0);

  spello( 143, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 5, 
    LOW_IMMORTAL,  LOW_IMMORTAL,  3,  LOW_IMMORTAL,
  2,	 TAR_CHAR_ROOM | TAR_SELF_NONO, cast_animal_friendship, 0);

  spello( 144, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 11, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  8,  LOW_IMMORTAL,
  10,	 TAR_SELF_ONLY, cast_invis_to_animals, 0);

  spello( 145, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 6, 
  LOW_IMMORTAL,  10,  10,  LOW_IMMORTAL,
  20,	 TAR_CHAR_ROOM, cast_slow_poison, 0);

  spello( 146, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL, 16, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  20,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_entangle, 0);

  spello( 147, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL, 8, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  5,  LOW_IMMORTAL,
  10,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_snare, 0);

  spello( 148, 12, POSITION_SITTING, 10, LOW_IMMORTAL, IMMORTAL, 
  10,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  2,	 TAR_IGNORE, cast_gust_of_wind, 30);

  spello( 149, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 3, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  5,	 TAR_CHAR_ROOM, cast_barkskin, 0);

  spello( 150, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL, 27, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  15,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_sunray, 30);

  spello( 151, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL, 19, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  20,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | 
	 TAR_OBJ_ROOM | TAR_OBJ_INV, cast_warp_weapon, 60);

  spello( 152, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL, 23, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  30,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_heat_stuff, 30);

  spello( 153, 12, POSITION_STANDING, LOW_IMMORTAL, 16, 15, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  13,  LOW_IMMORTAL,
  10,	 TAR_SELF_ONLY, cast_find_traps, 0);

  spello( 154, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL, 22, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  15,	 TAR_IGNORE, cast_firestorm, 30);

  spello( 155, 12, POSITION_STANDING, 23, IMMORTAL, IMMORTAL, 
  23,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  20, 	 TAR_CHAR_ROOM | TAR_VIOLENT, cast_haste, 0);

  spello( 156, 12, POSITION_STANDING, 19, IMMORTAL, IMMORTAL, 
  19,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  20, 	 TAR_CHAR_ROOM | TAR_VIOLENT, cast_slow, 0);

  spello(157,24,POSITION_STANDING, LOW_IMMORTAL, 3, 1, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  10,	 TAR_IGNORE, cast_dust_devil,0);

  spello(158, 12, POSITION_FIGHTING, 9, IMMORTAL, IMMORTAL, 
  9,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  20,	 TAR_SELF_NONO| TAR_CHAR_ROOM | TAR_FIGHT_VICT, 
	 cast_know_monster, 50);

  spello( 159, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 10, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  20,	 TAR_OBJ_WORLD, cast_transport_via_plant, 0);

  spello( 160, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 7, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  5,	 TAR_OBJ_ROOM, cast_speak_with_plants, 0);

  spello( 161, 12, POSITION_FIGHTING, 21, 23, 25, 
  21,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  30,	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_silence, 100);

  spello( 162, 12, POSITION_STANDING, 1, LOW_IMMORTAL, LOW_IMMORTAL, 
  1,  LOW_IMMORTAL,1,LOW_IMMORTAL, 
  5,	 0, 0, 0);
  
  spello( 163, 12, POSITION_STANDING, 30, BIG_GUY, BIG_GUY, 
  30,  BIG_GUY,  BIG_GUY,  BIG_GUY,   
  20,	 TAR_CHAR_WORLD, cast_teleport_wo_error, 0);
  
  spello( 164, 12, POSITION_STANDING, 43, BIG_GUY, BIG_GUY, 
  43,  BIG_GUY,  BIG_GUY,  BIG_GUY, 
  50,	 TAR_CHAR_WORLD, cast_portal, 0);
  
  spello( 165, 12, POSITION_STANDING, BIG_GUY, BIG_GUY, BIG_GUY, 
  BIG_GUY,  BIG_GUY,  BIG_GUY,  BIG_GUY, 
  20,	 TAR_IGNORE, cast_dragon_ride, 0);
  
  spello( 166, 12, POSITION_STANDING, 47, BIG_GUY, 15, 
  47,  BIG_GUY,  BIG_GUY,  BIG_GUY, 
  20,	 TAR_IGNORE, cast_mount, 0);
  

/* few skills */
  
  spello(45,0,POSITION_STANDING,BIG_GUY+1,BIG_GUY+1, BIG_GUY+1,
  BIG_GUY+1,  BIG_GUY+1,  4,  BIG_GUY+1, 
  200,	 TAR_IGNORE, 0, 0); /* sneak */

  spello(46,0,POSITION_STANDING,BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,
   BIG_GUY+1,  BIG_GUY+1,  2,  BIG_GUY+1, 
   200,	 TAR_IGNORE, 0, 0); /* hide */

  spello(51,0,POSITION_STANDING,BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,
  BIG_GUY+1,  1,  1,  BIG_GUY+1, 
  200,	 TAR_IGNORE, 0, 0); /* bash */

  spello(183,0, POSITION_STANDING,
  BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,
  LOW_IMMORTAL,  1,  1,  LOW_IMMORTAL,
  200, 0, 0, 0); /* disarm */

  spello(186,0, POSITION_STANDING,
  BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,
  LOW_IMMORTAL,  LOW_IMMORTAL,  1,  LOW_IMMORTAL,
  200, 0, 0, 0); /* spy */

  spello(173,0, POSITION_STANDING,
  BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,
  LOW_IMMORTAL,  1,  1,  LOW_IMMORTAL,
  200, 0, 0, 0); /* switch opp */


  spello(198,0, POSITION_STANDING,
  BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,
  LOW_IMMORTAL,  LOW_IMMORTAL,  1,  LOW_IMMORTAL,
  200, 0, 0, 0); /* climb */


  spello(180,0, POSITION_STANDING,
  BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,
  LOW_IMMORTAL,  LOW_IMMORTAL,  1,  LOW_IMMORTAL,
  200, 0, 0, 0); /* hunt */


  spello(174,0, POSITION_STANDING,
  BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,
  LOW_IMMORTAL,  1,  1,  LOW_IMMORTAL,
  200, 0, 0, 0); /* dodge */


  spello(170,0, POSITION_STANDING,
  BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,
  LOW_IMMORTAL,  1,  1,  LOW_IMMORTAL,
  200, 0, 0, 0); /* first aid */

  spello(207,0, POSITION_STANDING,
  BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  200, 0, 0, 0); /* berserk */
  
  spello(208,0, POSITION_STANDING,
  BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,
  LOW_IMMORTAL,  LOW_IMMORTAL,  1,  LOW_IMMORTAL,
  200, 0, 0, 0); /* tan */
  
  spello(209,0, POSITION_STANDING,
  BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  200, 0, 0, 0); /* avoid ba */
  
  spello(210,0, POSITION_STANDING,
  BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,
  LOW_IMMORTAL,  LOW_IMMORTAL,  1,  LOW_IMMORTAL,
  200, 0, 0, 0); /* find food */
  
  spello(211,0, POSITION_STANDING,
  BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,
  LOW_IMMORTAL,  LOW_IMMORTAL,  1,  LOW_IMMORTAL,
  200, 0, 0, 0); /* find water */

  spello(214,0, POSITION_STANDING,
  BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,  
  BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,BIG_GUY+1,
  200, 0, 0, 0); /* bellow */


/* end skills */


/* NEW SPELLS  msw */

  spello(215,12, POSITION_STANDING,5 ,3 ,3 ,
  5,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  15,         TAR_CHAR_ROOM, cast_globe_darkness, 0);
  
  spello(216,12, POSITION_STANDING,20,IMMORTAL,IMMORTAL,
  20,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  25,         TAR_CHAR_ROOM| TAR_SELF_ONLY, cast_globe_minor_inv, 0);
         
  spello(217,24, POSITION_STANDING,27,IMMORTAL,IMMORTAL,
  27,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  50,         TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_globe_major_inv, 0);
         
  spello(218,12, POSITION_STANDING,IMMORTAL,48 ,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  40,         TAR_CHAR_ROOM| TAR_SELF_ONLY, cast_prot_energy_drain, 0);
         
  spello(219,12, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  200,         TAR_CHAR_ROOM, cast_prot_dragon_breath, 0);
         
  spello(220,24, POSITION_STANDING,48,BIG_GUY,BIG_GUY,
  48,  BIG_GUY,  BIG_GUY,  BIG_GUY,
  100, TAR_CHAR_ROOM|TAR_SELF_ONLY, cast_anti_magic_shell, 0);

/* end new spells */


/* psionist stuff */
  spello(221,0, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  3,
  20, 0, 0, 0); /* doorway */

/* need to add a astral travel at about level 20 */

  spello(222,0, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  32,
  20, 0, 0, 0); /* portal */

  spello(223,0, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  10,
  20, 0, 0, 0); /* summon */

  spello(224,0, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  1,
  20, 0, 0, 0); /* invis */

  spello(225,0, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  2,
  20, 0, 0, 0); /* canibalize */

  spello(226,0, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  15,
  20, 0, 0, 0);/* flame shroud */

  spello(227,0, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  5,
  20, 0, 0, 0);/* aura site */

  spello(228,0, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  11,
  20, 0, 0, 0); /* great site */
  
  spello(229,0, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  1,
  20, 0, 0, 0); /* blast */
  
  spello(230,0, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  2,
  20, 0, 0, 0); /* hypnosis */


  spello(231,0, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  1,
  20, 0, 0, 0); /* meditate */

  spello(232,0, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  7,
  20, 0, 0, 0); /* scry */

  spello(233,0, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  20,
  20, 0, 0, 0);/* adrenalize */

  spello(241,24, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  1,
  20, TAR_SELF_ONLY|TAR_CHAR_ROOM, 0, 0);/* psi shield */

  spello(254,24, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  1,
  10, TAR_SELF_ONLY|TAR_CHAR_ROOM, 0, 0);/* psi esp */

  spello(268,24,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  5,
  15,	 TAR_IGNORE| TAR_VIOLENT, mind_use_burn, 5);

  spello(269,24,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  8,
  45,	 TAR_SELF_ONLY|TAR_CHAR_ROOM, mind_use_clairvoyance, 0);

  spello(270,24,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  23,
  50,	 TAR_SELF_ONLY|TAR_CHAR_ROOM, mind_use_danger_sense, 0);


  spello(271,24,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  38,
  50,	 TAR_CHAR_ROOM|TAR_VIOLENT|TAR_FIGHT_VICT, mind_use_disintergrate, 10);

  spello(272,24,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL, 35,
  35,	 TAR_CHAR_ROOM|TAR_VIOLENT|TAR_FIGHT_VICT, mind_use_telekinesis,20);

  spello(273,24,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  10,
  20,	 TAR_CHAR_ROOM, mind_use_levitation, 0);

  spello(274,24,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  14,
  100,	 TAR_SELF_ONLY|TAR_CHAR_ROOM, mind_use_cell_adjustment, 0);

  spello(275,24,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  8,
  40,	 TAR_SELF_ONLY|TAR_CHAR_ROOM, mind_use_chameleon, 0);


  spello(276,24,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  6,
  15,	 TAR_SELF_ONLY|TAR_CHAR_ROOM, mind_use_psi_strength, 0);

  spello(277,24,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  14,
  60,	 TAR_SELF_ONLY|TAR_CHAR_ROOM, mind_use_mind_over_body, 0);


  spello(278,24,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  21,
  40,	 TAR_SELF_ONLY|TAR_CHAR_ROOM, mind_use_probability_travel, 0);

  spello(279,24,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  12,
  15,	 TAR_CHAR_ROOM|TAR_VIOLENT|TAR_FIGHT_VICT, mind_use_teleport, 0);

  spello(280,24,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  51,
  30,	 TAR_CHAR_ROOM|TAR_VIOLENT|TAR_FIGHT_VICT, mind_use_domination, 0);

  spello(281,24,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  21,
  20,	 TAR_CHAR_ROOM|TAR_VIOLENT|TAR_FIGHT_VICT, mind_use_mind_wipe, 0);

  spello(282,34,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  37,
  45,	 TAR_CHAR_ROOM|TAR_VIOLENT|TAR_FIGHT_VICT, mind_use_psychic_crush, 0);


  spello(283,24,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  34,
  50,	TAR_SELF_ONLY|TAR_CHAR_ROOM, mind_use_tower_iron_will, 0);


  spello(284,24,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  17,
  50,	 TAR_SELF_ONLY|TAR_CHAR_ROOM, mind_use_mindblank, 0);


  spello(285,24,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  3,
  50,	 TAR_SELF_ONLY|TAR_CHAR_ROOM, mind_use_psychic_impersonation, 0);

  spello(286,35,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  20,
  60,	 TAR_CHAR_ROOM|TAR_VIOLENT|TAR_FIGHT_VICT|TAR_IGNORE, mind_use_ultra_blast, 0);


  spello(287,24,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  100,	 TAR_SELF_ONLY|TAR_CHAR_ROOM, mind_use_intensify, 0);

  
/* end psionist stuff */

/* ranger skills */

  spello(235,0, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  1,  LOW_IMMORTAL,
  200, 0, 0, 0); /* ration */

  spello(240,0, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  1,  LOW_IMMORTAL,
  200, 0, 0, 0); /* dual wield */

/* end ranger skills */

/* paladin skills */

  spello(236,0, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  5,  LOW_IMMORTAL,  LOW_IMMORTAL,
  200, 0, 0, 0); /* paladin warcry */

  spello(237,0, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  1,  LOW_IMMORTAL,  LOW_IMMORTAL,
  200, 0, 0, 0); /* paladin blessing */


  spello(238,0, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  1,  LOW_IMMORTAL,  LOW_IMMORTAL,
  200, 0, 0, 0); /* paladin lay on hands */


  spello(239,0, POSITION_STANDING,IMMORTAL,IMMORTAL,IMMORTAL,
  LOW_IMMORTAL,  1,  LOW_IMMORTAL,  LOW_IMMORTAL,
  200, 0, 0, 0); /* paladin heroic rescue */

/* end paladin skills */

/* new spells here */

  spello(242,12,POSITION_STANDING,LOW_IMMORTAL,  20, LOW_IMMORTAL, 
  LOW_IMMORTAL,  35,  45,  LOW_IMMORTAL,
  45,	 TAR_IGNORE, cast_protection_from_evil_group, 0);


  spello(243,36,POSITION_FIGHTING,17,  LOW_IMMORTAL, LOW_IMMORTAL, 
  17,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  40,	 TAR_IGNORE| TAR_VIOLENT, cast_prismatic_spray,0);

  spello(244,36,POSITION_FIGHTING,45,  LOW_IMMORTAL, LOW_IMMORTAL, 
  45,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  60,	 TAR_IGNORE| TAR_VIOLENT, cast_incendiary_cloud, 0);
  
  spello(245,12,POSITION_FIGHTING,48,  LOW_IMMORTAL, LOW_IMMORTAL, 
  48,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  45,	 TAR_CHAR_ROOM|TAR_VIOLENT,  cast_disintergrate, 0);

  spello( 255,12,POSITION_STANDING, 
   1,  3, LOW_IMMORTAL,
   1,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL, 
   10,	 TAR_CHAR_ROOM, cast_comp_languages, 0);

  spello(256,12, POSITION_STANDING,BIG_GUY,21,22,
  BIG_GUY,  BIG_GUY,  BIG_GUY,  BIG_GUY,
  40, TAR_CHAR_ROOM|TAR_SELF_ONLY, cast_prot_fire, 0);

  spello(257,12, POSITION_STANDING,BIG_GUY,20,21,
  BIG_GUY,  BIG_GUY,  BIG_GUY,  BIG_GUY,
  40, TAR_CHAR_ROOM|TAR_SELF_ONLY, cast_prot_cold, 0);

  spello(258,12, POSITION_STANDING,BIG_GUY,19,BIG_GUY,
  BIG_GUY,  BIG_GUY,  BIG_GUY,  BIG_GUY,
  40, TAR_CHAR_ROOM|TAR_SELF_ONLY, cast_prot_energy, 0);

  spello(259,12, POSITION_STANDING,BIG_GUY,18,19,
  BIG_GUY,  BIG_GUY,  BIG_GUY,  BIG_GUY,
  40, TAR_CHAR_ROOM|TAR_SELF_ONLY, cast_prot_elec, 0);
  
  spello(260,48,POSITION_STANDING,16, LOW_IMMORTAL, LOW_IMMORTAL, 
  16,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  100,	 TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_enchant_armor,0);

  spello( 261, 12, POSITION_STANDING, LOW_IMMORTAL,1 ,1, 
  LOW_IMMORTAL,  1,LOW_IMMORTAL,LOW_IMMORTAL, 
  5,	 0, 0, 0);

  spello(262,24, POSITION_STANDING,IMMORTAL,40,41,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  50,         TAR_CHAR_ROOM|TAR_SELF_ONLY, cast_prot_dragon_breath_fire, 0);

  spello(263,24, POSITION_STANDING,IMMORTAL,38,39,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  50,         TAR_CHAR_ROOM|TAR_SELF_ONLY, cast_prot_dragon_breath_frost, 0);
  
  spello(264,24, POSITION_STANDING,IMMORTAL,39,40,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  50,         TAR_CHAR_ROOM|TAR_SELF_ONLY, cast_prot_dragon_breath_elec, 0);  
  
  spello(265,24, POSITION_STANDING,IMMORTAL,40,41,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  50,         TAR_CHAR_ROOM|TAR_SELF_ONLY, cast_prot_dragon_breath_acid, 0);
  
  spello(266,24, POSITION_STANDING,IMMORTAL,37,IMMORTAL,
  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL,
  50,         TAR_CHAR_ROOM|TAR_SELF_ONLY, cast_prot_dragon_breath_gas, 0);

  spello( 267, 24, POSITION_STANDING, 
  35,  LOW_IMMORTAL,  LOW_IMMORTAL, 
  35,  LOW_IMMORTAL,  LOW_IMMORTAL,  LOW_IMMORTAL, 
  40,  TAR_CHAR_ROOM|TAR_SELF_ONLY,cast_wizard_eye,20);


  spello(288,24,POSITION_FIGHTING,LOW_IMMORTAL,  LOW_IMMORTAL, LOW_IMMORTAL, 
  LOW_IMMORTAL,1,1,  LOW_IMMORTAL,
  100,	0 ,0, 0); /*spot */

} 



void SpellWearOffSoon(int s, struct char_data *ch)
{

  if (s > MAX_SKILLS+10) 
    return;

  if (spell_wear_off_soon_msg[s] && *spell_wear_off_soon_msg[s]) {
    send_to_char(spell_wear_off_soon_msg[s], ch);
    send_to_char("\n\r", ch);
  }

  if (spell_wear_off_soon_room_msg[s] && *spell_wear_off_soon_room_msg[s]) {
    act(spell_wear_off_soon_room_msg[s], FALSE, ch, 0, 0, TO_ROOM);
  }

}


void SpellWearOff(int s, struct char_data *ch)
{

  if (s > MAX_SKILLS+10) 
    return;

  if (spell_wear_off_msg[s] && *spell_wear_off_msg[s]) {
    send_to_char(spell_wear_off_msg[s], ch);
    send_to_char("\n\r", ch);
  }

  if (spell_wear_off_room_msg[s] && *spell_wear_off_room_msg[s]) {
    act(spell_wear_off_room_msg[s], FALSE, ch, 0, 0, TO_ROOM);
  }


  if (s == SPELL_CHARM_PERSON || s == SPELL_CHARM_MONSTER) {
    check_decharm(ch);
  }

  if (s == SPELL_FLY)
    check_falling(ch);		

  if (s == SPELL_WATER_BREATH) {
    check_drowning(ch);
  }

 
}

void check_decharm( struct char_data *ch)
{
  struct char_data *m;

  if (!ch->master) return;

  m = ch->master;
  stop_follower(ch);   /* stop following the master */
  REMOVE_BIT(ch->specials.act, ACT_SENTINEL);
  AddFeared( ch, m);
  do_flee(ch, "", 0);

}


int check_falling( struct char_data *ch)
{
  struct room_data *rp, *targ;
  int done, count, saved;
  char buf[256];

if (IS_IMMORTAL(ch)) 	/* so if this guy is using redit the mud does not */
return(FALSE);		/* crash when he falls... */

if (GET_POS(ch) <= POSITION_DEAD)
    return(FALSE);    /* ch will be purged, check corpse instead, bug fix msw */
    
  if (IS_AFFECTED(ch, AFF_FLYING))
    return(FALSE);

  rp = real_roomp(ch->in_room);
  if (!rp) return(FALSE);

  if (rp->sector_type != SECT_AIR) 
    return(FALSE);

  if (ch->skills && number(1,101) < ch->skills[SKILL_SAFE_FALL].learned) {
    act("You manage to slow your fall, enough to stay alive..", 
	TRUE, ch, 0, 0, TO_CHAR);
    saved = TRUE;
  } else {
    act("The world spins, and you sky-dive out of control",
	TRUE, ch, 0, 0, TO_CHAR);
    saved = FALSE;
  }

  done = FALSE;
  count = 0;

  while (!done && count < 100) {

/*
  check for an exit down.
  if there is one, go through it.
*/
    if (rp->dir_option[DOWN] && rp->dir_option[DOWN]->to_room > -1) {
      targ = real_roomp(rp->dir_option[DOWN]->to_room);
    } else {
      /*
	pretend that this is the smash room.
	*/
      if (count > 1) {

	send_to_char("You are smashed into tiny pieces.\n\r", ch);
	act("$n smashes against the ground at high speed", 
	    FALSE, ch, 0, 0, TO_ROOM);
	act("You are drenched with blood and gore", 
	    FALSE,ch, 0, 0, TO_ROOM);

/*
  should damage all their stuff
*/
	DamageAllStuff(ch, BLOW_DAMAGE);

	if (!IS_IMMORTAL(ch)) {
	  GET_HIT(ch) = 0;
	  sprintf(buf, "%s has fallen to death", GET_NAME(ch));
	  log(buf);
	if (!ch->desc)
	  GET_GOLD(ch) = 0;
	  die(ch,NULL); /* change to the smashed type */
	}
	return(TRUE);
	
      } else {

	send_to_char("You land with a resounding THUMP!\n\r", ch);
	GET_HIT(ch) = 0;
	GET_POS(ch) = POSITION_STUNNED;
	act("$n lands with a resounding THUMP!", FALSE, ch, 0, 0, TO_ROOM);
/*
  should damage all their stuff
*/
	DamageAllStuff(ch, BLOW_DAMAGE);

	return(TRUE);

      }
    }

    act("$n plunges towards oblivion", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("You plunge from the sky\n\r", ch);
    char_from_room(ch);
    char_to_room(ch, rp->dir_option[DOWN]->to_room);
    act("$n falls from the sky", FALSE, ch, 0, 0, TO_ROOM);
    count++;

    do_look(ch, "", 0);

    if (IS_SET(targ->room_flags, DEATH) && !IS_IMMORTAL(ch)) {
      NailThisSucker(ch);
      return(TRUE);
    }

    if (targ->sector_type != SECT_AIR) {
      /* do damage, or kill */
      if (count == 1) {
	send_to_char("You land with a resounding THUMP!\n\r", ch);
	GET_HIT(ch) = 0;
	GET_POS(ch) = POSITION_STUNNED;
	act("$n lands with a resounding THUMP!", FALSE, ch, 0, 0, TO_ROOM);
/*
  should damage all their stuff
*/
	DamageAllStuff(ch, BLOW_DAMAGE);

	return(TRUE);

      } else if (!saved) {
        send_to_char("You are smashed into tiny pieces.\n\r", ch);
        if (targ->sector_type >= SECT_WATER_SWIM)
	  act("$n is smashed to a pulp by $s impact with the water", 
	    FALSE, ch, 0, 0, TO_ROOM);
        else 
	  act("$n is smashed to a bloody pulp by $s impact with the ground", 
	    FALSE, ch, 0, 0, TO_ROOM);
	act("You are drenched with blood and gore", FALSE,ch, 0, 0, TO_ROOM);

/*
  should damage all their stuff
*/
	DamageAllStuff(ch, BLOW_DAMAGE);

	if (!IS_IMMORTAL(ch)) {
	  GET_HIT(ch) = 0;
	  sprintf(buf, "%s has fallen to death", GET_NAME(ch));
	  log(buf);
	if (!ch->desc)
	  GET_GOLD(ch) = 0;
	  die(ch,NULL);
	}
	return(TRUE);

      } else {
	send_to_char("You land with a resounding THUMP!\n\r", ch);
	GET_HIT(ch) = 0;
	GET_POS(ch) = POSITION_STUNNED;
	act("$n lands with a resounding THUMP!", FALSE, ch, 0, 0, TO_ROOM);
/*
  should damage all their stuff
*/
	DamageAllStuff(ch, BLOW_DAMAGE);

	return(TRUE);
	
      }
    } else {
/*
  time to try the next room
*/
      rp = targ;
      targ = 0;
    }
  }  

  if (count >= 100) {
    log("Someone fucked up an air room.");
    char_from_room(ch);
    char_to_room(ch, 2);
    do_look(ch, "", 0);
    return(FALSE);
  }
}

void check_drowning( struct char_data *ch)
{
  struct room_data *rp;
  char buf[256];

  if (IS_AFFECTED(ch, AFF_WATERBREATH))
    return;

  rp = real_roomp(ch->in_room);

  if (!rp) return;

  if (rp->sector_type == SECT_UNDERWATER) {
      send_to_char("PANIC!  You're drowning!!!!!!", ch);
      GET_HIT(ch)-=number(1,30);
      GET_MOVE(ch) -= number(10,50);
      update_pos(ch);
      if (GET_HIT(ch) < -10) {
	sprintf(buf, "%s killed by drowning", GET_NAME(ch));
	log(buf);
	if (!ch->desc)
	  GET_GOLD(ch) = 0;
	die(ch,NULL);
      }
   }
}


void check_falling_obj( struct obj_data *obj, int room)
{
  struct room_data *rp, *targ;
  int done, count;

  if (obj->in_room != room) {
    log("unusual object information in check_falling_obj");
    return;
  }

  rp = real_roomp(room);
  if (!rp) return;

  if (rp->sector_type != SECT_AIR) 
    return;

  done = FALSE;
  count = 0;

  while (!done && count < 100) {

    if (rp->dir_option[DOWN] && rp->dir_option[DOWN]->to_room > -1) {
      targ = real_roomp(rp->dir_option[DOWN]->to_room);
    } else {
      /*
	pretend that this is the smash room.
	*/
      if (count > 1) {

	if (rp->people) {
	  act("$p smashes against the ground at high speed", 
	      FALSE, rp->people, obj, 0, TO_ROOM);
	  act("$p smashes against the ground at high speed", 
	      FALSE, rp->people, obj, 0, TO_CHAR);
	}
	return;

      } else {

	if (rp->people) {
	  act("$p lands with a loud THUMP!", 
	      FALSE, rp->people, obj, 0, TO_ROOM);
	  act("$p lands with a loud THUMP!", 
	      FALSE, rp->people, obj, 0, TO_CHAR);
	}
	return;

      }
    }

    if (rp->people) { /* have to reference a person */
      act("$p falls out of sight", FALSE, rp->people, obj, 0, TO_ROOM);
      act("$p falls out of sight", FALSE, rp->people, obj, 0, TO_CHAR);
    }
    obj_from_room(obj);
    obj_to_room(obj, rp->dir_option[DOWN]->to_room);
    if (targ->people) {
      act("$p falls from the sky", FALSE, targ->people, obj, 0, TO_ROOM);
      act("$p falls from the sky", FALSE, targ->people, obj, 0, TO_CHAR);
    }
    count++;

    if (targ->sector_type != SECT_AIR) {
      if (count == 1) {
	if (targ->people) {
	  act("$p lands with a loud THUMP!", FALSE, targ->people, obj, 0, TO_ROOM);
	  act("$p lands with a loud THUMP!", FALSE, targ->people, obj, 0, TO_CHAR);
	}
	return;
      } else {
	if (targ->people) {
	  if (targ->sector_type >= SECT_WATER_SWIM){	  
	    act("$p smashes against the water at high speed", 
		FALSE, targ->people, obj, 0, TO_ROOM);
	    act("$p smashes against the water at high speed", 
		FALSE, targ->people, obj, 0, TO_CHAR);
	  } else {
	    act("$p smashes against the ground at high speed", 
		FALSE, targ->people, obj, 0, TO_ROOM);
	    act("$p smashes against the ground at high speed", 
		FALSE, targ->people, obj, 0, TO_CHAR);
	  }
	}
	return;

      }
    } else {
/*
  time to try the next room
*/
      rp = targ;
      targ = 0;
    }
  }  

  if (count >= 100) {
    log("Someone screwed up an air room.");
    obj_from_room(obj);
    obj_to_room(obj, 4);
    return;
  }
}

int check_nature( struct char_data *i)
{

  if (check_falling(i)) {
    return(TRUE);
  }
  check_drowning(i);

}
