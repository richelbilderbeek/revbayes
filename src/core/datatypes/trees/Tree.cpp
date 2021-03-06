#include "NewickConverter.h"
#include "RbConstants.h"
#include "RbException.h"
#include "RbMathLogic.h"
#include "RbOptions.h"
#include "Tree.h"
#include "Taxon.h"
#include "TopologyNode.h"
#include "TypedDagNode.h"
#include "TreeUtilities.h"

#include <cmath>

using namespace RevBayesCore;

/* Default constructor */
Tree::Tree(void) :
    changeEventHandler(),
    root( NULL ),
    binary( true ),
    rooted( false ),
    is_negative_constraint( false ),
    num_tips( 0 ),
    num_nodes( 0 )
{

}


/* Copy constructor */
Tree::Tree(const Tree& t) :
    changeEventHandler( ),
    root( NULL ),
    binary( t.binary ),
    rooted( t.rooted ),
    is_negative_constraint( t.is_negative_constraint ),
    num_tips( t.num_tips ),
    num_nodes( t.num_nodes ),
    taxon_bitset_map( t.taxon_bitset_map )
{

    // need to perform a deep copy of the BranchLengthTree nodes
    if (t.root != NULL)
    {
        TopologyNode * newRoot = t.getRoot().clone();

        // set the root. This will also set the nodes vector.
        // do not reorder node indices when copying (WP)
        setRoot(newRoot, false);
    }

}


/* Destructor */
Tree::~Tree(void)
{

    nodes.clear();
    std::set<TreeChangeEventListener*> l = changeEventHandler.getListeners();
    for ( std::set<TreeChangeEventListener*>::iterator it = l.begin(); it != l.end(); ++it )
    {
        changeEventHandler.removeListener( *it );
    }

    delete root;

}


Tree& Tree::operator=(const Tree &t)
{

    if (this != &t)
    {
        // nothing really to do here, should be done in the derived classes
        // @todo: Find a better solution - Sebastian
        // Problem: If we redraw the tree because the initial states are invalid,
        // then we somehow need to remember the tree event change listeners.
        // But it is not nice if the tree distribution needs to remember this!!!
//        changeEventHandler = t.changeEventHandler;



        nodes.clear();
        delete root;
        root = NULL;
        binary                 = t.binary;
        num_tips               = t.num_tips;
        num_nodes              = t.num_nodes;
        rooted                 = t.rooted;
        is_negative_constraint = t.is_negative_constraint;

        TopologyNode* newRoot = t.root->clone();

        // set the root. This will also set the nodes vector
        // do not reorder node indices when copying (WP)
        setRoot(newRoot, false);

    }

    return *this;
}


bool Tree::operator==(const Tree &t) const
{

    return getNewickRepresentation() == t.getNewickRepresentation();
}


bool Tree::operator!=(const Tree &t) const
{

    return !operator==(t);
}


bool Tree::operator<(const Tree &t) const
{

    return getNewickRepresentation() < t.getNewickRepresentation();
}


bool Tree::operator<=(const Tree &t) const
{

    return operator<(t) || operator==(t);
}


void Tree::addBranchParameter(std::string const &name, const std::vector<double> &parameters, bool internalOnly)
{

    getRoot().addBranchParameters(name,parameters,internalOnly);

}


void Tree::addNodeParameter(std::string const &name, const std::vector<double> &parameters, bool internalOnly)
{

    getRoot().addNodeParameters(name,parameters,internalOnly);

}

void Tree::addNodeParameter(std::string const &name, const std::vector<std::string*> &parameters, bool internalOnly)
{

    getRoot().addNodeParameters(name,parameters,internalOnly);

}


void Tree::clearParameters( void )
{

    clearNodeParameters();
    clearBranchParameters();

}

void Tree::clearBranchParameters( void )
{

    getRoot().clearBranchParameters();

}


void Tree::clearNodeParameters( void )
{

    getRoot().clearNodeParameters();

}


/* Clone function */
Tree* Tree::clone(void) const
{

    return new Tree(*this);
}


void Tree::collapseNegativeBranchLengths(double l)
{
    double age = 0.0;
    double parent_age = 0.0;
    for (size_t i = 0; i < nodes.size(); i++)
    {
        if ( nodes[i]->isRoot() == false )
        {
            age = nodes[i]->getAge();
            parent_age = nodes[i]->getParent().getAge();

            if ( parent_age < age )
            {
                nodes[i]->setAge( parent_age - l, false );
            }
        }
    }

}


