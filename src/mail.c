/* ************************************************************************
*   File: mail.c                                        Part of CircleMUD *
*  Usage: Internal funcs and player spec-procs of mud-mail system         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993 by the Trustees of the Johns Hopkins University     *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/******* MUD MAIL SYSTEM MAIN FILE ***************************************

Written by Jeremy Elson (jelson@server.cs.jhu.edu)

INSTALLATION INSTRUCTIONS
-------------------------

0.  Change your makefile so that mail.c is compiled and linked into the MUD.

1.  Edit mail.h and change the constants to your personal preferences.

2.  Create a room in your MUD designated as the Post Office and create a
    mobile to be used as the postmaster; in spec_assign.c, assign the function
    "postmaster" to your mailman MOB.

3.  In db.h, define MAIL_FILE as the mail file's filename.

4.  In db.c, define an int called no_mail to indicate whether the mail
    system is active or not.  Include mail.h in db.c.  Somewhere in the mud's
    bootup sequence, call the mail system's boot-up function like this:

	log("Booting mail system.");
	if (!scan_file()) {
	   log("   Mail system error -- mail system disabled!");
	   no_mail = 1;
	}

5.  In structs.h, define a player ACT flag called PLR_MAILING.  If your MUD
    has a flag to indicate whether or not the player is writing (i.e. on
    the board) change PLR_WRITING (in this file) to the name of your writing
    flag.  If your mud has no writing flag, change (PLR_MAILING | PLR_WRITING)
    (in this file) to simply PLR_MAILING.

6.  Include mail.h in interpreter.c.  As a character is logging in, make
    sure to clear the PLR_MAILING bit (and PLR_WRITING bit, if necessary)
    right after the character is loaded (this happens after the name is
    entered).  This prevents strange things from happening in case a player
    cuts their link while writing mail.

7.  Include mail.h in modify.c.  In the function string_add in modify.c,
    make the following modification:

	if (terminator)	{
           if (!d->connected && (IS_FLAGGED(d->character, PLR_MAILING))) {
              store_mail(d->name, d->character->player.name, *d->str);
	if (*d->str)
	      free(*d->str);
	if (d->str)
	      free(d->str);
	if (d->name)
	      free(d->name);
	      d->name = 0;
	      SEND_TO_Q("Message sent!\n\r", d);
	      if (!IS_NPC(d->character))
	         REMOVE_BIT(d->character->specials.act, PLR_MAILING | PLR_WRITING);
	   }

	   d->str = 0;
	   if (d->connected == CON_EXDSCR) {
	      SEND_TO_Q(MENU, d);
		. . . . .

END OF INSTALLATION INSTRUCTIONS

Note: you may notice some similarity in this mail system to another mail
system written for Alex(?) MUD.  Originally, I had intended to use their
low-level mail system and write my own interface for it, but I later
found that their mail system didn't work (or I wasn't using it correctly :)).
So, I wrote my own low-level system with the same function names so that
the interface I had just written would work with it.

Bottom line -- all code you see below was written by me.

Send comments, bug reports, etc. to jelson@server.cs.jhu.edu

*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "protos.h"

extern struct room_data *world;
extern struct index_data *mob_index;
extern struct obj_data *object_list;
extern int	no_mail;
int	find_name(char *name);
int	_parse_name(char *arg, char *name);

mail_index_type		*mail_index = 0; /* list of recs in the mail file  */
position_list_type 	*free_list = 0;  /* list of free positions in file */
long	file_end_pos = 0; /* length of file */


void	push_free_list(long pos)
{
   position_list_type * new_pos;

   new_pos = (position_list_type * )malloc(sizeof(position_list_type));
   new_pos->position = pos;
   new_pos->next = free_list;
   free_list = new_pos;
}



long	pop_free_list(void)
{
   position_list_type * old_pos;
   long	return_value;

   if ((old_pos = free_list) != 0) {
      return_value = free_list->position;
      free_list = old_pos->next;
if (old_pos)
      free(old_pos);
      return return_value;
   } else
      return file_end_pos;
}




mail_index_type *find_char_in_index(char *searchee)
{
   mail_index_type * temp_rec;

   if (!*searchee) {
      log("SYSERR: Mail system -- non fatal error #1.");
      return 0;
   }

   for (temp_rec = mail_index; 
       (temp_rec && str_cmp(temp_rec->recipient, searchee)); 
       temp_rec = temp_rec->next)
      ;

   return temp_rec;
}




