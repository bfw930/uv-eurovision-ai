

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


''' config parameters '''

# Optional input sequence length after tokenization.
# The training dataset will be truncated in block of this size for training
# Default to the model max input length for single sentence inputs (take into account special tokens)
block_size: int = -1


# Number of updates steps to accumulate before performing a backward/update pass
gradient_accumulation_steps: int = 1


# The initial learning rate for Adam
learning_rate: float = 5e-5

# Weight decay if we apply some
weight_decay: float = 0.

# Epsilon for Adam optimizer
adam_epsilon: float = 1e-8

# Max gradient norm
max_grad_norm: float = 1.


# Total number of training epochs to perform
num_train_epochs: float = 1.

# If > 0: set total number of training steps to perform. Override num_train_epochs
max_steps: int = -1

# Linear warmup over warmup_steps
warmup_steps: int = 0




''' init train env '''

device = torch.device("cuda" if torch.cuda.is_available() and not args.no_cuda else "cpu")
n_gpu = 1


# Set seed
seed = 42
random.seed(seed)
np.random.seed(seed)
torch.manual_seed(seed)
if args.n_gpu > 0:
    torch.cuda.manual_seed_all(seed)



''' Load pretrained model and tokenizer '''

# load pretrained model and tokenizer
config_class, model_class, tokenizer_class = GPT2Config, GPT2LMHeadModel, GPT2Tokenizer

# init config
config = config_class()


# Our input block size will be the max possible for the model
if block_size <= 0:
    block_size = tokenizer.max_len
else:
    block_size = min(block_size, tokenizer.max_len)


# Training new model from scratch
model = model_class(config=config)

# push model to device
model.to(device)



''' Training '''

# get training dataset
file_path = '../data/lyrics/'
train_dataset = LineByLineTextDataset(tokenizer, file_path = file_path, block_size = block_size)
#train_dataset = TextDataset(tokenizer, args, file_path=file_path, block_size = block_size)

# step training
global_step, tr_loss = train(args, train_dataset, model, tokenizer)





""" Train the model """

train_batch_size = 4


def collate(examples: List[torch.Tensor]):

    if tokenizer._pad_token is None:

        return pad_sequence(examples, batch_first = True)

    return pad_sequence(examples, batch_first = True, padding_value = tokenizer.pad_token_id)

# init random sampler on dataset
train_sampler = RandomSampler(train_dataset)

# init dataloader
train_dataloader = DataLoader(
    train_dataset, sampler = train_sampler, batch_size = train_batch_size, collate_fn = collate)



if max_steps > 0:

    t_total = max_steps
    num_train_epochs = max_steps // (len(train_dataloader) // gradient_accumulation_steps) + 1

else:
    t_total = len(train_dataloader) // gradient_accumulation_steps * num_train_epochs


# Prepare optimizer and schedule (linear warmup and decay)
no_decay = ["bias", "LayerNorm.weight"]
optimizer_grouped_parameters = [
    {
        "params": [p for n, p in model.named_parameters() if not any(nd in n for nd in no_decay)],
        "weight_decay": weight_decay,
    },
    {"params": [p for n, p in model.named_parameters() if any(nd in n for nd in no_decay)], "weight_decay": 0.0},
]

# init optimiser on model parameters
optimizer = AdamW(optimizer_grouped_parameters, lr = learning_rate, eps = adam_epsilon)

# init learning rate scheduler
scheduler = get_linear_schedule_with_warmup(
    optimizer, num_warmup_steps = warmup_steps, num_training_steps = t_total)



''' perform training '''

global_step = 0
epochs_trained = 0
steps_trained_in_current_epoch = 0

tr_loss, logging_loss = 0.0, 0.0

model.resize_token_embeddings(len(tokenizer))

model.zero_grad()

for _ in epochs_trained:

    for step, batch in enumerate(train_dataloader):

        # Skip past any already trained steps if resuming training
        if steps_trained_in_current_epoch > 0:
            steps_trained_in_current_epoch -= 1
            continue


        # unpack batch data
        inputs, labels = (batch, batch)

        # push data to device
        inputs = inputs.to(args.device)
        labels = labels.to(args.device)


        # set model to train
        model.train()

        # perform forward pass through model
        outputs = model(inputs, labels=labels)

        # obtain loss; model outputs are always tuple in transformers
        loss = outputs[0]

        # gradient accumulation
        if gradient_accumulation_steps > 1:
            loss = loss / gradient_accumulation_steps

        # backprop loss
        loss.backward()

        # store loss
        tr_loss += loss.item()


        if (step + 1) % gradient_accumulation_steps == 0:

            # perform gradient normalisation
            torch.nn.utils.clip_grad_norm_(model.parameters(), max_grad_norm)

            # step optimiser
            optimizer.step()

            # Update learning rate schedule
            scheduler.step()


            model.zero_grad()
            global_step += 1


        if max_steps > 0 and global_step > max_steps:
            break
    if max_steps > 0 and global_step > max_steps:
        break

print(global_step, tr_loss / global_step)



''' saving '''


#model.save_pretrained(output_dir)
#tokenizer.save_pretrained(output_dir)

# Load a trained model and vocabulary that you have fine-tuned
#model = model_class.from_pretrained(args.output_dir)
#tokenizer = tokenizer_class.from_pretrained(args.output_dir)
#model.to(device)
