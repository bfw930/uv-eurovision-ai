
''' Imports '''

# data array processing
import numpy as np

# density clustering using HDBSCAN* algorithm
import hdbscan

# dimensionality reduction using UMAP
import umap


''' Dimensionality Reduction Functions '''

def umap_embedding(dimensions, n_neighbours = 15, min_dist = 0.8, n_components = 2):

    ''' Get UMAP Embedding

        Perform dimensionality reduction on collection of audio data by extracted features using UMAP library; by
        default only uses left channel feature data; feature data embedded on 2d manifold for exploration

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

    ## UMAP for dimansionality reduction, prepare for clustering

    # initialise umap mapper instance
    mapper = umap.UMAP(n_neighbors = n_neighbours, n_components = n_components, min_dist = min_dist)


    # perform embedding using dataset, optionally supervised with labels
    mapper.fit( dimensions )

    #mapper.fit(data['dimensions'], y = data['labels'])


    # get embedding for training data
    embedding = mapper.embedding_

    # use trained mapper and transform data into embedding
    #embedding = mapper.transform(data)


    # return 2d embedding
    return mapper, embedding





    ## UMAP

    # initialise umap mapper instance
    # mapper = umap.UMAP(n_neighbors=15, n_components=2, min_dist=0.1)

    # perform embedding using dataset, optionally supervised with labels
    # mapper.fit(data, y = labels)

    # get embedding for training data
    # embedding = mapper.embedding_

    # use trained mapper and transform data into embedding
    # embedding = mapper.transform(data)


    ## HDBSCAN

    # initialise hdbscan clusterer instance
    # clusterer = hdbscan.HDBSCAN(min_cluster_size=100, min_samples=15, prediction_data=True)

    # perform density clustering using dataset
    # clusterer.fit(data)

    # get existing clusterer fit labels
    # clusterer.labels_

    # get existing clusterer fit probability score within each cluster
    # clusterer.probabilities_

    # use trained clusterer for classification
    # labels, strengths = hdbscan.approximate_predict(clusterer, data)



## Clustering Analysis Functions

def dimension_reduction_umap(_nodes_list, _label_param, _feature_list):

    ''' Dimension Reduction using UMAP

        Perform dimensionality reduction on collection of audio data by extracted features using UMAP library; by
        default only uses left channel feature data; feature data embedded on 2d manifold for exploration

    Args:
        _db (dict): database instance
        _nodes_list (list): indicies of feature nodes within database features list
        _feature_list (list): list of features by name to include as dimensions

    Returns:
        np.array: results of dimensionality reduction, np.array of shape [data nodes, embedding dimensions]
    '''


    # return results
    return results



### clustering_label_umap_hdbscan
    # given dataset of feature dimensions, perform dimensionality reduction using umap in n dimensions
    # perform clustering on umap embedding using hdbscan
    # use trained classifier to label data entries


### clustering_label_hdbscan
    # given dataset of feature dimensions, cluster and label entries
    # need to normalise feature dimensions


### clustering_train_hdbscan
    # given labelled dataset and features list, train classifier
    # need to normalise feature dimensions


