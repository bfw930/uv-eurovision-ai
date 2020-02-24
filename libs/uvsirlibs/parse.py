
''' Imports '''

# audio analysis library
from librosa import load


''' File Parsing Functions '''

def parse_audio_file( file_path ):

    ''' Parse Audio File

        Parse source audio data file using librosa library

    Args:
        file_path (str): full file path including name with extension

    Returns:
        dict: data and parameters parsed from file
    '''

    # import audio data from file
    raw_audio, sample_rate = load( file_path, mono = False, sr = 44100 )

    # store raw audio data and parameters
    data = {
        'raw_audio': raw_audio,
        'sample_rate': sample_rate,
        'channels': raw_audio.shape[0],
        'length': (raw_audio.shape[1] / sample_rate),
    }

    # return parsed audio data and parameters
    return data