/**
 * Drop the tip node with the given name.
 * The name should correspond to the taxon name, not the species name.
 * This will throw an error if the name doesn't exist.
 */
void Tree::dropTipNodeWithName( const std::string &n )
{
    // get the index of this name
    size_t index = getTipIndex( n );
    dropTipNode( index );
}



/**
 * Drop the tip node with the given name.
 * The name should correspond to the taxon name, not the species name.
 * This will throw an error if the name doesn't exist.
 */
void Tree::dropTipNode( size_t index )
{
    // get the index of this name
    TopologyNode &node          = getTipNode( index );
    if (node.isRoot() == true && nodes.size() == 1)
    {
        // there is nothing left to prune
        node.setName("");
        node.setNodeType(false, true, false);
        return;
    }
    TopologyNode &parent        = node.getParent();
    TopologyNode &grand_parent  = parent.getParent();
    if (parent.isRoot() == false)
    {
        TopologyNode *sibling = &parent.getChild( 0 );
        if ( sibling == &node )
        {
            sibling = &parent.getChild( 1 );
        }
        grand_parent.removeChild( &parent );
        parent.removeChild( sibling );
        grand_parent.addChild( sibling );
        sibling->setParent( &grand_parent );
    }
    else
    {
        if (root->getNumberOfChildren() > 1)
        {
            TopologyNode *sibling = &root->getChild( 0 );
            if ( sibling == &node )
            {
                sibling = &root->getChild( 1 );
            }
            root->removeChild(&node);
            sibling->setParent(NULL);
            root = sibling;
        }
        else
        {
            root->removeChild(&node);
        }
    }
    
    bool resetIndex = true;
    
    nodes.clear();
    
    // bootstrap all nodes from the root and add the in a pre-order traversal
    fillNodesByPhylogeneticTraversal(root);
    if ( resetIndex == true )
    {
        for (unsigned int i = 0; i < nodes.size(); ++i)
        {
            nodes[i]->setIndex(i);
        }
    }
    else
    {
        orderNodesByIndex();
    }
    
    num_nodes = nodes.size();
    
    // count the number of tips
    num_tips = 0;
    for (size_t i = 0; i < num_nodes; ++i)
    {
        if ( nodes[i] == NULL )
        {
            std::cerr << "#nodes after filling:\t\t" << nodes.size() << std::endl;
            std::cerr << i << " - " << nodes[i] << std::endl;
            throw RbException("Problem while reading in tree.");
        }
        num_tips += ( nodes[i]->isTip() ? 1 : 0);
    }
    
}



void Tree::executeMethod(const std::string &n, const std::vector<const DagNode *> &args, double &rv) const
{

    if ( n == "rootAge" )
    {
        rv = getRoot().getAge();
    }
    else if ( n == "branchLength" )
    {
        int index = (int)static_cast<const TypedDagNode<long> *>( args[0] )->getValue()-1;
        rv = getNode( index ).getBranchLength();
    }
    else if ( n == "nodeAge" )
    {
        int index = (int)static_cast<const TypedDagNode<long> *>( args[0] )->getValue()-1;
        rv = getNode( index ).getAge();
    }
    else if ( n == "treeLength" )
    {
        rv = getTreeLength();
    }
    else if (n == "gammaStatistic")
    {
        rv = RevBayesCore::TreeUtilities::getGammaStatistic( *this );
    }
    else
    {
        throw RbException("A tree object does not have a member method called '" + n + "'.");
    }

}


void Tree::executeMethod(const std::string &n, const std::vector<const DagNode *> &args, long &rv) const
{

    if ( n == "parent" )
    {
        long index = static_cast<const TypedDagNode<long> *>( args[0] )->getValue()-1;
        rv = long( getNode( index ).getParent().getIndex() )+1;
    }
    else if ( n == "child" )
    {
        int parent_index = static_cast<const TypedDagNode<int> *>( args[0] )->getValue()-1;
        if (parent_index < num_tips)
        {
            throw RbException("Tips have no children");
        }
        int child_index = static_cast<const TypedDagNode<int> *>( args[1] )->getValue()-1;
        if (child_index < 0)
        {
            throw RbException("Child indices begin at 1.");
        }
        if (child_index > getNode( parent_index ).getNumberOfChildren() - 1)
        {
            throw RbException("Node does not have child with this index.");
        }
        rv = int( getNode( parent_index ).getChild(child_index).getIndex() )+1;
    }

    else if ( n == "numSampledAncestors")
    {
        rv = 0;
        for (size_t i=0; i< num_tips; i++)
        {
            rv += nodes[i]->isSampledAncestor();
        }
    }
    else if (n == "colless")
    {
        int s = 0;

        rv = RevBayesCore::TreeUtilities::getCollessMetric( getRoot(), s);
    }
    else if (n == "nnodes")
    {
        rv = nodes.size();
    }
    else if (n == "ntips")
    {
        rv = num_tips;
    }
    else
    {
        throw RbException("A tree object does not have a member method called '" + n + "'.");
    }

}


