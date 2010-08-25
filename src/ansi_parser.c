/*
*** DaleMUD	ANSI_PARSER.C
***		Parser ansi colors for act();
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "ansi.h"
#include "protos.h"

extern long SystemFlags;

/*	
    $CMBFG, where M is modier, B is back group color and FG is fore 
    $C0001 would be normal, black back, red fore.
    $C1411 would be bold, blue back, light yellow fore 
*/    

char *ansi_parse(char *code )
{
  static char m[MAX_STRING_LENGTH]; /* increased from 255 to MAX 2-18 msw */
  char b[128],f[128];

if (!code)
    return(""); /* changed this from NULL to "" 2-18 msw */

			/* do modifier */
  	switch(code[0]) {
  	 case '0':sprintf(m,"%s",MOD_NORMAL);
  	 	break;
  	 case '1':sprintf(m,"%s",MOD_BOLD);
  	 	break;
  	 case '2':sprintf(m,"%s",MOD_FAINT);
  	 	break;
			/* not used in ansi that I know of */
  	 case '3':sprintf(m,"%s",MOD_NORMAL);
  	 	break;
  	 case '4':sprintf(m,"%s",MOD_UNDERLINE);
  	 	break;
  	 case '5': sprintf(m,"%s",MOD_BLINK);
  	 	break;

  	 case '6': sprintf(m,"%s",MOD_REVERSE);
  	 	break;

  	 default: sprintf(m,"%s",MOD_NORMAL);
  	 	break;
  	}

	/* do back ground color */
  	switch(code[1]) {
  		case '0': sprintf(b,"%s",BK_BLACK);
  	 		break;
  		case '1': sprintf(b,"%s",BK_RED);
  	 		break;
  		case '2': sprintf(b,"%s",BK_GREEN);
  	 		break;
  		case '3': sprintf(b,"%s",BK_BROWN);
  	 		break;
  		case '4': sprintf(b,"%s",BK_BLUE);
  	 		break;
  		case '5': sprintf(b,"%s",BK_MAGENTA);
  	 		break;
  		case '6': sprintf(b,"%s",BK_CYAN);
  	 		break;
  		case '7': sprintf(b,"%s",BK_LT_GRAY);
  	 		break;
  		default:sprintf(b,"%s",BK_BLACK);
  	 		break;
  	}
  	
	/* do foreground color */
  	switch(code[2]) {     
	 case '0':  	switch(code[3]) {  		/* 00-09 */
	 			case '0': sprintf(f,"%s",FG_BLACK);
  	 				break;
	 			case '1': sprintf(f,"%s",FG_RED);
  	 				break;
	 			case '2': sprintf(f,"%s",FG_GREEN);
  	 				break;
	 			case '3': sprintf(f,"%s",FG_BROWN);
  	 				break;
	 			case '4': sprintf(f,"%s",FG_BLUE);
  	 				break;
	 			case '5': sprintf(f,"%s",FG_MAGENTA);
  	 				break;
	 			case '6': sprintf(f,"%s",FG_CYAN);
  	 				break;
	 			case '7': sprintf(f,"%s",FG_LT_GRAY);
  	 				break;
	 			case '8': sprintf(f,"%s",FG_DK_GRAY);
  	 				break;
	 			case '9': sprintf(f,"%s",FG_LT_RED);
  	 				break;
	 			default: sprintf(f,"%s",FG_DK_GRAY);
  	 				break;
  				} break;

  	 case '1':  	switch(code[3]) {  		/* 10-15 */
  	 			case '0': sprintf(f,"%s",FG_LT_GREEN);
  	 				break;
  	 			case '1': sprintf(f,"%s",FG_YELLOW);
  	 				break;
  	 			case '2': sprintf(f,"%s",FG_LT_BLUE);
  	 				break;
  	 			case '3': sprintf(f,"%s",FG_LT_MAGENTA);
  	 				break;
  	 			case '4': sprintf(f,"%s",FG_LT_CYAN);
  	 				break;
  	 			case '5': sprintf(f,"%s",FG_WHITE);
  	 				break;
  	 			default: sprintf(f,"%s",FG_LT_GREEN);
  	 				break;
  				} break;

	default : sprintf(f,"%s",FG_LT_RED);
  	 	  break;  				
  	}
  	

   strcat(m,b); /* add back ground */
   strcat(m,f); /* add foreground */

 return(m);
}