void	write_to_file(void *buf, int size, long filepos)
{
   FILE * mail_file;

   mail_file = fopen(MAIL_FILE, "r+b");

   if (filepos % BLOCK_SIZE) {
      log("SYSERR: Mail system -- fatal error #2!!!");
      no_mail = 1;
      return;
   }

   fseek(mail_file, filepos, SEEK_SET);
   fwrite(buf, size, 1, mail_file);

   /* find end of file */
   fseek(mail_file, 0L, SEEK_END);
   file_end_pos = ftell(mail_file);
   fclose(mail_file);
   return;
}


void	read_from_file(void *buf, int size, long filepos)
{
   FILE * mail_file;

   mail_file = fopen(MAIL_FILE, "r+b");

   if (filepos % BLOCK_SIZE) {
      log("SYSERR: Mail system -- fatal error #3!!!");
      no_mail = 1;
      return;
   }

   fseek(mail_file, filepos, SEEK_SET);
   fread(buf, size, 1, mail_file);
   fclose(mail_file);
   return;
}




void	index_mail(char *raw_name_to_index, long pos)
{
   mail_index_type     * new_index;
   position_list_type  * new_position;
   char	name_to_index[100]; /* I'm paranoid.  so sue me. */
   char	*src;
   int	i;

   if (!raw_name_to_index || !*raw_name_to_index) {
      log("SYSERR: Mail system -- non-fatal error #4.");
      return;
   }

   for (src = raw_name_to_index, i = 0; *src; )
      name_to_index[i++] = tolower(*src++);
   name_to_index[i] = 0;

   if (!(new_index = find_char_in_index(name_to_index))) {
      /* name not already in index.. add it */
      new_index = (mail_index_type * )malloc(sizeof(mail_index_type));
      strncpy(new_index->recipient, name_to_index, NAME_SIZE);
      new_index->recipient[strlen(name_to_index)] = '\0';
      new_index->list_start = 0;

      /* add to front of list */
      new_index->next = mail_index;
      mail_index = new_index;
   }

   /* now, add this position to front of position list */
   new_position = (position_list_type * )malloc(sizeof(position_list_type));
   new_position->position = pos;
   new_position->next = new_index->list_start;
   new_index->list_start = new_position;
}


/* SCAN_FILE */
/* scan_file is called once during boot-up.  It scans through the mail file
   and indexes all entries currently in the mail file. */
int	scan_mail_file(void)
{
   FILE 		   * mail_file;
   header_block_type  next_block;
   int	total_messages = 0, block_num = 0;
   char	buf[100];

   if (!(mail_file = fopen(MAIL_FILE, "r"))) {
      log("Mail file non-existant... creating new file.");
      mail_file = fopen(MAIL_FILE, "w");
      fclose(mail_file);
      return 1;
   }

   while (fread(&next_block, sizeof(header_block_type), 1, mail_file)) {
      if (next_block.block_type == HEADER_BLOCK) {
	 index_mail(next_block.to, block_num * BLOCK_SIZE);
	 total_messages++;
      } else if (next_block.block_type == DELETED_BLOCK)
	 push_free_list(block_num * BLOCK_SIZE);
      block_num++;
   }

   file_end_pos = ftell(mail_file);
   fclose(mail_file);
   sprintf(buf, "   %ld bytes read.", file_end_pos);
   log(buf);
   if (file_end_pos % BLOCK_SIZE) {
      log("SYSERR: Error booting mail system -- Mail file corrupt!");
      log("SYSERR: Mail disabled!");
      return 0;
   }
   sprintf(buf, "   Mail file read -- %d messages.", total_messages);
   log(buf);
   return 1;
} /* end of scan_file */


/* HAS_MAIL */
/* a simple little function which tells you if the guy has mail or not */
int	has_mail(char *recipient)
{
   if (find_char_in_index(recipient))
      return 1;
   return 0;
}



/* STORE_MAIL  */
/* call store_mail to store mail.  (hard, huh? :-) )  Pass 3 pointers..
   who the mail is to (name), who it's from (name), and a pointer to the
   actual message text.			*/

