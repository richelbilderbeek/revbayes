/**
 * @file
 * This file contains the implementation of DistributionFunction, which
 * is used for functions related to a statistical distribution.
 *
 * @brief Implementation of DistributionFunction
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date$
 * @author The RevBayes core team
 * @license GPL version 3
 * @version 1.0
 * @since 2009-09-17, version 1.0
 *
 * $Id$
 */

#include "ArgumentRule.h"
#include "RbBoolean.h"
#include "ConstantNode.h"
#include "DAGNode.h"
#include "Distribution.h"
#include "DistributionContinuous.h"
#include "DistributionFunction.h"
#include "RbException.h"
#include "RbUtil.h"
#include "RealPos.h"
#include "TypeSpec.h"
#include "ValueRule.h"
#include "VectorString.h"

#include <sstream>


// Definition of the static type spec member
const TypeSpec DistributionFunction::typeSpec(DistributionFunction_name);

/** Constructor */
DistributionFunction::DistributionFunction( RbPtr<Distribution> dist, FuncType funcType ) : RbFunction(), returnType( funcType == DENSITY || funcType == PROB ? TypeSpec( funcType == DENSITY ? Real_name : RealPos_name ) : dist->getVariableType() ) {

    /* Ininitalize the argument rules */
    argumentRules = RbPtr<ArgumentRules>( new ArgumentRules() );
    
    /* Set the distribution */
    distribution = dist;

    /* Set the function type */
    functionType = funcType;

    /* Get the distribution parameter rules and set type to value argument */
    const RbPtr<ArgumentRules> memberRules = dist->getMemberRules();
    for ( ArgumentRules::const_iterator i = memberRules->begin(); i != memberRules->end(); i++ ) {
        // check if this rule has a default value
        if ((*i)->hasDefault()) {
            argumentRules->push_back( RbPtr<ArgumentRule>( new ValueRule( (*i)->getArgumentLabel(), (*i)->getArgumentTypeSpec(), RbPtr<RbLanguageObject>( (*i)->getDefaultVariable()->getValue()->clone() ) ) ) );
        } else {
            argumentRules->push_back( RbPtr<ArgumentRule>( new ValueRule( (*i)->getArgumentLabel(), (*i)->getArgumentTypeSpec() ) ) );
        }
    }

    /* Modify argument rules based on function type */
    if (functionType == DENSITY) {

        argumentRules->insert( argumentRules->begin(), RbPtr<ArgumentRule>( new ValueRule( "x"  , distribution->getVariableType() ) ) );
        argumentRules->push_back(                     RbPtr<ArgumentRule>( new ValueRule( "log", RbPtr<RbLanguageObject>( new RbBoolean(false) ) ) ) );
    }
    else if (functionType == RVALUE) {

        // Nothing to do
    }
    else if (functionType == PROB) {

        argumentRules->insert( argumentRules->begin(), RbPtr<ArgumentRule>( new ValueRule( "q"  , distribution->getVariableType() ) ) );
    }
    else if (functionType == QUANTILE) {

        argumentRules->insert( argumentRules->begin(), RbPtr<ArgumentRule>( new ValueRule( "p"  , RealPos_name                    ) ) );
    }
}


/** Copy constructor */
DistributionFunction::DistributionFunction( const DistributionFunction& x ) : RbFunction(x), returnType( x.returnType ) {

    argumentRules = x.argumentRules;
    distribution  = RbPtr<Distribution>( x.distribution->clone() );
    functionType  = x.functionType;

    /* Modify argument rules based on function type */
    if (functionType == DENSITY) {
        
        argumentRules->insert( argumentRules->begin(), RbPtr<ArgumentRule>( new ValueRule( "x"  , distribution->getVariableType() ) ) );
        argumentRules->push_back(                     RbPtr<ArgumentRule>( new ValueRule( "log", RbPtr<RbLanguageObject>( new RbBoolean(false) ) ) ) );
    }
    else if (functionType == RVALUE) {
        
        // Nothing to do
    }
    else if (functionType == PROB) {
        
        argumentRules->insert( argumentRules->begin(), RbPtr<ArgumentRule>( new ValueRule( "q"  , distribution->getVariableType() ) ) );
    }
    else if (functionType == QUANTILE) {
        
        argumentRules->insert( argumentRules->begin(), RbPtr<ArgumentRule>( new ValueRule( "p"  , RealPos_name                    ) ) );
    }
}


