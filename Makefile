# Makefile for ultraleap

lib.name = ultraleap

# this project is built using the LeapC API (SDK 5.7.2 or higher), and requires the same version of LeapC.so (Linux) or LeapC.dll (Windows)

# specify the location of the LeapSDK directory
leapSDKdir = /Applications/Ultraleap\ Hand\ Tracking.app/Contents/LeapSDK
# leapSDKdir = "C:\Program Files\Ultraleap\LeapSDK"

define forLinux
  # specify the location and name of the LEAP library
  ldflags = -L$(leapSDKdir)/lib/x64
	ldlibs = -lLeapC

  # specify include directory with the LEAP headers
  cflags = -Iinclude -I$(leapSDKdir)/include
endef

# in case of future macOS support
define forDarwin
  # specify the location and name of the LEAP library
	ldflags = -L$(leapSDKdir)/lib -rpath $(leapSDKdir)/lib
  ldlibs = -lLeapC

  # specify include directory with the LEAP headers
  cflags = -Iinclude -I$(leapSDKdir)/include
endef

define forWindows
  # specify the location and name of the LEAP library
  ldflags = -L$(leapSDKdir)/lib/x64
	ldlibs = -lLeapC

  # specify include directory with the LEAP headers
  cflags = -Iinclude -I$(leapSDKdir)/include
endef

$(lib.name).class.sources = src/$(lib.name).c

common.sources = src/leapConnection.c

datafiles = help/ultraleap-help.pd README.md LICENSE

# provide the path to pd-lib-builder
PDLIBBUILDER_DIR=./pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder
