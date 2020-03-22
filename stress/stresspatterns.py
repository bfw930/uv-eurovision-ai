#!/usr/bin/env python2.7
#
# Forked from text2syllables
'''
Output stress patterns of a text

Run with -h for help text.

Version: 0.2 

'''

import sys
import os
import argparse
import unicodedata
import string
import re

import nltk, numpy
from nltk.corpus import cmudict
import curses
from curses.ascii import isdigit

d = cmudict.dict()  # global...

###############################################################################

def getTextFromFile(filename):
    'Return contents of file as a string'
    
    print 'file is ', filename
    f = open( filename );    
    return f.read()

###############################################################################

def syllabify( word ):
    'Return string with dash + space between syllables '
    s = ''
    syllablecount = 0
    vowels = 0
    i = 0
    lastcharpos = len( word ) - 1
    for ch in word:
              i += 1 
              matches = re.search('[aeiouy]', ch )
              if matches: # it's the vowel sound of a syllable
 #               print ch+' is a vowel!!!'
                if not vowels: # first vowel of the syllable, presumably
                    syllablecount += 1
                    vowels = 1
              else: # it's a consonant
                  if i < lastcharpos and vowels: # not last letter and previous letter was a vowel
                       s += '- '
                  vowels = 0
              s+=ch          
    return s

###############################################################################

# Main

if __name__ == '__main__':

    parser = argparse.ArgumentParser(
                                 description='Convert text into syllables.')
    
    parser.add_argument(  "lyricsfile", type =argparse.FileType('r'),   help='a text file containing lyrics' )
                        
    parser.add_argument('-e', dest='encoding', metavar='Encoding', action='store',
                        choices={'utf-8','utf-16','ascii'}, default='utf-8',
                        help='utf-8, utf-16 or ascii (default=utf-8)')
                        
    args = parser.parse_args()

    # Confirm that the arguments supplied from command line make sense:

 #   print 'file name is ' + args.lyricsfile.name
  
  #  if not os.path.isfile(args.filename)
     #  raise SystemExit(os.path.basename(sys.argv[0]) +
        #                 ': error: no such file ' + args.filename)

    # Do the thing...


globalWordSet = set()
allWordDictionaries = {}

lyrics = args.lyricsfile.read()
 
# print lyrics

text = lyrics.decode('UTF-8').lower() # converts all to lower case
text = re.sub('[^a-zA-Z \'\n]', ' ', text) # Anything that isn't alpha or whitespace replaced with space
lines = text.splitlines() # not used yet, but probably useful to keep lines
for line in lines:
    line = re.split(r'\s+', line) # splits into "words"

    #print text

    results = []
    for word in line:
        if word == ','  or word == '': # This gets rid of the empy string at the end of the list
            continue
        else:
            results.append( word )

    for word in results:
        numMatches = 0
        s = ''
        if word  in d:
            for phone in d[word][0]: # counting syllables
                matches = re.search('[0-9]', phone )
                if matches:
                    numMatches += 1
                    s += matches.group()
        else: # it's not in the cmudict dictionary, so throw in a dummy value
            s = "00" # non-dictionary words are likely to be polysyllabic, but stress pattern unknown, hence a dummy value of "00", which probably won't occur in the dictionary words
        print  s,
    print
