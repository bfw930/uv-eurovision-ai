This folder contains scripts written/edited by Uitdenbogerd to process MIDI files, and to generate stress strings that are used for matching melodies and lyrics

addLyrics.py is an updated script from the original project https://github.com/NorskJoe/sinsy-project, and Uitdenbogerd's future updates will be found at https://github.com/aluit/sinsy-project
The original project adds lyrics to notes in a modular fashion, so that "la di dah" applied to a melody of 5 notes will end up with the 5 notes' lyrics being "la di dah la di". The current version handles hyphens as syllable markers, but still expects a space after the hyphen to apply syllables to separate notes. It also handles xml syllabic tags correctly, allowing sinsy to have a better chance of correctly pronouncing the words.
Melisma within a word is achieved by having a dash on its own in the word, eg. "mel- is- - ma" will stretch the "is- " syllable to two notes. Melisma on the last note is not yet fully implemented, but will eventually appear in https://github.com/aluit/sinsy-project

The output of addLyrics.py is an uncompressed musicxml, which can then be uploaded to sinsy using the upload.py script from https://github.com/NorskJoe/sinsy-project or directly on the sinsy.jp website.

cmu2mon1.awk converts the cmudict syllable output of stresspatterns.py into a monotonic representation without spaces for matching.
eg. "00 1 102" becomes "441142", which is the same 3-level stress alphabet as that produced by midi2stress.py. Why 4? That was due to an abandoned more ambitious plan... Also, powers of 2 are nice.

midi2stress.py takes a midicsv file (https://www.fourmilab.ch/webtools/midicsv/) as input and outputs a text stress pattern based on inter-onset intervals in the following manner:
1 for note inter-onset intervals > 1 beat
2 for 1-beat
4 for notes < 1 beat

It ignores all track information, processing events in the sequence read.

midistats.py reports number of notes and average pitch, with midicsv file as input

normaliseOctave.py reduces extreme average pitch to something in the more singable range. It was used as postprocessing to produce more realistic output from sinsy.

stresspatterns.py outputs the cmudict stress syllables of the lyric input text file. Out of dictionary words are represented by 00, meaning two unstressed syllables, on the assumption that oov words are more likely to be polysyllabic.

text2syllables.py is a buggy syllabifier, that turns text input into syllables using "- " to separate syllables. eg. "forever" becomes "for- e- ver". This version still has major bugs, in that it not only incorrectly syllabifies, it loses letters. Use at your own risk!


