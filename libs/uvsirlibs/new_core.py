
''' Imports '''

# database module
from . import database

# data processing module
from . import process

# data analysis module
from . import analysis

# data import module
#from . import parse
from .parse import parse_audio_file


# filesystem navigation, system
import os, sys, glob

import numpy as np



'''

Database Abstraction Layer

function: create new node


function: get node by index
    returns: node



Import Process

function: import audio file by path
    input: audio file path
    return: binary of audio file, file properties

function: extract audio data from file
    input: binary of audio file, file properties
    return: data as dict

function: create new node in database
    input: data as dict, props, rels
    call: database create new node



Data Process

function: process audio data node
    input: audio data node, process
    returns: results data as dict

function: add results data to node
    input: process results data
    call: database update node



'''



'''
directory of tracks

import track metadata

iterate each track

    temporal segmentation

    aggregate segments

'''


'''

## nodes (metadata only) ##

    track node:

        props:

            # from track file
                file_name (str): track file name
                file_path (str): full file path to track directory

            # from raw_audio data import
                sample_rate (int): track audio sample rate
                length (int): track audio length in samples
                channels (int): number of track audio channels

            # from tag metadata: for each tag axis
                tag_axis (float): track value on continous tag axis
                ...

        rels:


    segment node:

        props:

            # from track temporal segmentation
                start (int): start frame within track
                end (int): start frame within track
                mid (int): start frame within track
                length (int): length of segment in frames

            # from feature extraction: for each feature axis
                feature (float): segment value on continous feature axis
                ...

        rels:

            track (int): source track node relation




### functions structure (track to features)


# build track node

        # input track name, path

    # import track audio data

    # import track tag data

        # return track raw_audio


# build segment nodes

        # input track node, raw_audio data

    # temporal segmentation

        # raw_audio transformations

            # return transform data

        # temporal segmentation transformations

        # temporal segmentation

            # return segmentation


# segment feature extraction

        # input segment, transform data






## feature extraction modules

        # input segment node, relation to track node, audio transform data (optional)

    # each module declares required audio transform data

        # if data not available, get raw_audio, calculate transforms

    # calculate feature from audio data

        # return feature values





## audio data transform protocol

        # input track raw_audio, list of transforms

    # transform raw_audio in dependency order

        # return all audio data




## audio data calculation functions library

    # direct function access, input required data

        # raw_audio to cqt

        # complex spectrogram to amplitude in dB

        # cqt to harm / perc cqt

        # perc_cqt to onset strength

        # onset strengths to tempo and beats



'''








# get track list from directory search

def get_tracks(base_path, props):

    ''' Get Track List Database

        Given directory (str) and database instance (list), recursively search directory for files
        of given file extension (props); append properties (dict) to node

    Args:
        db (list): database instance as list of file nodes (dict)
        base_path (str): full directory path to search
        props (dict): file properties to store in file node

    Returns:
        (none): file node added to database instance
    '''

    # store track nodes (dict) in database (list)
    db = []

    # recursive directory search for all desired pl images
    file_paths =  glob.glob(base_path + '/**/*.' + props['file_ext'] , recursive = True)

    # iterate files
    for path in file_paths:

        # extract file names from matched file paths
        file_name = path.split('/')[-1:][0]

        # get directory paths, remove file name
        file_path = path[:-len(file_name)-1]

        # define new database node
        node = {**props, 'file_name': file_name, 'file_path': file_path}


        # store node in database
        db.append(node)

        # print file name
        #print('imported reference to file: {}'.format(file_name))


    # return track list database
    return db



# track to segments

