

"""
Fine-tuning the library models for language modeling on a text file (GPT, GPT-2, BERT, RoBERTa).
GPT and GPT-2 are fine-tuned using a causal language modeling (CLM) loss while BERT and RoBERTa are fine-tuned
using a masked language modeling (MLM) loss.
"""


#import glob
#import logging
import os
import pickle
import random
import re
import shutil
from typing import Dict, List, Tuple


import numpy as np
import torch

from torch.nn.utils.rnn import pad_sequence

from torch.utils.data import DataLoader, Dataset, RandomSampler, SequentialSampler


from .dataset import TextDataset, LineByLineTextDataset


from transformers import (

    WEIGHTS_NAME,
    AdamW,

    GPT2Config,
    GPT2LMHeadModel,
    GPT2Tokenizer,

    PreTrainedModel,
    PreTrainedTokenizer,

    get_linear_schedule_with_warmup,

)

