
''' Imports '''

# core orchestration protocols
#from .core import init

from .model import SampleRNN
from .model import Predictor
from .model import Generator

# wrapper for optimiser
from .optim import gradient_clipping

# training criterion
from .nn import sequence_nll_loss_bits

# import audio dataset management
#from dataset import SampleRNNDataset
from .dataset import MelodyDataset

#from dataset import DataLoader
from .dataset import MelodyDataLoader