def temporal_segmentation(audio_data, config):

    ''' Track Temporal Segmentation

        perform temporal segmentation of track, return segments database

    Args:
        track (dict): track node containing imported audio data

    Returns:
        db (list of dict): segment database instance
    '''



    ''' perform temporal segmentation '''

    '''
    # pack audio data required for temporal segmentation
    audio_data = {
        'raw_audio': raw_audio,
        'sample_rate': sample_rate,
        'beats': beats,
        'cqt': cqt,
        'evecs': evecs,
        'cnorm': cnorm,
    }

    # define depths for temporal segmentation (array by average segment length)
    depths = [4]
    i = 0 # hard code single first depth

    # temporal segmentation of audio data
    _segments = process.segment_audio_data(audio_data, depths)


    # unpack temporal segments
    split_beats = _segments[i]['split_beats']
    seg_ranges = _segments[i]['seg_ranges']


    # generate segments from split beat ranges

    # list of segments
    segments = []

    # iterate over segment ranges
    for i in range(len(seg_ranges)):

        # generate segment from ranges
        segment = {
            'start_frame': int(seg_ranges[i][0]),
            'end_frame': int(seg_ranges[i][1]),
            'mid_frame': int(seg_ranges[i][0] + (seg_ranges[i][1] - seg_ranges[i][0])/2),
            'length': int(seg_ranges[i][1] - seg_ranges[i][0]),
            #'depth': depths[0],
        }

        # store generated segment
        segments.append(segment)
    '''

    # unpack required audio data
    sample_rate = audio_data['sample_rate']
    n_frames = audio_data['n_frames']
    beats = audio_data['beats']
    evecs = audio_data['evecs']
    cnorm = audio_data['cnorm']

    # unpack config values
    X = config['X']
    lim = config['lim']
    thin = config['thin']


    ''' updated segmentation '''

    # get list segment depths as ints (n segments), log spaced from ~5 to avg. 2 beats, thin / resample
    #lim = 5
    #thin = 2

    ks = list(np.unique([ int(i) for i in np.exp(np.linspace(1, np.log(int(len(beats))/lim)) ) ]))[::thin]


    # store cluster edge beats for all k values
    edges = []

    # iterate over k values
    for i in range(len(ks)):

        # get current k value
        k = ks[i]

        print('calc segment split level {}, {} segments'.format(i, k))


        # perform segmentation using similarity and k value
        bound_frames = process.calc_segment(k = k, evecs = evecs, cnorm = cnorm, beats = beats, n_frames = n_frames,
            sample_rate = sample_rate)

        # append iteration cluster edge beats
        #edges = [ *edges, *bound_frames ]
        edges.append(bound_frames)


    # stack segment edges (split beats) for all depths, keep depth level
    k = []

    for i in range(len(edges)):
        for j in range(len(edges[i])):
            k.append( [edges[i][j], i] )

    k = np.stack(k)


    # calculate segment centre and length from edges, stack
    u = []

    for i in range(k.shape[0]-1):
        if k[i+1,0] != 0:
            u.append( [k[i,0] + (k[i+1,0] - k[i,0])/2, k[i,1], k[i+1,0] - k[i,0]] )
    u = np.stack(u)


    # get tuple unique segments (by mid frame and length), stack [mid-frame, index, count]
    unq, ind, cnt = np.unique(u[:,::2], axis =0, return_index = True, return_counts = True)
    b = np.stack([ind, cnt]).T


    # filter for segments that appear more than X
    #X = 2
    j = np.where(b[:,1] > X)[0]

    # get index of segments
    o = b[j][:,0].astype(int)


    # final segment list as [centre, length] in frames
    s = u[o,::2]

    # list of segments
    segments = []

    # iterate over segment ranges
    for i in range(len(s)):

        seg_mid = s[i, 0]
        seg_len = s[i, 1]

        # generate segment from ranges
        segment = {
            'start_frame': int(seg_mid - seg_len/2),
            'end_frame': int(seg_mid + seg_len/2),
            'mid_frame': int(seg_mid),
            'length': int(seg_len),
        }

        # store generated segment
        segments.append(segment)


    # return segments database
    return segments



# orchestration function
# iterate tracks, aggregate segments