void Tree::executeMethod(const std::string &n, const std::vector<const DagNode *> &args, Boolean &rv) const
{

    if ( n == "isContainedInClade" )
    {
        int index = (int)static_cast<const TypedDagNode<long> *>( args[0] )->getValue()-1;
        Clade clade = static_cast<const TypedDagNode<Clade> *>( args[1] )->getValue();
        clade.resetTaxonBitset( getTaxonBitSetMap() );

        if ( index < 0 || index >= nodes.size() )
        {
            std::stringstream s;
            s << "The index of the node must be between 1 and " << int(nodes.size()) << ".";
            throw RbException( s.str() );
        }


        size_t clade_index = 0;
        bool found = false;
        size_t min_clade_size = nodes.size() + 2;
        size_t taxa_count = clade.size();

        for (size_t i = getNumberOfTips(); i < nodes.size(); ++i)
        {

            TopologyNode *node = nodes[i];
            size_t clade_size = size_t( (node->getNumberOfNodesInSubtree(true) + 1) / 2);
            if ( clade_size < min_clade_size && clade_size >= taxa_count && node->containsClade( clade, false ) )
            {
                found = true;
                clade_index = node->getIndex();
                min_clade_size = clade_size;
                if ( taxa_count == clade_size )
                {
                    break;
                }

            }

        }

        if ( found == true )
        {
            while ( index != clade_index && nodes[index]->isRoot() == false )
            {
                index = int( nodes[index]->getParent().getIndex() );
            }

        }

        rv = Boolean( index == clade_index );
    }
    else
    {
        throw RbException("A tree object does not have a member method called '" + n + "'.");
    }

}

/* fill the nodes vector by a phylogenetic traversal recursively starting with this node.
 * The tips fill the slots 0,...,n-1 followed by the internal nodes and then the root.
 */
void Tree::fillNodesByPhylogeneticTraversal(TopologyNode* node)
{
    // now call this function recursively for all your children
    for (size_t i=0; i<node->getNumberOfChildren(); i++)
    {
        fillNodesByPhylogeneticTraversal(&node->getChild(i));
    }

    if (node->isTip())
    {
        // all the tips go to the beginning
        nodes.insert(nodes.begin(), node);
    }
    else
    {
        // this is phylogenetic ordering so the internal nodes come last
        nodes.push_back(node);
    }
}

const std::vector<std::vector<double> > Tree::getAdjacencyMatrix(void) const
{
    std::vector<std::vector<double> > adjacency(num_nodes, std::vector<double>(num_nodes, 0.0));

    for (size_t i = 0; i < nodes.size(); i++)
    {
        const TopologyNode* nd = nodes[i];
        std::vector<TopologyNode*> children = nd->getChildren();
        for (std::vector<TopologyNode*>::iterator ch = children.begin(); ch != children.end(); ch++)
        {
            adjacency[nd->getIndex()][(*ch)->getIndex()] = (*ch)->getBranchLength();
        }
        if (!nd->isRoot())
        {
            const TopologyNode* pa = &nd->getParent();
            adjacency[nd->getIndex()][pa->getIndex()] = pa->getBranchLength();
        }
    }

    return adjacency;
}


std::vector<Taxon> Tree::getFossilTaxa() const
{
    std::vector< Taxon > taxa;
    for (size_t i = 0; i < getNumberOfTips(); ++i)
    {
        const TopologyNode& n = getTipNode( i );
        if ( n.isFossil() == true )
        {
            taxa.push_back( n.getTaxon() );
        }

    }

    return taxa;
}


/** We provide this function to allow a caller to randomly pick one of the interior nodes.
 This version assumes that the root is always the last and the tips the first in the nodes vector. */
const TopologyNode& Tree::getInteriorNode( size_t indx ) const
{

    // \TODO: Bound checking, maybe draw from downpass array instead
    return *nodes[ indx + getNumberOfTips() ];
}


std::string Tree::getNewickRepresentation() const
{

    return root->computeNewick();
}



