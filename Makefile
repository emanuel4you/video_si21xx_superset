PATH := $(HOME)/dreambox/amlogic-linux/arm-gnu-toolchain-12.2.rel1-x86_64-aarch64-none-elf/bin:$(PATH)

COMPILER := aarch64-none-elf-

PWD := $(shell pwd)
KERNEL := $(HOME)/dreambox/amlogic-linux/linux-90ca7a874a9093e500cc6147cecd85ad2e6a2852
MODULE := $(HOME)/dreambox/amlogic-linux/video_si21xx_superset

I2C_DIR := SKYWORKS_SUPERSET/Si_I2C
DTV_DIR := SKYWORKS_SUPERSET/Si2183
SAT_DIR := SKYWORKS_SUPERSET/SAT
TER_DIR := SKYWORKS_SUPERSET/TER

#=============================================================
#include path for silabs_superset internal files
#-------------------------------------------------------------
ccflags-y+= -I$(MODULE)
ccflags-y+= -I$(MODULE)/$(I2C_DIR)
ccflags-y+= -I$(MODULE)/$(DTV_DIR)
#=============================================================
#=============================================================
# General compilation flags for silabs_superset
#-------------------------------------------------------------
ccflags-y+= -Wall
ccflags-y+= -DSILABS_SUPERSET
ccflags-y+= -DFRONT_END_COUNT=2
#ccflags-y+= -DCONFIG_MACROS -DSILABS_EVB_MACROS
#ccflags-y+= -DTS_CROSSBAR
#=============================================================
#=============================================================
# +++<porting>+++ possible FWs to load in demodulators
# (depending on part number)
# Several values can be used together, for compatibility
# with several versions.
# The more FW files you include, the bigger the code size
# Check Si2183_L2_API.c file for possible flags
# (between change log and first function implementation)
#-------------------------------------------------------------
ccflags-y+= -DSi2180_A55_COMPATIBLE
ccflags-y+= -DSi2180_A50_COMPATIBLE
ccflags-y+= -DSi2183_B60_COMPATIBLE
ccflags-y+= -DSi2183_B5A_COMPATIBLE
ccflags-y+= -DSi2183_A55_COMPATIBLE
ccflags-y+= -DSi2183_A50_COMPATIBLE
# ---<porting>--- End of FWs selection
#=============================================================
#=============================================================



# ---<porting>--- End of FWs selection
#=============================================================
#=============================================================
# General SILABS SUPERSET source objects
#-------------------------------------------------------------
OBJFILES += $(I2C_DIR)/Silabs_L0_Connection.o
OBJFILES += $(DTV_DIR)/Si2183_L1_API.o
OBJFILES += $(DTV_DIR)/Si2183_L1_Commands.o
OBJFILES += $(DTV_DIR)/Si2183_L1_Properties.o
OBJFILES += $(DTV_DIR)/Si2183_L2_API.o
OBJFILES += $(DTV_DIR)/SiLabs_API_L3_Wrapper_TS_Crossbar.o
OBJFILES += $(DTV_DIR)/SiLabs_API_L3_Wrapper.o
#OBJFILES += $(DTV_DIR)/SiLabs_API_L3_Config_Macros.o
#OBJFILES += $(DTV_DIR)/SiLabs_API_L3_Console.o
#=============================================================
#=============================================================
# TER compilation flags
#-------------------------------------------------------------
# +++<porting>+++ TER tuner selection
# (use 'none' for no_TER compilation)
# Check TER/SiLabs_TER_Tuner_API.c/SiLabs_TER_Tuner_SW_Init
# for possible values
# Also add the corresponding TER tuner code under TER/
# the current TER tuner being selected in the SW configuration
# using SiLabs_API_Select_TER_Tuner
#-------------------------------------------------------------
TER_TUNER=Si2141
TER_TUNER_2=Si2144

ccflags-y+=-DSi2141_TUNER_COUNT=2
ccflags-y+=-DSi2144_TUNER_COUNT=2
# ---<porting>--- End of TER tuner selection
#-------------------------------------------------------------

ifneq (none, $(TER_TUNER))
#-------------------------------------------------------------
# +++<porting>+++TER standards selection
#(comment unused standards)
# Restrictions related to the TER standards selection:
#If DVB-T2 support is required, DVB-T support is also mandatory
#If DVB-C2 support is required, DVB-C support is also mandatory
#If MCNS support is required, DVB-C support is also mandatory
ccflags-y+=-DTERRESTRIAL_FRONT_END

ccflags-y+=-DDEMOD_DVB_T
# for DVBT
ccflags-y+=-DDEMOD_DVB_T2 
# for DVBT2
#ccflags-y+=-DDEMOD_DVB_C # for DVBC_ANNEX_AC
ccflags-y+=-DDEMOD_DVB_C2 
# for DVBC2
#ccflags-y+=-DDEMOD_MCNS # for DVBC_ANNEX_B
#ccflags-y+=-DDEMOD_ISDB_T # for ISDBT
# ---<porting>--- End of TER standards selection
#-------------------------------------------------------------
#include path for silabs_superset TER files
ccflags-y+= -I$(MODULE)/$(TER_DIR)
ccflags-y+= -I$(MODULE)/$(TER_DIR)/$(TER_TUNER)
# TER compilation flags
ccflags-y+= -DTERRESTRIAL_FRONT_END -DTER_TUNER_SILABS -DTER_TUNER_$(TER_TUNER)
#object files for TER tuner
OBJFILES += $(TER_DIR)/$(TER_TUNER)/$(TER_TUNER)_L1_API.o
OBJFILES += $(TER_DIR)/$(TER_TUNER)/$(TER_TUNER)_L1_Commands.o
OBJFILES += $(TER_DIR)/$(TER_TUNER)/$(TER_TUNER)_L1_Properties.o
OBJFILES += $(TER_DIR)/$(TER_TUNER)/$(TER_TUNER)_Properties_Strings.o
OBJFILES += $(TER_DIR)/$(TER_TUNER)/$(TER_TUNER)_User_Properties.o
OBJFILES += $(TER_DIR)/$(TER_TUNER)/$(TER_TUNER)_L2_API.o
OBJFILES += $(TER_DIR)/SiLabs_TER_Tuner_API.o
endif

