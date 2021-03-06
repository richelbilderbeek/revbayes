#include "NewickConverter.h"
#include "ProgressBar.h"
#include "RandomNumberFactory.h"
#include "RandomNumberGenerator.h"
#include "RbConstants.h"
#include "RbException.h"
#include "RbMathCombinatorialFunctions.h"
#include "RbMathLogic.h"
#include "RbVectorUtilities.h"
#include "RlUserInterface.h"
#include "StringUtilities.h"
#include "TopologyNode.h"
#include "TraceTree.h"
#include "TreeUtilities.h"

#include <iomanip>

using namespace RevBayesCore;


/*
 * Default AnnotationReport constructor
 */
TraceTree::AnnotationReport::AnnotationReport() :
    clade_probs                   (true),
    conditional_clade_ages        (false),
    conditional_clade_probs       (false),
    conditional_tree_ages         (false),
    MAP_parameters                (false),
    node_ages                     (true),
    mean_node_ages                (true),
    node_ages_HPD                 (0.95),
    sampled_ancestor_probs        (true),
    force_positive_branch_lengths (false)
{}


/*
 * TraceTree constructor
 */
TraceTree::TraceTree( bool c ) : Trace<Tree>(),
    clock( c ),
    rooted( true )
{
}


/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'b'.
 *
 * \return A new copy of the process.
 */

TraceTree* TraceTree::clone(void) const
{
    
    return new TraceTree(*this);
}


// annotate the MAP node/branch parameters
void TraceTree::mapParameters( Tree &tree, bool verbose ) const
{
    
    const Tree& sample_tree = objectAt( 0 );
    
    // first we annotate the node parameters
    // we need an internal node because the root might not have all parameter (e.g. rates)
    // and the tips might neither have all parameters
    const TopologyNode *n = &sample_tree.getRoot().getChild( 0 );
    if ( n->isTip() == true )
    {
        n = &sample_tree.getRoot().getChild( 1 );
    }
    const std::vector<std::string> &nodeParameters = n->getNodeParameters();
    for (size_t i = 0; i < nodeParameters.size(); ++i)
    {
        
        std::string tmp = nodeParameters[i];
        if ( tmp[0] == '&')
        {
            tmp = tmp.substr(1,tmp.size());
        }
        std::vector<std::string> pair;
        StringUtilities::stringSplit(tmp, "=", pair);
        
        if ( pair[0] == "index" ) continue;
        
        if ( StringUtilities::isNumber( pair[1] ) && !StringUtilities::isIntegerNumber( pair[1] ) )
        {
            mapContinuous(tree, pair[0], i, 0.95, true, verbose);
        }
        else
        {
            mapDiscrete(tree, pair[0], i, 3, true, verbose);
        }
        
    }
    
    // then we annotate the branch parameters
    const std::vector<std::string> &leftBranchParameters = sample_tree.getRoot().getChild(0).getBranchParameters();
    const std::vector<std::string> &rightBranchParameters = sample_tree.getRoot().getChild(1).getBranchParameters();
    
    std::vector<std::string> branchParameters;
    if ( leftBranchParameters.size() > rightBranchParameters.size() )
    {
        branchParameters = leftBranchParameters;
    }
    else
    {
        branchParameters = rightBranchParameters;
    }
    
    for (size_t i = 0; i < branchParameters.size(); ++i)
    {
        
        std::string tmp = branchParameters[i];
        if ( tmp[0] == '&')
        {
            tmp = tmp.substr(1,tmp.size());
        }
        std::vector<std::string> pair;
        StringUtilities::stringSplit(tmp, "=", pair);
        
        if ( pair[0] == "index" ) continue;
        
        if ( StringUtilities::isNumber( pair[1] ) )
        {
            mapContinuous(tree, pair[0], i, 0.95, false, verbose);
        }
        else
        {
            mapDiscrete(tree, pair[0], i, 3, false, verbose);
        }
        
    }
    
}



/*
 * this method calculates the MAP ancestral character states for the nodes on the input_tree
 */
