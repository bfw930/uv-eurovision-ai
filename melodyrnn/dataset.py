
''' imports '''

# filesystem management
import os

# tensors and nn modules
import torch

# array handling
import numpy as np

# midi file import and parse
from mido import MidiFile



class MelodyDataset(torch.utils.data.Dataset):

    ''' dataset class for midi files '''

    def __init__(self, dir_path: str, cache = False, ds: int = 20):

        ''' init dataset, import midi files '''

        super().__init__()

        # store downsampling factor
        self.ds = ds

        # get and store list midi files in directory
        self.file_names = [ name for name in os.listdir(dir_path) if 'mid' in name[-4:] ]

        # import and store midi files
        self.midi_files = [ MidiFile(os.path.join(dir_path, file_name))
                for file_name in self.file_names ]


        # case filter by key
        if False:

            # get index for only midi with meta plus [melody, chords, bass] tracks
            j = [ i for i in range(len(self.file_names))
                    if len(self.midi_files[i].tracks) > 3
                    and "key='{}'".format(key) in str(self.midi_files[i].tracks[0][2]) ]


        if False:

            # get index for only midi with meta plus [melody, chords, bass] tracks
            j = [ i for i in range(len(self.file_names))
                    if len(self.midi_files[i].tracks) > 3 ]

            # filter midi file and file name lists
            self.midi_files = [ self.midi_files[i] for i in j ]
            self.file_names = [ self.file_names[i] for i in j ]


        # init store of import state
        self.import_list = [ None for _ in range(len(self.midi_files)) ]


        # pre-cache all data
        if cache:

            # iterate through midi files
            for index in range(len(self.file_names)):

                # import data to memory
                self.import_data(index)


    def import_data(self, index):

        ''' import midi data to memory '''

        # get midi by index
        midi = self.midi_files[index]

        # get midi tracks
        tracks = self.midi2tracks(midi)

        # get note tracks matrix
        matrix = self.tracks2matrix(tracks)

        # get melody format from matrix
        melody = self.matrix2melody(matrix)


        # downsample over time
        melody = melody[::self.ds]


        # store matrix in import list
        self.import_list[index] = melody


    def midi2tracks(self, midi):

        ''' extract tracks from mido.MidiFile '''

        # initialise tracks list
        tracks = []

        if len(midi.tracks) == 1:
            ts = [0]
        else:
            ts = range(len(midi.tracks))[1:4]

        # iterate over tracks in midi (excl. meta track, extra), [melody, chords, bass]
        #for i in range(len(midi.tracks))[1:4]:
        for i in ts:

            # store track data as dict for processing
            track = []

            # iterate messages in track
            for msg in midi.tracks[i][:]:

                # ensure note data only
                if msg.type in ['note_on', 'note_off']:

                    # init note data dict
                    note = {}

                    # store each note data
                    #note['type'] = msg.type
                    #note['channel'] = msg.channel
                    note['note'] = msg.note
                    note['time'] = msg.time
                    #note['velocity'] = msg.velocity
                    note['velocity'] = 0 if msg.type == 'note_off' else 1

                    # store note data
                    track.append(note)

            # store track notes
            tracks.append(track)

        # return extracted midi tracks
        return tracks


    def tracks2matrix(self, tracks: list):

        ''' convert tracks to matrix '''

        # initialise track matricies list
        m = []

        # iterate tracks
        for track in tracks:

            # initialise note state vector, 7-bit note depth
            N = np.zeros(128, dtype = np.int16)

            # initialise track note matrix (zero init column)
            M = np.zeros((128, 1), dtype = np.int16)

            # iterate messages in track
            for msg in track:

                # if time step changes, store intermediate notes
                if int(msg['time']) != 0:

                    # extend note state vector over range time step
                    n = np.stack([ N for _ in range( int(msg['time']) ) ]).T

                    # append note state vector to track note matrix
                    M = np.concatenate( [M, n], axis = 1 )

                # update value of note vector by index
                N[int(msg['note'])] = int(msg['velocity'])

            # store track note matrix
            m.append(M)


        # get max length track
        s = max([ track.shape[1] for track in m ])

        # pad tracks to max length of time axis, stack on new axis
        M = np.stack([ np.pad(track, ((0, 0), (0, s - track.shape[1])))
            for track in m ], axis = 2)

        # return stacked tracks note matrix
        return M


    def matrix2melody(self, matrix):

        ''' extract melody from note matrix '''

        # get track note matrix for melody only
        M = matrix[:,:,0]

        # init zero melody, default negative one
        #melody = np.ones(M.shape[1])*-1

        melody = np.zeros(M.shape[1])

        # get index (note, time) where nonzero
        j = np.where( M != 0 )

        # set melody note at time by index
        melody[j[1]] = j[0]

        # return extracted melody
        return melody


    def __getitem__(self, index):

        ''' return tracks note matrix '''

        # check for import state
        if self.import_list[index] is None:

            # import data to memory
            self.import_data(index)


        # return data if already imported
        return self.import_list[index]


        '''
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
        '''


    def __len__(self):

        ''' return total midi files '''

        # return number of midi files
        return len(self.file_names)



class MelodyDataLoader(torch.utils.data.DataLoader):

    def __init__(self, dataset, batch_size, seq_len, overlap_len,
                 *args, **kwargs):

        super().__init__(dataset, batch_size, *args, **kwargs)

        self.seq_len = seq_len
        self.overlap_len = overlap_len

    def __iter__(self):

        for batch in super().__iter__():

            (batch_size, n_samples) = batch.size()

            reset = True

            #print(self.overlap_len, n_samples, self.seq_len)

            for seq_begin in range(self.overlap_len, n_samples, self.seq_len)[:-1]:

                from_index = seq_begin - self.overlap_len

                to_index = seq_begin + self.seq_len

                sequences = batch[:, from_index : to_index]

                input_sequences = sequences[:, : -1]

                #print(input_sequences.shape)

                target_sequences = sequences[:, self.overlap_len :].contiguous()

                yield (input_sequences, reset, target_sequences)

                reset = False


    def __len__(self):
        raise NotImplementedError()

