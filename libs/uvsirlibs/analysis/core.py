
''' Data Analysis Functions

    Feature Aggregation, Dimensionality Reduction, Clustering Analysis
'''



''' Imports '''

# perform umap embedding
from .cluster import umap_embedding


# nd array handling
import numpy as np



''' Core Analysis Functions '''

def feature_aggregation(segments, features):

    ''' Feature Aggregation

        Given audio segment indicies and features list, aggregate extracted audio features and return as dimensions;
        returned dimensions ready for dimensionality reduction and clustering

    Args:
        seg_indicies (list): list of audio segment indicies
        features (dict): features to be aggregated

    Returns:
        dict: feature dimensions and labels
    '''

    # feature dimension data storage
    data = {'dimensions': []}


    # iterate segment nodes
    for segment in segments:

        try:

            # store feature dimensions and headers
            dimensions = []
            headers = []

            # iterate list of features
            for feature, subset in features.items():

                # if passed only feature name, aggregate all feature data
                if len(subset) == 0:

                    # store feature data as dimension
                    [ dimensions.append(dim) for dim in segment['props'][feature].values() ]

                    # store dimension header from feature name and sub-name
                    [ headers.append( '{}-{}'.format(feature, name) ) for name in segment['props'][feature].keys() ]

                # if passed a feature with subset list, aggregate subset of feature data
                elif len(subset) > 0:

                    # store feature data as dimension
                    [ dimensions.append(dim) for name, dim in segment['props'][feature].items()
                        if name in subset ]

                    # store dimension header from feature name and sub-name
                    [ headers.append( '{}-{}'.format(feature, name) ) for name in segment['props'][feature].keys()
                        if name in subset ]


            # aggregate dimensions data
            data['dimensions'].append( dimensions )

        except:
            print('failed a segment')
            pass


    # aggregate and stack all segment dimension data as array
    data['dimensions'] = np.stack( data['dimensions'] )


    # aggregate dimension headers
    data['headers'] = headers


    # return aggregate feature dimensions
    return data



def dimension_reduction(dimensions, params):

    ''' Dimensionality reduction


    Args:
        dimensions (np.array): dimension data
        params (dict): parameters for dimensionality reduction

    Returns:
        np.array: embedding of dimensions with reduced dimensionality
    '''

    # UMAP for dimensionality reduction
    mapper, embedding = umap_embedding(dimensions = dimensions, n_neighbours = params['n_neighbours'],
        min_dist = params['min_dist'], n_components = params['n_components'])


    # return dimensions embedding
    return mapper, embedding