/** Destructor */
DistributionFunction::~DistributionFunction(void) {

}


/** Assignment operator */
DistributionFunction& DistributionFunction::operator=( const DistributionFunction& x ) {

    if (this != &x) {
        
        if ( returnType != x.returnType )
            throw RbException( "Invalid assignment involving distributions on different types of random variables" );
        
        

        argumentRules = x.argumentRules;
        distribution  = RbPtr<Distribution>( x.distribution->clone() );
        functionType  = x.functionType;

        /* Modify argument rules based on function type */
        if (functionType == DENSITY) {
            
            argumentRules->insert( argumentRules->begin(), RbPtr<ArgumentRule>( new ValueRule( "x"  , distribution->getVariableType() ) ) );
            argumentRules->push_back(                     RbPtr<ArgumentRule>( new ValueRule( "log", RbPtr<RbLanguageObject>( new RbBoolean(false) ) ) ) );
        }
        else if (functionType == RVALUE) {
            
            // Nothing to do
        }
        else if (functionType == PROB) {
            
            argumentRules->insert( argumentRules->begin(), RbPtr<ArgumentRule>( new ValueRule( "q"  , distribution->getVariableType() ) ) );
        }
        else if (functionType == QUANTILE) {
            
            argumentRules->insert( argumentRules->begin(), RbPtr<ArgumentRule>( new ValueRule( "p"  , RealPos_name                    ) ) );
        }
    }

    return (*this);
}


/** Clone object */
DistributionFunction* DistributionFunction::clone( void ) const {

    return new DistributionFunction(*this);
}


/** Execute operation: switch based on type */
RbPtr<RbLanguageObject> DistributionFunction::execute( void ) {

    if ( functionType == DENSITY ) {

        if ( static_cast<const RbBoolean*>( args["log"]->getValue().get() )->getValue() == false )
            return RbPtr<RbLanguageObject>( new RealPos( distribution->pdf  ( args[0]->getValue() ) ) );
        else
            return RbPtr<RbLanguageObject>( new Real   ( distribution->lnPdf( args[0]->getValue() ) ) );
    }
    else if (functionType == RVALUE) {

        RbPtr<RbLanguageObject> draw = distribution->rv();
        
        return draw;
    }
    else if (functionType == PROB) {

        return RbPtr<RbLanguageObject>( new RealPos( static_cast<DistributionContinuous*>( distribution.get() )->cdf( args[0]->getValue() ) ) );
    }
    else if (functionType == QUANTILE) {

        double    prob  = static_cast<const RealPos*>( args[0]->getValue().get() )->getValue();
        RbPtr<RbLanguageObject> quant( static_cast<DistributionContinuous*>( distribution.get() )->quantile( prob ).get() );
        
        return quant;
    }

    throw RbException( "Unrecognized distribution function" );
}


/** Get argument rules */
const RbPtr<ArgumentRules> DistributionFunction::getArgumentRules(void) const {

    return argumentRules;
}


/** Get class vector describing type of object */
const VectorString& DistributionFunction::getClass(void) const { 

    static VectorString rbClass = VectorString(DistributionFunction_name) + RbFunction::getClass();
	return rbClass; 
}


/** Get return type */
const TypeSpec& DistributionFunction::getReturnType(void) const {

    return returnType;
}


/** Get the type spec of this class. We return a static class variable because all instances will be exactly from this type. */
const TypeSpec& DistributionFunction::getTypeSpec(void) const {
    return typeSpec;
}

/** Process arguments */
bool DistributionFunction::processArguments( const std::vector<RbPtr<Argument> >& args, RbPtr<VectorInteger> matchScore ) {

    if ( !RbFunction::processArguments( args, matchScore ) )
        return false;

    /* Set member variables of the distribution */
    size_t i = 0;
    if ( functionType != RVALUE )
        i++;    // Add one because first argument is not a distribution parameter
    size_t k = argumentRules->size();
    if ( functionType == DENSITY )
        k--;    // Subtract one because last argumet is log

    for ( ; i < k; i++ ) {

        std::string name = (*argumentRules)[i]->getArgumentLabel();

        /* All distribution variables are references but we have value arguments here
           so a const cast is needed to deal with the mismatch */
        RbPtr<VariableSlot> arg = this->args[i];
        
        distribution->setMemberVariable( name, arg->getVariable() );
    }

    return true;
}

