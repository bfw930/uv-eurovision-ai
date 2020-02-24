
''' Database Interface Functions

    Functions for insert nodes and relations, direct interaction with database instance
'''



''' Imports '''



''' Database Interaction '''

def insert_relation(db, node_index, rel_type, rel_index):

    ''' Insert Node Relation

        Insert relation to related node from node by index and relation type

    Args:
        db (list): database instance
        node_index (int): index of node
        rel_type (str): type of relation
        rel_index (int): index of related node

    Returns:
        none: relation added to related node within database instance
    '''

    # get related node
    rel_node = db[rel_index]


    # check for relation type in related node
    if rel_type not in rel_node['rels'].keys():

        # if relation type not found, add relation type list
        rel_node['rels'][rel_type] = [ ]


    # add relation to related node
    rel_node['rels'][rel_type].append( node_index )



def insert_node( db, node ):

    ''' Insert Node into Database Instance

        Insert node to database instance

    Args:
        db (list): database instance
        node (dict): node to add to database

    Returns:
        (int): index of node added to database instance
    '''

    # store node in database
    db.append( node )

    # get index of added node
    index = ( len(db) - 1 )


    # return index of added node
    return index









'''
database as dict
    path to db directory
    node type index reference
    class index reference




nodes

    000000.node


edge

    000000.edge


node class index

    node.class

edge class index



'''
