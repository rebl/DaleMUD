##  these are compile time constructs, that affect the running of the ga
## See farther down for explaination of these and more! msw
##
CPPFLAGS = -DPREP_SPELLS -DSUN_MALLOC -DLOG_ALL
#
## -DPREP_SPELLS -DDEBUG_LOG
## -DDEBUG -DMALLOC_DEBUG -DDEBUG_LOG
## -DCLEAN_AT_BOOT -DCHECK_RENT_INACTIVE
## -DSTRANGE_WACK -DBLOCK_WRITE 
## -DUSE_DIMD
#
## The compiler your gonna use to compile this with...
##
CC=gcc
##
LD=$(CC)
##
##  these are architecture dependent flags
TITANFLAGS = -g -43 -vector_c -safe=ptrs -DTITAN
MYFLAGS = -ggdb3 -m486 -mno-fp-ret-in-387 -DNETBSD
##
CFLAGS = $(MYFLAGS) $(CPPFLAGS)
##
## **********************************************************
##	COMPILE TIME DEFINES, CPPFLAGS!
##
##  For lower exp loss at lower levels -DNEWEXP
##
##  For level loss for really low exp -DLEVEL_LOSS
##
##  To make some optimizations, you need to add things to the line above.
##  to use hash tables for rooms  (More cpu, less storage), add
##  -DHASH    
##
##  To get rid of tracking (lower cpu)
##  -DNOTRACK
##
##  To limit items above a specific rent cost
##  -DLIMITED_ITEMS
##
##  To force new players to be authorized
##  -DPLAYER_AUTH
##
##  To add some new debugging stuff (mostly limited to malloc_debug)
##  -DDEBUG
##  To add some commands for locking out specific hosts
##  -DSITELOCK
##
## To lock out oak.grove.iup.edu from boot time.
## -DLOCKGROVE
##
##
##  To modify some commands to make item duplication near impossible:
##  -DNODUPLICATES
##
##  To have each 58+ god restricted to one site, or set of sites
##  -DIMPL_SECURITY
##
##  To use BSD memory allocation/deallocation routines instead of the
##  native memory routines:
##  -DKLUDGE_MEM
##
##  If your system doesn't have built in functions of strdup() and
##  strstr(), then:
##  -DKLUDGE_STRING
##
##  If you want to use a faster mana/hit/move regen system.
##  -DNEWGAIN
##
##  If you want to save every room and mob items use this.
##  -DSAVEWORLD
##
##  If you want to use the Quest to Gain system use this. Not all classes
##  -DQUEST_GAIN
##
## If you want to use a lower gold ration for your game use this
## -DLOW_GOLD
##
## - DUSE_EGOS (check ego on give and gets )
##
##
## -DLOG_MOB
##		LOG_MOB logs all mob command sent to command_interpreter
##
##	-DLOG_DEBUG
##		This will add a bit more info to the logs, I use it to try
##		and track down impossible bugs.
##
## -DCLEAN_AT_BOOT
##		Forces the mud to remove outdated or inactive players and
##		demote inactive gods a boot up. Slows boot process down a
##		bit.
##
## -DSTRANGE_WACK
##	When used in combination with CLEAN_AT_BOOT it will clear out corrupted
##	user files. Do this once or twice a month if needed, otherwise do not
##	use it.
##
##
## -DZONE_COMM_ONLY
##		Enabling this will force all communications to be limited to
##		the zone that person is in. Making gossip/auction/tell NOT
##		GLOBAL. Tell is limited to people in the zone you are currently
##		standing in. This does NOT include SHOUT.
##
## -DLAG_MOBILES		
##		This define should make mobiles lag with commands as
##		pc's do. I.e. bashed pause, etc...
##
## -DPREVENT_PKILL
##		This define will add in a few checks in fight.c to keep
##		pc's from killing each other, willing or Un-willing.
##
## -DCHECK_RENT_INACTIVE
##		Check user inactivity against RENT_INACTIVE, if greater, wack
##		the rent file. RENT_INACTIVE located in db.c
##
## -DPERSONAL_PERM_LOCKOUTS
##		My personal annoying people I lock out perminatly. You can add
##		whomever is annoying in there.
##
## -DNEW_RENT
##		Charge 100 coins per day of rent, regardless of item types.
##
##

