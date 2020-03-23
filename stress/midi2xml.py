#!/usr/bin/python
# Hacked together from midi2stress.py and generateStochasticSong.py
# some code taken from D Field's alg choral comp code
'''
Read a midicsv file and output an xml file

Version 0.1
'''

import sys
import random
import re
import argparse
import xml.etree.ElementTree as ET
from addLyrics import addLyrics

syllableDictionary = {
'melbourne' : 'mell born',
    'ghouls' : 'gools',
'london' : 'lon don',
'weather' : 'weather',
'is' : 'is',
'forecasted' : 'fore casted',
'to' : 'to',
'be' : 'be',
'partly' : 'part lee',
'cloudy' : 'cloudy',
'daytime' : 'day time',
'temperature' : 'tem pera chure',
'reaching' : 'rea ching',
'c' : 'sell seeus',
'expected' : 'ex pected',
'cit' : 'cit',
'precipitation' : 'prihsih pihtay shun',
'visibility' : 'visi bili tee',
'going' : 'go ing',
'around' : 'uh round',
'km' : 'kee low mee ters',
'ie' : 'eye ee',
'miles' : 'my ules',
'atmospheric' : 'atmoe spherick',
'pressure' : 'presh shure',
'of' : 'of',
'mb' : 'em bee',
'covering' : 'kah vering',
'humidity' : 'huemih dihtee',
'expect' : 'ex pect',
'mm' : 'millih meeters',
}

def breakIntoSyllables(text):
    text = text.decode('UTF-8').lower() # converts all to lower case
    text = re.sub('[^a-zA-Z \n\-]', ' ', text) # Anything that isn't alpha or whitespace replaced with space
#    print text
    text = re.sub('\-', '- ', text) # Put a space after hyphens
#    print text
    text = re.split(r'\s+', text) # splits into "words"
#    print text
    results = []
    for word in text:
        if not len(word):
            continue
        if word in syllableDictionary:
            results.extend(re.split(r'\s+', syllableDictionary[word]))
        else:
            results.append(word)
    return results

def MIDI2Note( MIDI_No, flats=None):
      # this function converts a MIDI number into a note and octave
      # 'flats' means to notate 'black notes' as flats; if flats is False then they'll be notated as sharps
      if flats is None:
         flats = True
      octave = (int(MIDI_No)/12)-1 # based on MIDI note 'zero' being in octave '-1'
      note = int(MIDI_No)%12 # MIDI note 'zero' is a C
      alter = 0 # this is the individual note's sharp/flat designation in MusicXML
      if note == 0:
         step = "C"
      elif note == 1:
         if flats is True:
            step = "D"
            alter = -1
         else:
            step = "C"
            alter = 1
      elif note == 2:
         step = "D"
      elif note == 3:
         if flats is True:
            step = "E"
            alter = -1
         else:
            step = "D"
            alter = 1
      elif note == 4:
         step = "E"
      elif note == 5:
         step = "F"
      elif note == 6:
         if flats is True:
            step = "G"
            alter = -1
         else:
            step = "F"
            alter = 1
      elif note == 7:
         step = "G"
      elif note == 8:
         if flats is True:
            step = "A"
            alter = -1
         else:
            step = "G"
            alter = 1
      elif note == 9:
         step = "A"
      elif note == 10:
         if flats is True:
            step = "B"
            alter = -1
         else:
            step = "A"
            alter = 1
      elif note == 11:
         step = "B"
      return step, alter, octave
  
#################3
#
# shiftNote
# Defines a note for musicXML
# Parameters
#  note is a note name used as an offset
#  alter is for sharps or flats (of the offset)
#  octave is the octave of the offset note 
#  delta is the desired note relative to the offset note
def shiftNote(note, alter, octave, delta):
    notes = [ 'C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B' ]
    numberOfNotes = len(notes)
    index = notes.index(note)
    index += alter
    index += delta
    while index < 0:
        index += numberOfNotes
        octave -= 1
    while index >= numberOfNotes:
        index -= numberOfNotes
        octave += 1
    newNote = notes[index][0]
    newAlter = 0
    if len(notes[index]) > 1:
        newAlter = 1
    return newNote, newAlter, octave
    
def generateXML(noteList, tempo, xmldivisions, totalTime ):
    score_partwise = ET.Element('score-partwise')
    part_list = ET.SubElement(score_partwise, 'part-list')
    score_part = ET.SubElement(part_list, 'score-part')
    score_part.attrib['id'] = 'P1'
    part_name = ET.SubElement(score_part, 'part-name');
    part_name.text = 'Melody'
    part = ET.SubElement(score_partwise, 'part')
    part.attrib['id'] = 'P1'
    measureNumber = 1
    barduration = 0
    numbeats = totalTime / xmldivisions
    maxbarduration = numbeats * xmldivisions
    measure = None
    for xmlnote in noteList: # loop through notes
        # xmlnote is list: note, alter, octave, start time (divisions?), duration
        barduration += xmlnote[4]
        #print 'current note duration is ', xmlnote[4], ' barduration is ', barduration
        if barduration >= maxbarduration:
            # we've filled up a bar, start a new one
            measureNumber += 1
            barduration = xmlnote[4]
            measure = ET.SubElement(part, 'measure')
            measure.attrib['number'] = str(measureNumber)
        elif measureNumber == 1 and barduration == xmlnote[4]:
            # must be first note of first bar
            measure = ET.SubElement(part, 'measure')
            measure.attrib['number'] = str(measureNumber)
            attributes = ET.SubElement(measure, 'attributes')
            divisions = ET.SubElement(attributes, 'divisions')
            divisions.text = str( xmldivisions )
            key = ET.SubElement( attributes, 'key')
            fifths = ET.SubElement( key, 'fifths')
            fifths.text = '0'
            time = ET.SubElement(attributes, 'time')
            beats = ET.SubElement( time, 'beats')
            beats.text = str( numbeats )
            beattype = ET.SubElement( time, 'beat-type')
            beattype.text = '4'
            direction = ET.SubElement(measure, 'direction')
            sound = ET.SubElement(direction, 'sound')
            sound.attrib['tempo'] = tempo
  #      else:
            # we have another note for the same bar, do nothing special
        note = ET.SubElement(measure, 'note')