void TraceTree::mapDiscrete(Tree &tree, const std::string &n, size_t paramIndex, size_t num, bool isNodeParameter, bool verbose ) const
{
    
    // 2-d vectors to keep the data (posteriors and states) of the inputTree nodes: [node][data]
    const std::vector<TopologyNode*> &summary_nodes = tree.getNodes();
    //std::vector<std::map<std::string, Sample<std::string> > > stateAbsencePresence(summary_nodes.size(), std::map<std::string, Sample<std::string> >());
    std::vector<std::map<std::string, long> > state_counts(summary_nodes.size(), std::map<std::string, long>());
    
    bool interiorOnly = true;
    bool tipsChecked = false;
    //    bool useRoot = true;
    
    // loop through all trees in tree trace
    for (size_t iteration = burnin; iteration < size(); ++iteration)
    {
        const Tree &sample_tree = objectAt( iteration );
        const TopologyNode& sample_root = sample_tree.getRoot();
        
        // loop through all nodes in inputTree
        for (size_t node_index = 0; node_index < summary_nodes.size(); ++node_index)
        {
            TopologyNode *node = summary_nodes[node_index];
            
            if ( node->isTip() == true )
            {
                if ( tipsChecked == false )
                {
                    
                    tipsChecked = true;
                    size_t sample_clade_index = sample_root.getCladeIndex( node );
                    
                    const TopologyNode &sample_node = sample_tree.getNode( sample_clade_index );
                    
                    std::vector<std::string> params;
                    if ( isNodeParameter == true )
                    {
                        params = sample_node.getNodeParameters();
                    }
                    else
                    {
                        params = sample_node.getBranchParameters();
                    }
                    
                    // check if this parameter exists
                    if ( params.size() > paramIndex )
                    {
                        
                        std::string tmp = params[paramIndex];
                        if ( tmp[0] == '&')
                        {
                            tmp = tmp.substr(1,tmp.size());
                        }
                        std::vector<std::string> pair;
                        StringUtilities::stringSplit(tmp, "=", pair);
                        
                        // check if this parameter has the correct name
                        interiorOnly = pair[0] != n;
                    }
                    
                    
                }
                
                if ( interiorOnly == true )
                {
                    continue;
                }
            }
            
            if ( sample_root.containsClade(node, true) )
            {
                // if the inputTree node is also in the sample tree
                // we get the ancestral character state from the ancestral state trace
                size_t sample_clade_index = sample_root.getCladeIndex( node );
                
                const TopologyNode &sample_node = sample_tree.getNode( sample_clade_index );
                
                std::vector<std::string> params;
                if ( isNodeParameter == true )
                {
                    params = sample_node.getNodeParameters();
                }
                else
                {
                    params = sample_node.getBranchParameters();
                }
                
                // check if this parameter exists
                if ( params.size() <= paramIndex )
                {
                    if ( sample_node.isRoot() == true )
                    {
                        continue;
                    }
                    else
                    {
                        throw RbException("Too few parameter for this tree during the tree annotation.");
                    }
                    
                }
                
                std::string tmp = params[paramIndex];
                if ( tmp[0] == '&')
                {
                    tmp = tmp.substr(1,tmp.size());
                }
                std::vector<std::string> pair;
                StringUtilities::stringSplit(tmp, "=", pair);
                
                // check if this parameter has the correct name
                if ( pair[0] != n )
                {
                    throw RbException("The parameter for this tree doesn't match during the tree annotation.");
                }
                
                const std::string &state = pair[1];
                
                state_counts[node_index][state]++;
                
            } // end if the sampled tree contained this clade
            
        } // end loop over all nodes in the tree
        
    } // end loop over each iteration in the trace
    
    
    std::vector<double> posteriors;
    for (int i = 0; i < summary_nodes.size(); i++)
    {
        
        TopologyNode &node = *summary_nodes[i];
        if ( node.isTip() && interiorOnly == true )
        {
            // make parameter string for this node
            if ( isNodeParameter == true )
            {
                node.addNodeParameter(n,"{}");
            }
            else
            {
                node.addBranchParameter(n,"{}");
            }
        }
        else
        {
            
            // collect the samples
            std::set<Sample<std::string> > stateSamples;
            for (std::map<std::string, long>::iterator it = state_counts[i].begin(); it != state_counts[i].end(); ++it)
            {
                stateSamples.insert( Sample<std::string>(it->first, it->second) );
            }
            
            double total_node_pp = 0.0;
            std::string final_state = "{";
            for (std::set<Sample<std::string> >::iterator it = stateSamples.begin(); it != stateSamples.end(); ++it)
            {
                if ( total_node_pp > 0.9999 ) continue;
                
                if (it != stateSamples.begin())
                {
                    final_state += ",";
                }
                
                double pp = it->second / size(true);
                final_state += it->first + "=" + StringUtilities::toString(pp);
                total_node_pp += pp;
                
            }
            
            final_state += "}";
            
            // make parameter string for this node
            if ( isNodeParameter == true )
            {
                node.addNodeParameter(n,final_state);
            }
            else
            {
                node.addBranchParameter(n,final_state);
            }
        }
    }
    
}


/*
 * this method calculates the MAP ancestral character states for the nodes on the input_tree
 */