HEADERFILES = structs.h utils.h comm.h interpreter.h db.h
##
OBJS = comm.o act.comm.o act.info.o act.move.o act.obj1.o act.obj2.o \
	act.off.o act.other.o act.social.o act.wizard.o handler.o \
	db.o interpreter.o utility.o spec_assign.o shop.o limits.o mobact.o \
	fight.o modify.o weather.o spells1.o spells2.o spell_parser.o \
	reception.o constants.o spec_procs.o signals.o board.o magic.o \
	magic2.o skills.o Opinion.o Trap.o magicutils.o multiclass.o hash.o \
	Sound.o Heap.o spec_procs2.o magic3.o security.o spec_procs3.o \
        create.o bsd.o parser.o ansi_parser.o mail.o mindskills1.o \
	mind_use1.o create.mob.o create.obj.o dimd.o sunmalloc.o

SRCS = comm.c act.comm.c act.info.c act.move.c act.obj1.c act.obj2.c \
	act.off.c act.other.c act.social.c act.wizard.c handler.c \
	db.c interpreter.c utility.c spec_assign.c shop.c limits.c mobact.c \
	fight.c modify.c weather.c spells1.c spells2.c spell_parser.c \
	reception.c constants.c spec_procs.c signals.c board.c magic.c \
	magic2.c skills.c Opinion.c Trap.c magicutils.c multiclass.c hash.c \
	Sound.c Heap.c spec_procs2.c magic3.c security.c spec_procs3.c \
        create.c bsd.c parser.c ansi_parser.c mail.o mind_use1.c \
	mindskills1.c create.mob.c create.obj.c dimd.c sunmalloc.c

all : dmserver
	chmod g+rw *.o

dmserver : ${OBJS}

## if you want to add debugging stuff, uncomment the following, and comment
##  out the line after that
##	$(LD) -o $@ -g ${OBJS} /local/lib/gnu/gcc-gnulib /usr/lib/debug/malloc.o
##	$(LD) -o $@ -g ${OBJS} /usr/lib
	$(LD) -o $@ ${OBJS}
##	$(LD) -o $@ -g ${OBJS} -L/usr/local/gnu/lib/gcc-lib/sun4-sunos4.1.1/2.3

depend :
	makedepend ${SRCS}



# DO NOT DELETE THIS LINE -- make depend depends on it.

comm.o: protos.h
act.comm.o: protos.h
act.move.o: protos.h
act.off.o: protos.h
act.obj1.o: protos.h
act.obj2.o: protos.h
act.info.o: protos.h
act.other.o: protos.h
act.social.o: protos.h
act.wizard.o: protos.h
handler.o: protos.h
db.o: protos.h
interpreter.o: protos.h
utility.o: protos.h
spec_assign.o: protos.h
shop.o: protos.h
limits.o: protos.h
mobact.o: protos.h
fight.o: protos.h
modify.o: protos.h
weather.o: protos.h
spells1.o: protos.h
spells2.o: protos.h
spell_parser.o: protos.h
reception.o: protos.h
constants.o: protos.h
spec_procs.o: protos.h
signals.o: protos.h
board.o: protos.h
magic.o: protos.h
magic2.o: protos.h
magic3.o: protos.h
magicutils.o: protos.h
multiclass.o: protos.h
skills.o: protos.h
Opinion.o: protos.h
Trap.o: protos.h
hash.o: protos.h
Sound.o: protos.h
Heap.o: protos.h
spec_procs2.o: protos.h
spec_procs3.o: protos.h
security.o:
create.o: protos.h
bsd.o:
parser.o: protos.h
ansi_parser.o: protos.h
mail.o:protos.h
mind_use1.o: protos.h
mindskills1.o:protos.h
create.mob.o:protos.h
create.obj.o:protos.h
dimd.o:
sunmalloc.o: