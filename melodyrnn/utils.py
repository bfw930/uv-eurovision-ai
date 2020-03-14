import torch
from torch import nn
import numpy as np


EPSILON = 1e-2

def linear_quantize(samples, q_levels):
    samples = samples.clone()
    samples -= samples.min(dim=-1)[0].expand_as(samples)
    samples /= samples.max(dim=-1)[0].expand_as(samples)
    samples *= q_levels - EPSILON
    samples += EPSILON / 2
    return samples.long()

def linear_dequantize(samples, q_levels):
    return samples.float() / (q_levels / 2) - 1

def q_zero(q_levels):
    return q_levels // 2



def build_audio(M, sr: int = 16000, fr: int = 1014, soft: float = 0.0001):

    ''' build audio from track note matrix

    Args:
        M (np.array): track note matrix
        sr (int): sample rate of output audio
        acc (int): playback speed acceleration factor
        soft (float): softness per note, hanning for rise/fall smoothing

    Returns:
        (np.array): track audio
    '''

    # initialise note event list
    events = []

    # iterate each timestep
    for dt in range(M.shape[1])[:-1]:

        # get note index for note onset at timestep
        j = np.where(M[:, dt+1] > M[:, dt])[0]

        # note onset at timestep
        if len(j) != 0:

            # iterate each note onset
            for k in j:

                # store note index, onset time
                events.append( [k, dt] )


    # initialise note list
    notes = []

    # iterate over note events
    for note in events[:]:

        # get time from onset to note end (zero velocity)
        dt = np.where( M[note[0], note[1]+1:] == 0 )[0]

        # ensure duration exists
        if len(dt) != 0:

            # store note as list [note, onset, duration]
            notes.append([*note, dt[0]])


    # generate short hanning filter for note rise / fall
    hann = np.hanning(soft * sr)

    # initialise track-length audio array [32-bit]
    audio = np.zeros(M.shape[1] * fr // sr, ).astype(np.float32)

    # iterate each note in track
    for note in notes:

        # midi note index to audio frequency conversion (mid c std., 7-bit)
        f = 440 * 2**((note[0] - 69)/12)

        # calculate note start and duration
        s = note[1] * fr // sr
        t = note[2] * fr // sr

        # generate sin wave, duration of note
        note_audio = (np.sin(2*np.pi*np.arange(t) * f/sr)).astype(np.float32)

        # apply hanning filter to note rise / fall
        note_audio[ :hann.shape[0]//2] *= hann[ :hann.shape[0]//2]
        note_audio[-hann.shape[0]//2: ] *= hann[-hann.shape[0]//2: ]

        # add note to audio waveform at start time
        audio[ s : s + t ] += note_audio


    # return generated audio
    return audio
