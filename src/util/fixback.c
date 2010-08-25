/* add ITEM_WEAR_BACK to all "bag" and "backpack" objects.. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void main(int argc, char *argv[])
{
        FILE    *f,*fo;
        char    b[512];
        char    name[120];
 static long    t0,t1,t2;
        int     i;

 f=fopen("tinyworld.obj","rt"); if(f==NULL) { perror("1"); exit(1); }
 fo=fopen("tinyworld.obj.new","wt"); if(f==NULL) { perror("2"); exit(1); }

 for(;;) {
        fgets(b,512,f); fputs(b,fo);
        if(b[0]=='$') exit(0);
        if(b[0]=='#') {
                do {fgets(b,512,f); fputs(b,fo); } while(strchr(b,'~')==NULL);  /*namelist*/
                strcpy(name,b);
                do {fgets(b,512,f); fputs(b,fo); }while(strchr(b,'~')==NULL);  /*short desc*/
                do {fgets(b,512,f); fputs(b,fo); }while(strchr(b,'~')==NULL);  /*long desc*/
                do {fgets(b,512,f); fputs(b,fo); }while(strchr(b,'~')==NULL);  /*action*/
                fgets(b,512,f);                                 /*type, extraflag, wearflag*/
                sscanf(b,"%ld %ld %ld",&t0,&t1,&t2);
                if(t0==15) { /* item_container */
                        for(i=0;i<strlen(name);i++) name[i]=tolower(name[i]);
                        if(strstr(name,"backpack") || strstr(name,"bag")) {
                                printf("fixing item %s", name);
                                t2 |= 0x20000l;
                        }
                }
                fprintf(fo,"%ld %ld %ld\n", t0,t1,t2);
        }
 }
}