TopologyNode& Tree::getNode(size_t idx)
{

    if ( idx >= nodes.size() )
    {
        throw RbException("Index out of bounds in getNode.");
    }

    return *nodes[idx];
}


const TopologyNode& Tree::getNode(size_t idx) const
{

    if ( idx >= nodes.size() )
    {
        throw RbException("Index out of bounds in getNode.");
    }

    return *nodes[idx];
}


const std::vector<TopologyNode*>& Tree::getNodes(void) const
{

    return nodes;
}



std::vector<RbBitSet> Tree::getNodesAsBitset(void) const
{

    std::vector<RbBitSet> bs;

    for ( size_t i=0; i<nodes.size(); ++i )
    {
        TopologyNode *n = nodes[i];
        if ( n->isTip() == false )
        {
            RbBitSet taxa_this_node = RbBitSet(num_tips);
            n->getTaxa(taxa_this_node);
            bs.push_back( taxa_this_node );
        }
    }

    return bs;
}




/**
 * Calculate the number of interior nodes in the BranchLengthTree by deducing the number of
 * tips from number of nodes, and then subtract 1 more if the BranchLengthTree is rooted.
 */
size_t Tree::getNumberOfInteriorNodes( void ) const
{

    size_t preliminaryNumIntNodes = getNumberOfNodes() - getNumberOfTips();

    if ( isRooted() )
    {
        return preliminaryNumIntNodes - 1;
    }
    else
    {
        return preliminaryNumIntNodes;
    }

}


size_t Tree::getNumberOfNodes(void) const
{

    return num_nodes;
}


/**
 * return the number of tips.
 */
size_t Tree::getNumberOfTips( void ) const
{

    return num_tips;
}


/**
 * return the number of tips.
 */
size_t Tree::getNumberOfExtinctTips( void ) const
{
    size_t num_extinct = 0;
    for (size_t i = 0; i < num_tips; i++)
    {
        num_extinct += nodes[i]->isFossil();
    }

    return num_extinct;
}


/**
 * return the number of tips.
 */
size_t Tree::getNumberOfExtantTips( void ) const
{
    return num_tips - getNumberOfExtinctTips();
}


/**
 * return the number of tips.
 */
size_t Tree::getNumberOfSampledAncestors( void ) const
{
    size_t num_sa = 0;
    for (size_t i = 0; i < num_tips; i++)
    {
        num_sa += nodes[i]->isSampledAncestor();
    }

    return num_sa;
}


std::string Tree::getPlainNewickRepresentation() const
{

    return root->computePlainNewick();
}


TopologyNode& Tree::getRoot(void)
{
    return *root;
}


const TopologyNode& Tree::getRoot(void) const
{
    return *root;
}


/**
 * Get the tree and character history in newick format
 * compatible with SIMMAP and phytools
 */
std::string Tree::getSimmapNewickRepresentation() const
{
    return root->computeSimmapNewick();
}


/**
 * Get all the species names for this topology.
 * This might include duplicates.
 */
std::vector<std::string> Tree::getSpeciesNames() const
{
    std::vector< std::string > snames;
    for (size_t i = 0; i < getNumberOfTips(); ++i)
    {
        const TopologyNode& n = getTipNode( i );
        snames.push_back( n.getTaxon().getSpeciesName() );
    }

    return snames;
}

std::vector<Taxon> Tree::getTaxa() const
{
    std::vector< Taxon > taxa;
    for (size_t i = 0; i < getNumberOfTips(); ++i)
    {
        const TopologyNode& n = getTipNode( i );
        taxa.push_back( n.getTaxon() );
    }

    return taxa;
}


/**
 * Returns a map of the taxa to their BitSet indices.
 * The taxa are ordered alphabetically in the BitSet.
 * Eventually this should be refactored with the TaxonMap class.
 */
const std::map<std::string, size_t>& Tree::getTaxonBitSetMap( void ) const
{
    if (taxon_bitset_map.size() == 0)
    {
        // get all taxon names
        std::vector<Taxon> unordered_taxa = getTaxa();
        std::vector<std::string> ordered_taxa;
        for (size_t i = 0; i < unordered_taxa.size(); ++i)
        {
            ordered_taxa.push_back(unordered_taxa[i].getName());
        }

        // order taxon names
        std::sort(ordered_taxa.begin(), ordered_taxa.end());

        // add taxa to bitset map
        for (size_t i = 0; i < ordered_taxa.size(); ++i)
        {
            taxon_bitset_map[ordered_taxa[i]] = i;
        }
    }
    return taxon_bitset_map;
}

