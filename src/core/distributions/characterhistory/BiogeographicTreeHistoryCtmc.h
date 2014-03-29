//
//  BiogeographicTreeHistoryCtmc.h
//  rb_mlandis
//
//  Created by Michael Landis on 3/29/14.
//  Copyright (c) 2014 Michael Landis. All rights reserved.
//

#ifndef __rb_mlandis__BiogeographicTreeHistoryCtmc__
#define __rb_mlandis__BiogeographicTreeHistoryCtmc__

#include "DnaState.h"
#include "GeneralTreeHistoryCtmc.h"
#include "RateMatrix.h"
#include "RbVector.h"
#include "TopologyNode.h"
#include "TransitionProbabilityMatrix.h"
#include "TreeChangeEventListener.h"
#include "TypedDistribution.h"

namespace RevBayesCore {
    
    template<class charType, class treeType>
    class BiogeographicTreeHistoryCtmc : public GeneralTreeHistoryCtmc<charType, treeType> {
        
    public:
        BiogeographicTreeHistoryCtmc(const TypedDagNode< treeType > *t, size_t nChars, bool c, size_t nSites);
        BiogeographicTreeHistoryCtmc(const BiogeographicTreeHistoryCtmc &n);                                                                                                //!< Copy constructor
        virtual                                            ~BiogeographicTreeHistoryCtmc(void);                                                                   //!< Virtual destructor
        
        // public member functions
        BiogeographicTreeHistoryCtmc*             clone(void) const;                                                                          //!< Create an independent clone
        void                                                swapParameter(const DagNode *oldP, const DagNode *newP);                                    //!< Implementation of swaping parameters
        
    protected:
        
        virtual void                                        computeRootLikelihood(size_t root, size_t l, size_t r);
        virtual void                                        computeInternalNodeLikelihood(const TopologyNode &n, size_t nIdx, size_t l, size_t r);
        virtual void                                        computeTipLikelihood(const TopologyNode &node, size_t nIdx);
        // (not needed)        void                         keepSpecialization(DagNode* affecter);
        // (not needed)        void                         restoreSpecialization(DagNode *restorer);
        virtual void                                        touchSpecialization(DagNode *toucher);
        virtual void                                        updateTransitionProbabilities(size_t nodeIdx, double brlen);
        
        
    private:
        // members
        const TypedDagNode< double >*                       distancePower;
        
        // flags specifying which model variants we use
        bool                                                useDistancePower;
    };
    
}


#include "ConstantNode.h"
#include "DiscreteCharacterState.h"
#include "RateMatrix_JC.h"
#include "RandomNumberFactory.h"
#include "TopologyNode.h"
#include "TransitionProbabilityMatrix.h"

#include <cmath>
#include <cstring>

template<class charType, class treeType>
RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::BiogeographicTreeHistoryCtmc(const TypedDagNode<treeType> *t, size_t nChars, bool c, size_t nSites) : GeneralTreeHistoryCtmc<charType, treeType>(  t, nChars, 1, c, nSites ) {
    
    // initialize with default parameters
    distancePower               = NULL;

    // flags specifying which model variants we use
    useDistancePower            = false;
    
    this->redrawValue();
}


template<class charType, class treeType>
RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::BiogeographicTreeHistoryCtmc(const BiogeographicTreeHistoryCtmc &d) : GeneralTreeHistoryCtmc<charType, treeType>( d ) {
    // parameters are automatically copied
    // initialize with default parameters
    distancePower        = d.distancePower;
    
    
    // flags specifying which model variants we use
    useDistancePower     = d.useDistancePower;
    
}


template<class charType, class treeType>
RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::~BiogeographicTreeHistoryCtmc( void ) {
    // We don't delete the parameters, because they might be used somewhere else too. The model needs to do that!
    
}


template<class charType, class treeType>
RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>* RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::clone( void ) const {
    
    return new BiogeographicTreeHistoryCtmc<charType, treeType>( *this );
}



