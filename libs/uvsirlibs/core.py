
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
import os, sys




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






''' Initialise Database '''


def init():

    ''' Initialise Database

    simple initialise protocol, empty defaults

    Returns:
        dict: database instance
    '''

    # generate database instance
    db = database.init_db()

    # return database instance
    return db




''' Import Audio Data '''

def import_audio_file_data(db, base_path, file_name, node_props):

    ''' Import Audio Data from File

        Import raw audio data from file and store in database; extract audio features and store in database

    Args:
        db (dict): database instance
        base_path (str): full path to directory containing file for processing
        file_name (str): source file name including extension
        node_vars (dict): required / additional audio data variables

    Returns:
        (int): index of parsed and imported raw audio data node appended to referenced database instance
    '''

    # build full file path
    file_path = '{}/{}'.format(base_path, file_name)

    # import data from file
    data = parse_audio_file(file_path = file_path)


    # define entry node
    node_props = {**node_props, 'file_name': file_name, **data, 'process_state': 'imported',}

    # generate and add new raw_audio entry node to database instance
    node_index = database.add_entry(db = db, node_class = 'raw_audio', node_props = node_props, node_rels = {} )


    # return added entry node index
    return node_index



def import_audio_files(db, base_path, node_props, file_format = 'mp3', limit = None):

    ''' Import Audio Data from Files

        Batch import audio files within a directory

    Args:
        db (dict): database instance
        base_path (str): full path to directory containing files for import
        node_vars (dict): additional vars to store in data entry node
        file_format (str): file format by extension
        limit (int): limit number of files to import

    Returns:
        none: database instance updated with imported audio data
    '''

    # get list files within directory
    files_list = [f for f in os.listdir(base_path) if os.path.isfile(os.path.join(base_path, f))]

    # filter files list for valid format by file extension
    files_list = [f for f in files_list if f.split('.')[-1] == file_format]


    # limit import number files if passed
    if limit is not None:
        files_list = files_list[:int(limit)]


    # iterate files
    for file_name in files_list:

        # parse each data file, store measurement node in database instance
        index = import_audio_file_data(db = db, base_path = base_path, file_name = file_name, node_props = node_props)


        print('successful import data from file: {}'.format(file_name))





''' Process Audio Data Node '''


def process_raw_audio(db, node_index):

    ''' Process Raw Audio Data

        Perform standard processing on raw_audio data node; includes config and signal transforms; calculation results
        stored within raw_audio node properties

        ## process raw audio data

        # perform standard processing on raw_audio data node
        # calculation results stored within raw_audio node properties
        # include config values for calculations

            # signal transforms: energy envelope, spectrograms
            # onset and beat detection, beat synchronisation
            # label raw_audio node with process_state as process

    Args:
        db (dict): database instance
        node_index (int): index of audio data node

    Returns:
        none: raw audio data node updated with process results
    '''

    # get node from index
    node = db[node_index]

    # check node_index is of node type entry and node class raw audio
    #if node['props']['node_type'] == 'entry' and node['props']['node_class'] == 'raw_audio':

    # check node process state
    if 'process_state' in node['props'].keys() and node['props']['process_state'] in ['imported']:


        # perform standard raw audio data processing, return results
        results = process.process_audio_data(raw_audio_props = node['props'])

        # iterate each process results
        for name, data in results.items():

            # store process result values as property in raw audio node
            node['props'][name] = data


        # perform standard segmentation audio data processing, return results
        results = process.process_segment_data(raw_audio_props = node['props'])

        # iterate each process results
        for name, data in results.items():

            # store process result values as property in raw audio node
            node['props'][name] = data


        # set process state flag as processed
        node['props']['process_state'] = 'processed'