/**
 * Get the tip index for this name.
 */
size_t Tree::getTipIndex( const std::string &name ) const
{
    for (size_t i = 0; i < getNumberOfTips(); ++i)
    {
        const TopologyNode& n = getTipNode( i );
        if ( name == n.getName() )
        {
            return n.getIndex();
        }
    }

    // if name not found
    throw RbException("Could not find tip node with name '" + name + "' in tree." );
}


std::vector<std::string> Tree::getTipNames() const
{

    std::vector<std::string> names;
    for (size_t i = 0; i < getNumberOfTips(); ++i)
    {
        const TopologyNode& n = getTipNode( i );
        names.push_back( n.getName() );
    }

    return names;
}


/**
 * We provide this function to allow a caller to randomly pick one of the tip nodes.
 * This version assumes that the tips are first in the nodes vector.
 */
TopologyNode& Tree::getTipNode( size_t index )
{

//
//    if ( index >= getNumberOfTips() )
//    {
//        throw RbException("Index out of bounds in getTipNode()!");
//    }
//    if (!nodes[ index ]->isTip())
//    {
//        throw RbException("Node at index is not a tip but should have been!");
//    }

    return *nodes[ index ];
}


const TopologyNode& Tree::getTipNode(size_t index) const
{

    //
    //    if ( index >= getNumberOfTips() )
    //    {
    //        throw RbException("Index out of bounds in getTipNode()!");
    //    }
    //    if (!nodes[ index ]->isTip())
    //    {
    //        throw RbException("Node at index is not a tip but should have been!");
    //    }

    return *nodes[index];
}


/**
 * Get the tip node with the given name.
 * The name should correspond to the taxon name, not the species name.
 * This will throw an error if the name doesn't exist.
 */
TopologyNode& Tree::getTipNodeWithName( const std::string &n )
{
    // get the index of this name
    size_t index = getTipIndex( n );

    return *nodes[ index ];
}



/**
 * Get the tip node with the given name.
 * The name should correspond to the taxon name, not the species name.
 * This will throw an error if the name doesn't exist.
 */
const TopologyNode& Tree::getTipNodeWithName( const std::string &n ) const
{
    // get the index of this name
    size_t index = getTipIndex( n );

    return *nodes[ index ];
}


/**
 * Get all the tip nodes with this species name.
 * If there is none, then we return an empty vector.
 */
std::vector<TopologyNode*> Tree::getTipNodesWithSpeciesName( const std::string &name )
{
    // create the vector of the tip nodes with this species name
    std::vector<TopologyNode*> tipNodes;

    // loop over all tips
    for (size_t i = 0; i < getNumberOfTips(); ++i)
    {
        // get the i-th tip
        TopologyNode& n = getTipNode( i );

        // test if the species name matches
        if ( name == n.getSpeciesName() )
        {
            // add this tip node to our list
            tipNodes.push_back( &n );
        }
    }

    // return the vector
    return tipNodes;
}


double Tree::getTmrca(const Clade &c)
{

    return root->getTmrca( c );
}


double Tree::getTmrca(const TopologyNode &n)
{

    return root->getTmrca( n );
}


double Tree::getTmrca(const std::vector<Taxon> &t)
{

    return root->getTmrca( t );
}


TreeChangeEventHandler& Tree::getTreeChangeEventHandler( void ) const
{

    return changeEventHandler;
}


double Tree::getTreeLength( void ) const
{

    double tl = 0.0;
    // loop over all nodes
    for (size_t i = 0; i < num_nodes; ++i)
    {
        // get the i-th node
        const TopologyNode& n = *nodes[i];

        if ( n.isRoot() == false )
        {
            // add the branch length
            tl += n.getBranchLength();
        }

    }

    return tl;
}


bool Tree::hasSameTopology(const Tree &t) const
{

    return getPlainNewickRepresentation() == t.getPlainNewickRepresentation();
}


// Serialize (resurrect) the object from a file
void Tree::initFromFile( const std::string &dir, const std::string &fn )
{
    RbFileManager fm = RbFileManager(dir, fn + ".newick");
    fm.createDirectoryForFile();

    // open the stream to the file
    std::fstream inStream;
    inStream.open( fm.getFullFileName().c_str(), std::fstream::in);


    std::string s = "";
    while ( inStream.good() )
    {

        // Read a line
        std::string line;
        fm.safeGetline( inStream, line );

        // append
        s += line;

    }

    return initFromString( s );
}


