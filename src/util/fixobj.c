#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"

void main(int argc, char *argv[])
{
        FILE    *f,*fo;
        char    b[512];
        char    name[80];
        long    v0,v1,v2;
        int     i;

 f=fopen("tinyworld.obj","rt"); if(f==NULL) { perror("1"); exit(1); }
 fo=fopen("tinyworld.obj.new","wt"); if(fo==NULL) { perror("2"); exit(1); }

 for(;;) {
        fgets(b,512,f); fputs(b,fo);
  next:
        if(b[0]=='$') exit(0);
        if(b[0]=='#') {
                i=atol(b+1);
                do {fgets(b,512,f); fputs(b,fo); }while(strchr(b,'~')==NULL);  /*namelist*/
                strcpy(name,b);
                do {fgets(b,512,f); fputs(b,fo); }while(strchr(b,'~')==NULL);  /*short desc*/
                do {fgets(b,512,f); fputs(b,fo); }while(strchr(b,'~')==NULL);  /*long desc*/
                do {fgets(b,512,f); fputs(b,fo); }while(strchr(b,'~')==NULL);  /*action*/
                fgets(b,512,f);                                 /*type, wearflag, extraflag*/
                sscanf(b,"%ld %ld %ld",&v0,&v1,&v2);
/* well, lets do something      */
                if(v1&ITEM_ANTI_FIGHTER) {
                        v1 |= ITEM_ANTI_PALADIN;
                        v1 |= ITEM_ANTI_RANGER;
                        v1 |= ITEM_ANTI_MONK;
                }
                if(v1&ITEM_ANTI_THIEF) {
                        v1 |= ITEM_ANTI_PSI;
                        v1 |= ITEM_ANTI_MONK;
                        v1 |= ITEM_ANTI_DRUID;
                }
                if(v1&ITEM_ANTI_CLERIC) {
                        v1 |= ITEM_ANTI_MONK;
                        v1 |= ITEM_ANTI_DRUID;
                }
                if(v1&ITEM_ANTI_MAGE) {
                        v1 |= ITEM_ANTI_MONK;
                }

                fprintf(fo,"%ld %ld %ld\n",v0,v1,v2);
                fgets(b,512,f);                                 /*<value 0> <value 1> <value 2> <value 3>*/
                fputs(b,fo);
                fgets(b,512,f);                                 /*<weight> <value> <cost/day>*/
                fputs(b,fo);
        }
 }

}
