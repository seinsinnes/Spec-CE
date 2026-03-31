# Spec-CE
SpecCE is an assembly shim that allows the TI-84 Plus CE graphing calculator to natively execute ZX Spectrum 48K software.

#Building

To build Spec-CE you need the [CE C/C++ Toolchain](https://github.com/CE-Programming/toolchain)
Then it is just a matter of running make.

The following libraries will also need to be present on the calculator:
fileioc
graphx
libload
keypadc

[CE Libraries](https://github.com/CE-Programming/libraries)

#Loading Games

Spec-CE currently supports spectrum 48K .SNA snapshots. The 48K .SNA snapshots need to be converted into TI-84 Plus CE AppVars (.8xv files) so they can be read natively from the calculator's memory. Use the CE toolchain (e.g., ConvBin) to package the snapshots before transferring them via TI Connect CE or CEmu. The ZX spectrum rom also needs to be converted to an AppVar.


```
convbin -i 48.rom -o ZXGAME.8xv -k 8xv -n ZXROM -j bin

convbin -i AllHallows.sna -o ZXGAME.8xv -k 8xv -n ZXGAME -j bin
```

[Spectrum 48k rom]([https://pages.github.com/](https://spectrumcomputing.co.uk/zxdb/sinclair/entries/1000486/48.rom.zip)

# Usage once running on the calculator (or calculator emulator)
SpecCE runs by dropping the eZ80 processor into 16-bit compatibility mode. However, the TI-84 Plus CE has a vastly different keyboard layout than a ZX Spectrum.

## Keyboard Mapping

The Alphabetic keys have been mapped 1:1 to the labelled keys on caculator. This presents some difficulties as direction keys in games rely on their relative positions on the Spectrum keyboard

The D-Pad is currently optimised from adventure games
Meaning the TI-84's directional pad is mapped directly to compass directions:

[ Up ] = Types N (North)

[ Down ] = Types S (South)

[ Right ] = Types E (East)

[ Left ] = Types W (West)

Other possible mappings for the arrow keys (perhaps in future swappable/configable) are the QAOP keys commonly used in more action-based spectrum games.

Action & Modifier Keys
[ 0 ] = SPACE

[ enter ] = ENTER

[ 2nd ] = CAPS SHIFT (Hold this while pressing numbers to access symbols)

[ alpha ] = SYMBOL SHIFT

[ del ] = BACKSPACE (Simulates holding CAPS SHIFT + 0 to delete!)

[ clear ] = HARD EXIT. Immediately drops out of Spec-CE and returns you to the calculator OS.

Numbers
Because the Alpha keys take up the main keypad, the Spectrum's number row (0-9) is mapped to the outer edges of the calculator:

1 to 5: The top graphing row ([y=], [window], [zoom], [trace], [graph])

6 to 0: The right-side editing keys ([(-)], [vars], [stat], [mode], [del])