template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::computeRootLikelihood( size_t root, size_t left, size_t right)
{
    
    // reset the likelihood
    this->lnProb = 0.0;
    
    
    /*
    // get the root frequencies
    const std::vector<double> &f                    = getRootFrequencies();
    std::vector<double>::const_iterator f_end       = f.end();
    std::vector<double>::const_iterator f_begin     = f.begin();
    
    // get the pointers to the partial likelihoods of the left and right subtree
    const double* p_left   = this->partialLikelihoods + this->activeLikelihood[left]*this->activeLikelihoodOffset + left*this->nodeOffset;
    const double* p_right  = this->partialLikelihoods + this->activeLikelihood[right]*this->activeLikelihoodOffset + right*this->nodeOffset;
    
    // create a vector for the per mixture likelihoods
    // we need this vector to sum over the different mixture likelihoods
    std::vector<double> per_mixture_Likelihoods = std::vector<double>(this->numPatterns,0.0);
    
    // get pointers the likelihood for both subtrees
    const double*   p_mixture_left     = p_left;
    const double*   p_mixture_right    = p_right;
    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < this->numSiteRates; ++mixture)
    {
        
        // get pointers to the likelihood for this mixture category
        const double*   p_site_mixture_left     = p_mixture_left;
        const double*   p_site_mixture_right    = p_mixture_right;
        // iterate over all sites
        for (size_t site = 0; site < this->numPatterns; ++site)
        {
            // temporary variable storing the likelihood
            double tmp = 0.0;
            // get the pointer to the stationary frequencies
            std::vector<double>::const_iterator f_j             = f_begin;
            // get the pointers to the likelihoods for this site and mixture category
            const double* p_site_left_j   = p_site_mixture_left;
            const double* p_site_right_j  = p_site_mixture_right;
            // iterate over all starting states
            for (; f_j != f_end; ++f_j)
            {
                // add the probability of starting from this state
                tmp += *p_site_left_j * *p_site_right_j * *f_j;
                
                // increment pointers
                ++p_site_left_j; ++p_site_right_j;
            }
            // add the likelihood for this mixture category
            per_mixture_Likelihoods[site] += tmp;
            
            // increment the pointers to the next site
            p_site_mixture_left+=this->siteOffset; p_site_mixture_right+=this->siteOffset;
            
        } // end-for over all sites (=patterns)
        
        // increment the pointers to the next mixture category
        p_mixture_left+=this->mixtureOffset; p_mixture_right+=this->mixtureOffset;
        
    } // end-for over all mixtures (=rate categories)
    
    // sum the log-likelihoods for all sites together
    std::vector< size_t >::const_iterator patterns = this->patternCounts.begin();
    for (size_t site = 0; site < this->numPatterns; ++site, ++patterns)
    {
        this->lnProb += log( per_mixture_Likelihoods[site] ) * *patterns;
    }
    // normalize the log-probability
    this->lnProb -= log( this->numSiteRates ) * this->numSites;
     
     */
    
}


