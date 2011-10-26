/**
 * @file
 * This file contains the implementation of Dist_multinomial, which is used to hold
 * parameters and functions related to a multinomial distribution.
 *
 * @brief Implementation of Dist_multinomial
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date: 2009-12-14 12:43:32 +0100 (Mån, 14 Dec 2009) $
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 * @since 2009-08-27, version 1.0
 *
 * $Id: MemberObject.h 194 2009-12-14 11:43:32Z ronquist $
 */

#include "ArgumentRule.h"
#include "DAGNode.h"
#include "Dist_multinomial.h"
#include "DistributionMultinomial.h"
#include "Move_mmultinomial.h"
#include "RealPos.h"
#include "RandomNumberGenerator.h"
#include "Real.h"
#include "RbException.h"
#include "RbUtil.h"
#include "Simplex.h"
#include "ValueRule.h"
#include "VectorInteger.h"
#include "VectorReal.h"
#include "VectorString.h"
#include "Workspace.h"

#include <cmath>
#include <sstream>
#include <vector>


// Definition of the static type spec member
const TypeSpec Dist_multinomial::typeSpec(Dist_multinomial_name);
const TypeSpec Dist_multinomial::varTypeSpec(Natural_name);

/** Default constructor for parser use */
Dist_multinomial::Dist_multinomial( void ) : DistributionContinuous( getMemberRules() ) {
}


/** Cdf function */
double Dist_multinomial::cdf( const RbPtr<RbLanguageObject> value ) {

    throw RbException( "Cdf function of multinomial not implemented yet" );
}


/** Clone this object */
Dist_multinomial* Dist_multinomial::clone( void ) const {

    return new Dist_multinomial( *this );
}


/** Get class vector showing type of object */
const VectorString& Dist_multinomial::getClass( void ) const {

    static VectorString rbClass = VectorString(Dist_multinomial_name) + DistributionContinuous::getClass();
    return rbClass;
}


/** Get member variable rules */
const RbPtr<MemberRules> Dist_multinomial::getMemberRules( void ) const {

    static RbPtr<MemberRules> memberRules( new MemberRules() );
    static bool        rulesSet = false;

    if ( !rulesSet )
		{
        memberRules->push_back( RbPtr<ArgumentRule>( new ValueRule( "p", Simplex_name ) ) );

        rulesSet = true;
		}

    return memberRules;
}


/** Get the type spec of this class. We return a static class variable because all instances will be exactly from this type. */
const TypeSpec& Dist_multinomial::getTypeSpec(void) const {
    return typeSpec;
}


/** Get random variable type */
const TypeSpec& Dist_multinomial::getVariableType( void ) const {

    return varTypeSpec;
}


/**
 * This function calculates the natural log of the probability
 * density for a multinomially-distributed random variable.
 *
 * @brief Natural log of multinomial probability density
 *
 * @param value Observed value
 * @return      Natural log of the probability density
 */
double Dist_multinomial::lnPdf( const RbPtr<RbLanguageObject> value ) {

	// Get the value and the parameters of the Dirichlet
    std::vector<double>       p = static_cast<const Simplex*      >( getMemberValue( "p" ).get() )->getValue();
    std::vector<unsigned int> x = static_cast<const VectorNatural*>( value.get()                 )->getValue();

	// Check that the vectors are both the same size
	if ( p.size() != x.size() )
		throw RbException( "Inconsistent size of vectors when calculating Dirichlet log probability density" );

	return RbStatistics::Multinomial::lnPdf( p, x );
}


/**
 * This function calculates the probability
 * for a multinomially-distributed random variable.
 *
 * @brief Multinomial probability
 *
 * @param value Observed value
 * @return      Probability density
 */
double Dist_multinomial::pdf( const RbPtr<RbLanguageObject> value ) {

	// Get the value and the parameters of the Dirichlet
    std::vector<double>       p = static_cast<const Simplex*      >( getMemberValue( "p" ).get() )->getValue();
    std::vector<unsigned int> x = static_cast<const VectorNatural*> (value.get()                 )->getValue();

	// check that the vectors are both the same size
	if ( p.size() != x.size() )
		throw RbException( "Inconsistent size of vectors when calculating Dirichlet log probability density" );

	return RbStatistics::Multinomial::pdf( p, x );
}


/** Quantile function */
RbPtr<Real> Dist_multinomial::quantile(const double p) {

    throw RbException( "Quantile function of multinomial not implemented yet" );
}


/**
 * This function generates an multinomial-distributed
 * random variable.
 *
 * @brief Random draw from multinomial distribution
 *
 * @return      Random draw from multinomial distribution
 */
RbPtr<RbLanguageObject> Dist_multinomial::rv( void ) {

    std::vector<double> p      = static_cast<const Simplex*>( getMemberValue( "p" ).get() )->getValue();
    RbPtr<RandomNumberGenerator> rng = GLOBAL_RNG;
	std::vector<int>       r( p.size() );

	r = RbStatistics::Multinomial::rv( p, rng );
    return RbPtr<RbLanguageObject>( new VectorNatural( r ) );
}