void TraceTree::mapContinuous(Tree &tree, const std::string &n, size_t paramIndex, double hpd, bool isNodeParameter, bool verbose  ) const
{
    
    // 2-d vectors to keep the data (posteriors and states) of the inputTree nodes: [node][data]
    const std::vector<TopologyNode*> &summary_nodes = tree.getNodes();
    std::vector<std::vector<double> > samples(summary_nodes.size(),std::vector<double>());
    
    // flag if only interior nodes are used
    bool interior_only  = false;
    bool tips_checked   = false;
    bool root_checked   = false;
    bool use_root       = true;
    
    // get the reference newick representation
    std::string reference_newick = tree.getPlainNewickRepresentation();
    
    // we also need to know the sample size
    size_t num_samples = size();
    
    // start the progress bar
    ProgressBar progress = ProgressBar(num_samples, burnin);
    if ( verbose == true )
    {
        progress.start();
    }
    
    // for fast access, get the newick strings for all clades
    std::vector<std::string> summary_newick;
    size_t num_nodes = summary_nodes.size();
    for (size_t j = 0; j < num_nodes; ++j)
    {
        TopologyNode *node = summary_nodes[j];
        summary_newick.push_back( node->computePlainNewick() );
    }
    
    // loop through all trees in tree trace
    for (size_t i = burnin; i < num_samples; ++i)
    {
        
        if ( verbose == true )
        {
            progress.update( i );
        }
        
        const Tree &sample_tree = objectAt( i );
        const TopologyNode& sample_root = sample_tree.getRoot();
        
        // create a map from newick strings to clade indices
        // we also get the newick representation of this sample
        std::map<std::string,size_t> sample_clade_indices;
        std::string sample_newick = sample_root.fillCladeIndices(sample_clade_indices);
        
        // compare if this tree is the same as the reference tree
        bool same_tree = ( reference_newick == sample_newick );
        
        // loop through all nodes in inputTree
        size_t num_nodes = summary_nodes.size();
        for (size_t j = 0; j < num_nodes; ++j)
        {
            TopologyNode *node = summary_nodes[j];
            if ( node->isTip() == true )
            {
                if ( tips_checked == false )
                {
                    
                    tips_checked = true;
                    size_t sample_clade_index = sample_clade_indices[ summary_newick[j] ];
                    
                    const TopologyNode &sample_node = sample_tree.getNode( sample_clade_index );
                    
                    std::vector<std::string> params;
                    if ( isNodeParameter == true )
                    {
                        params = sample_node.getNodeParameters();
                    }
                    else
                    {
                        params = sample_node.getBranchParameters();
                    }
                    
                    // check if this parameter exists
                    if ( params.size() > paramIndex )
                    {
                        
                        std::string tmp = params[paramIndex];
                        if ( tmp[0] == '&')
                        {
                            tmp = tmp.substr(1,tmp.size());
                        }
                        std::vector<std::string> pair;
                        StringUtilities::stringSplit(tmp, "=", pair);
                        
                        // check if this parameter has the correct name
                        interior_only = (pair[0] != n);
                    }
                    else
                    {
                        interior_only = true;
                    }
                    
                    
                }
                
                if ( interior_only == true )
                {
                    continue;
                }
            }
            
            if ( node->isRoot() == true )
            {
                if ( root_checked == false )
                {
                    
                    root_checked = true;
                    
                    size_t sample_clade_index = sample_clade_indices[ summary_newick[j] ];
                    
                    const TopologyNode &sample_node = sample_tree.getNode( sample_clade_index );
                    
                    std::vector<std::string> params;
                    if ( isNodeParameter == true )
                    {
                        params = sample_node.getNodeParameters();
                    }
                    else
                    {
                        params = sample_node.getBranchParameters();
                    }
                    
                    // check if this parameter exists
                    if ( params.size() > paramIndex )
                    {
                        
                        std::string tmp = params[paramIndex];
                        if ( tmp[0] == '&')
                        {
                            tmp = tmp.substr(1,tmp.size());
                        }
                        std::vector<std::string> pair;
                        StringUtilities::stringSplit(tmp, "=", pair);
                        
                        // check if this parameter has the correct name
                        use_root = pair[0] == n;
                    }
                    else
                    {
                        use_root = false;
                    }
                    
                    
                }
                
                if ( use_root == false )
                {
                    continue;
                }
                
            }
            
            
            if ( same_tree == true || sample_clade_indices.find( summary_newick[j] ) != sample_clade_indices.end() )
            {
                // if the inputTree node is also in the sample tree
                // we get the ancestral character state from the ancestral state trace
                size_t sample_clade_index = sample_clade_indices[ summary_newick[j] ];
                const TopologyNode &sample_node = sample_tree.getNode( sample_clade_index );
                
                std::vector<std::string> params;
                if ( isNodeParameter == true )
                {
                    params = sample_node.getNodeParameters();
                }
                else
                {
                    params = sample_node.getBranchParameters();
                }
                
                // check if this parameter exists
                if ( params.size() <= paramIndex )
                {
                    throw RbException("Too few parameter for this tree during the tree annotation.");
                }
                
                std::string tmp = params[paramIndex];
                if ( tmp[0] == '&')
                {
                    tmp = tmp.substr(1,tmp.size());
                }
                std::vector<std::string> pair;
                StringUtilities::stringSplit(tmp, "=", pair);
                
                // check if this parameter has the correct name
                if ( pair[0] != n )
                {
                    
                    throw RbException("The parameter for this tree doesn't match during the tree annotation.");
                }
                
                double state = atof(pair[1].c_str());
                
                std::vector<double> &entries = samples[j];
                entries.push_back( state );
                
            } // end if the sampled tree contained this clade
            
        } // end loop over all nodes in the tree
        
    } // end loop over each iteration in the trace
    
    
    // end the progress bar
    if ( verbose == true )
    {
        progress.finish();
    }
    
    std::vector<double> posteriors;
    for (int idx = 0; idx < num_nodes; ++idx)
    {
        
        TopologyNode &node = *summary_nodes[idx];
        if ( ( node.isTip() == false || interior_only == false ) && ( node.isRoot() == false || use_root == true ) )
        {
            
            // collect the samples
            std::vector<double> stateSamples = samples[idx];
            
            // sort the samples by frequency
            sort(stateSamples.begin(), stateSamples.end());
            
            
            size_t interval_start = ((1.0-hpd)/2.0) * stateSamples.size();
            size_t interval_median = 0.5 * stateSamples.size();
            size_t interval_end = (1.0-(1.0-hpd)/2.0) * stateSamples.size();
            interval_end = (interval_end >= stateSamples.size() ? stateSamples.size()-1 : interval_end);
            double lower = stateSamples[interval_start];
            double median = stateSamples[interval_median];
            double upper = stateSamples[interval_end];
            
            // make node age annotation
            std::string param = "{" + StringUtilities::toString(lower)
            + "," + StringUtilities::toString(upper) + "}";
            
            if ( isNodeParameter == true )
            {
                // make parameter string for this node
                node.addNodeParameter(n+"_range",param);
                
                // make parameter string for this node
                node.addNodeParameter(n,median);
            }
            else
            {
                
                // make parameter string for this node
                node.addBranchParameter(n+"_range",param);
                
                // make parameter string for this node
                node.addBranchParameter(n,median);

            }
            
        }
        
    }
    
}