ifneq (none, $(TER_TUNER_2))
#-------------------------------------------------------------
#include path for silabs_superset TER files
ccflags-y+= -I$(MODULE)/$(TER_DIR)/$(TER_TUNER_2)
# TER compilation flags
ccflags-y+= -DTER_TUNER_$(TER_TUNER_2)
#object files for TER tuner
OBJFILES += $(TER_DIR)/$(TER_TUNER_2)/$(TER_TUNER_2)_L1_API.o
OBJFILES += $(TER_DIR)/$(TER_TUNER_2)/$(TER_TUNER_2)_L1_Commands.o
OBJFILES += $(TER_DIR)/$(TER_TUNER_2)/$(TER_TUNER_2)_L1_Properties.o
OBJFILES += $(TER_DIR)/$(TER_TUNER_2)/$(TER_TUNER_2)_Properties_Strings.o
OBJFILES += $(TER_DIR)/$(TER_TUNER_2)/$(TER_TUNER_2)_User_Properties.o
OBJFILES += $(TER_DIR)/$(TER_TUNER_2)/$(TER_TUNER_2)_L2_API.o
OBJFILES += $(TER_DIR)/SiLabs_TER_Tuner_API.o
endif
#-------------------------------------------------------------
# End of TER compilation flags
#=============================================================
#=============================================================
#SAT compilation flags
#-------------------------------------------------------------
# +++<porting>+++SAT tuner selection
# (use 'none' for no_SAT compilation)
# Check SAT/SiLabs_SAT_Tuner_API.c/SiLabs_SAT_Tuner_SW_Init
# for possible values
# Also add the corresponding SAT tuner code under SAT/
# the current SAT tuner being selected in the SW configuration
# using SiLabs_API_Select_SAT_Tuner
SAT_TUNER=RDA5816SD
SAT_LNB=LNBH26
# ---<porting>--- End of SAT tuner selection
#-------------------------------------------------------------
ifneq (none, $(SAT_TUNER))
#-------------------------------------------------------------
# +++<porting>+++SAT standards selection
# (only one option: comment DVB-S2X if unused)
ccflags-y+=-DSATELLITE_FRONT_END

ccflags-y+= -DDEMOD_DVB_S_S2_DSS 
# for DVBS, DVBS2 and DSS
ccflags-y+= -DDEMOD_DVB_S2X 
ccflags-y+= -DTS_CROSSBAR
# if support for DVB-S2X is required
# ---<porting>--- End of SAT standards selection
#-------------------------------------------------------------
#include path for silabs_superset SAT files
ccflags-y+= -I$(MODULE)/$(SAT_DIR)
ccflags-y+= -I$(MODULE)/$(SAT_DIR)/Unicable
ccflags-y+= -I$(MODULE)/$(SAT_DIR)/$(SAT_TUNER)
# SAT compilation flags
ccflags-y+= -DSATELLITE_FRONT_END -DSAT_TUNER_SILABS -DSAT_TUNER_$(SAT_TUNER)
ccflags-y+= -DUNICABLE_COMPATIBLE -DNO_FLOATS_ALLOWED
#object file for SAT tuner
OBJFILES += $(SAT_DIR)/$(SAT_TUNER)/SiLabs_L1_RF_$(SAT_TUNER)_API.o
#object file for SAT tuner wrapper
OBJFILES += $(SAT_DIR)/SiLabs_SAT_Tuner_API.o
#object file for SAT Unicable
OBJFILES += $(SAT_DIR)/Unicable/SiLabs_Unicable_API.o
endif
ifneq (none, $(SAT_LNB))
ccflags-y+= -D$(SAT_LNB)_COMPATIBLE
ccflags-y+= -I$(MODULE)/$(SAT_DIR)/LNB/$(SAT_LNB)
OBJFILES += $(SAT_DIR)/LNB/$(SAT_LNB)/$(SAT_LNB)_L1_API.o
endif
#-------------------------------------------------------------
# End of SAT compilation flags
#=============================================================

ccflags-y+= -DLINUX_CUSTOMER_I2C -DNO_WIN32

#OBJFILES :=
OBJFILES += si2183_dev.o si2183_fe.o

obj-m += si2183.o
si2183-objs += $(OBJFILES)


#-------------------------------------------------------------
# build
#=============================================================

all: 
	make ARCH=arm64 CROSS_COMPILE=$(COMPILER) -C $(KERNEL) M=$(PWD) modules

clean: 
	make -C $(KERNEL) M=$(PWD) clean 