def build_segment_database(base_path = './', props = None):

    ''' Build Segment Database

        build segment database from track search

    Args:
        base_path (string): track directory to search
        props (dict): additional track / database properties

    Returns:
        db (list of dict): segment database instance
    '''

    # if no config values, sane defaults
    if props is None:

        props = {'file_ext': 'wav',}


    # aggregate track segments
    segments = []

    # get track list database
    db = get_tracks(base_path, props)


    # trim track list to limit
    if 'file_limit' in props.keys():

        file_limit = props['file_limit']

        db = db[:int(file_limit)]


    # iterate over tracks in database
    for i in range(len(db)):

        node = db[i]

        print('processing track {}/{}: {}'.format(i+1, len(db), node['file_name']))


        try:


            ''' import track audio data '''

            # define track import file path
            file_path = '/'.join([node['file_path'], node['file_name']])

            # import track data from file
            _track = parse_audio_file(file_path)


            # unpack audio data
            raw_audio = _track['raw_audio']
            sample_rate = _track['sample_rate']


            ''' process transform audio data '''

            # pack audio data required for processing
            audio_data = {
                'raw_audio': raw_audio,
                'sample_rate': sample_rate,
            }

            # define transform config values
            bins_per_octave = 12 * 1
            n_bins = bins_per_octave * 7
            hop_length = 2048
            n_fft = 4096

            # pack config values for transformation
            config = {
                'bins_per_octave': bins_per_octave,
                'n_bins': n_bins,
                'hop_length': hop_length,
                'n_fft': n_fft,
            }


            # process transform audio data
            _transform = process.process_audio_data(audio_data, config)


            # unpack transform audio data
            beats = _transform['beats']
            cqt = _transform['cqt']
            cqt_db = _transform['cqt_db']


            ''' process segment audio data '''

            # pack audio data required for segmenting
            audio_data = {
                'raw_audio': raw_audio,
                'sample_rate': sample_rate,
                'beats': beats,
                'cqt_db': cqt_db,
            }

            # process segment audio data
            _segment = process.process_segment_data(audio_data)


            # unpack segment audio data
            evecs = _segment['evecs']
            cnorm = _segment['cnorm']


            ''' perform temporal segmentation '''

            # calculate total frame length
            n_frames = cqt.shape[1]

            # pack audio data required for segmentation
            audio_data = {
                'sample_rate': sample_rate,
                'n_frames': n_frames,
                'beats': beats,
                'evecs': evecs,
                'cnorm': cnorm,
            }

            # define and pack config values for temporal segmentation
            config = {
                # get list segment depths as ints (n segments), log spaced from ~5 to avg. 2 beats, thin / resample
                'lim': 5,
                'thin': 2,
                # filter for segments that appear more than X
                'X': 2,
            }


            # get segments from track data
            segs = temporal_segmentation(audio_data, config)


            # iterate over segments
            for seg in segs:

                # store track metadata in segment
                seg['track_name'] = node['file_name']
                seg['track_path'] = node['file_path']

                seg['track_length'] = n_frames


                # store segment in segments database
                segments.append(seg)

        except:

            print('failed to process track {}/{}: {}'.format(i+1, len(db), node['file_name']))


    # return segments database
    return segments




''' process segment database, feature extraction '''