void TraceTree::annotateTree( Tree &tree, AnnotationReport report, bool verbose )
{
    summarize( verbose );
    
    RBOUT("Annotating tree ...");
    
    std::string newick;
    
    if ( report.conditional_tree_ages )
    {
        Tree* tmp_tree = NULL;
        if ( clock == true )
        {
            tmp_tree = TreeUtilities::convertTree( tree );
        }
        else
        {
            tmp_tree = tree.clone();
        }
        
        if ( tmp_tree->isRooted() == false && rooted == false )
        {
            std::vector<std::string> tip_names = objectAt(0).getTipNames();
            std::sort(tip_names.begin(),tip_names.end());
            std::string outgroup = tip_names[0];
            tmp_tree->reroot( outgroup, true );
        }
        else if ( tmp_tree->isRooted() != rooted )
        {
            throw(RbException("Rooting of input tree differs from the tree sample"));
        }
        
        newick = tmp_tree->getPlainNewickRepresentation();
        
        delete tmp_tree;
        
        if ( tree_clade_ages.find(newick) == tree_clade_ages.end() )
        {
            throw(RbException("Could not find input tree in tree sample"));
        }
    }
    
    const std::vector<TopologyNode*> &nodes = tree.getNodes();
    
    double sampleSize = size() - burnin;
    
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        TopologyNode* n = nodes[i];
        
        Clade clade = n->getClade();
        Split split( clade.getBitRepresentation(), clade.getMrca(), rooted);
        
        // annotate clade posterior prob
        if ( ( !n->isTip() || ( n->isRoot() && !clade.getMrca().empty() ) ) && report.clade_probs )
        {
            double pp = cladeProbability( clade, false );
            n->addNodeParameter("posterior",pp);
        }
        
        // are we using sampled ancestors?
        if ( sampled_ancestor_counts.empty() == false )
        {
            double saFreq = sampled_ancestor_counts[n->getTaxon()];
            
            // annotate sampled ancestor prob
            if ( ((n->isTip() && n->isFossil()) || saFreq > 0) && report.sampled_ancestor_probs )
            {
                n->addNodeParameter("sampled_ancestor", saFreq / sampleSize);
            }
        }
        
        // annotate conditional clade probs and get node ages
        std::vector<double> node_ages;
        
        if ( !n->isRoot() )
        {
            Clade parent_clade = n->getParent().getClade();
            Split parent_split = Split( parent_clade.getBitRepresentation(), parent_clade.getMrca(), rooted);

            std::map<Split, std::vector<double> >& condCladeAges = conditional_clade_ages[parent_split];
            node_ages = report.conditional_clade_ages ? condCladeAges[split] : clade_ages[split];
            
            // annotate CCPs
            if ( !n->isTip() && report.conditional_clade_probs )
            {
                double parentCladeFreq = splitFrequency( parent_split );
                double ccp = condCladeAges[split].size() / parentCladeFreq;
                n->addNodeParameter("ccp",ccp);
            }
        }
        else
        {
            node_ages = clade_ages[split];
        }
        
        if ( report.conditional_tree_ages )
        {
            node_ages = tree_clade_ages[newick][split];
        }
        
        // set the node ages/branch lengths
        if ( report.node_ages )
        {
            double age = 0.0;
            
            if ( report.mean_node_ages )
            {
                // finally, we compute the mean conditional age
                for (size_t i = 0; i<node_ages.size(); ++i)
                {
                    age += node_ages[i];
                }
                age /= node_ages.size();
            }
            else // use median
            {
                
                size_t idx = node_ages.size() / 2;
                std::sort( node_ages.begin(), node_ages.end() );
                if (node_ages.size() % 2 == 1)
                {
                    age = node_ages[idx];
                }
                else
                {
                    age = (node_ages[idx-1] + node_ages[idx]) / 2;
                }
                
            }
            
            // finally, we set the age/length
            if ( clock )
            {
                n->setAge( age );
            }
            else
            {
                n->setBranchLength( age );
            }
        }
        
        // annotate the HPD node age intervals
        if ( report.node_ages_HPD )
        {
            //node_ages = cladeAges[c];
            
            std::sort(node_ages.begin(), node_ages.end());
            
            size_t total_branch_lengths = node_ages.size();
            double min_range = std::numeric_limits<double>::max();
            
            size_t interval_start = 0;
            double lower = node_ages[(int)(0.5 * (double)total_branch_lengths)];
            double upper = node_ages[(int)(0.5 * (double)total_branch_lengths)];
            
            int interval_size = (int)(report.node_ages_HPD * (double)total_branch_lengths);
            // we need to make sure that we sampled more than one age
            if ( interval_size > 1 )
            {

                // find the smallest interval that contains x% of the samples
                for (size_t j = 0; j <= (total_branch_lengths - interval_size); j++)
                {
                    double temp_lower = node_ages[j];
                    double temp_upper = node_ages[j + interval_size - 1];
                    double temp_range = std::fabs(temp_upper - temp_lower);
                    if (temp_range < min_range)
                    {
                        min_range = temp_range;
                        interval_start = j;
                    }
                
                }
                lower = node_ages[interval_start];
                upper = node_ages[interval_start + interval_size - 1];

            }
            
            // make node age annotation
            std::string interval = "{" + StringUtilities::toString(lower)
            + "," + StringUtilities::toString(upper) + "}";
            
            if ( clock == true )
            {
                if ( n->isTip() == false || ( ( n->isFossil() || upper != lower) && !n->isSampledAncestor() ) )
                {
                    std::string label = "age_" + StringUtilities::toString( (int)(report.node_ages_HPD * 100) ) + "%_HPD";
                    n->addNodeParameter(label, interval);
                }
            }
            else if ( n->isRoot() == false )
            {
                std::string label = "brlen_" + StringUtilities::toString( (int)(report.node_ages_HPD * 100) ) + "%_HPD";
                n->addBranchParameter(label, interval);
            }
        }
        
    }
    
    if ( report.node_ages && clock && report.force_positive_branch_lengths )
    {
        enforceNonnegativeBranchLengths( tree.getRoot() );
    }
    
    if ( report.MAP_parameters )
    {
        mapParameters( tree, verbose );
    }
    
}



double TraceTree::cladeProbability(const RevBayesCore::Clade &c, bool verbose )
{
    summarize(verbose);
    
    double f = 0.0;
    Clade tmp = c;
    tmp.resetTaxonBitset( objectAt(0).getTaxonBitSetMap() );
    
    try
    {
        double freq = splitFrequency( Split( tmp.getBitRepresentation(), tmp.getMrca(), rooted) );
        f = freq / size(true);
    }
    catch (RbException& e)
    {
        // do nothing
    }
    
    return f;
}


