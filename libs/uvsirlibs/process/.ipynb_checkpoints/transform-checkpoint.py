
''' Imports '''

# data array processing
import numpy as np

# audio analysis library
import librosa

# image manipulation
from scipy import ndimage



''' Audio Transformation Functions '''

def calc_cqt(raw_audio, sample_rate, bins_per_octave, n_bins, hop_length):

    ''' Calculate CQT Transforms

        Calculate cqt transform from audio data

    Args:

    Returns:

    '''

    # generate constant-q transform (cqt)
    cqt = librosa.cqt(y = raw_audio, sr = sample_rate, hop_length = hop_length, fmin = None,
        n_bins = n_bins, bins_per_octave = bins_per_octave,
        tuning = 0.0, filter_scale = 1, norm = 1, sparsity = 0.01,
        window = 'hann', scale = True, pad_mode = 'reflect')


    # return calculated spec data
    return cqt



def calc_hpss(cqt):

    ''' Calculate Harmonic, Percussive CQT

    Args:

    Returns:

    '''

    # perform harmonic / percusive separation with margin
    cqt_h, cqt_p = librosa.decompose.hpss(S = cqt, margin = 10, kernel_size = 20, power = 2.0, mask = False)


    # return calculated spec data
    return cqt_h, cqt_p



def calc_onset(cqt_db, sample_rate, hop_length):

    ''' Calculate Onset

    Args:

    Returns:

    '''

    # compute the onset strength envelope
    onset_strength = librosa.onset.onset_strength(y = None, sr = sample_rate, S = cqt_db, lag = 1, max_size = 2,
        detrend = False, center = True, feature = None, aggregate = None, centering = None)


    # compute onset events
    onset_events = librosa.onset.onset_detect(y = None, sr = sample_rate, onset_envelope = onset_strength,
        hop_length = hop_length, backtrack = True, energy = None, units = 'frames')


    # return calculated onset data
    return onset_strength, onset_events



def calc_beats(onset_strength, sample_rate, hop_length):

    ''' Calculate Beats

        Calculate beats from onset events, sync spec to beats

    Args:

    Returns:

    '''

    # dynamic beat tracking (onset strength, tempo from onset correlation, pick peaks)
    tempo, beats = librosa.beat.beat_track(y = None, sr = sample_rate, onset_envelope = onset_strength,
        hop_length = hop_length, start_bpm = 100.0, tightness = 100,
        trim = False, bpm = None, units = 'frames')

    # update beats to pad frames at limits
    beats = librosa.util.fix_frames(frames = beats, x_min = 0, x_max = onset_strength.shape[0], pad = True)


    # return calculated beats data
    return tempo, beats



def calc_sync(cqt_db, beats):

    ''' Calculate Spec synced to Beats

    Args:

    Returns:

    '''

    # beat-synchronise the cqt_db
    cqt_db_sync = librosa.util.sync(data = cqt_db, idx = beats, aggregate = np.median, pad = True, axis = -1)

    # return calculated beats data
    return cqt_db_sync



def calc_rmse(cqt, n_fft, hop_length):

    ''' Calculate RMS Energy

    Args:

    Returns:

    '''

    # calculate root mean square spectral energy from cqt amplitude
    rmse = librosa.feature.rms(y = None, S = np.abs(cqt), frame_length = n_fft, hop_length = hop_length,
        center = True, pad_mode = 'reflect')[0]


    # return calculated spec data
    return rmse



def calc_mfcc(raw_audio, sample_rate):

    ''' Calculate MFCC

    Args:

    Returns:

    '''

    # generate mel-frequency cepstral coefficients (mfcc)
    mfcc = librosa.feature.mfcc(y = raw_audio, sr = sample_rate, S = None, n_mfcc = 20, dct_type = 2, norm = 'ortho')


    # return calculated spec data
    return mfcc




def calc_chroma(cqt_h, n_bins, bins_per_octave, hop_length):

    ''' Calculate Chroma

    Args:

    Returns:

    '''

    # get chroma using cqt of harmonic component, 3 CQT bins per semi-tone
    chroma = librosa.feature.chroma_cqt(y = None, sr = None, C = np.abs(cqt_h), hop_length = hop_length,
        bins_per_octave = bins_per_octave, fmin = None, norm = np.inf, threshold = 0.0, tuning = None,
        n_chroma = 12, n_octaves = (n_bins / bins_per_octave), window = None, cqt_mode = 'full')


    # clean using non-local filtering, effectively remove sparse additive noise from features
    chroma = np.minimum(chroma, librosa.decompose.nn_filter(chroma, aggregate = np.median, metric = 'cosine'))

    # suppress local discontinuities, transients using horizontal median filter
    chroma = ndimage.median_filter(chroma, size = (1, 9))


    # return calculated spec data
    return chroma