void Tree::initFromString(const std::string &s)
{
    NewickConverter converter;
    Tree* bl_tree = converter.convertFromNewick( s );
    Tree *tree = TreeUtilities::convertTree( *bl_tree );

    *this = *tree;

    delete tree;
}


bool Tree::isBinary(void) const
{

    return binary;
}


bool Tree::isBroken( void ) const
{

    for (size_t i = 0; i < getNumberOfInteriorNodes(); ++i)
    {

        const TopologyNode &n = getInteriorNode( i );
        double age = n.getAge();

        for (size_t j = 0; j < n.getNumberOfChildren(); ++j)
        {
            const TopologyNode &child = n.getChild( j );

            double est_age = child.getAge() + child.getBranchLength();

            if ( std::fabs(age-est_age) > 1E-4 )
            {
                return true;
            }

        }

    }

    for (size_t i = 0; i < getNumberOfNodes(); ++i)
    {

        const TopologyNode &n = getNode( i );

        if ( n.isRoot() == false )
        {
            double my_age = n.getAge();
            double my_parents_age = n.getParent().getAge();

            if ( std::fabs( my_parents_age - my_age - n.getBranchLength() ) > 1E-4 )
            {
                return true;
            }

        }

    }


    return false;
}

bool Tree::isNegativeConstraint(void) const
{

    return is_negative_constraint;
}

bool Tree::isRooted(void) const
{

    return rooted;
}


bool Tree::isUltrametric( void ) const
{

    double tip_age = getTipNode( 0 ).getAge();
    for (size_t i = 1; i < getNumberOfTips(); ++i)
    {

        if ( std::fabs(tip_age-getTipNode(i).getAge()) > 1E-4 )
        {
            return false;
        }

    }

    return true;
}


void Tree::makeInternalNodesBifurcating(bool reindex)
{

    getRoot().makeBifurcating();

    // we need to reset the root so that the vector of nodes get filled again with the new number of nodes
    setRoot( &getRoot(), reindex );

}


// method to order nodes by their existing index
// used when reading in tree with existing node indexes we need to keep
void Tree::orderNodesByIndex( void )
{


    std::vector<TopologyNode*> nodes_copy = std::vector<TopologyNode*>(nodes.size());
    std::vector<bool> used = std::vector<bool>(nodes.size(),false);
    for (int i = 0; i < nodes.size(); i++)
    {
        if ( nodes[i]->getIndex() > nodes.size() )
        {
            throw RbException("Problem while working with tree: Node had bad index. Index was '" + StringUtilities::to_string( nodes[i]->getIndex() ) + "' while there are only '" + StringUtilities::to_string( nodes.size() ) + "' nodes in the tree.");
        }
        else if ( used[nodes[i]->getIndex()] == true )
        {
            throw RbException("Problem while working with tree: Node had bad index. Two nodes had same index of '" + StringUtilities::to_string( nodes[i]->getIndex() ) + "'.");
        }
        else
        {
            used[nodes[i]->getIndex()] = true;
        }
        nodes_copy[ nodes[i]->getIndex() ] = nodes[i];
    }

    nodes = nodes_copy;

}

void Tree::pruneTaxa(const RbBitSet& prune_map )
{
    nodes.clear();

    // bootstrap all nodes from the root and add the in a pre-order traversal
    recursivelyPruneTaxa(root, prune_map);

    for (unsigned int i = 0; i < nodes.size(); ++i)
    {
        nodes[i]->setIndex(i);
    }
}

