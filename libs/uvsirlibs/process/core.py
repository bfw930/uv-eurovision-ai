
''' Data Processing Functions

    Manage audio data processing, segmentation, feature extraction
'''



''' Imports '''

# audio transform functions
from .transform import calc_cqt, calc_onset, calc_beats, calc_sync, calc_hpss, calc_rmse, calc_mfcc, calc_chroma

# audio segmentation functions
from .segment import calc_simil, calc_segment

# audio feature extraction
from .features import extract_energy_statistics, extract_chroma_statistics, extract_segment_temporal


from .general import calc_amp_db



# nd array handling
import numpy as np



''' Processing Orchestration Functions '''

def process_audio_data(raw_audio_props, config = None):

    ''' Process Audio Data Node

        Perform range of processes on audio node: signal transforms: energy envelope, spectrograms; onset and beat
        detection, beat synchronisation; define and store config values for calculations

    Args:
        raw_audio_props (dict): raw audio node properties

    Returns:
        dict: audio process results
    '''

    # unpack required audio data and properties
    sample_rate = raw_audio_props['sample_rate']
    #raw_audio = raw_audio_props['raw_audio'][0,:(sample_rate*20)] ## select single channel, trim audio length
    raw_audio = raw_audio_props['raw_audio'][0,:] ## select single channel


    # define config variables for calculations
    #bins_per_octave = 12 * 3
    #n_bins = bins_per_octave * 7
    #hop_length = 512
    #n_fft = 4096

    # if passed config values, unpack
    if config is not None:

        bins_per_octave = config['bins_per_octave']
        n_bins = config['n_bins']
        hop_length = config['hop_length']

        n_fft = config['n_fft']

    # otherwise sane defaults
    else:

        bins_per_octave = 12 * 1
        n_bins = bins_per_octave * 7
        hop_length = 2048

        n_fft = 4096


    print('begin cqt')

    # calculate cqt from raw audio
    cqt = calc_cqt(raw_audio = raw_audio, sample_rate = sample_rate, bins_per_octave = bins_per_octave,
        n_bins = n_bins, hop_length = hop_length)

    # pre-compute a global reference power from cqt
    rp_cqt = np.max(np.abs(cqt))

    # calculate cqt amplitude in db with power reference cqt
    cqt_db = calc_amp_db(cqt = cqt, ref = rp_cqt)

    print('cqt complete')


    print('begin cqt harm/perc')

    # calculate harmonic, percusive cqt from cqt
    cqt_h, cqt_p = calc_hpss(cqt = cqt)

    # calculate percussive cqt amplitude in db with power reference cqt
    cqt_p_db = calc_amp_db(cqt = cqt_p, ref = rp_cqt)

    # calculate percussive cqt amplitude in db with power reference cqt
    #cqt_h_db = calc_amp_db(cqt = cqt_h, ref = rp_cqt)

    print('cqt harm/perc complete')


    print('begin onset, beats')

    # calculate onset strength, events from percussive cqt amplitude in db
    onset_strength, onset_events = calc_onset(cqt_db = cqt_p_db, sample_rate = sample_rate, hop_length = hop_length)

    # calculate tempo and beats using percussive onset strength
    tempo, beats = calc_beats(onset_strength = onset_strength, sample_rate = sample_rate, hop_length = hop_length)

    print('onset, beats complete')


    if False:
        print('begin rmse')

        # calculate rms energy from cqt
        rmse = calc_rmse(cqt = cqt, n_fft = n_fft, hop_length = hop_length)
        rmse_h = calc_rmse(cqt = cqt_h, n_fft = n_fft, hop_length = hop_length)
        rmse_p = calc_rmse(cqt = cqt_p, n_fft = n_fft, hop_length = hop_length)

        print('rmse complete')


    if False:
        print('begin chroma cqt')

        # calculate chroma cqt
        chroma_cqt = calc_chroma(cqt_h, n_bins, bins_per_octave, hop_length)

        print('chroma cqt complete')


    # store calculated properties to return
    store = {'bins_per_octave': bins_per_octave,
             'n_bins': n_bins,
             'hop_length': hop_length,
             'n_fft': n_fft,

             'cqt': cqt,
             'cqt_db': cqt_db,

             'cqt_h': cqt_h,
             'cqt_p': cqt_p,

             'onset_strength': onset_strength,
             'onset_events': onset_events,

             'tempo': tempo,
             'beats': beats,

             #'rmse': rmse,
             #'rmse_h': rmse_h,
             #'rmse_p': rmse_p,

             #'chroma_cqt': chroma_cqt,

             }


    # return calculated data
    return store



