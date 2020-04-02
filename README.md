
# Uncanny Valley et al. - Eurovision AI competition submission


## repo summary

Code, bits, pieces, misc. used for the development and preparation of Eurovision AI submission.



## deps

using python 3.8 with pipenv to manage venv + pip; see ./Pipfile (or ./requirements.txt)



##  repo layout


#### ./nbks/

ipython notebook files with example of each core component implimentation used in submission process


#### ./docs/

misc. docs pertaining to Eurovision AI competition process and submission


#### ./data/

small example subset of data used for development and model training; (limited in scope due to file sizes)


#### ./libs/

dep. libs. or reference implimentations used in development


#### ./melodyrnn/

melodyrnn: libs for modified sample-rnn model implimentation in pytorch; (further details below)


#### ./lyrics-transform/

lyrics-transform: libs for slightly customised GTP-2 transformer model implimentation in pytorch


## melody-rnn

generation of base melody samples used for track components, vocals aligned to lyrics; based on sample-rnn architecture however input quantisation translated to input direct 7-bit MIDI note input with softmax output per timestep used as MIDI note, zero as silence; training data comprised lead melody track from 200 track MIDI dataset.