bool Tree::recursivelyPruneTaxa( TopologyNode* n, const RbBitSet& prune_map )
{
    if( n->isTip()  )
    {
        bool prune = prune_map[n->getIndex()];

        if( prune == false )
        {
            nodes.insert(nodes.begin(), n);
        }

        return prune;
    }

    std::vector<TopologyNode*> children = n->getChildren();

    //std::vector<TopologyNode*> retained_children;
    std::vector<TopologyNode*> pruned_children;
    for(size_t i = 0; i < children.size(); i++)
    {
        if( recursivelyPruneTaxa(children[i], prune_map) )
        {
            pruned_children.push_back(children[i]);
        }
    }

    // if we don't prune any children, then add this node to the list of keepers
    if( pruned_children.empty() )
    {
        nodes.push_back(n);
    }

    // if we prune all or zero children, then just continue up the tree
    if( pruned_children.empty() || pruned_children.size() == children.size() )
    {
        return pruned_children.size() == children.size();
    }

    // if we prune some, but not all children, then patch over this node

    // first prune the dead branches
    for(size_t i = 0; i < pruned_children.size(); i++)
    {
        n->removeChild(pruned_children[i]);
        pruned_children[i]->setParent(NULL);
        delete pruned_children[i];
    }

    // if there are still enough retained root children, then return
    if( n->isRoot() && rooted == false && children.size() - pruned_children.size() < 3 )
    {
        // there are not enough retained children
        // patch up the node
        std::vector<TopologyNode*> retained_children = n->getChildren();

        for(size_t i = 0; i < retained_children.size(); i++)
        {
            n->removeChild(retained_children[i]);
            retained_children[i]->setParent(NULL);
        }

        delete n;

        if( retained_children.size() == 1 )
        {
            n = retained_children.back();

            retained_children = n->getChildren();
            for(size_t i = 0; i < retained_children.size(); i++)
            {
                n->removeChild(retained_children[i]);
                retained_children[i]->setParent(NULL);
            }

            delete n;
        }

        root = retained_children.back();
        root->addChild(retained_children.front());
        retained_children.front()->setParent(root);
    }
    // if there are still at least 2 retained children, then return
    else if( children.size() - pruned_children.size() < 2 )
    {
        // there is only one retained child
        // patch up the 2-degree node
        TopologyNode* retained_child = &n->getChild(0);

        n->removeChild(retained_child);

        if( n->isRoot() )
        {
            retained_child->setParent(NULL);
            root = retained_child;
        }
        else
        {
            TopologyNode* parent = &n->getParent();

            parent->removeChild(n);
            parent->addChild(retained_child);
            retained_child->setParent(parent);
            n->setParent(NULL);
        }

        delete n;
    }

    return false;
}


void Tree::removeDuplicateTaxa( void )
{
    
    bool removed_replicate = true;
    while ( removed_replicate == true )
    {
        removed_replicate = false;
        for ( size_t i=0; i<(num_tips-1); ++i )
        {
            const std::string &name_a = nodes[ i ]->getName();
            for ( size_t j=i+1; j<num_tips; ++j )
            {
                const std::string &name_b = nodes[ j ]->getName();
                if ( name_a == name_b )
                {
                    removed_replicate = true;
                    dropTipNode( j );
                    break;
                }
                
            }

            if ( removed_replicate == true )
            {
                break;
            }
            
        }

    }
    
}


void Tree::renameNodeParameter(const std::string &old_name, const std::string &new_name)
{
    getRoot().renameNodeParameter(old_name, new_name);
}


void Tree::reroot(const Clade &o, bool reindex)
{

    bool strict = true;

    // for safety we reset the bitrepresentation of the clade
    Clade outgroup = o;
    outgroup.resetTaxonBitset( getTaxonBitSetMap() );

    if ( root->containsClade(outgroup, strict ) == false )
    {
        bool found = false;

        // check for the inverted clade
        RbBitSet b = outgroup.getBitRepresentation();
        b.flip();
        outgroup.setBitRepresentation(b);

        if ( root->containsClade(outgroup, strict ) == false )
        {
            found = true;
        }

        if( found == false )
        {
            throw RbException("Cannot reroot the tree because we could not find an outgroup with name '" + outgroup.toString() + "'.");
        }
    }

    // reset parent/child relationships
    TopologyNode *outgroup_node = root->getNode( outgroup, strict);

    if ( outgroup_node == NULL )
    {
        throw RbException("Cannot reroot the tree because we could not find an outgroup with name '" + outgroup.toString() + "'.");
    }

    if ( outgroup_node->isRoot() == false )
    {

//        TopologyNode &new_root = outgroup_node->getParent();
        reverseParentChild( outgroup_node->getParent() );
        outgroup_node->getParent().setParent( NULL );

        // set the new root
        setRoot( &outgroup_node->getParent(), reindex );
    }

}


void Tree::reroot(const std::string &outgroup, bool reindex)
{
    std::vector<std::string> tip_names = getTipNames();
    size_t outgroup_index = tip_names.size();
    for (size_t i=0; i<tip_names.size(); ++i)
    {
        if ( tip_names[i] == outgroup )
        {
            outgroup_index = i;
            break;
        }
    }

    if ( outgroup_index == tip_names.size() )
    {
        throw RbException("Cannot reroot the tree because we could not find an outgroup with name '" + outgroup + "'.");
    }

    // reset parent/child relationships
	TopologyNode& outgroup_node = getTipNode( outgroup_index );
    reverseParentChild( outgroup_node.getParent() );
    outgroup_node.getParent().setParent( NULL );

	// set the new root
	setRoot( &outgroup_node.getParent(), reindex );

}