TraceTree::Split TraceTree::collectTreeSample(const TopologyNode& n, RbBitSet& intaxa, std::string newick, std::map<Split, long>& cladeCountMap)
{
    double age = (clock ? n.getAge() : n.getBranchLength() );

    std::vector<Split> child_splits;

    RbBitSet taxa(intaxa.size());
    std::set<Taxon> mrca;

    if( n.isTip() )
    {
        n.getTaxa(taxa);

        if( rooted && n.isSampledAncestor() )
        {
            sampled_ancestor_counts[n.getTaxon()]++;

            mrca.insert( n.getTaxon() );
        }
    }
    else
    {
        for (size_t i = 0; i < n.getNumberOfChildren(); i++)
        {
            const TopologyNode &child_node = n.getChild(i);

            child_splits.push_back( collectTreeSample(child_node, taxa, newick, cladeCountMap) );

            if( rooted && child_node.isSampledAncestor() )
            {
                mrca.insert(child_node.getTaxon());
            }
        }
    }

    intaxa |= taxa;

    Split parent_split(taxa, mrca, rooted);

    // store the age for this split
    clade_ages[parent_split].push_back( age );

    // increment split count
    cladeCountMap[parent_split]++;

    // add conditional clade ages
    for (std::vector<Split>::iterator child=child_splits.begin(); child !=child_splits.end(); ++child )
    {
        // inserts new entries if doesn't already exist
        conditional_clade_ages[parent_split][*child].push_back( clade_ages[*child].back() );
    }

    // store the age for this split, conditional on the tree topology
    tree_clade_ages[newick][parent_split].push_back( age );

    return parent_split;
}


double TraceTree::computeEntropy( double credible_interval_size, int num_taxa, bool verbose )
{
    summarize( verbose );
    NewickConverter converter;
    double total_prob = 0;
    double total_samples = size();
    double entropy = 0.0;
    /*double tree_count = 0.0;*/
    for (std::set<Sample<std::string> >::const_reverse_iterator it = tree_samples.rbegin(); it != tree_samples.rend(); ++it)
    {
        double freq = it->second;
        double p = freq/(total_samples-burnin);
        /*double p = freq/(total_samples);*/
        total_prob += p;
        entropy += (p * log(p));
        if ( total_prob >= credible_interval_size )
        {
            break;
        }
        
    }

    /*This calculation is directly from AMP / Jeremy's paper.*/
    double ln_ntopologies = RbMath::lnFactorial(2*num_taxa - 5) - RbMath::lnFactorial(num_taxa - 3) - (num_taxa-3)*RbConstants::LN2;
    entropy += ln_ntopologies;
    /*std::cout << ntopologies << '\n';*/
    
    return entropy;
}


std::vector<double> TraceTree::computePairwiseRFDistance( double credible_interval_size, bool verbose )
{
    summarize( verbose );
    std::vector<Tree> unique_trees;
    std::vector<size_t> sample_count;
    NewickConverter converter;
    double total_prob = 0;
    double total_samples = size();
    for (std::set<Sample<std::string> >::const_reverse_iterator it = tree_samples.rbegin(); it != tree_samples.rend(); ++it)
    {
        double freq = it->second;
        double p = freq/(total_samples-burnin);
        total_prob += p;
        
        sample_count.push_back( freq );
        
        Tree* current_tree = converter.convertFromNewick( it->first );
        unique_trees.push_back( *current_tree );
        delete current_tree;
        if ( total_prob >= credible_interval_size )
        {
            break;
        }
        
    }
    
    std::vector<double> rf_distances;
    for (size_t i=0; i<unique_trees.size(); ++i)
    {
        // first we need to compare the tree to 'itself'
        for (size_t k=0; k<(sample_count[i]*(sample_count[i]-1)); ++k )
        {
            rf_distances.push_back( 0.0 );
        }
        
        for (size_t j=i+1; j<unique_trees.size(); ++j)
        {
            const Tree &a = unique_trees[i];
            const Tree &b = unique_trees[j];
            double rf = TreeUtilities::computeRobinsonFouldDistance(a, b);
            
            for (size_t k=0; k<(sample_count[i]*sample_count[j]); ++k )
            {
                rf_distances.push_back( rf );
            }
        }
    }
    
    return rf_distances;
}


std::vector<double> TraceTree::computeTreeLengths( void )
{
    
    std::vector<double> tree_lengths;
    NewickConverter converter;
    
    for (size_t i = burnin; i < size(); ++i)
    {
        
        const Tree &tree = objectAt(i);
        tree_lengths.push_back( tree.getTreeLength() );
        
    }
    
    return tree_lengths;
}


void TraceTree::enforceNonnegativeBranchLengths(TopologyNode& node) const
{
    std::vector<TopologyNode*> children = node.getChildren();
    
    double minimum_branch_length = 1e-6;
    for (size_t i = 0; i < children.size(); i++)
    {
        if (children[i]->getAge() > node.getAge())
        {
            children[i]->setAge( node.getAge() - minimum_branch_length );
        }
        enforceNonnegativeBranchLengths( *children[i] );
    }
}


long TraceTree::splitFrequency(const Split &n) const
{
    
    std::set<Sample<Split> >::const_iterator it = find_if (clade_samples.begin(), clade_samples.end(), n );
    
    if ( it != clade_samples.end() )
    {
        return it->second;
    }
    
    throw RbException("Couldn't find split in set of samples");
}


TopologyNode* TraceTree::findParentNode(TopologyNode& n, const Split& tmp, std::vector<TopologyNode*>& children, RbBitSet& child_b ) const
{
    size_t num_taxa = child_b.size();
    
    RbBitSet node( num_taxa );
    n.getTaxa(node);
    
    RbBitSet clade = tmp.first;
    
    RbBitSet mask  = node | clade;
    
    bool compatible = (mask == node);
    bool child      = (mask == clade);
    
    Split c = tmp;
    // check if the flipped unrooted split is compatible
    if ( !rooted && !compatible && !child)
    {
        RbBitSet clade_flip = clade; ~clade_flip;
        mask  = node | clade_flip;
        
        compatible = (mask == node);
        
        if ( compatible )
        {
            c.first = clade_flip;
        }
    }
    
    TopologyNode* parent = NULL;
    
    if (compatible)
    {
        parent = &n;
        
        std::vector<TopologyNode*> x = n.getChildren();
        
        std::vector<TopologyNode*> new_children;
        
        // keep track of which taxa we found in the children
        RbBitSet child_mask( num_taxa );
        
        for (size_t i = 0; i < x.size(); i++)
        {
            RbBitSet child_b(clade.size());
            
            TopologyNode* child = findParentNode(*x[i], c, new_children, child_b );
            
            // add this child to the mask
            child_mask = (child_b | child_mask);
            
            // check if child is a compatible parent
            if (child != NULL)
            {
                parent = child;
                break;
            }
        }
        
        children = new_children;
        
        // check that we found all the children
        if ( parent == &n && child_mask != clade)
        {
            parent = NULL;
        }
    }
    else if (child)
    {
        child_b = node;
        children.push_back(&n);
    }
    
    return parent;
}