template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::computeInternalNodeLikelihood(const TopologyNode &node, size_t nodeIndex, size_t left, size_t right)
{
    
    /*
     
    // compute the transition probability matrix
    updateTransitionProbabilities( nodeIndex, node.getBranchLength() );
    
    // get the pointers to the partial likelihoods for this node and the two descendant subtrees
    const double*   p_left  = this->partialLikelihoods + this->activeLikelihood[left]*this->activeLikelihoodOffset + left*this->nodeOffset;
    const double*   p_right = this->partialLikelihoods + this->activeLikelihood[right]*this->activeLikelihoodOffset + right*this->nodeOffset;
    double*         p_node  = this->partialLikelihoods + this->activeLikelihood[nodeIndex]*this->activeLikelihoodOffset + nodeIndex*this->nodeOffset;
    
    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < this->numSiteRates; ++mixture)
    {
        // the transition probability matrix for this mixture category
        const double*                       tp_begin    = this->transitionProbMatrices[mixture].theMatrix;
        
        // get the pointers to the likelihood for this mixture category
        size_t offset = mixture*this->mixtureOffset;
        double*          p_site_mixture          = p_node + offset;
        const double*    p_site_mixture_left     = p_left + offset;
        const double*    p_site_mixture_right    = p_right + offset;
        // compute the per site probabilities
        for (size_t site = 0; site < this->numPatterns ; ++site)
        {
            
            // get the pointers for this mixture category and this site
            const double*       tp_a    = tp_begin;
            // iterate over the possible starting states
            for (size_t c1 = 0; c1 < this->numChars; ++c1)
            {
                // temporary variable
                double sum = 0.0;
                
                // iterate over all possible terminal states
                for (size_t c2 = 0; c2 < this->numChars; ++c2 )
                {
                    sum += p_site_mixture_left[c2] * p_site_mixture_right[c2] * tp_a[c2];
                    
                } // end-for over all distination character
                
                // store the likelihood for this starting state
                p_site_mixture[c1] = sum;
                
                // increment the pointers to the next starting state
                tp_a+=this->numChars;
                
            } // end-for over all initial characters
            
            // increment the pointers to the next site
            p_site_mixture_left+=this->siteOffset; p_site_mixture_right+=this->siteOffset; p_site_mixture+=this->siteOffset;
            
        } // end-for over all sites (=patterns)
        
    } // end-for over all mixtures (=rate-categories)
    */
    
}


template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::computeTipLikelihood(const TopologyNode &node, size_t nodeIndex)
{
    
    /*
     
    double* p_node = this->partialLikelihoods + this->activeLikelihood[nodeIndex]*this->activeLikelihoodOffset + nodeIndex*this->nodeOffset;
    
    const std::vector<bool> &gap_node = this->gapMatrix[nodeIndex];
    const std::vector<unsigned long> &char_node = this->charMatrix[nodeIndex];
    
    // compute the transition probabilities
    updateTransitionProbabilities( nodeIndex, node.getBranchLength() );
    
    double*   p_mixture      = p_node;
    
    // iterate over all mixture categories
    for (size_t mixture = 0; mixture < this->numSiteRates; ++mixture)
    {
        // the transition probability matrix for this mixture category
        const double*                       tp_begin    = this->transitionProbMatrices[mixture].theMatrix;
        
        // get the pointer to the likelihoods for this site and mixture category
        double*     p_site_mixture      = p_mixture;
        
        // iterate over all sites
        for (size_t site = 0; site != this->numPatterns; ++site)
        {
            
            // is this site a gap?
            if ( gap_node[site] )
            {
                // since this is a gap we need to assume that the actual state could have been any state
                
                // iterate over all initial states for the transitions
                for (size_t c1 = 0; c1 < this->numChars; ++c1)
                {
                    
                    // store the likelihood
                    p_site_mixture[c1] = 1.0;
                    
                }
            }
            else // we have observed a character
            {
                
                // get the original character
                unsigned long org_val = char_node[site];
                
                // iterate over all possible initial states
                for (size_t c1 = 0; c1 < this->numChars; ++c1)
                {
                    
                    if ( this->usingAmbiguousCharacters )
                    {
                        // compute the likelihood that we had a transition from state c1 to the observed state org_val
                        // note, the observed state could be ambiguous!
                        unsigned long val = org_val;
                        
                        // get the pointer to the transition probabilities for the terminal states
                        const double* d  = tp_begin+(this->numChars*c1);
                        
                        double tmp = 0.0;
                        
                        while ( val != 0 ) // there are still observed states left
                        {
                            // check whether we observed this state
                            if ( (val & 1) == 1 )
                            {
                                // add the probability
                                tmp += *d;
                            }
                            
                            // remove this state from the observed states
                            val >>= 1;
                            
                            // increment the pointer to the next transition probability
                            ++d;
                        } // end-while over all observed states for this character
                        
                        // store the likelihood
                        p_site_mixture[c1] = tmp;
                        
                    }
                    else // no ambiguous characters in use
                    {
                        
                        // store the likelihood
                        p_site_mixture[c1] = tp_begin[c1*this->numChars+org_val];
                        
                    }
                    
                } // end-for over all possible initial character for the branch
                
            } // end-if a gap state
            
            // increment the pointers to next site
            p_site_mixture+=this->siteOffset;
            
        } // end-for over all sites/patterns in the sequence
        
        // increment the pointers to next mixture category
        p_mixture+=this->mixtureOffset;
        
    } // end-for over all mixture categories
    */
    
}




