# RRWMC, July 4 2019
# TGT, July 13 2020

import sys
import pathlib

import ndspy.fnt
import ndspy.rom

print('Script credits:\n - RoadrunnerWMC\n - TheGameratorT')

def waitEnterExit():
    input("\nPress Enter to exit...")

if (not (len(sys.argv) > 1)):
    print('\nFor installing the table you can either: ')
    print(' - Drag and drop your file into the script.')
    print(' - Pass the file path as the first argument of the script.')
    waitEnterExit()
    sys.exit(0)

# Path to input rom
INPUT_ROM = sys.argv[1]
# Path to the output rom
OUTPUT_ROM = str(pathlib.Path(INPUT_ROM).with_suffix('')) + '_new.nds'

fID = 0
tSize = 0

def importFolder(dirWaves):
    global fID
    global tSize

    for i in range(0, tSize):
        rom.files.append([])
        flname = 'track' + str(i).zfill(3) + '.nwav'
        print('  /' + flname)
        dirWaves.files.append(flname)

    print()
    fID += 1

def askTableSize():
    global tSize
    
    txt = input("What is the size of your Nitro WAV table? (default is 76): ")
    txt2 = txt
    if(txt2[0] == '-'):
        txt2 = txt2[1:]
    if (txt2.isnumeric()):
        n = int(txt)
        if (n > 0):
            if(n > 256):
                print('Are you trying to ascend to the skies or...? Please keep your table size lower than 256.')
            else:
                tSize = n
                return
        else:
            if (n == 0):
                print('Genius, just for that, I will close myself.')
                waitEnterExit()
                sys.exit(0)
            else:
                print('Going negative? Really? How original of you.')
    else:
        print('Bruh, that is not even a number.')
    
    askTableSize()

print('\nDetected input file: ' + INPUT_ROM)
print('Destination file path: ' + OUTPUT_ROM)
print('\nCreating Nitro WAV table for the ROM.\n')

rom = ndspy.rom.NintendoDSRom.fromFile(INPUT_ROM)

askTableSize()

print('/waves')
dirWaves = ndspy.fnt.Folder(firstID=len(rom.files))
rom.filenames.folders.append(('waves', dirWaves))

importFolder(dirWaves)

rom.saveToFile(OUTPUT_ROM)

print('\nAll the Nitro WAV template files were created.')
tblStartFileID = str(rom.filenames.idOf('/waves/track000.nwav'))
print('Table start file ID: ' + tblStartFileID)
print('\nPlease change the following in the "NWAVPlayer_nsmb.cpp" file:')
print('  "constexpr int firstWavID = x;" -> "constexpr int firstWavID = ' + tblStartFileID + ';"')
waitEnterExit()