void	store_mail(char *to, char *from, char *message_pointer)
{
   header_block_type  	header;
   data_block_type		data;
   long	last_address, target_address;
   char	*msg_txt = message_pointer;
   char	*tmp;
   int	bytes_written = 0;
   int	total_length = strlen(message_pointer);

   assert(sizeof(header_block_type) == sizeof(data_block_type));
   assert(sizeof(header_block_type) == BLOCK_SIZE);

   if (!*from || !*to || !*message_pointer) {
      log("SYSERR: Mail system -- non-fatal error #5.");
      return;
   }
   memset(&header, 0, sizeof(header)); /* clear the record */
   header.block_type = HEADER_BLOCK;
   header.next_block = LAST_BLOCK;
   strncpy(header.txt, msg_txt, HEADER_BLOCK_DATASIZE);
   strncpy(header.from, from, NAME_SIZE);
   strncpy(header.to, to, NAME_SIZE);
   for (tmp = header.to; *tmp; tmp++)
      *tmp = tolower(*tmp);
   header.mail_time = time(0);
   header.txt[HEADER_BLOCK_DATASIZE] = header.from[NAME_SIZE] = header.to[NAME_SIZE] = '\0';

   target_address = pop_free_list();  /* find next free block */
   index_mail(to, target_address);	   /* add it to mail index in memory */
   write_to_file(&header, BLOCK_SIZE, target_address);

   if (strlen(msg_txt) <= HEADER_BLOCK_DATASIZE)
      return; /* that was the whole message */

   bytes_written = HEADER_BLOCK_DATASIZE;
   msg_txt += HEADER_BLOCK_DATASIZE; /* move pointer to next bit of text */

   /* find the next block address, then rewrite the header
	   to reflect where the next block is.	*/
   last_address = target_address;
   target_address = pop_free_list();
   header.next_block = target_address;
   write_to_file(&header, BLOCK_SIZE, last_address);

   /* now write the current data block */
   memset(&data, 0, sizeof(data)); /* clear the record */
   data.block_type = LAST_BLOCK;
   strncpy(data.txt, msg_txt, DATA_BLOCK_DATASIZE);
   data.txt[DATA_BLOCK_DATASIZE] = '\0';
   write_to_file(&data, BLOCK_SIZE, target_address);
   bytes_written += strlen(data.txt);
   msg_txt += strlen(data.txt);

   /* if, after 1 header block and 1 data block there is STILL
	   part of the message left to write to the file, keep writing
	   the new data blocks and rewriting the old data blocks to reflect
	   where the next block is.  Yes, this is kind of a hack, but if
	   the block size is big enough it won't matter anyway.  Hopefully,
	   MUD players won't pour their life stories out into the Mud Mail
	   System anyway.
 
	   Note that the block_type data field in data blocks is either
	   a number >=0, meaning a link to the next block, or LAST_BLOCK
	   flag (-2) meaning the last block in the current message.  This
	   works much like DOS' FAT.
	*/

   while (bytes_written < total_length) {
      last_address = target_address;
      target_address = pop_free_list();

      /* rewrite the previous block to link it to the next */
      data.block_type = target_address;
      write_to_file(&data, BLOCK_SIZE, last_address);

      /* now write the next block, assuming it's the last.  */
      data.block_type = LAST_BLOCK;
      strncpy(data.txt, msg_txt, DATA_BLOCK_DATASIZE);
      data.txt[DATA_BLOCK_DATASIZE] = '\0';
      write_to_file(&data, BLOCK_SIZE, target_address);

      bytes_written += strlen(data.txt);
      msg_txt += strlen(data.txt);
   }
} /* store mail */




/* READ_DELETE */
/* read_delete takes 1 char pointer to the name of the person whose mail
you're retrieving.  It returns to you a char pointer to the message text.
The mail is then discarded from the file and the mail index. */

