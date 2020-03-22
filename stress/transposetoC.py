#!/usr/bin/env python2.7
#
# Forked from midi2stress.py
'''
Read a midicsv file and output a transposed midicsv file



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

def getSemitones( key ):
   ' Calculate semitones to transpose to get to C major/A minor'
   return ( key * 7 ) % 12
   


###############################################################################

# Main

if __name__ == '__main__':

    parser = argparse.ArgumentParser(
                                 description='Transpose midicsv file ')
    
    parser.add_argument(  "csvfile", type =argparse.FileType('r'),   help='a text file in midicsv format' )
 #   parser.add_argument('-s', metavar='Count', dest='semitones', action='store',
 #                       default='1', help='Number of semitones to transpose (default = 1)')

    parser.add_argument('-e', dest='encoding', metavar='Encoding', action='store',
                        choices={'utf-8','utf-16','ascii'}, default='utf-8',
                        help='utf-8, utf-16 or ascii (default=utf-8)')
                        
    args = parser.parse_args()


mididata = args.csvfile.read()
division = 480 # initialise ticks per beat to something
lastNoteTime = 0 # initialise to something... 
#print mididata
semitones = 0

mididata = re.sub('[^a-zA-Z0-9_ \'\n]', ' ', mididata) # Anything that isn't alphanumeric or whitespace replaced with space
#print mididata

lines = mididata.splitlines()

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
   # MIDI is track, time, eventtype, channel, note, value
    elif eventType == 'Key_signature':
       key = int( results[3] )
       semitones = getSemitones( key )
    elif eventType == 'Note_on_c' or eventType == 'Note_off_c':
       results[4] = str( int( results[4]) + int( semitones ) )
    line = results
    outputline = ''
    for word in line:
       outputline += word + ', '
    outputline = outputline[0:len(outputline)-2]
    print outputline

        
        
        
    
