/**
 * @file
 * This file contains the implementation of Func_citation, which is
 * the function used to provide basic help.
 *
 * @brief Implementation of Func_citation
 *
 * @author Fredrik Ronquist
 * @license GPL version 3
 */

#include "Argument.h"
#include "ArgumentRule.h"
#include "Func_citation.h"
#include "RbException.h"
#include "RlBoolean.h"
#include "RlUtils.h"
#include "TypeSpec.h"
#include "RlUserInterface.h"
#include "Workspace.h"

#include <fstream>

using namespace RevLanguage;

/** Default constructor */
Func_citation::Func_citation( void ) : Function() {
    
}


/** Clone object */
Func_citation* Func_citation::clone( void ) const {
    
    return new Func_citation( *this );
}


/** Execute function */
RbLanguageObject* Func_citation::execute( void ) {
    
    
    RBOUT( "\n" );
    RBOUT( "Info on how to cite the program coming soon..." );
    RBOUT( "\n" );
    
    return NULL;
}


/** Get argument rules */
const ArgumentRules& Func_citation::getArgumentRules( void ) const {
    
    static ArgumentRules argumentRules = ArgumentRules();
    static bool rulesSet = false;
    
    if ( !rulesSet ) {
        
        argumentRules.push_back( new ArgumentRule( "all", true, RlBoolean::getClassTypeSpec(), new RlBoolean(false) ) );
        rulesSet = true;
    }
    
    return argumentRules;
}


/** Get class name of object */
const std::string& Func_citation::getClassName(void) {
    
    static std::string rbClassName = "Func_citation";
    
	return rbClassName;
}

/** Get class type spec describing type of object */
const TypeSpec& Func_citation::getClassTypeSpec(void) {
    
    static TypeSpec rbClass = TypeSpec( getClassName(), new TypeSpec( Function::getClassTypeSpec() ) );
    
	return rbClass;
}

/** Get type spec */
const TypeSpec& Func_citation::getTypeSpec( void ) const {
    
    static TypeSpec typeSpec = getClassTypeSpec();
    
    return typeSpec;
}


/** Get return type */
const TypeSpec& Func_citation::getReturnType( void ) const {
    
    static TypeSpec returnTypeSpec = RlUtils::Void;
    
    return returnTypeSpec;
}