char	*read_delete(char *recipient, char *recipient_formatted)
/* recipient is the name as it appears in the index.
   recipient_formatted is the name as it should appear on the mail
   header (i.e. the text handed to the player) */
{
   header_block_type	header;
   data_block_type		data;
   mail_index_type		 * mail_pointer, *prev_mail;
   position_list_type	 * position_pointer;
   long	mail_address, following_block;
   char	*message, *tmstr, buf[200];
   size_t			string_size;

   if (!*recipient || !*recipient_formatted) {
      log("SYSERR: Mail system -- non-fatal error #6.");
      return 0;
   }
   if (!(mail_pointer = find_char_in_index(recipient))) {
      log("SYSERR: Mail system -- post office spec_proc error?  Error #7.");
      return 0;
   }
   if (!(position_pointer = mail_pointer->list_start)) {
      log("SYSERR: Mail system -- non-fatal error #8.");
      return 0;
   }

   if (!(position_pointer->next)) /* just 1 entry in list. */ {
      mail_address = position_pointer->position;
if (position_pointer)
      free(position_pointer);

      /* now free up the actual name entry */
      if (mail_index == mail_pointer) { /* name is 1st in list */
	 mail_index = mail_pointer->next;
if (mail_pointer)
	 free(mail_pointer);
      } else {
	 /* find entry before the one we're going to del */
	 for (prev_mail = mail_index; 
	     prev_mail->next != mail_pointer; 
	     prev_mail = prev_mail->next)
	    ;
	 prev_mail->next = mail_pointer->next;
if (mail_pointer)
	 free(mail_pointer);
      }
   } else {
      /* move to next-to-last record */
      while (position_pointer->next->next)
	 position_pointer = position_pointer->next;
      mail_address = position_pointer->next->position;
if (position_pointer->next)
      free(position_pointer->next);
      position_pointer->next = 0;
   }

   /* ok, now lets do some readin'! */
   read_from_file(&header, BLOCK_SIZE, mail_address);

   if (header.block_type != HEADER_BLOCK) {
      log("SYSERR: Oh dear.");
      no_mail = 1;
      log("SYSERR: Mail system disabled!  -- Error #9.");
      return 0;
   }

   tmstr = asctime(localtime(&header.mail_time));
   *(tmstr + strlen(tmstr) - 1) = '\0';

   sprintf(buf, " * * * * Shadowdale Mail System * * * *\n\r"
       "Date: %s\n\r"
       "  To: %s\n\r"
       "From: %s\n\r\n\r",
       tmstr,
       recipient_formatted,
       header.from);

   string_size = (CHAR_SIZE * (strlen(buf) + strlen(header.txt) + 1));
   message = (char *)malloc(string_size);
   strcpy(message, buf);
   message[strlen(buf)] = '\0';
   strcat(message, header.txt);
   message[string_size - 1] = '\0';
   following_block = header.next_block;

   /* mark the block as deleted */
   header.block_type = DELETED_BLOCK;
   write_to_file(&header, BLOCK_SIZE, mail_address);
   push_free_list(mail_address);

   while (following_block != LAST_BLOCK) {
      read_from_file(&data, BLOCK_SIZE, following_block);

      string_size = (CHAR_SIZE * (strlen(message) + strlen(data.txt) + 1));
      message = (char *)realloc(message, string_size);
      strcat(message, data.txt);
      message[string_size - 1] = '\0';
      mail_address = following_block;
      following_block = data.block_type;
      data.block_type = DELETED_BLOCK;
      write_to_file(&data, BLOCK_SIZE, mail_address);
      push_free_list(mail_address);
   }

   return message;
}




int	mail_ok(struct char_data *ch)
{
   if (no_mail) {
      send_to_char("Sorry, the message system is having technical difficulties.\n\r",
          ch);
      return 0;
   }

   return 1;
}




struct char_data *find_mailman(struct char_data *ch)
/* find the postmaster in the room where ch is standing */
{
   struct char_data *mailman, *temp;

   if (!mail_ok(ch))
      return 0;
#if 0
   for (temp = world[ch->in_room].people, mailman = 0; 
       (temp) && (!mailman); temp = temp->next_in_room)
      if (IS_NPC(temp))
	 if (mob_index[temp->nr].func == PostMaster)
	    mailman = temp;
#else
   mailman = FindMobInRoomWithFunction(ch->in_room, PostMaster);
#endif
   
   if (!mailman)
      send_to_char("Whoa!  Buggy post office.  Please report this.  Error #10.\n\r", ch);

   return mailman;
}