#        type = ET.SubElement(note, 'type')
#        type.text = 'quarter'
        duration = ET.SubElement(note, 'duration')
        duration.text = str( xmlnote[4] )
        voice = ET.SubElement(note, 'voice')
        voice.text = '1'
#        if arr[i] == 99: # Don't know how I'll handle rests yet...
#            rest = ET.SubElement(note, 'rest')
 #       else:
        pitch = ET.SubElement(note, 'pitch')
        step = ET.SubElement(pitch, 'step')
        alter = ET.SubElement(pitch, 'alter')
        octave = ET.SubElement(pitch, 'octave')
        newNote, newAlter, newOctave = xmlnote[0], xmlnote[1], xmlnote[2]
#           newNote, newAlter, newOctave = shiftNote('C', 0, 4, arr[i])
        step.text = newNote
        alter.text = str(newAlter)
        octave.text = str(newOctave)

    return score_partwise




def readMIDIData( mididata ):
    'Turn input file into a set of lines of MIDI events'
    #print mididata

    mididata = re.sub('[^a-zA-Z0-9_ \'\n]', ' ', mididata) # Anything that isn't alphanumeric or whitespace replaced with space
    #print mididata

    lines = mididata.splitlines()
    events = []
    for line in lines: # Each line is a MIDI event or meta-event
        line = re.split(r'\s+', line) # splits into "words"
        results = []
        for word in line:
            if word == ','  or word == '': # This gets rid of the empy string at the end of the list
                continue
            else:
                results.append( word )
        events.append( results )
#    print events
    return events

def getMatchingNote( event, noteOns ):
    ' Find the matching event in the sequence'
    return noteOns[ 0 ] , noteOns[1:len( noteOns ) - 1]# Lazy version, assume it's the first one

def processMIDIData( midiEvents ):
    ' Create a list of notes for passing to the xml generator'
        # So now we have each midi event in 6 elements of an array 
    division = 480 # initialise ticks per beat to something
    lastNoteTime = 0 # initialise to something... 
    notes = [] # here I'll store a list of notes, as needed by musicxml
    noteOns = [] # here are notes that haven't had a note off event yet
    durmultiplier = 2
    for event in midiEvents:
        track = int( event[0] ) 
        time  = int( event[1]  )
        eventType = event[2]
        if eventType == 'Header':
            format =    event[3]
            nTracks = int( event[4] )
            division = int( event[5] )
        elif eventType == 'Note_on_c':
            noteOns.append( event )
        elif eventType == 'Note_off_c':
            # let's retrieve its matching noteOn and process them together
            matchingNoteOn, noteOns = getMatchingNote( event, noteOns )
#            print matchingNoteOn, event
            # MIDI is track, time, eventtype, channel, note, value
            # We want time, pitch, duration
            startTime = int( matchingNoteOn[1] )
            note = [ startTime * durmultiplier, int( event[4] ), ( time - startTime ) * durmultiplier ]
#            print note
            notes.append( note )
#    print noteOns
# maybe repeat the last note so it can be thrown away, thanks to my long bar hack...
    totalTime = note[0] + note[2]
    return notes, division, totalTime

def makeXMLNotes( notes, division ):
    'Convert notes to xml note, alter, octave values'
    xmlnotes = []
    duration = 1
    for note in notes:
#        print note
        step, alter, octave = MIDI2Note( note[1] )
        xmlnote = [step, alter, octave, note[0], note[2] ]
        xmlnotes.append( xmlnote )
 #       print xmlnote
    return xmlnotes

#
# Parse command line arguments
#

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
#    parser.add_argument("songname", help="name of the song")
    parser.add_argument(  "csvfile", type =argparse.FileType('r'),   help='a text file in midicsv format' )
#    parser.add_argument("lyricsfile", type=argparse.FileType('r'), help="name of the lyrics file")
    parser.add_argument("--tempo", default='120', help="tempo, default is 120")
    args = parser.parse_args()
    midifile = args.csvfile.read()
 #   print midifile
    mididata = readMIDIData( midifile )
#    print mididata
    notes, division, totalTime = processMIDIData( mididata )
#    print notes
    xmlNotes = makeXMLNotes( notes, division )
#    generateSong = None
 #   song = generateSong()
    root = generateXML(xmlNotes, args.tempo, division, totalTime )
#    addLyrics(root, lyrics)

    #
    # Output XML
    #

#print """<?xml version="1.0" encoding='UTF-8' standalone='no' ?>
#<!DOCTYPE score-partwise PUBLIC "-//Recordare//DTD MusicXML 3.0 Partwise//EN" "http://www.musicxml.org/dtds/partwise.dtd">"""
print ET.tostring(root, encoding='UTF-8')

