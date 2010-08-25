void do_look(struct char_data *ch, char *argument, int cmd)
{
  char buffer[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int keyword_no, res;
  int j, bits, temp;
  bool found;
  struct obj_data *tmp_object, *found_object;
  struct char_data *tmp_char;
  char *tmp_desc;
  static char *keywords[]= { 
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "in",
    "at",
    "",  /* Look at '' case */
    "room",
    "\n" };
  
  if (!ch->desc)
    return;
  
  if (GET_POS(ch) < POSITION_SLEEPING)
    send_to_char("You can't see anything but stars!\n\r", ch);
  else if (GET_POS(ch) == POSITION_SLEEPING)
    send_to_char("You can't see anything, you're sleeping!\n\r", ch);
  else if ( IS_AFFECTED(ch, AFF_BLIND) )
    send_to_char("You can't see a damn thing, you're blinded!\n\r", ch);
  else if  ((IS_DARK(ch->in_room)) && (!IS_IMMORTAL(ch)) &&
	    (!IS_AFFECTED(ch, AFF_TRUE_SIGHT))) {
    send_to_char("It is very dark in here...\n\r", ch);
    if (IS_AFFECTED(ch, AFF_INFRAVISION)) {
      list_char_in_room(real_roomp(ch->in_room)->people, ch);
    }
  } else {

    only_argument(argument, arg1);

    if (0==strn_cmp(arg1,"at",2) && isspace(arg1[2])) {
      only_argument(argument+3, arg2);
      keyword_no = 7;
    } else if (0==strn_cmp(arg1,"in",2) && isspace(arg1[2])) {
      only_argument(argument+3, arg2);
      keyword_no = 6;
    } else {
      keyword_no = search_block(arg1, keywords, FALSE);
    }

    if ((keyword_no == -1) && *arg1) {
      keyword_no = 7;
      only_argument(argument, arg2);
    }
    

    found = FALSE;
    tmp_object = 0;
    tmp_char	 = 0;
    tmp_desc	 = 0;
    
    switch(keyword_no) {
      /* look <dir> */
    case 0 :
    case 1 :
    case 2 : 
    case 3 : 
    case 4 :
    case 5 : {   
      struct room_direction_data	*exitp;
      exitp = EXIT(ch, keyword_no);
      if (exitp) {
	if (exitp->general_description) {
	  send_to_char(exitp-> general_description, ch);
	} else {
	  send_to_char("You see nothing special.\n\r", ch);
	}
	
	if (IS_SET(exitp->exit_info, EX_CLOSED) && 
	    (exitp->keyword)) {
	   if ((strcmp(fname(exitp->keyword), "secret")) &&
	      (!IS_SET(exitp->exit_info, EX_SECRET))) {
	      sprintf(buffer, "The %s is closed.\n\r", 
		    fname(exitp->keyword));
	      send_to_char(buffer, ch);
	    } 
	 } else {
	   if (IS_SET(exitp->exit_info, EX_ISDOOR) &&
	      exitp->keyword) {
	      sprintf(buffer, "The %s is open.\n\r",
		      fname(exitp->keyword));
	      send_to_char(buffer, ch);
	    }
	 }
      } else {
	send_to_char("You see nothing special.\n\r", ch);
      }
      if (exitp && exitp->to_room && (!IS_SET(exitp->exit_info, EX_ISDOOR) ||
	 (!IS_SET(exitp->exit_info, EX_CLOSED)))) {
	if (IS_AFFECTED(ch, AFF_SCRYING) || IS_IMMORTAL(ch)) {
	  struct room_data	*rp;
	  sprintf(buffer,"You look %swards.\n\r", dirs[keyword_no]);
	  send_to_char(buffer, ch);

	  sprintf(buffer,"$n looks %swards.", dirs[keyword_no]);
	  act(buffer, FALSE, ch, 0, 0, TO_ROOM);

	  rp = real_roomp(exitp->to_room);
	  if (!rp) {
	    send_to_char("You see swirling chaos.\n\r", ch);
	  } else if(exitp) {
	    sprintf(buffer, "%d look", exitp->to_room);
	    do_at(ch, buffer, 0);
	  } else {
	    send_to_char("You see nothing special.\n\r", ch);
	  }
	}
      }
    }
      break;
      
      /* look 'in'	*/
    case 6: {
      if (*arg2) {
	/* Item carried */
	bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
			    FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
	
	if (bits) { /* Found something */
	  if (GET_ITEM_TYPE(tmp_object)== ITEM_DRINKCON) 	{
	    if (tmp_object->obj_flags.value[1] <= 0) {
	      act("It is empty.", FALSE, ch, 0, 0, TO_CHAR);
	    } else {
	      temp=((tmp_object->obj_flags.value[1]*3)/tmp_object->obj_flags.value[0]);
	      sprintf(buffer,"It's %sfull of a %s liquid.\n\r",
		      fullness[temp],color_liquid[tmp_object->obj_flags.value[2]]);
	      send_to_char(buffer, ch);
	    }
	  } else if (GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER) {
	    if (!IS_SET(tmp_object->obj_flags.value[1],CONT_CLOSED)) {
	      send_to_char(fname(tmp_object->name), ch);
	      switch (bits) {
	      case FIND_OBJ_INV :
		send_to_char(" (carried) : \n\r", ch);
		break;
	      case FIND_OBJ_ROOM :
		send_to_char(" (here) : \n\r", ch);
		break;
	      case FIND_OBJ_EQUIP :
		send_to_char(" (used) : \n\r", ch);
		break;
	      }
	      list_obj_in_heap(tmp_object->contains, ch);
	    } else
	      send_to_char("It is closed.\n\r", ch);
	  } else {
	    send_to_char("That is not a container.\n\r", ch);
	  }
	} else { /* wrong argument */
	  send_to_char("You do not see that item here.\n\r", ch);
	}
      } else { /* no argument */
	send_to_char("Look in what?!\n\r", ch);
      }
    }
      break;
      
      /* look 'at'	*/
    case 7 : {
      if (*arg2) {
	bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
			    FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &tmp_char, &found_object);
	if (tmp_char) {
	  show_char_to_char(tmp_char, ch, 1);
	  if (ch != tmp_char) {
	    act("$n looks at you.", TRUE, ch, 0, tmp_char, TO_VICT);
	    act("$n looks at $N.", TRUE, ch, 0, tmp_char, TO_NOTVICT);
	  }
	  return;
	}
	/* 
	  Search for Extra Descriptions in room and items 
	  */
	
	/* Extra description in room?? */
	if (!found) {
	  tmp_desc = find_ex_description(arg2, 
					 real_roomp(ch->in_room)->ex_description);
	  if (tmp_desc) {
	    page_string(ch->desc, tmp_desc, 0);
	    return; 
	  }
	}
	
	/* extra descriptions in items */
	
	/* Equipment Used */
	if (!found) {
	  for (j = 0; j< MAX_WEAR && !found; j++) {
	    if (ch->equipment[j]) {
	      if (CAN_SEE_OBJ(ch,ch->equipment[j])) {
		tmp_desc = find_ex_description(arg2, 
					       ch->equipment[j]->ex_description);
		if (tmp_desc) {
		  page_string(ch->desc, tmp_desc, 1);
		  found = TRUE;
		}
	      }
	    }
	  }
	}
	/* In inventory */
	if (!found) {
	  for(tmp_object = ch->carrying; 
	      tmp_object && !found; 
	      tmp_object = tmp_object->next_content) {
	    if CAN_SEE_OBJ(ch, tmp_object) {
	      tmp_desc = find_ex_description(arg2, 
					     tmp_object->ex_description);
	      if (tmp_desc) {
		page_string(ch->desc, tmp_desc, 1);
		found = TRUE;
	      }
	    }
	  }
	}
	/* Object In room */
	
	if (!found) {
	  for(tmp_object = real_roomp(ch->in_room)->contents; 
	      tmp_object && !found; 
	      tmp_object = tmp_object->next_content) {
	    if CAN_SEE_OBJ(ch, tmp_object) {
	      tmp_desc = find_ex_description(arg2, 
					     tmp_object->ex_description);
	      if (tmp_desc) {
		page_string(ch->desc, tmp_desc, 1);
		found = TRUE;
	      }
	    }
	  }
	}
	/* wrong argument */
	if (bits) { /* If an object was found */
	  if (!found)
	    show_obj_to_char(found_object, ch, 5); 
	  /* Show no-description */
	  else
	    show_obj_to_char(found_object, ch, 6); 
	  /* Find hum, glow etc */
	} else if (!found) {
	  send_to_char("You do not see that here.\n\r", ch);
	}
      } else {
	/* no argument */	
	send_to_char("Look at what?\n\r", ch);
      }
    }
      break;
      
      /* look ''		*/ 
    case 8 : {
      send_to_char(real_roomp(ch->in_room)->name, ch);
      send_to_char("\n\r", ch);
      if (!IS_SET(ch->specials.act, PLR_BRIEF))
	send_to_char(real_roomp(ch->in_room)->description, ch);
      
      if (!IS_NPC(ch)) {
	if (IS_SET(ch->specials.act, PLR_HUNTING)) {
	  if (ch->specials.hunting) {
	    res = track(ch, ch->specials.hunting);
	    if (!res) {
	      ch->specials.hunting = 0;
	      ch->hunt_dist = 0;
	      REMOVE_BIT(ch->specials.act, PLR_HUNTING);
	    }
	  } else {
	    ch->hunt_dist = 0;
	    REMOVE_BIT(ch->specials.act, PLR_HUNTING);
	  }
	}
      } else {
	if (IS_SET(ch->specials.act, ACT_HUNTING)) {
	  if (ch->specials.hunting) {
	    res = track(ch, ch->specials.hunting);
	    if (!res) {
	      ch->specials.hunting = 0;
	      ch->hunt_dist = 0;
	      REMOVE_BIT(ch->specials.act, ACT_HUNTING);
	    }  
	  } else {
	    ch->hunt_dist = 0;
	    REMOVE_BIT(ch->specials.act, ACT_HUNTING);
	  }
	}
      }
      
      list_obj_in_room(real_roomp(ch->in_room)->contents, ch);
      list_char_in_room(real_roomp(ch->in_room)->people, ch);
      
    }
      break;
      
      /* wrong arg	*/
    case -1 : 
      send_to_char("Sorry, I didn't understand that!\n\r", ch);
      break;
      
      /* look 'room' */
    case 9 : {
      
      send_to_char(real_roomp(ch->in_room)->name, ch);
      send_to_char("\n\r", ch);
      send_to_char(real_roomp(ch->in_room)->description, ch);
      
      if (!IS_NPC(ch)) {
	if (IS_SET(ch->specials.act, PLR_HUNTING)) {
	  if (ch->specials.hunting) {
	    res = track(ch, ch->specials.hunting);
	    if (!res) {
	      ch->specials.hunting = 0;
	      ch->hunt_dist = 0;
	      REMOVE_BIT(ch->specials.act, PLR_HUNTING);
	    }
	  } else {
	    ch->hunt_dist = 0;
	    REMOVE_BIT(ch->specials.act, PLR_HUNTING);
	  }
	}
      } else {
	if (IS_SET(ch->specials.act, ACT_HUNTING)) {
	  if (ch->specials.hunting) {
	    res = track(ch, ch->specials.hunting);
	    if (!res) {
	      ch->specials.hunting = 0;
	      ch->hunt_dist = 0;
	      REMOVE_BIT(ch->specials.act, ACT_HUNTING);
	    }  
	  } else {
	    ch->hunt_dist = 0;
	    REMOVE_BIT(ch->specials.act, ACT_HUNTING);
	  }
	}
      }
      
      list_obj_in_room(real_roomp(ch->in_room)->contents, ch);
      list_char_in_room(real_roomp(ch->in_room)->people, ch);
      
    }
      break;
    }
  }
}

/* end of look */
