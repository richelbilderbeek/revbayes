//
//  Func_TreeTrace.cpp
//  RevBayes_development_branch
//
//  Created by will freyman on 12/15/15.
//  Copyright (c) 2015 will freyman. All rights reserved.
//



#include "Func_treeTrace.h"
#include "ArgumentRule.h"
#include "ConstantNode.h"
#include "ModelVector.h"
#include "OptionRule.h"
#include "RbException.h"
#include "RlTree.h"
#include "RlTraceTree.h"
#include "TraceTree.h"
#include "TypedDagNode.h"

#include <map>
#include <set>
#include <sstream>


using namespace RevLanguage;


Func_treeTrace* Func_treeTrace::clone( void ) const
{
    
    return new Func_treeTrace( *this );
}


/** Execute function */
RevPtr<RevVariable> Func_treeTrace::execute( void )
{
    RevBayesCore::TraceTree t(false);
    
    if ( args[0].getVariable()->getRevObject().isType( ModelVector<TimeTree>::getClassTypeSpec() ) )
    {
        const ModelVector<RevLanguage::TimeTree>& trees = static_cast<const ModelVector<RevLanguage::TimeTree>&>( args[0].getVariable()->getRevObject() );
    
        t = RevBayesCore::TraceTree( true );

        for (size_t i = 0; i < trees.size(); ++i)
        {
            t.addObject( new RevBayesCore::Tree( trees[i] ) );
        }
    }
    else
    {
        const ModelVector<RevLanguage::BranchLengthTree>& trees = static_cast<const ModelVector<RevLanguage::BranchLengthTree>&>( args[0].getVariable()->getRevObject() );

        for (size_t i = 0; i < trees.size(); ++i)
        {
            t.addObject( new RevBayesCore::Tree( trees[i] ) );
        }
    }

    int burnin = static_cast<const Integer &>(args[1].getVariable()->getRevObject()).getValue();

    t.setBurnin(burnin);

    return new RevVariable( new TraceTree( t ) );
}


/** Get argument rules */
const ArgumentRules& Func_treeTrace::getArgumentRules( void ) const
{
    
    static ArgumentRules argumentRules = ArgumentRules();
    static bool rules_set = false;
    
    if (!rules_set)
    {
        std::vector<TypeSpec> treeTypes;
        treeTypes.push_back( ModelVector<TimeTree>::getClassTypeSpec() );
        treeTypes.push_back( ModelVector<BranchLengthTree>::getClassTypeSpec() );
        argumentRules.push_back( new ArgumentRule( "trees", treeTypes, "Vector of trees.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
        argumentRules.push_back( new ArgumentRule( "burnin"   , Integer::getClassTypeSpec()     , "The number of samples to discard as burnin.", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new Integer(-1) ) );
        
        rules_set = true;
    }
    
    return argumentRules;
}


/** Get Rev type of object */
const std::string& Func_treeTrace::getClassType(void)
{
    
    static std::string rev_type = "Func_treeTrace";
    
    return rev_type;
}


/** Get class type spec describing type of object */
const TypeSpec& Func_treeTrace::getClassTypeSpec(void)
{
    
    static TypeSpec rev_type_spec = TypeSpec( getClassType(), new TypeSpec( Function::getClassTypeSpec() ) );
    
    return rev_type_spec;
}


/**
 * Get the primary Rev name for this function.
 */
std::string Func_treeTrace::getFunctionName( void ) const
{
    // create a name variable that is the same for all instance of this class
    std::string f_name = "treeTrace";
    
    return f_name;
}


/** Get type spec */
const TypeSpec& Func_treeTrace::getTypeSpec( void ) const
{
    
    static TypeSpec type_spec = getClassTypeSpec();
    
    return type_spec;
}


/** Get return type */
const TypeSpec& Func_treeTrace::getReturnType( void ) const
{
    
    static TypeSpec returnTypeSpec = TraceTree::getClassTypeSpec();
    return returnTypeSpec;
}
