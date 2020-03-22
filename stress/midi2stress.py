#!/usr/bin/env python2.7
#
# Forked from stressSummary.py
'''
Read a midicsv file and output a stress string based on IOI



Version: 0.2 

'''

import sys
import os
import argparse
import unicodedata
import string
import re

import nltk, numpy
import curses
from curses.ascii import isdigit


###############################################################################

def stressFromIOI( IOI, division ):
   'Return an integer representing the stress number (1-5, with 1 meaning high)'
   beats = ioi / division
#   print 'ioi = ', ioi, 'beats = ', beats
   if beats > 1:
       return 1
   elif beats == 1:
       return 2
   else:
       return 4
   

# Main

if __name__ == '__main__':

    parser = argparse.ArgumentParser(
                                 description='Convert two stress strings into one.')
    
    parser.add_argument(  "csvfile", type =argparse.FileType('r'),   help='a text file in midicsv format' )
                        
    parser.add_argument('-e', dest='encoding', metavar='Encoding', action='store',
                        choices={'utf-8','utf-16','ascii'}, default='utf-8',
                        help='utf-8, utf-16 or ascii (default=utf-8)')
                        
    args = parser.parse_args()


mididata = args.csvfile.read()
division = 480 # initialise ticks per beat to something
lastNoteTime = 0 # initialise to something... 
#print mididata

mididata = re.sub('[^a-zA-Z0-9_ \'\n]', ' ', mididata) # Anything that isn't alphanumeric or whitespace replaced with space
#print mididata

lines = mididata.splitlines()
ioiList = []
stressList = []

for line in lines: # Each line is a MIDI event or meta-event
    line = re.split(r'\s+', line) # splits into "words"
    results = []
    for word in line:
        if word == ','  or word == '': # This gets rid of the empy string at the end of the list
            continue
        else:
            results.append( word )

# So now we have a midi event in 6 elements of an array called results
#    print results
    track = int( results[0] ) 
    time  = int( results[1]  )
    eventType = results[2]
    if eventType == 'Header':
        format =    results[3]
        nTracks = int( results[4] )
        division = int( results[5] )
    elif eventType == 'Note_on_c':
        ioi = time - lastNoteTime
        ioiList.append( ioi )
        stressList.append( stressFromIOI( ioi, division ) )
        lastNoteTime = time
#print ioiList
#print stressList
stressStr = '';
numNotes = len( stressList )
#print numNotes
for stressdigit in stressList:
    digitstr = str( stressdigit )
 #   print digitstr,
    stressStr += str( stressdigit )
print stressStr

        
        
        
    
