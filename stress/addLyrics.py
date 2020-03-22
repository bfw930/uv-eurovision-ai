#!/usr/bin/python

import re
import argparse
import xml.etree.ElementTree as ET

def addLyrics(root, lyrics):
    # Sandra version, process dashes as syllable markers
    # Process dashes in isolation as melisma (middle syllables auto extend unlyricked notes)
    # Also process underscores at the end of a word as melisma
    # If syllabic tags are correctly applied, and dashes removed, sinsy works better.
    word = ''
    n = len(lyrics)
    edlyrics = list( lyrics ) # need a copy, since addLyric loops through text as often as needed
    #print edlyrics
    #print lyrics
    i = 0
    for a in root.findall('part'):
        middle = False
        extend = False
        for b in a.findall('measure'):
            for c in b.findall('note'):
                hasRest = False
                for d in c.findall('rest'):
                    hasRest = True
                if hasRest:
                    continue
                hasLyric = False
                for d in c.findall('lyric'):
                    hasLyric = True
                if not hasLyric:
                    lyricElt = ET.SubElement(c, 'lyric')
                for d in c.findall('lyric'):
                    hasSyllabic = False
                    hasLyricText = False
                    for e in d.findall('syllabic'):
                        hasSyllabic = True
                    for e in d.findall('text'):
                        hasLyricText = True
                    if not hasSyllabic:
                        syllabicElt = ET.SubElement(d, 'syllabic')
                    for e in d.findall('syllabic'):
                        word = lyrics[i%n]
                        lastchar = word[len(word) - 1] 
                        # print word, word[len(word) - 1] , lastchar
                        if not middle:
                            # start of a word
                            if lastchar != '-' and lastchar != '_':
                                # doesn't end in dash or underscore, so single
                                e.text = 'single'
                            elif lastchar == '-':
                                # first syllable of a word (assuming it isn't just a dash...)
                                e.text = 'begin'
                                middle = True
                                word = word[ 0:len(word) - 1 ] # remove dash
                            elif lastchar == '_':
                                # monosyllabic extended
                                e.text = 'single'
                                word = word[ 0:len(word) - 1 ] # remove underscore
                                extend = True
                            else:
                                # how did that happen?!!
                                print 'Error: how did this happen, with word = ', word, '?!!'
                        else:
                            # in middle of a word
                            if lastchar != '-' and lastchar != '_':
                                # end of word
                                e.text = 'end'
                                middle = False
                            elif lastchar == '-':
                                # middle of word
                                e.text = 'middle'
                                word = word[ 0:len(word) - 1 ] # remove dash
                            elif lastchar == '_':
                                # polysyllabic extended
                                e.text = 'end'
                                word = word[ 0:len(word) - 1 ] # remove underscore
                                extend = True
                                middle = False
                            else:
                                # how did that happen?!!
                                print 'Error: how did this happen, with word = ', word, '?!!'
                    if not hasLyricText:
                        lyricTextElt = ET.SubElement(d, 'text')
                    for e in d.findall('text'):
                        e.text = word
                        i+=1
                    hasExtend = False
                    for e in d.findall( 'extend'):
                        hasExtend = True
                    if extend and not  hasExtend:
                        # syllable extends over following notes, indicated by underscore in input
                        # and we need to create an extend element
                        extendElt = ET.SubElement( d, 'extend' )
                        extend = False
                    elif not extend and hasExtend:
                        # uh oh, we need to get rid of an element. How do I do that?
                        d.remove( extendElt )
                    else:
                        # either there is an extend already and we're extending
                        # or there isn't an extend and we're not extending
                        # so do nothing
                        continue
                
                            

#
# Parse command line arguments
#

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("xmlfile", type=argparse.FileType('r'), help="name of the xml file or - for stdin")
    parser.add_argument("lyrics", help="add lyrics, syllables separated by whitespace, in a single argument enclosed in quotes")
    args = parser.parse_args()

    lyrics = args.lyrics.decode('UTF-8')
    lyrics = re.sub('[<>()]', '', lyrics)
    lyrics = re.split(r'\s+', lyrics)

    #
    # Parse XML
    #

    root = ET.fromstring(args.xmlfile.read())
    addLyrics(root, lyrics)

    #
    # Output modified XML
    #

#    print """<?xml version="1.0" encoding='UTF-8' standalone='no' ?>
#<!DOCTYPE score-partwise PUBLIC "-//Recordare//DTD MusicXML 3.0 Partwise//EN" "http://www.musicxml.org/dtds/partwise.dtd">"""
    print ET.tostring(root, encoding='UTF-8')