def seg_extract_feats(seg_db, feat_list):
    '''
    # given segments database

        # group segments by track (file_name, file_path)

        # perform transforms for required data types

        # iterate through features list

        # return feature database
    '''

    # store calculated features per segment in db
    feat_db = []


    u_tracks = list(set([ (node['track_name'], node['track_path']) for node in seg_db ]))

    print('feature extraction: {} segments from {} tracks'.format(len(seg_db), len(u_tracks)))

    # group segments by track, iterate
    for i in range(len(u_tracks)):

        # get current track
        track = u_tracks[i]


        print('processing segments for track {}/{}'.format(i+1, len(u_tracks)))


        # get track segments
        track_segs = [ node for node in seg_db
            if node['track_name'] == track[0] and node['track_path'] == track[1] ]


        ''' import track audio data '''

        # define track import file path
        file_path = '/'.join([track[1], track[0]])

        # import track data from file
        _track = parse_audio_file(file_path)


        # unpack audio data (single channel)
        raw_audio = _track['raw_audio'][0,:]
        sample_rate = _track['sample_rate']


        ''' process required transform audio data '''

        ## impliment check feature list

        # define transform config values
        bins_per_octave = 12 * 1
        n_bins = bins_per_octave * 7
        hop_length = 2048
        n_fft = 4096


        # calculate cqt from raw audio
        cqt = process.transform.calc_cqt(raw_audio = raw_audio, sample_rate = sample_rate, bins_per_octave = bins_per_octave,
            n_bins = n_bins, hop_length = hop_length)

        # pre-compute a global reference power from cqt
        rp_cqt = np.max(np.abs(cqt))

        # calculate cqt amplitude in db with power reference cqt
        cqt_db = process.general.calc_amp_db(cqt = cqt, ref = rp_cqt)


        # calculate harmonic, percusive cqt from cqt
        cqt_h, cqt_p = process.transform.calc_hpss(cqt = cqt)

        # calculate percussive cqt amplitude in db with power reference cqt
        cqt_p_db = process.general.calc_amp_db(cqt = cqt_p, ref = rp_cqt)

        # calculate percussive cqt amplitude in db with power reference cqt
        #cqt_h_db = process.transform.calc_amp_db(cqt = cqt_h, ref = rp_cqt)


        # calculate onset strength, events from percussive cqt amplitude in db
        onset_strength, onset_events = process.transform.calc_onset(cqt_db = cqt_p_db, sample_rate = sample_rate, hop_length = hop_length)

        # calculate tempo and beats using percussive onset strength
        tempo, beats = process.transform.calc_beats(onset_strength = onset_strength, sample_rate = sample_rate, hop_length = hop_length)


        # calculate rms energy from cqt
        rmse = process.transform.calc_rmse(cqt = cqt, n_fft = n_fft, hop_length = hop_length)
        rmse_h = process.transform.calc_rmse(cqt = cqt_h, n_fft = n_fft, hop_length = hop_length)
        rmse_p = process.transform.calc_rmse(cqt = cqt_p, n_fft = n_fft, hop_length = hop_length)


        # calculate chroma cqt
        chroma_cqt = process.transform.calc_chroma(cqt_h, n_bins, bins_per_octave, hop_length)


        ''' calculate features each segment '''

        # iterate segments
        for seg in track_segs:

            try:

                # copy segment data to feature
                #feat = seg.copy()
                feat = {}


                ## store track and segment metadata in feature node
                feat['track_name'] = track[0]
                feat['track_path'] = track[1]

                feat['track_tempo'] = tempo
                feat['track_length'] = seg['track_length']


                feat['seg_start_frame'] = seg['start_frame']
                feat['seg_end_frame'] = seg['end_frame']
                feat['seg_mid_frame'] = seg['mid_frame']
                feat['seg_length'] = seg['length']


                feat['seg_track_length'] = seg['length'] / seg['track_length']
                feat['seg_track_depth'] = seg['mid_frame'] / seg['track_length']


                # unpack required properties
                _rmse = rmse[seg['start_frame']:seg['end_frame']]

                _rmse_h = rmse_h[seg['start_frame']:seg['end_frame']]
                _rmse_p = rmse_p[seg['start_frame']:seg['end_frame']]

                _chroma_cqt = chroma_cqt[:,seg['start_frame']:seg['end_frame']]


                track_length = rmse.shape[0]


                # calculate energy statistics
                energy = process.features.extract_energy_statistics(_rmse)
                for k,v in energy.items():
                    feat['energy-{}'.format(k)] = v

                energy_h = process.features.extract_energy_statistics(_rmse_h)
                for k,v in energy_h.items():
                    feat['energy_h-{}'.format(k)] = v

                energy_p = process.features.extract_energy_statistics(_rmse_p)
                for k,v in energy_p.items():
                    feat['energy_p-{}'.format(k)] = v

                # calculate chroma statistics
                chroma = process.features.extract_chroma_statistics(_chroma_cqt)
                for k,v in chroma.items():
                    feat['chroma-{}'.format(k)] = v

                # calculate segment temporal details
                #temporal = process.features.extract_segment_temporal(frames, track_length)
                #for k,v in temporal.items():
                #    feat['temporal-{}'.format(k)] = v


                # store segment features in database
                feat_db.append(feat)

            except:

                pass


    # return extracted features
    return feat_db








'''

define function by name and process type

    process type as module
        functions by name within module


    wrapper function that takes input data and config
        has knowledge of required inputs, validate input
            sane defaults for config or data, or obtain where required
                search for data by type, obtain required process to generate data
                    find origin state and compare to input data, calculate dependencies


    process spawns recursive sub-processess, recieves return data or termination error (unable to generate all data)



## intelligent system (function)

query language for automated data processing within database

    feed in high-level processing objective (generate segments)

        analyse objective

            determine high-level processes required (look-up)

            determine if sufficient data in database

                data requirements (from recursive process dependancies)

        feed data through process chain

            objectives determine process config values


    returns / updates database



database of information nodes


have look-up table for process definition, contains required input data, config values


standard data node types by definition





'''

