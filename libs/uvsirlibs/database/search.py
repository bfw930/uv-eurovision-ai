
''' Database Search Functions

    Search database instance by properties or relations; return indicies
'''



''' Imports '''



''' Search and Filter Helper Functions '''

def get_class_node_from_index(db, node_class):

    ''' Get Class Nodes from Index

        Search database instance, get index of class nodes from class index node

    Args:
        db (dict): database instance
        node_class (str): node class to search

    Returns:
        list: matched class node indicies within database instance
    '''

    # get database meta node
    meta_node = db[0]

    # get class index node from meta node index relation
    class_index_node_index = meta_node['rels']['class_index'][0]
    class_index_node = db[class_index_node_index]

    # get class node indicies by node class from class index node relations
    indicies = class_index_node['rels'][node_class]


    # return index list
    return indicies



''' Database Search and Filter '''

def get_nodes_by_class(db, node_class):

    ''' Get Entry Nodes by Class

        Search database instance, get index of entry nodes by class; get class node by class index node

    Args:
        db (dict): database instance
        node_class (str): node class to search

    Returns:
        list: matched node indicies within database instance
    '''

    # get index of class nodes by class index node
    class_indicies = get_class_node_from_index(db = db, node_class = node_class)


    # entry node indicies in class
    node_indicies = []

    # iterate class nodes by index
    for class_index in class_indicies:

        # get class node by index
        class_node = db[class_index]

        # store entry node indicies in class node
        node_indicies += class_node['rels']['class_entry']


    # return index list
    return list(set(node_indicies))



def get_nodes_by_rel(db, node_index, rel_type):

    ''' Get Nodes by Relation

        Given database instance and node index, search node relations by relation type; return related node indicies

    Args:
        db (dict): database instance
        node_index (int): node index
        rel_type (str): relation type

    Returns:
        list: related node indicies within database instance
    '''

    # get node by index
    node = db[node_index]


    # ensure relation type exists within node relations
    if rel_type in node['rels'].keys():

        # get related node indicies by relation type
        indicies = node['rels'][rel_type]


        # return related node index list
        return list(set(indicies))

    else:
        # if no relation type in node, return None
        return None



def get_nodes_by_props(db, props, indicies = None):

    ''' Get Nodes by Match on Properties

        Given a list of properties (key: value), match nodes within database instance, return node indicies;
        greedy AND match to params: given list of key:value param pairs, node is a match all property pairs match

    Args:
        db (dict): database instance
        props (dict): node properties to match
        indicies (list): indicies of nodes to search within database instance

    Returns:
        list: matched node indicies
    '''

    # if not provided list of indicies
    if indicies is None:

        # set search indicies as all nodes in database instance
        indicies = range(len(db))


    # build node index list from properties match
    match_indicies = []

    # iterate each node in database instance
    for i in indicies:


        # get node properties
        node_props = db[i]['props']

        # get number parameter matches within node
        match = sum([ 1 for key, value in props.items()
            if key in node_props.keys() and node_props[key] == value ])


        # if all properties have successfully matched
        if match == len(props.keys()):

            # add node index to incidies list
            match_indicies.append(i)


    # return index list
    return match_indicies