int TraceTree::getBurnin( void ) const
{
    
    return (int)burnin;
}


int TraceTree::getTopologyFrequency(const RevBayesCore::Tree &tree, bool verbose)
{
    summarize( verbose );
    
    std::vector<std::string> tip_names = objectAt(0).getTipNames();
    std::sort(tip_names.begin(),tip_names.end());
    std::string outgroup = tip_names[0];
    
    Tree t = tree;
    
    if ( t.isRooted() == false && rooted == false )
    {
        t.reroot( outgroup, true );
    }
    
    std::string newick = t.getPlainNewickRepresentation();
    
    //double total_samples = trace.size();
    double freq = 0;
    
    for (std::set<Sample<std::string> >::const_reverse_iterator it = tree_samples.rbegin(); it != tree_samples.rend(); ++it)
    {
        
        if ( newick == it->first )
        {
            freq =it->second;
            //            p = freq/(total_samples-burnin);
            
            // now we found it
            break;
        }
        
    }
    
    return freq;
}


std::vector<Tree> TraceTree::getUniqueTrees( double credible_interval_size, bool verbose )
{
    summarize( verbose );
    
    std::vector<Tree> unique_trees;
    NewickConverter converter;
    double total_prob = 0;
    double total_samples = size();
    for (std::set<Sample<std::string> >::const_reverse_iterator it = tree_samples.rbegin(); it != tree_samples.rend(); ++it)
    {
        double freq =it->second;
        double p =freq/(total_samples-burnin);
        total_prob += p;
        
        Tree* current_tree = converter.convertFromNewick( it->first );
        unique_trees.push_back( *current_tree );
        delete current_tree;
        if ( total_prob >= credible_interval_size )
        {
            break;
        }
        
    }
    
    return unique_trees;
}



bool TraceTree::isCoveredInInterval(const std::string &v, double ci_size, bool verbose)
{
    
    summarize( verbose );
    

    RandomNumberGenerator *rng = GLOBAL_RNG;
    
    std::vector<std::string> tip_names = objectAt(0).getTipNames();
    std::sort(tip_names.begin(),tip_names.end());
    std::string outgroup = tip_names[0];
    
    Tree tree;
    tree.initFromString(v);

    if ( tree.isRooted() == false && rooted == false )
    {
        tree.reroot( outgroup, true );
    }
    
    std::string newick = tree.getPlainNewickRepresentation();
    
    double totalSamples = size();
    double totalProb = 0.0;
    for (std::set<Sample<std::string> >::reverse_iterator it = tree_samples.rbegin(); it != tree_samples.rend(); ++it)
    {
        
        double p = it->second/(totalSamples-burnin);
//        double include_prob = p / (1.0-totalProb) * (ci_size - totalProb) / (1.0-totalProb);
        double include_prob = (ci_size-totalProb)/p;
//        double include_prob = p * ci_size;
        
        if ( include_prob > rng->uniform01() )
        {
            const std::string &current_sample = it->first;
            if ( newick == current_sample )
            {
                return true;
            }
            
        }
        
        totalProb += p;
        
        
        if ( totalProb >= ci_size )
        {
            break;
        }
        
    }
    
    return false;
}


bool TraceTree::isClock(void)
{
    return clock;
}


Tree* TraceTree::mapTree( AnnotationReport report, bool verbose )
{
    std::stringstream ss;
    ss << "Compiling maximum a posteriori tree from " << size() << " trees in tree trace, using a burnin of " << burnin << " trees.\n";
    RBOUT(ss.str());
    
    summarize( verbose );
    
    // get the tree with the highest posterior probability
    std::string bestNewick = tree_samples.rbegin()->first;
    NewickConverter converter;
    Tree* tmp_best_tree = converter.convertFromNewick( bestNewick );
    
    Tree* tmp_tree = NULL;
    
    if ( clock == true )
    {
        tmp_tree = TreeUtilities::convertTree( *tmp_best_tree );
    }
    else
    {
        tmp_tree = tmp_best_tree->clone();
    }
    
    delete tmp_best_tree;
    
    TaxonMap tm = TaxonMap( objectAt(0) );
    tmp_tree->setTaxonIndices( tm );
    
    report.MAP_parameters = true;
    report.node_ages      = true;
    annotateTree(*tmp_tree, report, verbose );
    
    return tmp_tree;
}


