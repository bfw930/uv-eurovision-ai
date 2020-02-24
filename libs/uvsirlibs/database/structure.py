
''' Database Structure Definitions

    All nodes have both node_type and node_class properties where node_type in 'index' (structural relations),
    'class' (entry node class relations), 'entry' (standard data node); and node_class is list of class nodes
'''



''' Imports '''



''' Database Structure '''

def gen_node(node_type, node_props, node_rels):

    ''' Generate New Node

    Args:
        node_type (str): node type in 'index', 'class', 'entry'
        node_props (dict): additional node properties as dict of property_name as key, property data as value
        node_rels (dict): additional node relations as dict of class_name as key, list of class node indicies as value

    Returns:
        (dict): generated node
    '''

    # generate new node using node type, class and node props, rels provided
    node = {
        'props': {
            'node_type': node_type,
            **node_props,
        },
        'rels': {
            **node_rels,
        },
    }


    # return generated node
    return node



def gen_db(db_type = 'std'):

    ''' Generate New Database Instance

    Args:
        db_type (str): database type

    Returns:
        (list): new database instance
    '''

    if db_type == 'std':

        # define database metadata node properties and relations
        meta_node_props = {}
        meta_node_rels = {'class_index': [1]}

        # generate database metadata node
        meta_node = gen_node( node_type = 'meta', node_props = meta_node_props, node_rels = meta_node_rels )


        # generate class index node
        class_index_node = gen_node( node_type = 'index', node_props = {'index_type': 'class'}, node_rels = {} )


        # generate new database instance with generated index nodes
        db = [ meta_node, class_index_node, ]


    # return database instance
    return db

