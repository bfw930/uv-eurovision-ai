
''' Imports '''

# data array processing
import numpy as np

# import scipy libraries for matricies
import scipy

# sci-kit learn clustering
import sklearn.cluster

# audio analysis library
import librosa



''' Audio Segmentation Functions '''

def calc_simil(cqt_db_sync, mfcc_sync):

    ''' Calculate Similarity

        ## calculate balanced combination of recurrence and path similarity

    Args:
        cqt_db_sync (np.array): audio data cqt synced to beats

    Returns:

    '''

    # build a weighted recurrence matrix using beat-synchronous CQT
    R = librosa.segment.recurrence_matrix(data = cqt_db_sync, k = None, width = 3, metric = 'euclidean',
        sym = True, sparse = False, mode = 'affinity', bandwidth = None, axis = -1)

    # prepare median timelag filter
    df = librosa.segment.timelag_filter(function = scipy.ndimage.median_filter, pad = True, index = 0)

    # enhance diagonals with a median filter
    Rf = df(R, size = (1, 7))


    ## build the sequence matrix (S_loc) using mfcc-similarity

    # calculate path distance from mfcc
    path_distance = np.sum(np.diff(mfcc_sync, axis = 1)**2, axis = 0)


    # sigma as median distance between successive beats, calculate path similarity
    sigma = np.median(path_distance)
    path_sim = np.exp(-path_distance / sigma)

    ## calculate path similarity, trim R_path to shape Rf due to diag padd
    R_path = (np.diag(path_sim, k = 1) + np.diag(path_sim, k = -1))[:Rf.shape[0],:Rf.shape[0]]


    # compute the balanced combination
    deg_path = np.sum(R_path, axis = 1)
    deg_rec = np.sum(Rf, axis = 1)

    mu = deg_path.dot(deg_path + deg_rec) / np.sum((deg_path + deg_rec)**2)
    A = mu * Rf + (1 - mu) * R_path


    # compute the normalized Laplacian
    L = scipy.sparse.csgraph.laplacian(A, normed = True)

    # spectral decomposition
    evals, evecs = scipy.linalg.eigh(L)

    # smooth small discontinuities with median filter
    evecs = scipy.ndimage.median_filter(evecs, size = (9, 1))

    # cumulative normalization for symmetric normalize laplacian eigenvectors
    cnorm = np.cumsum(evecs**2, axis = 1)**0.5


    # return calculated similarity data
    return A, evecs, cnorm



def calc_segment(k, evecs, cnorm, beats, n_frames, sample_rate):

    ''' Calculate segmentation

        ## use normalised laplacian from balanced combination of recurrence and path similarity;

        # segment using matched number of normalized eigenvectors and k-means clusters

    Args:

    Returns:

    '''

    # select depth of segmentation, number of normalized eigenvectors, match to desired clusters
    X = evecs[:, :k] / cnorm[:, k-1:k]


    # use these k components to cluster beats into segments
    KM = sklearn.cluster.KMeans(n_clusters = k)
    seg_ids = KM.fit_predict(X)


    # locate segment boundaries from the label sequence
    bound_beats = 1 + np.flatnonzero(seg_ids[:-1] != seg_ids[1:])

    # Count beat 0 as a boundary
    bound_beats = librosa.util.fix_frames(frames = bound_beats, x_min = 0)

    # Compute the segment label for each boundary
    bound_segs = list(seg_ids[bound_beats])


    # Convert beat indices to frames
    bound_frames = beats[bound_beats]

    # Make sure we cover to the end of the track
    bound_frames = librosa.util.fix_frames(frames = bound_frames, x_min = None, x_max = n_frames-1)


    # return calculated segment data
    return bound_frames