Tree* TraceTree::mccTree( AnnotationReport report, bool verbose )
{
    std::stringstream ss;
    ss << "Compiling maximum clade credibility tree from " << size() << " trees in tree trace, using a burnin of " << burnin << " trees.\n";
    RBOUT(ss.str());
    
    summarize( verbose );
    
    Tree* best_tree = NULL;
    double max_cc = 0;
    
    // find the clade credibility score for each tree
    for (std::set<Sample<std::string> >::reverse_iterator it = tree_samples.rbegin(); it != tree_samples.rend(); ++it)
    {
        std::string newick = it->first;
        
        // now we summarize the clades for the best tree
        std::map<Split, std::vector<double> > cladeAges = tree_clade_ages[newick];
        
        double cc = 0;
        
        // find the product of the clade frequencies
        for (std::map<Split, std::vector<double> >::iterator clade = cladeAges.begin(); clade != cladeAges.end(); clade++)
        {
            cc += log( splitFrequency(clade->first) );
        }
        
        if (cc > max_cc)
        {
            max_cc = cc;
            
            delete best_tree;
            
            NewickConverter converter;
            Tree* tmp_tree = converter.convertFromNewick( newick );
            if ( clock == true )
            {
                best_tree = TreeUtilities::convertTree( *tmp_tree );
            }
            else
            {
                best_tree = tmp_tree->clone();
            }
            
            TaxonMap tm = TaxonMap( objectAt(0) );
            best_tree->setTaxonIndices( tm );
            
            delete tmp_tree;
        }
    }
    
    report.node_ages = true;
    annotateTree(*best_tree, report, verbose );
    
    return best_tree;
}


Tree* TraceTree::mrTree(AnnotationReport report, double cutoff, bool verbose)
{
    if (cutoff < 0.0 || cutoff > 1.0) cutoff = 0.5;
    
    std::stringstream ss;
    ss << "Compiling majority rule consensus tree (cutoff = " << cutoff << ") from " << size() << " trees in tree trace, using a burnin of " << burnin << " trees.\n";
    RBOUT(ss.str());
    
    //fill in clades, use all above 50% to resolve the bush with the consensus partitions
    summarize( verbose );        //fills std::vector<Sample<std::string> > cladeSamples, sorts them by descending freq
    
    //set up variables for consensus tree assembly
    std::vector<std::string> tipNames = objectAt(0).getTipNames();
    
    //first create a bush
    TopologyNode* root = new TopologyNode(tipNames.size()); //construct root node with index = nb Tips
    root->setNodeType(false, true, true);
    
    for (size_t i = 0; i < tipNames.size(); i++)
    {
        TopologyNode* tipNode = new TopologyNode(tipNames.at(i), i); //Topology node constructor adding tip name and index=taxon nb
        tipNode->setNodeType(true, false, false);
        
        // set the parent-child relationship
        root->addChild(tipNode);
        tipNode->setParent(root);
    }
    
    //now put the tree together
    Tree* consensusTree = new Tree();
    consensusTree->setRoot(root, true);
    
    size_t nIndex = tipNames.size();
    
    for (std::set<Sample<Split> >::reverse_iterator it = clade_samples.rbegin(); it != clade_samples.rend(); ++it)
    {
        float cladeFreq = it->second / (float)(size() - burnin);
        if (cladeFreq < cutoff)  break;
        
        Split clade = it->first;
        
        //make sure we have an internal node
        size_t clade_size = clade.first.getNumberSetBits();
        if (clade_size == 1 || clade_size == tipNames.size())  continue;
        
        //find parent node
        std::vector<TopologyNode*> children;
        RbBitSet tmp(tipNames.size());
        TopologyNode* parentNode = findParentNode(*root, clade, children, tmp );
        
        if (parentNode != NULL )
        {
            // skip this node if we've already found a clade compatible with it
            if ( children.size() == parentNode->getNumberOfChildren() ) continue;
            
            std::vector<TopologyNode*> mrca;
            
            // find the mrca child if it exists
            if ( clade.second.empty() == false )
            {
                for (size_t i = 0; i < children.size(); i++)
                {
                    if ( children[i]->isTip() && std::find(clade.second.begin(), clade.second.end(), children[i]->getTaxon() ) != clade.second.end() )
                    {
                        mrca.push_back(children[i]);
                    }
                }
                
                // if we couldn't find the mrca, then this clade is not compatible
                if ( mrca.size() != clade.second.size() )
                {
                    continue;
                }
                else
                {
                    for (size_t i = 0; i < mrca.size(); i++)
                    {
                        mrca[i]->setSampledAncestor(true);
                    }
                }
            }
            
            nIndex++;   //increment node index
            TopologyNode* intNode = new TopologyNode(nIndex); //Topology node constructor, with proper node index
            intNode->setNodeType(false, false, true);
            
            
            // move the children to a new internal node
            for (size_t i = 0; i < children.size(); i++)
            {
                parentNode->removeChild(children[i]);
                intNode->addChild(children[i]);
                children[i]->setParent(intNode);
            }
            
            intNode->setParent(parentNode);
            parentNode->addChild(intNode);
            
            // add a mrca child if it exists and there is more than one non-mrca taxa
            if ( mrca.empty() == false && children.size() > 2 )
            {
                TopologyNode* old_parent = parentNode;
                
                nIndex++;   //increment node index
                parentNode = new TopologyNode(nIndex); //Topology node constructor, with proper node index
                parentNode->setNodeType(false, false, true);
                
                intNode->removeChild(mrca[0]);
                parentNode->addChild(mrca[0]);
                mrca[0]->setParent(parentNode);
                
                old_parent->removeChild(intNode);
                old_parent->addChild(parentNode);
                parentNode->setParent(old_parent);
                
                parentNode->addChild(intNode);
                intNode->setParent(parentNode);
            }
        }
        
        root->setTree(consensusTree);
    }
    
    //now put the tree together
    consensusTree->setRoot(root, true);
    
    report.conditional_clade_ages  = false;
    report.conditional_clade_probs = false;
    report.conditional_tree_ages   = false;
    report.node_ages               = true;
    annotateTree(*consensusTree, report, verbose );
    
    return consensusTree;
}


