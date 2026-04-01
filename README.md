# Spec-CE
SpecCE is an assembly shim that allows the TI-84 Plus CE graphing calculator to natively execute ZX Spectrum 48K software.

Note: This is really proof of concept and work in progress. Most games will crash at start or randomly during play and I've included no way to save progress.

# Building

To build Spec-CE you need the [CE C/C++ Toolchain](https://github.com/CE-Programming/toolchain)
Then it is just a matter of running make.

The following libraries will also need to be present on the calculator:
fileioc
graphx
libload
keypadc

[CE Libraries](https://github.com/CE-Programming/libraries)

# Loading Games

Spec-CE currently only supports spectrum 48K .SNA snapshots. The 48K .SNA snapshots need to be converted into TI-84 Plus CE AppVars (.8xv files) so they can be read natively from the calculator's memory. Use the CE toolchain (e.g., ConvBin) to package the snapshots before transferring them via TI Connect CE or CEmu. The ZX spectrum rom also needs to be converted to an AppVar.


```
convbin -i 48.rom -o ZXGAME.8xv -k 8xv -n ZXROM -j bin

convbin -i Heavy_on_the_Magick.sna -o ZXGAME.8xv -k 8xv -n ZXGAME -j bin
```

[Spectrum 48k rom](https://spectrumcomputing.co.uk/zxdb/sinclair/entries/1000486/48.rom.zip)

# Usage
(once running on the calculator or calculator emulator)

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

# Spectrum games running on TI-84 Plus CE

<img width="483" alt="Image" src="https://github.com/user-attachments/assets/90789837-5e0f-48c5-a4f6-348b1110602e" />

<img width="483" alt="Image" src="https://github.com/user-attachments/assets/86431808-4147-43bf-9574-bcce9e5fa32e" />

<img width="483" alt="Image" src="https://github.com/user-attachments/assets/7c9dc5cb-a03d-4135-9c0f-75b1a19590ef" />

<img width="483" alt="Image" src="https://github.com/user-attachments/assets/354d6e79-2724-482a-b346-5aa5c1b4739e" />

<img width="483" alt="Image" src="https://github.com/user-attachments/assets/6d668e48-d8ed-410f-9fbe-7df6cbb74344" />

<img width="483" alt="Image" src="https://github.com/user-attachments/assets/2dcba3b1-cbec-46fa-a08a-eb36e1b58344" />

<img width="483" alt="Image" src="https://github.com/user-attachments/assets/36420576-9d07-4aab-b7c0-668d3fc3d717" />

<img width="483" alt="Image" src="https://github.com/user-attachments/assets/919f7bfd-e608-4702-bbbc-ad56d96c3184" />

