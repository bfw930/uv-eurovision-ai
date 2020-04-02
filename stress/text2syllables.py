#!/usr/bin/env python2.7
#
# Forked from text2cmudict
'''
Convert text into syllables for sheet music and/or sinsy.

Run with -h for help text.

Version: 0.3 - works for most words, but not "homeowner", "struggled" or "enunciate" 

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

syllableDictionary = { # Just in case we want to add exceptions
'melbourne' : 'mell born',
'e' : 'ee',
}

###############################################################################

def getTextFromFile(filename):
    'Return contents of file as a string'
    
    print 'file is ', filename
    f = open( filename );    
    return f.read()

###############################################################################

def syllabifypolysyllabic( word ):
    'Return string with dash + space between syllables '
    s = ''
    syllablecount = 0
    vowels = 0
    i = 0
    lastcharpos = len( word )
    prevvowelpos = -1
    smatches = 0
    prev = ' '
    #print word
    for ch in word:
        i += 1 
        vmatches = re.search('[aeiouy]', ch )
        if vmatches: # it's the vowel sound of a syllable
            #               print ch+' is a vowel!!!'
            prevvowelpos = i
            if not vowels: # first vowel of the syllable, presumably
                syllablecount += 1
                vowels = 1
            else: # it's a vowel and previous was vowel
                if smatches: # but previous was really a singable constant
                    s += '- '
                    prevdash = len( s ) -1 # the position of the dash in s
                    # print prevdash, s
        else: # it's a consonant
            smatches = re.search('[rlwnm]', ch) # treat as vowel sometimes...
            if not smatches: 
                if i < lastcharpos and vowels: # not last letter and previous letter was a vowel
                    s += '- '
                    prevdash = len( s ) -2 # the position of the dash in s
                else:
                    #                    print "testing for 'ed' endings"
                    if i == lastcharpos and ch == 'd' and prev == 'e': # uh oh we're screwed, it's a "-ed" ending
                        # remove last dash
                        temp = s
                        #print "prevdash is ", prevdash, " and the char is ", s[prevdash] 
                        if s[prevdash+2] != 'd' and s[prevdash+2] !=  't':
                            # need to remove previous dash
                            #print "Not a d or t"
                            s = s[0:prevdash]+s[prevdash+2:len(s)]
                        #else:
                            #print "Must be a d or t"
                            # s += '- '
                        # print 'uh oh we\'re screwed, it\'s a "-ed" ending, temp is "'+ temp+ '" but s is "'+ s+ '".'
                    elif i == lastcharpos:
                        # last character is a consonant but not -ed ending
                        # print "prevdash is ", prevdash, " prevvowelpos is ", prevvowelpos
                        if prevvowelpos < prevdash:
                            # print "no vowels in last bit, so we should get rid of the dash"
                            s = s[0:prevdash]+s[prevdash+2:len(s)]
                vowels = 0
        s+=ch
        prev = ch
    return s

def syllabify( word ):
    numMatches = 0
    if word  in d:
        for phone in d[word][0]: # counting syllables
            matches = re.search('[0-9]', phone )
            if matches:
                numMatches += 1
        #      print word, len( word ), d[ word ][0], numMatches,
        if numMatches <= 1:
            return word
        else: # more than one syllable
            return syllabifypolysyllabic( word )
    else:
        return syllabifypolysyllabic( word ) #should syllabify non-cmudict word


def cleanuplist( list ):
    results = []
    for word in list:
        if word == ','  or word == '': # This gets rid of the empy string at the end of the list
            continue
        else:
            results.append( word )
    return results

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
lines = text.splitlines() # useful to keep lines
for line in lines:
    #   print line
    line = re.split(r'\s+', line) # splits into "words"
    #   print line
    results = cleanuplist( line )
    for word in results:
        #        print word,
        print syllabify( word ),
    print
