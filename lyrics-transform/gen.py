
""" Conditional text generation with the auto-regressive models of the library (GPT/GPT-2/CTRL/Transformer-XL/XLNet)
"""

#import argparse
#import logging

import numpy as np
import torch

from transformers import (
    GPT2LMHeadModel,
    GPT2Tokenizer,
)



# select Model type
model_type = 'gpt2'

# select Path to pre-trained model or shortcut name
model_name_or_path = 'gpt2'


# input prompt to model
prompt: str = ''

# length desired output ?
length: int = 20

# Token at which text generation is stopped
stop_token: str = None

# temperature of 1.0 has no effect, lower tend toward greedy sampling
temperature:float = 1.


# primarily useful for CTRL model; in that case, use 1.2
repetition_penalty: float = 1.


k: int = 0
p: float = 0.9


# The number of samples to generate
num_return_sequences: int = 1



device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
n_gpu = 1

# random seed
seed = 42

np.random.seed(seed)
torch.manual_seed(seed)
if n_gpu > 0:
    torch.cuda.manual_seed_all(seed)



# Initialize the model and tokenizer
model_class, tokenizer_class = (GPT2LMHeadModel, GPT2Tokenizer)

tokenizer = tokenizer_class.from_pretrained('gpt2')
model = model_class.from_pretrained('../data/lyrics/model-en-lyrics-02')
model.to(device)




MAX_LENGTH = int(10000)  # Hardcoded max length to avoid infinite loop

def adjust_length_to_model(length, max_sequence_length):
    if length < 0 and max_sequence_length > 0:
        length = max_sequence_length
    elif 0 < max_sequence_length < length:
        length = max_sequence_length  # No generation bigger than model size
    elif length < 0:
        length = MAX_LENGTH  # avoid infinite loop
    return length


length = adjust_length_to_model(
    length, max_sequence_length = model.config.max_position_embeddings)


# encode prompt text, push to device
prompt_text = prompt
encoded_prompt = tokenizer.encode(prompt_text, add_special_tokens = False, return_tensors = "pt")
encoded_prompt = encoded_prompt.to(device)

# generate output sequence
output_sequences = model.generate(

    input_ids=encoded_prompt,
    max_length=length + len(encoded_prompt[0]),
    temperature=temperature,
    top_k=k,
    top_p=p,
    repetition_penalty=repetition_penalty,
    do_sample=True,
    num_return_sequences=num_return_sequences,
)


# Remove the batch dimension when returning multiple sequences
if len(output_sequences.shape) > 2:
    output_sequences.squeeze_()

generated_sequences = []


for generated_sequence_idx, generated_sequence in enumerate(output_sequences):
    print("=== GENERATED SEQUENCE {} ===".format(generated_sequence_idx + 1))
    generated_sequence = generated_sequence.tolist()

    # Decode text
    text = tokenizer.decode(generated_sequence, clean_up_tokenization_spaces=True)

    # Remove all text after the stop token
    text = text[: text.find(args.stop_token) if args.stop_token else None]

    # Add the prompt at the beginning of the sequence. Remove the excess text that was used for pre-processing
    total_sequence = (
        prompt_text + text[len(tokenizer.decode(encoded_prompt[0], clean_up_tokenization_spaces=True)) :]
    )

    generated_sequences.append(total_sequence)
    print(total_sequence)

#return generated_sequences

