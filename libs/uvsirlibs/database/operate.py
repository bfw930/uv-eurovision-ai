
''' Database Interface Functions

    Functions for orchestration; maintain class index node relations, class node relations for entry nodes
'''



''' Imports '''

# database structure definitions
from .structure import gen_db, gen_node

# database update functions
from .interface import insert_relation, insert_node



''' Database Initialisation '''

def init_db(class_list = [], db_type = 'std'):

    ''' Initialise New Database Instance

        Generate database, initialise class nodes

    Args:
        class_list (list): list of class nodes
        db_type (str): database type

    Returns:
        (list): initialised database instance
    '''

    # generate new database instance
    db = gen_db(db_type = db_type)


    # iterate each node class in class list
    for node_class in class_list:

        # generate and add new class node to database instance
        class_node_index = add_class(db = db, node_class = node_class)


    # return initialised database instance
    return db



''' Database Orchestration '''

def add_class(db, node_class):

    ''' Add New Class

        Generate new class node, add to database instance, add index relation to class index node

    Args:
        db (list): database instance
        node_class (str): node class

    Returns:
        (int): index of new class node added to database instance
    '''

    # get database meta node
    meta_node = db[0]

    # get class index node from meta node index relation
    class_index_node_index = meta_node['rels']['class_index'][0]
    class_index_node = db[class_index_node_index]


    # ensure class node does not already exist within in index relations
    if node_class not in class_index_node['rels'].keys():


        # define class node properties and relations
        class_node_props = {'node_class': node_class}
        class_node_rels = {}

        # generate new class node
        class_node = gen_node(node_type = 'class', node_props = class_node_props, node_rels = class_node_rels)


        # add class node to database, return index
        class_node_index = insert_node(db = db, node = class_node)


        # add relation to class node in class index node
        insert_relation(db = db, node_index = class_node_index, rel_type = node_class,
            rel_index = class_index_node_index)


        # return index of added class node
        return class_node_index



def add_entry(db, node_class, node_props, node_rels):

    ''' Add New Entry

        Generate new entry node, add to database instance, add index relation to class node

    Args:
        db (list): database instance
        node_class (str): node class
        node_props (dict): additional node properties
        node_rels (dict): additional node relations

    Returns:
        (int): index of new node added to database instance
    '''

    # get database meta node
    meta_node = db[0]

    # get class index node from meta node index relation
    class_index_node_index = meta_node['rels']['class_index'][0]
    class_index_node = db[class_index_node_index]


    # check if class node exists in class node index
    if node_class not in class_index_node['rels'].keys():

        # if does not exist, add new class node
        class_node_index = add_class(db = db, node_class = node_class)

    else:
        # if does exist, get class node from class index node index relation
        class_node_index = class_index_node['rels'][node_class][0]


    # define entry node properties and relations
    entry_node_props = {'node_class': node_class, **node_props}
    entry_node_rels = {**node_rels}

    # generate new entry node
    entry_node = gen_node(node_type = 'entry', node_props = entry_node_props, node_rels = entry_node_rels )


    # add new entry node to database, return index
    entry_node_index = insert_node(db = db, node = entry_node)


    # add relation to entry node in class node
    insert_relation(db = db, node_index = entry_node_index, rel_type = 'class_entry', rel_index = class_node_index)


    # return index of added entry node
    return entry_node_index



def add_relation(db, node_index, rel_type, rel_index):

    ''' Add New Relation

        Insert new node-node relation

    Args:
        db (list): database instance
        node_index (int): index of node
        rel_type (str): type of relation
        rel_index (int): index of related node

    Returns:
        none: relation added to related node within database instance
    '''

    # add relation to entry node in class node
    insert_relation(db = db, node_index = node_index, rel_type = rel_type, rel_index = rel_index)

