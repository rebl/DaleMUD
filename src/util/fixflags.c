
/* this changes old style wld's to use the new bitwise door types */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "structs.h"

void main(int argc, char *argv[])
{
        FILE    *f,*fo;
        char    b[512];
        char    name[120];
 static long    t0,t1,t2,t3;
        int     i;

 f=fopen("tinyworld.wld","rt"); if(f==NULL) { perror("1"); exit(1); }
 fo=fopen("tinyworld.wld.new","wt"); if(f==NULL) { perror("2"); exit(1); }

 for(;;) {
        fgets(b,512,f); fputs(b,fo);
        if(*b=='$') exit(0);
        if(*b=='#') i=atoi(b+1); /* if(strncmp(b+1,"16938",5) == 0) */ 
        if(*b=='D' && b[1]>='0' && b[1]<='9' && strlen(b)<4) {
                do {fgets(b,512,f); fputs(b,fo); }while(strchr(b,'~')==NULL); 
                do {fgets(b,512,f); fputs(b,fo); }while(strchr(b,'~')==NULL); 
                fgets(b,512,f);
                sscanf(b,"%ld %ld %ld %ld",&t0,&t1,&t2,&t3);
                switch(t0) {
                        case 0: break; /* strange, eh? */
                        case 1: t0 = EX_ISDOOR;    break;
                        case 2: t0 = EX_ISDOOR | EX_PICKPROOF; break;
                        case 3: t0 = EX_ISDOOR | EX_SECRET; break;
                        case 4: t0 = EX_ISDOOR | EX_SECRET | EX_PICKPROOF; break;
                        case 5: t0 = EX_CLIMB; break;
                        case 6: t0 = EX_CLIMB | EX_ISDOOR; break;
                        case 7: t0 = EX_CLIMB | EX_ISDOOR | EX_PICKPROOF; break;
                        default:
                                printf("invalid exit_flag %ld in room %d\r\n",t0,i);
                                printf(" (it usually mean some shit in room description.. check it! )\r\n");
                                t0 = 0; break;
                }
                fprintf(fo,"%ld %ld %ld %ld\n", t0,t1,t2,t3);
        }
 }

rename("tinyworld.wld","tinyworld.wld.old");
rename("tinyworld.wld.new","tinyworld.wld");
printf("old .WLD file in tinyworld.wld.old\r\n");

}
