/*
 * this program will doadd the extra parameter onto room settings used for
 * the new twist/turn open type exits we need.
 */


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

 f=fopen("tinyworld.wld","rt"); if(f==NULL) { perror("1"); exit(1); }
 fo=fopen("tinyworld.wld.new","wt"); if(f==NULL) { perror("2"); exit(1); }

 for(;;) {
        fgets(b,512,f); fputs(b,fo);
        if(*b=='$') exit(0);
        if(*b=='#') 

#if 0
if(strncmp(b+1,"16938",5) == 0){
                printf("room 16938.. :|\n");
        /* Cel, it's AMAZINGLY stupid, but i not wanna write something else :( */

fprintf(fo,"Count's meeting place~\n");
fprintf(fo,"     You are in a large junction of the sewers, this appears to be were Count\n");
fprintf(fo,"Boarish meets with his wererat friends.  There are sewer pipes heading in all\n");
fprintf(fo,"directions except west, where you can see the cellar.\n");
fprintf(fo,"~\n");
fprintf(fo,"169 9 0\n");
fprintf(fo,"D0\n");
fprintf(fo,"The sewer pipes are too small for you to fit through.\n");
fprintf(fo,"~\n");
fprintf(fo,"~\n");
fprintf(fo,"0 0 -1 -1\n");
fprintf(fo,"D1\n");
fprintf(fo,"The sewer pipes are too small for you to fit through.\n");
fprintf(fo,"~\n");
fprintf(fo,"~\n");
fprintf(fo,"0 0 -1 -1\n");
fprintf(fo,"D2\n");
fprintf(fo,"The sewer pipes are too small for you to fit through.\n");
fprintf(fo,"~\n");
fprintf(fo,"~\n");
fprintf(fo,"0 0 -1 -1\n");
fprintf(fo,"D3\n");
fprintf(fo,"To the west you can see the cellar.\n");
fprintf(fo,"~\n");
fprintf(fo,"~\n");
fprintf(fo,"0 0 16937 -1\n");
fprintf(fo,"D4\n");
fprintf(fo,"The sewer pipe above you is too small for you to fit through.\n");
fprintf(fo,"~\n");
fprintf(fo,"~\n");
fprintf(fo,"0 0 -1 -1\n");
fprintf(fo,"D5\n");
fprintf(fo,"The sewer pipe below you is too small for you to fit through.\n");
fprintf(fo,"~\n");
fprintf(fo,"~\n");
fprintf(fo,"0 0 -1 -1\n");
fprintf(fo,"E\n");
fprintf(fo,"pipe pipes~\n");
fprintf(fo,"The sewer pipes are fairly small, you doubt that you could fit through them.\n");
fprintf(fo,"~\n");
                while(1) {
                  fgets(b,512,f); if(*b=='S'&&strlen(b)<3) break;
                }
}
#endif

      if(*b=='D' && b[1]>='0' && b[1]<='9' && strlen(b)<4) {
                do {fgets(b,512,f); fputs(b,fo); }while(strchr(b,'~')==NULL); 
                do {fgets(b,512,f); fputs(b,fo); }while(strchr(b,'~')==NULL); 
                fgets(b,512,f);
                sscanf(b,"%ld %ld %ld",&t0,&t1,&t2);
                fprintf(fo,"%ld %ld %ld -1\n", t0,t1,t2);
                if(strchr(b,'E')!=0) {
                 printf("another strange room.. fixing\n");
                 fprintf(fo,"E\n");
                }
        }
 }
}
