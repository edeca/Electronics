import os
import re
import sys
import Tkinter, tkFileDialog
from zipfile import ZipFile

########
# Author: David <david@edeca.net>
#   Date: 22nd February 2014
#    URL: http://edeca.net
# Source: https://github.com/edeca/Electronics
#
# This script renames Proteus ARES output ZIP files so they are suitable
# for easy upload to the OSHPark PCB service.
#
# Output gerber data from ARES like normal then run this script and choose
# the *CADCAM.ZIP file.  A new ZIP file will be created - the original is
# not modified in any way.
#
# Use layer Mech 1 for the board outline because no dedicated outline
# file is produced.  Mech 1 should be empty but the outline is placed
# on all layers, so it will appear here.
#
# If you wish to use internal cutouts or slots then draw these on the 
# Mech 1 layer as described in the tutorial: http://tinyurl.com/o2w8v83
# This will ensure they end up on the dimension layer.
#
# Files that are not needed (e.g. the README) are not copied to the new
# ZIP.  Extra files in the output ZIP will cause OSHPark validation to fail.
#
# Note that OSHPark seems to require all files to be present - even if
# there is nothing on the layer.  Therefore you should export all files
# from ARES (don't untick the checkboxes).
########

mapping = { 'Top Copper': 'gtl',
            'Bottom Copper': 'gbl',
            'Top Solder Resist': 'gts',
            'Bottom Solder Resist': 'gbs',
            'Top Silk Screen': 'gto',
            'Bottom Silk Screen': 'gbo',
            'Inner  1': 'g2l',
            'Inner  2': 'g3l',
            'Mechanical 1': 'gko',          # Board outline
            'Drill': 'xln' }

formats = [ ('ARES output ZIP','*CADCAM.zip') ]

def doExit(message):
    print message + "\n"
    raw_input('Press enter to quit..')
    sys.exit(0)

# Returns the renamed file if it should be included, or None otherwise
def mapFilename(original):
    for name in mapping.keys():
        search = "CADCAM " + name + ".TXT"
        if search in original:
            print " - Found file for the %s layer (%s)" % (name, mapping[name])
            return re.sub('TXT$', mapping[name], original)
    return None

def main():
    root = Tkinter.Tk()
    root.withdraw()

    inFile = tkFileDialog.askopenfile(parent=root,mode='rb',filetypes=formats,title='Choose the CADCAM input file')

    if inFile == None:
        doExit("No file chosen!")

    if not "CADCAM" in inFile.name:
        doExit('Input filename does not contain CADCAM, is this an ARES output file?')

    outFile = re.sub('CADCAM.ZIP$', 'OSHPark.ZIP', inFile.name)

    source = ZipFile(inFile, 'r')
    target = ZipFile(outFile, 'w')
    for file in source.filelist:
        renamed = mapFilename(file.filename)
        if renamed:
            target.writestr(renamed, source.read(file))
    target.close()
    source.close()

    print
    doExit('Files renamed and added to output file: ' + outFile)

if __name__ == "__main__":
    main()
