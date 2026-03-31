# ----------------------------
# Makefile Options
# ----------------------------

NAME = SPECCYCE
ICON = icon.png
DESCRIPTION = "ZX Spectrum on TI84 Plus CE"
COMPRESSED = NO

CFLAGS = -Wall -Wextra -Ofast
CXXFLAGS = -Wall -Wextra -Ofast

LTO = YES

# ----------------------------

include $(shell cedev-config --makefile)
