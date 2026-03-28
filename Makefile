# ----------------------------
# Makefile Options
# ----------------------------

NAME = SPECCYCE
ICON = icon.png
DESCRIPTION = "ZX Spectrum on TI84 Plus CE"
COMPRESSED = NO

CFLAGS = -Wall -Wextra
CXXFLAGS = -Wall -Wextra

# ----------------------------

include $(shell cedev-config --makefile)