def process_segment_data(raw_audio_props):

    ''' Segment pre-Process Audio Data Node

        Perform segmentation pre-processes (calculations);

    Args:
        raw_audio_props (dict): raw audio node properties

    Returns:
        dict: audio process results
    '''

    # unpack required audio data and properties
    sample_rate = raw_audio_props['sample_rate']
    #raw_audio = raw_audio_props['raw_audio'][0,:(sample_rate*20)] ## select single channel, trim audio length
    raw_audio = raw_audio_props['raw_audio'][0,:] ## select single channel

    beats = raw_audio_props['beats']
    cqt_db = raw_audio_props['cqt_db']


    print('begin sync cqt')

    # calculate cqt synced beats
    cqt_db_sync = calc_sync(cqt_db = cqt_db, beats = beats)

    print('sync cqt complete')


    print('begin calc mfcc')

    # calculate mfcc from raw audio
    mfcc = calc_mfcc(raw_audio = raw_audio, sample_rate = sample_rate)

    print('calc mfcc complete')


    print('begin mfcc sync')

    # calculate mfcc synced beats
    mfcc_sync = calc_sync(cqt_db = mfcc, beats = beats)

    print('mfcc sync complete')


    print('begin calc similarity')

    # calculate similarity data
    A, evecs, cnorm = calc_simil(cqt_db_sync = cqt_db_sync, mfcc_sync = mfcc_sync)

    print('calc similarity complete')


    # store calculated properties to return
    store = {
        'cqt_db_sync': cqt_db_sync,
        'mfcc_sync': mfcc_sync,

        'A': A,
        'evecs': evecs,
        'cnorm': cnorm,
    }


    # return calculated data
    return store



def segment_audio_data(raw_audio_props, depths):

    ''' Segment Audio Data

    Args:
        _node (dict): raw audio node properties

    Returns:
        dict:
    '''

    # unpack required audio data and properties
    sample_rate = raw_audio_props['sample_rate']
    #raw_audio = raw_audio_props['raw_audio'][0,:(sample_rate*20)] ## select single channel, trim audio length
    raw_audio = raw_audio_props['raw_audio'][0,:] ## select single channel


    n_frames = raw_audio_props['cqt'].shape[1]
    beats = raw_audio_props['beats']
    evecs = raw_audio_props['evecs']
    cnorm = raw_audio_props['cnorm']


    ## segmentation iteration

    # store cluster edge beats for all k values
    edges = []


    print('begin segmentation')

    # define range of k values for segment clusters, relative to number beats
    ks = list(range(2, int(len(beats)/2) ))[::20]

    print('segment ranges {}'.format(ks))


    # iterate over k values
    for i in range(len(ks)):

        # get current k value
        k = ks[i]

        print('calc segment split level {}, {} segments'.format(i, k))


        # perform segmentation using similarity and k value
        bound_frames = calc_segment(k = k, evecs = evecs, cnorm = cnorm, beats = beats, n_frames = n_frames,
            sample_rate = sample_rate)

        # append iteration cluster edge beats
        edges = [ *edges, *bound_frames ]


    print('segmentation complete')


    # count total cluster edges for each beat; ignore first, last frames
    counts = np.array([ edges.count(b) if b not in [0, beats[-1]] else 0 for b in beats ])


    results = []
    # iterate over each segmentation depth
    for bps in depths:

        # set average beats per segment
        #bps = 8

        # sort by count total desc, get index of top relative to total beats
        j = np.argsort(counts)[::-1][:int(len(beats)/bps)]

        # get split beats, add start and end frames; sub 2 frames from beats to align prior to hit
        #split_beats = np.concatenate([[0], np.sort(beats[j])-2, beats[-1:]])
        split_beats = np.concatenate([[0], np.sort(beats[j]), beats[-1:]])


        # get segment ranges from split beat frames
        seg_ranges = [ [split_beats[i], split_beats[i+1]] for i in range(len(split_beats)-1) ]


        results.append({'split_beats': split_beats, 'seg_ranges': seg_ranges})

    return results



def extract_audio_features(raw_audio_props, frames):

    ''' Extract Audio Features

    Args:
        _node (dict): raw audio node containing raw audio data and parameters
        _feature_list (list): list of feature types to extract

    Returns:
        dict: features node, audio features extracted from raw audio data
    '''

    # unpack required properties
    rmse = raw_audio_props['rmse'][frames]

    rmse_h = raw_audio_props['rmse_h'][frames]
    rmse_p = raw_audio_props['rmse_p'][frames]

    chroma_cqt = raw_audio_props['chroma_cqt'][:,frames]


    track_length = raw_audio_props['rmse'].shape[0]


    # calculate energy statistics
    energy = extract_energy_statistics(rmse)
    energy_h = extract_energy_statistics(rmse_h)
    energy_p = extract_energy_statistics(rmse_p)


    # calculate chroma statistics
    chroma = extract_chroma_statistics(chroma_cqt)

    # calculate segment temporal details
    temporal = extract_segment_temporal(frames, track_length)


    # compile features
    features = {

        'energy': energy,
        'energy_h': energy_h,
        'energy_p': energy_p,

        'chroma': chroma,

        'temporal': temporal,
    }


    # return extracted features
    return features





    '''
    ## segmentation

    # store number similarity data in raw_audio node
    # store segment relations in raw_audio node

    # iterate number of cluster segments, k = 5, 10, 15 (weight by audio sample length)
        # each tier represents:
            high level structure (intro, verse, chorus, etc)
            mid (melody repetition)
            low (note repetition, )
    # label each iteration as segment tier, grouped by degree of segmentation

    ## need iterative feedback to ensure no tiny segments, or high degree of splitting and inconsistency


    ## segment node
        # properties
            # segment tier: relative ratio number clusters to length
            # k value from segmentation

            # range (temporal segment): start, finish frames relative to raw audio node

            # [post feature extraction]: standard features for segment range audio sample

            # [if iterative segmentation of segments]: internal similarity data for further segmentation

        # relations
            # raw audio: source data node



    ## post segmentation feature extraction
        # count of segments, average length, degree of similarity, degree of branching

        ## then for each segment, calculate standard feature extraction


    # initial full raw audio similarity, primary segment
    '''
