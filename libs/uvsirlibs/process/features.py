
''' Imports '''

# data array processing
import numpy as np

# audio analysis library
import librosa



## impliment and test all advanced examples
## determine optimised components of each (chroma, onset, harmonic/percusive, segmentation, etc)
## integrate and combine all processes and components into series of feature extraction functions
## use feature (chroma class, harmonic, onset) as a function of time to cluster and label sonic elements
## analysis on aggregate feature elements, further cluster and average
## extract full audio track summary features





### librosa capabilities
    # segmentation
        # onset detection, dynamic beat / tempo (librosa.beat.beat_track)
    # melody
        # chroma energy within pitch classes (librosa.feature.chroma_cqt)


### Audio Dynamics

    ## beat / bar segmentation
        # use librosa.beat.beat_track to segment track based on onset envelope and dynamic tempo estimation
        # use backtrack to include onset of beat within segment

    ## segment dynamics
        # compare max, rms, min energy within segment, variance in dynamics per segment

    ## full track dynamics
        # compare average, variance of segment dynamics over full track


### Pitch Dynamics

    ## tone estimation
        # use librosa.feature.chroma_cqt to extract weighting (energy) for notes over time
        # use computed beat segmentation to average and compare tone within segment, variation or multiplicity


    ## full track relative weighting of tonal notes to fingerprint key of audio


### Spectral Distribution

    ## Harmonic / Percussive
        # decompose audio into harmonic / percussive components and extract relative energy over time


### Segmentation

    ## initial beat segmentation

    ## subsegmentation

    ## clustering of features within segment by sonic energy envelope


### Decompose a feature matrix

    # use librosa.decompose.decompose produce a decomposition into components and activations from spectrogram




#### See Advanced Examples notebooks for chroma and others





def extract_energy_statistics(energy):

    ''' Extract Energy Features

        Extract spectral energy features including rms, totals, variance, min/max

    Args:

    Returns:
        list: extracted energy feature values
    '''

    # spectral energy statistics
    _energy = {}

    #print(len(energy))

    # total segment energy weighted by length in frames
    _energy['rms'] = np.trapz(energy) / energy.shape[0]

    # min, max, variance
    _energy['max'] = np.max(energy)
    _energy['min'] = np.min(energy)
    _energy['var'] = np.std(energy)


    # return calculated energy statistics
    return _energy




def extract_chroma_statistics(chroma_cqt):

    ''' Extract Chroma Features

        Extract spectral energy features including rms, totals, variance, min/max

    Args:

    Returns:
        list: extracted energy feature values
    '''

    # chroma statistics
    chroma = {}


    # sum chroma cqt over segment and normalise
    chroma_sum = np.trapz(chroma_cqt, axis = 1)
    chroma_norm = chroma_sum / np.max(chroma_sum)


    # store normalised relative note sum
    #chroma['notes'] = chroma_norm


    # get primary note (max sum notes over segment as index) and melody extent (segment note variance)
    chroma['prime_note'] = np.where(chroma_norm == np.max(chroma_norm))[0][0]
    chroma['note_var'] = chroma_norm.std()


    # start and end note
    chroma_sum = np.trapz(chroma_cqt[:20], axis = 1)
    chroma['start_note'] = np.where(chroma_sum == np.max(chroma_sum))[0][0]

    chroma_sum = np.trapz(chroma_cqt[-20:], axis = 1)
    chroma['end_note'] = np.where(chroma_sum == np.max(chroma_sum))[0][0]


    # split halves, mean note, delta (trend over segment time)
    half_s = np.trapz(chroma_cqt[:int(len(chroma_cqt)/2)], axis = 1)
    half_s_n = np.where(half_s == np.max(half_s))[0][0]

    half_e = np.trapz(chroma_cqt[int(len(chroma_cqt)/2):], axis = 1)
    half_e_n = np.where(half_e == np.max(half_e))[0][0]

    chroma['note_delta'] = half_e_n - half_s_n

    #print(chroma['note_delta'])


    # return calculated chroma statistics
    return chroma



def extract_segment_temporal(frames, track_length):

    ''' Extract Temporal Segment Features

        Extract information about segment within track

    Args:
        frames (list): segment frames within track
        track_length (int): total frames in track

    Returns:
        list: extracted energy feature values
    '''

    # chroma statistics
    temporal = {}


    # get segment length
    temporal['length'] = np.ptp(frames)

    # get segment mean depth within total track length
    temporal['track_depth'] = np.mean(frames) / track_length


    # return calculated temporal statistics
    return temporal

