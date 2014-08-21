########################################################################
#
# fileslist.inc: List of application source files
#
########################################################################

########################################################################
# Sources in APP_SRC are delivered to Broadcom in the Application
# Release Package (make arp).
########################################################################

APP_SRC = sparinit.c blegen.c
# APP_SRC += myfile.c otherfile.cpp
# APP_SRC += another.cpp

########################################################################
# Sources in APP_SECRET_SRC are NOT delivered to Broadcom in the
# Application Release Package (make arp).-- only their objects
########################################################################

# APP_SECRET_SRC = mysecretsrc.cpp
# APP_SECRET_SRC += otherfile.c yetanother.cpp

########################################################################
# Add subdirectories to the includes
########################################################################

INCS += $(DIR)