void	postmaster_send_mail(struct char_data *ch, int cmd, char *arg)
{
   struct char_data *mailman;
   char	buf[200], recipient[100], *tmp;


   if (!(mailman = find_mailman(ch)))
      return;

   if (GetMaxLevel(ch) < MIN_MAIL_LEVEL) {
      sprintf(buf, "$n tells you, 'Sorry, you have to be level %d to send mail!'",
          MIN_MAIL_LEVEL);
      act(buf, FALSE, mailman, 0, ch, TO_VICT);
      return;
   }

   if (!*arg) { /* you'll get no argument from me! */
      act("$n tells you, 'Who did you want me to send this to?'",
          FALSE, mailman, 0, ch, TO_VICT);
      return;
   }

   if (GET_GOLD(ch) < STAMP_PRICE) {
      sprintf(buf, "$n tells you, 'It will costs %d coins to deliever this.'\n\r"
          "$n tells you, '...which I see you can't afford.'",
          STAMP_PRICE);
      act(buf, FALSE, mailman, 0, ch, TO_VICT);
      return;
   }

   _parse_name(arg, recipient);

   if (find_name(recipient) < 0) {
      act("$n tells you, 'Never heard of that person!'",
          FALSE, mailman, 0, ch, TO_VICT);
      return;
   }

   for (tmp = recipient; *tmp; tmp++)
      *tmp = tolower(*tmp);

   act("$n starts to write a note.", TRUE, ch, 0, 0, TO_ROOM);
   sprintf(buf, "$n tells you, 'I'll take %d coins for the delivery.'\n\r"
       "$n tells you, 'Write your message, use @ when done.'",
       STAMP_PRICE);
   act(buf, FALSE, mailman, 0, ch, TO_VICT);
   GET_GOLD(ch) -= STAMP_PRICE;
   SET_BIT(ch->specials.act, PLR_MAILING);

   ch->desc->name = (char *)strdup(recipient);
   ch->desc->str = (char **)malloc(sizeof(char *));
   *(ch->desc->str) = 0;
   ch->desc->max_str = MAX_MAIL_SIZE;
}


void	postmaster_check_mail(struct char_data *ch, int cmd, char *arg)
{
   struct char_data *mailman;
   char	buf[200], recipient[100], *tmp;


   if (!(mailman = find_mailman(ch)))
      return;

   _parse_name(GET_NAME(ch), recipient);

   for (tmp = recipient; *tmp; tmp++)
      *tmp = tolower(*tmp);

   if (has_mail(recipient))
      sprintf(buf, "$n tells you, 'You DO have waiting mail.'");
   else
      sprintf(buf, "$n tells you, 'Sorry, you DON'T have any messages waiting.'");
   act(buf, FALSE, mailman, 0, ch, TO_VICT);
}


void	postmaster_receive_mail(struct char_data *ch, int cmd, char *arg)
{
   struct char_data *mailman;
   char	buf[200], recipient[100], *tmp;
   struct obj_data *tmp_obj;


   if (!(mailman = find_mailman(ch)))
      return;

   _parse_name(GET_NAME(ch), recipient);

   for (tmp = recipient; *tmp; tmp++)
      *tmp = tolower(*tmp);

   if (!has_mail(recipient)) {
      sprintf(buf, "$n tells you, 'Sorry, you don't have any messages waiting.'");
      act(buf, FALSE, mailman, 0, ch, TO_VICT);
      return;
   }

   while (has_mail(recipient)) {
      CREATE(tmp_obj, struct obj_data, 1);
      clear_object(tmp_obj);

      tmp_obj->name = strdup("mail paper letter");
      tmp_obj->short_description = strdup("a piece of mail");
      tmp_obj->description = strdup("Someone has left a piece of mail here.");

      tmp_obj->obj_flags.type_flag = ITEM_NOTE;
      tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
      tmp_obj->obj_flags.weight = 1;
      tmp_obj->obj_flags.cost = 30;
      tmp_obj->obj_flags.cost_per_day = 10;

      tmp_obj->action_description = read_delete(recipient, GET_NAME(ch));
      if (!tmp_obj->action_description)
	 tmp_obj->action_description = strdup("Mail system error - please report.  Error #11.\n\r");

      tmp_obj->next = object_list;
      object_list = tmp_obj;

      obj_to_char(tmp_obj, ch);

      tmp_obj->item_number = -1;

      act("$n gives you a piece of mail.", FALSE, mailman, 0, ch, TO_VICT);
      act("$N gives $n a piece of mail.", FALSE, ch, 0, mailman, TO_ROOM);
   }
}