void TraceTree::printCladeSummary(std::ostream &o, double minCladeProbability, bool verbose)
{
    summarize( verbose );
    
    std::stringstream ss;
    ss << std::fixed;
    ss << std::setprecision(4);
    
    o << std::endl;
    o << "=========================================" << std::endl;
    o << "Printing Posterior Distribution of Clades" << std::endl;
    o << "=========================================" << std::endl;
    o << std::endl;
    
    // now the printing
    std::string s = "Samples";
    StringUtilities::fillWithSpaces(s, 16, true);
    o << "\n" << s;
    s = "Posterior";
    StringUtilities::fillWithSpaces(s, 16, true);
    o << s;
    /*s = "ESS";
     StringUtilities::fillWithSpaces(s, 16, true);
     o << s;*/
    s = "Clade";
    StringUtilities::fillWithSpaces(s, 16, true);
    o << s;
    o << std::endl;
    o << "--------------------------------------------------------------" << std::endl;
    
    double totalSamples = size();
    
    std::vector<Taxon> ordered_taxa = objectAt(0).getTaxa();
    VectorUtilities::sort( ordered_taxa );

    for (std::set<Sample<Split> >::reverse_iterator it = clade_samples.rbegin(); it != clade_samples.rend(); ++it)
    {
        Clade c(it->first.first, ordered_taxa);
        c.setMrca(it->first.second);

        if ( c.size() == 1 ) continue;
        
        double freq = it->second;
        double p = freq/(totalSamples-burnin);
        
        
        if ( p < minCladeProbability )
        {
            break;
        }
        
        ss.str(std::string());
        ss << freq;
        s = ss.str();
        StringUtilities::fillWithSpaces(s, 16, true);
        o << s;
        
        ss.str(std::string());
        ss << p;
        s = ss.str();
        StringUtilities::fillWithSpaces(s, 16, true);
        o << s;
        
        /*ss.str(std::string());
         if ( it->getFrequency() <  totalSamples - burnin && it->getFrequency() > 0 )
         {
         ss << it->getEss();
         }
         else
         {
         ss << " - ";
         
         }
         s = ss.str();
         StringUtilities::fillWithSpaces(s, 16, true);
         o << s;*/

        o << c;
        o << std::endl;
        
    }
    
    o << std::endl;
    o << std::endl;
    
}



void TraceTree::printTreeSummary(std::ostream &o, double credibleIntervalSize, bool verbose)
{
    summarize( verbose );
    
    std::stringstream ss;
    ss << std::fixed;
    ss << std::setprecision(4);
    
    o << std::endl;
    o << "========================================" << std::endl;
    o << "Printing Posterior Distribution of Trees" << std::endl;
    o << "========================================" << std::endl;
    o << std::endl;
    
    // now the printing
    std::string s = "Cum. Prob.";
    StringUtilities::fillWithSpaces(s, 16, true);
    o << s;
    s = "Samples";
    StringUtilities::fillWithSpaces(s, 16, true);
    o << s;
    s = "Posterior";
    StringUtilities::fillWithSpaces(s, 16, true);
    o << s;
    /*s = "ESS";
     StringUtilities::fillWithSpaces(s, 16, true);
     o << s;*/
    s = "Tree";
    StringUtilities::fillWithSpaces(s, 16, true);
    o << s;
    o << std::endl;
    o << "----------------------------------------------------------------" << std::endl;
    double totalSamples = size();
    double totalProb = 0.0;
    for (std::set<Sample<std::string> >::reverse_iterator it = tree_samples.rbegin(); it != tree_samples.rend(); ++it)
    {
        double freq =it->second;
        double p = freq/(totalSamples-burnin);
        totalProb += p;
        
        ss.str(std::string());
        ss << totalProb;
        s = ss.str();
        StringUtilities::fillWithSpaces(s, 16, true);
        o << s;
        
        ss.str(std::string());
        ss << freq;
        s = ss.str();
        StringUtilities::fillWithSpaces(s, 16, true);
        o << s;
        
        ss.str(std::string());
        ss << p;
        s = ss.str();
        StringUtilities::fillWithSpaces(s, 16, true);
        o << s;
        
        /*ss.str(std::string());
         ss << it->getEss();
         s = ss.str();
         StringUtilities::fillWithSpaces(s, 16, true);
         o << s;*/
        
        o << it->first;
        o << std::endl;
        
        if ( totalProb >= credibleIntervalSize )
        {
            break;
        }
        
    }
    
    o << std::endl;
    o << std::endl;
    
}


void TraceTree::summarize( bool verbose )
{
    if ( isDirty() == false ) return;
    
    std::vector<std::string> tip_names = objectAt(0).getTipNames();
    std::sort(tip_names.begin(),tip_names.end());
    std::string outgroup = tip_names[0];

    rooted = objectAt(0).isRooted();

    clade_samples.clear();
    tree_samples.clear();

    sampled_ancestor_counts.clear();

    clade_ages.clear();
    conditional_clade_ages.clear();
    tree_clade_ages.clear();
    
    std::map<Split, long>       clade_counts;
    std::map<std::string, long> tree_counts;

    
    ProgressBar progress = ProgressBar(size(), burnin);
    if ( verbose )
    {
        RBOUT("Summarizing clades ...\n");
        progress.start();
    }
    
    for (size_t i = burnin; i < size(); ++i)
    {
        if ( verbose )
        {
            progress.update(i);
        }
        
        Tree tree = objectAt(i);
        
        if ( rooted == false )
        {
            tree.reroot( outgroup, true );
        }
        
        std::string newick = tree.getPlainNewickRepresentation();

        tree_counts[newick]++;

        // get the clades for this tree
        RbBitSet b( tree.getNumberOfTips(), false );
        collectTreeSample(tree.getRoot(), b, newick, clade_counts);
    }
    
    // sort the clade samples in ascending frequency
    for(std::map<Split, long>::iterator it = clade_counts.begin(); it != clade_counts.end(); it++)
    {
        clade_samples.insert( Sample<Split>(it->first, it->second) );
    }

    // sort the tree samples in ascending frequency
    for(std::map<std::string, long>::iterator it = tree_counts.begin(); it != tree_counts.end(); it++)
    {
        tree_samples.insert( Sample<std::string>(it->first, it->second) );
    }

    // finish progress bar
    if ( verbose )
    {
        progress.finish();
    }
    
    dirty = false;
}