template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::updateTransitionProbabilities(size_t nodeIdx, double brlen) {
    
    /*
     
    // first, get the rate matrix for this branch
    const RateMatrix *rm;
    if ( this->branchHeterogeneousSubstitutionMatrices == true )
    {
        rm = &this->heterogeneousRateMatrices->getValue()[nodeIdx];
    }
    else
    {
        rm = &this->homogeneousRateMatrix->getValue();
    }
    
    // second, get the clock rate for the branch
    double branchTime;
    if ( this->branchHeterogeneousClockRates == true )
    {
        branchTime = this->heterogeneousClockRates->getValue()[nodeIdx] * brlen;
    }
    else
    {
        branchTime = this->homogeneousClockRate->getValue() * brlen;
    }
    
    // and finally compute the per site rate transition probability matrix
    if ( this->rateVariationAcrossSites == true )
    {
        const std::vector<double> &r = this->siteRates->getValue();
        for (size_t i = 0; i < this->numSiteRates; ++i)
        {
            rm->calculateTransitionProbabilities( branchTime * r[i], this->transitionProbMatrices[i] );
        }
    }
    else
    {
        rm->calculateTransitionProbabilities( branchTime, this->transitionProbMatrices[0] );
    }
     
    */
    
}

template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::swapParameter(const DagNode *oldP, const DagNode *newP) {
    
    if (oldP == distancePower)
    {
        distancePower = static_cast<const TypedDagNode< double >* >( newP );
    }
    else
    {
        GeneralTreeHistoryCtmc<charType, treeType>::swapParameter(oldP,newP);
    }
    
}

template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::touchSpecialization( DagNode* affecter ) {
    
    /*
    // if the topology wasn't the culprit for the touch, then we just flag everything as dirty
    if ( affecter == heterogeneousClockRates )
    {
        const std::set<size_t> &indices = heterogeneousClockRates->getTouchedElementIndices();
        
        // maybe all of them have been touched or the flags haven't been set properly
        if ( indices.size() == 0 )
        {
            // just delegate the call
            GeneralTreeHistoryCtmc<charType, treeType>::touchSpecialization( affecter );
        }
        else
        {
            const std::vector<TopologyNode *> &nodes = this->tau->getValue().getNodes();
            // flag recomputation only for the nodes
            for (std::set<size_t>::iterator it = indices.begin(); it != indices.end(); ++it)
            {
                this->recursivelyFlagNodeDirty( *nodes[*it] );
            }
        }
    }
    else if ( affecter == heterogeneousRateMatrices )
    {
        
        const std::set<size_t> &indices = heterogeneousRateMatrices->getTouchedElementIndices();
        
        // maybe all of them have been touched or the flags haven't been set properly
        if ( indices.size() == 0 )
        {
            // just delegate the call
            GeneralTreeHistoryCtmc<charType, treeType>::touchSpecialization( affecter );
        }
        else
        {
            const std::vector<TopologyNode *> &nodes = this->tau->getValue().getNodes();
            // flag recomputation only for the nodes
            for (std::set<size_t>::iterator it = indices.begin(); it != indices.end(); ++it)
            {
                this->recursivelyFlagNodeDirty( *nodes[*it] );
            }
        }
    }
    else if ( affecter == rootFrequencies )
    {
        
        const TopologyNode &root = this->tau->getValue().getRoot();
        this->recursivelyFlagNodeDirty( root );
    }
    else
    */
    {
        GeneralTreeHistoryCtmc<charType, treeType>::touchSpecialization( affecter );
    }
    
}

#endif /* defined(__rb_mlandis__BiogeographicTreeHistoryCtmc__) */
