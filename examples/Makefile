CONTIKI = $(DROPBOX_HOME)/neighbor-discovery/nddev-contiki
CONTIKI_PROJECT = example-accsl
ACCSLDIR=..
PROJECTDIRS += $(ACCSLDIR)

ifndef TARGET
TARGET=sky
endif

TIMEDATE=`date +%Y%m%d-%H%M`
#...Deterministic Neighbor Discovery with Epidemics...
#CONTIKI_SOURCEFILES += dnde-neigh-G.c generic-dnde-G.c

#..Group Merge using multi channel..
#CONTIKI_SOURCEFILES += acc-nodesinfo.c acc-sl.c

#DEFINES=NETSTACK_MAC=nullmac_driver,NETSTACK_RDC=accsl_driver,CC2420_CONF_AUTOACK=0
#DEFINES+=CONF_CHANNEL_SIZE=1,CONF_NETWORK_SIZE=105
#DEFINES+=HOPCOUNT_FILTER_NDISC=2,CONF_ND_DUTYCYCLE=5


CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
CONTIKIDIRS += $(ACCSLDIR) $(ACCSLDIR)/code
PROJECT_SOURCEFILES += uart1-putchar.c acc-nodesinfo-v2.c  acc-sl.c

DEFINES=NETSTACK_MAC=nullmac_driver,CC2420_CONF_AUTOACK=0
DEFINES+=CONF_CHANNEL_SIZE=1,CONF_NETWORK_SIZE=20
DEFINES+=HOPCOUNT_FILTER_NDISC=2,CONF_ND_DUTYCYCLE=5


DEFINES+=IN_INDRIYA=1
#DEFINES+=IN_TWIST=1
DEFINES+=CONF_ASYMMETRIC=1


install-accsl:
	make clear
	make example-accsl.upload


clear:
	rm -rf *.sky *.exe symbols.* obj_* *~
	make clean

motelist:
	MOTES=$(shell $(MOTELIST) 2>&- | grep USB | \
	cut -f 4 -d \  | \
	perl -ne 'print $$1 . " " if(m-(/dev/\w+)-);')

reload:
	make sky-reset


include $(CONTIKI)/Makefile.include

