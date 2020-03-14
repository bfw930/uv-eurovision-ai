
import os
import pickle
#import random
#import re
#import shutil
#from typing import Dict, List, Tuple


import numpy as np
import torch

#from torch.nn.utils.rnn import pad_sequence

from torch.utils.data import Dataset


class TextDataset(Dataset):

    def __init__(self, tokenizer: PreTrainedTokenizer, args, file_path: str, block_size = 512):

        block_size = block_size - (tokenizer.max_len - tokenizer.max_len_single_sentence)

        directory, filename = os.path.split(file_path)

        cached_features_file = os.path.join(
            directory, args.model_type + '_cached_lm_' + str(block_size) + '_' + filename)

        self.examples = []

        with open(file_path, encoding = 'utf-8') as f:

            text = f.read()


        tokenized_text = tokenizer.convert_tokens_to_ids(tokenizer.tokenize(text))

        # Truncate in block of block_size, last sample lost (no padding)
        for i in range(0, len(tokenized_text) - block_size + 1, block_size):

            self.examples.append(tokenizer.build_inputs_with_special_tokens(tokenized_text[i : i + block_size]))

        with open(cached_features_file, "wb") as handle:

                pickle.dump(self.examples, handle, protocol = pickle.HIGHEST_PROTOCOL)


    def __len__(self):
        return len(self.examples)


    def __getitem__(self, item):
        return torch.tensor(self.examples[item], dtype = torch.long)




class LineByLineTextDataset(Dataset):

    def __init__(self, tokenizer: PreTrainedTokenizer, args, file_path: str, block_size = 512):

        # no feature cache
        with open(file_path, encoding = 'utf-8') as f:

            lines = [line for line in f.read().splitlines() if (len(line) > 0 and not line.isspace())]


        self.examples = tokenizer.batch_encode_plus(
            lines, add_special_tokens = True, max_length = block_size)['input_ids']


    def __len__(self):
        return len(self.examples)


    def __getitem__(self, i):
        return torch.tensor(self.examples[i], dtype=torch.long)