def segment_raw_audio(db, node_index, depths):

    ''' Segment Raw Audio Data

        Perform standard segmentation process on raw_audio data node

        ## segment raw audio data

        # perform segmentation of raw_audio
        # (depends process_raw_audio completed)

            # generate segment nodes with relation to raw_audio node
            # segmented by time/frame range, referenced to complete raw_audio node data
            # default to generate single segement node for complete raw_audio data

                # tiered temporal segmentation; sub-segment;
                # segment using both self-similarity and beat segmentation

                # store similarity values for feature extraction

    Args:
        db (dict): database instance
        node_index (int): index of processed audio data node
        depth (list of int): segmentation depths in average beats per segment

    Returns:
        none: new segment nodes added to database instance
    '''

    # get node from index
    node = db[node_index]

    # check node process state
    if 'process_state' in node['props'].keys() and node['props']['process_state'] in ['processed', 'segmented']:


        # get segmentation results
        segments = process.segment_audio_data(raw_audio_props = node['props'], depths = depths)


        #node['props']['split_beats'] = segments['split_beats']

        for i in range(len(depths)):


            # iterate segment ranges
            for seg_range in segments[i]['seg_ranges']:


                # define segment node properties and relations
                node_props = {
                    'start_frame': seg_range[0],
                    'end_frame': seg_range[1],
                    'length': seg_range[1] - seg_range[0],
                    'depth': depths[i],
                }
                node_rels = {'raw_audio': [node_index],}

                # generate and add new segment entry node to database instance
                segm_node_index = database.add_entry(db = db, node_class = 'segment', node_props = node_props,
                    node_rels = node_rels )


                # add relation in raw audio node
                database.add_relation(db = db, node_index = segm_node_index, rel_type = 'segment', rel_index = node_index)


        # set process state flag as segmented
        node['props']['process_state'] = 'segmented'



def extract_audio_features(db, node_index):

    ''' Extract Audio Features

        ## extract features

        # perform feature extraction on segment node
        # (depends process_raw_audio completed)
        # (depends segment_audio completed)

            # calculate feature values for raw_audio segment
            # take reference to segment node, internal reference for raw_audio and temporal ranges

                # pre-process segment (include dimensionality reduction dependent on segment length)
                # decompose segment data into components (harmonic/percussive, energy envelope, chroma)
                # calculate feature dimension values

                    # signal energy: mean audio energy, variance, dynamics over segment
                    # harmonic/percussive content: average energy relative split and dynamics over segment
                    # chroma content: average chroma note, variance and dynamics over segment

                    # self-similarity: degree of repetition over multiple timescales (segment tiers)

    Args:
        db (dict): database instance
        node_index (int): index of audio data node

    Returns:
        none: raw audio data node updated with process results
    '''

    # get node from index
    seg_node = db[node_index]

    # generate frames range index
    seg_range = list(range(seg_node['props']['start_frame'], seg_node['props']['end_frame']))


    # get source raw audio node props through relation
    audio_node_index = seg_node['rels']['raw_audio'][0]

    # get audio data from raw audio node properties
    audio_node_data = db[audio_node_index]['props']


    # perform feature extraction on segment, return results
    results = process.extract_audio_features(raw_audio_props = audio_node_data, frames = seg_range)

    # iterate each process results
    for name, data in results.items():

        # store process result values as property in segment node
        seg_node['props'][name] = data




def get_dimensions(db, features, indicies = None, depth = None):

    ''' Get Audio Feature Dimensions

        ## feature dimension aggregation

            # perform aggregation of extracted features over segments into dimensions
            # (depends process_raw_audio completed)
            # (depends segment_audio completed)
            # (depends extract_features completed)

                # collect segmentation structure and extracted features for raw_audio node
                # calculate aggregate feature values for multi-segment results

                # normalise feature dimensions derived from each audio node
                # store normalised 'fingerprint' for each feature dimensions subset

    Args:
        db (dict): database instance
        features (dict): list of features to aggregate

    Returns:
        dict: audio feature dimensions and headers
    '''

    if indicies is not None:

        seg_indicies = []

        for index in indicies:
            for seg_index in db[index]['rels']['segment']:
                seg_indicies.append( seg_index )

    else:
        # get indicies for all segment nodes, get segment nodes
        seg_indicies = database.search.get_nodes_by_class(db, 'segment')


    # filter for single depth
    if depth is not None:

        # search segments for specific depth and update indicies with subset
        seg_indicies = database.get_nodes_by_props(
            db = db, props = {'depth': depth}, indicies = seg_indicies)


    segments = [ db[index] for index in seg_indicies ]


    # aggregate audio features from segment nodes into dimensions
    dimensions = analysis.feature_aggregation(segments = segments, features = features)


    ## segment labelling
    labels = []

    # get label from raw audio node for each segment
    for segment in segments:

        # get raw audio segment
        raw_audio_index = segment['rels']['raw_audio'][0]
        raw_audio_node = db[raw_audio_index]

        #label = raw_audio_node['props']['']

        # store raw audio node index as label
        labels.append( raw_audio_index )


    # store labels with dimension
    dimensions['labels'] = {'audio_index': labels, 'seg_index': seg_indicies}


    # return aggregated audio feature dimensions with headers
    return dimensions



