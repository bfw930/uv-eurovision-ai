

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

#from torch.utils.data.distributed import DistributedSampler

#from tqdm import tqdm, trange


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



MODEL_CLASSES = {
    'gpt2': (GPT2Config, GPT2LMHeadModel, GPT2Tokenizer),
}



''' required config parameters '''

# The input training data file (a text file)
train_data_file: str = None

# The output directory where the model predictions and checkpoints will be written
output_dir: str = None

# The model architecture to be trained or fine-tuned
model_type: str = None


''' additional config parameters '''

# An optional input evaluation data file to evaluate the perplexity on (a text file)
eval_data_file: str = None

# Whether distinct lines of text in the dataset are to be handled as distinct sequences
line_by_line: bool = True


# Whether to continue from latest checkpoint in output_dir
should_continue: bool = True

# The model checkpoint for weights initialization. Leave None if you want to train a model from scratch
model_name_or_path: str = None


# Train with masked-language modeling loss instead of language modeling
mlm: bool = True

# Ratio of tokens to mask for masked language modeling loss
mlm_probability: float = .15


# Optional pretrained config name or path if not the same as model_name_or_path. If both are None, initialize a new config
config_name: str = None

# Optional pretrained tokenizer name or path if not the same as model_name_or_path. If both are None, initialize a new tokenizer
tokenizer_name: str = None

# Optional directory to store the pre-trained models downloaded from s3 (instead of the default one)
cache_dir: str = None


# Optional input sequence length after tokenization.
# The training dataset will be truncated in block of this size for training
# Default to the model max input length for single sentence inputs (take into account special tokens)
block_size: int = -1


# Whether to run training
do_train: bool = True

# Whether to run eval on the dev set
do_eval: bool = True

# Run evaluation during training at each logging step
evaluate_during_training: bool = True


# Batch size per GPU/CPU for training
per_gpu_train_batch_size: int = 4

# Batch size per GPU/CPU for evaluation
per_gpu_eval_batch_size: int = 4


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


# Log every X updates steps
logging_steps: int = 500

# Save checkpoint every X updates steps
save_steps: int = 500

# Limit the total amount of checkpoints, delete the older checkpoints in the output_dir, does not delete by default
save_total_limit: int = None


# Evaluate all checkpoints starting with the same prefix as model_name_or_path ending and ending with step number
eval_all_checkpoints: bool = True

# Avoid using CUDA when available
no_cuda: bool = True

# Overwrite the content of the output directory
overwrite_output_dir: bool = True


# Overwrite the cached training and evaluation sets
overwrite_cache: bool = True

# random seed for initialization
seed: int = 42

# Whether to use 16-bit (mixed) precision (through NVIDIA apex) instead of 32-bit
fp16: bool = True

# For fp16: Apex AMP optimization level selected in ['O0', 'O1', 'O2', and 'O3']."
# "See details at https://nvidia.github.io/apex/amp.html
fp16_opt_level: str = '01'




''' init train env '''


# Setup CUDA
if args.local_rank == -1 or args.no_cuda:

    device = torch.device("cuda" if torch.cuda.is_available() and not args.no_cuda else "cpu")

    args.n_gpu = 0 if args.no_cuda else torch.cuda.device_count()

args.device = device


# Set seed
set_seed(args)



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



if args.model_name_or_path:

    # init new model
    model = model_class.from_pretrained(
        args.model_name_or_path,
        from_tf=bool(".ckpt" in args.model_name_or_path),
        config=config,
        cache_dir=args.cache_dir,
    )

else:

    # Training new model from scratch
    model = model_class(config=config)

# push model to device
model.to(args.device)



''' Training '''

if args.do_train:


    # get training dataset
    train_dataset = load_and_cache_examples(args, tokenizer, evaluate=False)

    # step training
    global_step, tr_loss = train(args, train_dataset, model, tokenizer)



# Saving best-practices: if you use save_pretrained for the model and tokenizer, you can reload them using from_pretrained()
if args.do_train and (args.local_rank == -1 or torch.distributed.get_rank() == 0):
    # Create output directory if needed
    if args.local_rank in [-1, 0]:
        os.makedirs(args.output_dir, exist_ok=True)


    logger.info("Saving model checkpoint to %s", args.output_dir)
    # Save a trained model, configuration and tokenizer using `save_pretrained()`.
    # They can then be reloaded using `from_pretrained()`


    model_to_save = (
        model.module if hasattr(model, "module") else model
    )  # Take care of distributed/parallel training

    model_to_save.save_pretrained(args.output_dir)

    tokenizer.save_pretrained(args.output_dir)

    # Good practice: save your training arguments together with the trained model
    torch.save(args, os.path.join(args.output_dir, "training_args.bin"))


    # Load a trained model and vocabulary that you have fine-tuned
    model = model_class.from_pretrained(args.output_dir)

    tokenizer = tokenizer_class.from_pretrained(args.output_dir)

    model.to(args.device)



''' Evaluation '''

results = {}

if args.do_eval and args.local_rank in [-1, 0]:

    checkpoints = [args.output_dir]

    if args.eval_all_checkpoints:
        checkpoints = list(
            os.path.dirname(c) for c in sorted(glob.glob(args.output_dir + "/**/" + WEIGHTS_NAME, recursive=True))
        )
        logging.getLogger("transformers.modeling_utils").setLevel(logging.WARN)  # Reduce logging
    logger.info("Evaluate the following checkpoints: %s", checkpoints)



    for checkpoint in checkpoints:

        global_step = checkpoint.split("-")[-1] if len(checkpoints) > 1 else ""

        prefix = checkpoint.split("/")[-1] if checkpoint.find("checkpoint") != -1 else ""

        # perform evaluation of checkpoint

        # load checkpoint from file into model
        model = model_class.from_pretrained(checkpoint)

        # push model to device
        model.to(args.device)

        # evaluate model
        result = evaluate(args, model, tokenizer, prefix=prefix)

        result = dict((k + "_{}".format(global_step), v) for k, v in result.items())

        results.update(result)