void Tree::reroot(TopologyNode &n, bool reindex)
{
	// reset parent/child relationships
	reverseParentChild( n.getParent() );
    n.getParent().setParent( NULL );

	// set the new root
	setRoot( &n.getParent(), reindex );

}


TopologyNode& Tree::reverseParentChild(TopologyNode &n)
{
    TopologyNode* ret = &n;

    if ( !n.isRoot() )
    {
        TopologyNode &p = n.getParent();
        ret = &(reverseParentChild(p));

        // we need to re-orient the branches/indices so that
        // nodes remain associated with the same parameters
        p.setIndex(n.getIndex());
        p.setBranchLength(n.getBranchLength());

        p.removeChild( &n );
        p.setParent( &n );
        n.addChild( &p );
    }

    return *ret;
}


void Tree::setNegativeConstraint(bool tf)
{
    is_negative_constraint = tf;
}


void Tree::setRooted(bool tf)
{
    rooted = tf;
}


void Tree::setRoot( TopologyNode* r, bool reindex )
{

    // delete the old root if it's not in this tree
    bool found = false;

    TopologyNode* old_root = root;

    // set the root
    root = r;

    nodes.clear();

    // bootstrap all nodes from the root and add the in a pre-order traversal
    fillNodesByPhylogeneticTraversal(r);

    if ( reindex == true )
    {
        for (unsigned int i = 0; i < nodes.size(); ++i)
        {
            nodes[i]->setIndex(i);
        }
    }
    else
    {
        orderNodesByIndex();
    }

    num_nodes = nodes.size();

    // count the number of tips
    num_tips = 0;
    for (size_t i = 0; i < num_nodes; ++i)
    {
        if ( nodes[i] == NULL )
        {
            std::cerr << "#nodes after filling:\t\t" << nodes.size() << std::endl;
            std::cerr << i << " - " << nodes[i] << std::endl;
            throw RbException("Problem while reading in tree.");
        }
        if ( nodes[i] == old_root)
        {
            found = true;
        }
        num_tips += ( nodes[i]->isTip() ? 1 : 0);
    }


    root->setTree( this );

    if ( found == false )
    {
        delete old_root;
    }


}


//!< Set the indices of the taxa from the taxon map
void Tree::setTaxonIndices(const TaxonMap &tm)
{

    // start a recursive call at the root
    if ( root != NULL )
    {
        root->setTaxonIndices(tm);
        orderNodesByIndex();
    }


}


/**
 * Change the name of a taxon
 *
 * \param[in] current_name    self explanatory.
 * \param[in] newName         self explanatory.
 */
void Tree::setTaxonName(const std::string& current_name, const std::string& newName)
{
    
    TopologyNode& node = getTipNodeWithName( current_name );
    Taxon& t = node.getTaxon();
    t.setName( newName );
    taxon_bitset_map.erase( current_name );
    taxon_bitset_map.insert( std::pair<std::string, size_t>( newName, node.getIndex() ) );
}


/**
 * Change the name of a taxon
 *
 * \param[in] current_name   self explanatory.
 * \param[in] new_taxon      self explanatory.
 */
void Tree::setTaxonObject(const std::string& current_name, const Taxon& new_taxon)
{
    
    const std::string &new_name = new_taxon.getName();
    
    TopologyNode& node = getTipNodeWithName( current_name );
    node.setTaxon( new_taxon );
    
    taxon_bitset_map.erase( current_name );
    taxon_bitset_map.insert( std::pair<std::string, size_t>( new_name, node.getIndex() ) );
    
}


// Write this object into a file in its default format.
void Tree::writeToFile( const std::string &dir, const std::string &fn ) const
{
    // do not write a file if the tree is invalid
    if (this->getNumberOfTips() > 1)
    {
        RbFileManager fm = RbFileManager(dir, fn + ".newick");
        fm.createDirectoryForFile();

        // open the stream to the file
        std::fstream outStream;
        outStream.open( fm.getFullFileName().c_str(), std::fstream::out);

        // write the value of the node
        outStream << getNewickRepresentation();
        outStream << std::endl;

        // close the stream
        outStream.close();
    }
}


std::ostream& RevBayesCore::operator<<(std::ostream& o, const Tree& x)
{

    o << x.getNewickRepresentation();

    return o;
}
