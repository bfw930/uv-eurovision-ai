
''' Imports '''

# orchestration protocols
from .operate import init_db, add_class, add_entry, add_relation

# database store / load, static file
from .storage import store_to_file, load_from_file

# searching functions
from .search import get_nodes_by_class, get_nodes_by_rel, get_nodes_by_props


''' development only - direct access to module functions '''

'''

# functions
from .operate import *
from .interface import *
from .structure import *

'''

'''

# module
from . import module

'''