def get_2d_embedding(dimensions, n_neighbors = 15, min_dist = 0.8):

    ''' Get 2-Dimensional Embedding

        ## clustering

            # perform clustering on feature dimensions against semantic tags
            # (depends process_raw_audio completed)
            # (depends segment_audio completed)
            # (depends extract_features completed)
            # (depends aggregate_feature_dimension completed)

                # given specified feature dimensions subset, collect fingerprint from audio data nodes
                # collect semantic tag information for each audio node
                # perform clustering over feature dimensions, classify and group by semantic tags

    Args:
        dimensions (dict): feature dimensions with headers and labels

    Returns:
        dict: audio feature dimensions and headers
    '''

    ## UMAP for dimansionality reduction to 2 dimensions, prepare for display

    # set UMAP parameters
    params = {'n_neighbours': n_neighbors, 'min_dist': min_dist, 'n_components': 2}

    # get embedding
    mapper, embedding = analysis.dimension_reduction(dimensions = dimensions, params = params)


    # return 2d embedding
    return mapper, embedding



def get_seg_audio(db, seg_index):

    ''' Get Segment Audio

    Args:
        db (dict): database instance
        seg_index (int): segment index

    Returns:
        dict: segment raw audio data and properties
    '''

    # get segment node
    seg_node = db[seg_index]

    # get raw audio node through segment relation
    raw_audio_node_index = seg_node['rels']['raw_audio'][0]
    raw_audio_node = db[raw_audio_node_index]


    # get raw audio and properties
    raw_audio = raw_audio_node['props']['raw_audio']
    hop_length = raw_audio_node['props']['hop_length']
    sample_rate = raw_audio_node['props']['sample_rate']


    # get start and end samples
    start = seg_node['props']['start_frame'] * hop_length
    end = seg_node['props']['end_frame'] * hop_length

    # get raw audio data within segment range
    seg_audio = raw_audio[:, start: end]

    #length = ( (seg_node['props']['end_frame'] * hop_length / sample_rate) -
    #    (seg_node['props']['start_frame'] * hop_length / sample_rate) )


    # return audio segment with sampling rate
    return seg_audio, sample_rate#, length




''' Orchestration Protocols '''



def process_audio_data(db):

    ''' Batch Process Audio Data

        Batch process imported raw audio data with standard processes; includes data processsing, segmentation, feature
        extraction, dimension aggregation

    Args:
        db (dict): database instance

    Returns:
        none: database instance updated with processed audio data
    '''

    # get indicies of raw audio nodes
    indicies = database.get_nodes_by_class(db = db, node_class = 'raw_audio')

    # iterate each raw audio node
    for node_index in indicies[:]:

        print( '{} of {}'.format(indicies.index(node_index)+1, len(indicies)+1 ) )

        # process raw audio node
        process_raw_audio(db = db, node_index = node_index)



def segment_audio_data(db, depths):

    ''' Batch Segment Audio Data

        Batch segment processed raw audio data

    Args:
        db (dict): database instance

    Returns:
        none: database instance updated with processed audio data
    '''

    # get indicies of raw audio nodes
    indicies = database.get_nodes_by_class(db = db, node_class = 'raw_audio')

    # iterate each raw audio node
    for node_index in indicies:


        # segment raw audio node
        segment_raw_audio(db = db, node_index = node_index, depths = depths)



def audio_feature_extraction(db):

    ''' Batch Segment Audio Data

        Batch segment processed raw audio data

    Args:
        db (dict): database instance

    Returns:
        none: database instance updated with processed audio data
    '''

    # get indicies of raw audio nodes
    indicies = database.get_nodes_by_class(db = db, node_class = 'segment')


    # iterate each raw audio node
    for node_index in indicies:

        try:

            # segment raw audio node
            extract_audio_features(db = db, node_index = node_index)

        except:
            print('failed on {}'.format(node_index))
            pass
