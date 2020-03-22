This folder contains scripts to generate stress strings that are used for matching melodies and lyrics

addLyrics.py is an updated script from the original project https://github.com/NorskJoe/sinsy-project, and Uitdenbogerd's future updates will be found at https://github.com/aluit/sinsy-project
The original project adds lyrics to notes in a modular fashion, so that "la di dah" applied to a melody of 5 notes will end up with the 5 notes' lyrics being "la di dah la di". The current version handles hyphens as syllable markers, but still expects a space after the hyphen to apply syllables to separate notes. It also handles xml syllabic tags correctly, allowing sinsy to have a better chance of correctly pronouncing the words.
Melisma within a word is achieved by having a dash on its own in the word, eg. "mel- is- - ma" will stretch the "is- " syllable to two notes. Melisma on the last note is not yet fully implemented, but will eventually appear in https://github.com/aluit/sinsy-project

The output of addLyrics.py is an uncompressed musicxml, which can then be uploaded to sinsy using the upload.py script from https://github.com/NorskJoe/sinsy-project or directly on sinsy.jp

