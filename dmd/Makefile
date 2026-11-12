#
# Makefile - OrbitDMD (Raspberry Pi 3, sans Linux, sur Circle)
#

CIRCLEHOME = ../circle

OBJS	= main.o kernel.o config.o matrix.o canvas.o font5x7.o gifanim.o \
	  rtc.o localtime.o clock.o testpattern.o remote.o network.o webserver.o \
	  fileutil.o gifmanager.o configedit.o netutil.o settings.o version.o latency.o stbimpl.o clockstyle.o clockrender.o weatherdata.o weathericons.o weather.o gifmarks.o messages.o schedule.o hapayload.o ha.o lang.o dmdapp.o

LIBS	= $(CIRCLEHOME)/addon/wlan/hostap/wpa_supplicant/libwpa_supplicant.a \
	  $(CIRCLEHOME)/addon/wlan/libwlan.a \
	  $(CIRCLEHOME)/addon/fatfs/libfatfs.a \
	  $(CIRCLEHOME)/addon/SDCard/libsdcard.a \
	  $(CIRCLEHOME)/lib/usb/libusb.a \
	  $(CIRCLEHOME)/lib/input/libinput.a \
	  $(CIRCLEHOME)/lib/net/libnet.a \
	  $(CIRCLEHOME)/lib/sched/libsched.a \
	  $(CIRCLEHOME)/lib/fs/libfs.a \
	  $(CIRCLEHOME)/lib/libcircle.a

# numero de version : fichier VERSION a la racine du projet (affiche sur la page web)
ORBITDMD_VERSION := $(shell cat ../VERSION)
DEFINE += -DORBITDMD_VERSION=\"$(ORBITDMD_VERSION)\"

include $(CIRCLEHOME)/Rules.mk

# interface web (pages, style, script, logo) integree au programme
webserver.o: webfiles.h

# langues de l'interface web : web/lang-xx.js depuis outils/traductions/web.txt
LANG_JS = $(foreach l,en nl es ru de it pt pl uk tr cs,web/lang-$(l).js)
$(LANG_JS): $(wildcard ../outils/traductions/web*.txt) ../outils/langues.py
	py -3.12 -I ../outils/langues.py ../outils/traductions/web.txt web

webfiles.h: $(wildcard web/*) $(LANG_JS) ../outils/web2h.py
	py -3.12 -I ../outils/web2h.py web/fichiers.txt webfiles.h

# date de compilation toujours a jour (affichee sur la page web)
version.o: FORCE
FORCE:

-include $(DEPS)
