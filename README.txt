SimpleMP3 - plays every .mp3 in ms0:/MUSIC/ in a loop (PSP, ARK-4).

BUILD (pick one):
 A) With Docker (any PC):
      docker run --rm -v "$PWD":/src -w /src pspdev/pspdev make
    -> produces EBOOT.PBP
 B) GitHub: upload this folder to a new repo; the Actions tab builds it,
    download the "SimpleMP3" artifact (contains EBOOT.PBP).
 C) Install pspdev (https://pspdev.github.io) and run: make

INSTALL:
 Copy EBOOT.PBP to  ms0:/PSP/GAME/SimpleMP3/EBOOT.PBP
 Put MP3s in        ms0:/MUSIC/
