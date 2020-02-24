
''' Imports '''

# data array processing
import numpy as np

# audio analysis library
import librosa



''' Audio Calculation Helper Functions '''

def calc_amp_db(cqt, ref):

    ''' Calculate spec amplitude in db

    Args:
        ref (float): pre-compute a global reference power from the input spectrum

    Returns:

    '''

    # get cqt amplitude in decibels
    #cqt_db = librosa.amplitude_to_db(S = np.abs(cqt), ref = ref, amin = 1e-05, top_db = 80.0)
    cqt_db = librosa.amplitude_to_db(S = np.abs(cqt), ref = ref, amin = 1e-05, top_db = 60.0)

    # return calculated spec data
    return cqt_db

