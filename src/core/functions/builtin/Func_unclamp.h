/**
 * @file
 * This file contains the declaration of Func_unclamp, which
 * is used to unclamp a stochastic node.
 *
 * @brief Declaration of Func_unclamp
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date$
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 *
 * $Id$
 */

#ifndef Func_unclamp_H
#define Func_unclamp_H

#include "RbFunction.h"

#include <map>
#include <string>
#include <vector>

class DAGNode;
class VectorString;

const std::string Func_unclamp_name = "Unclamp function";

class Func_unclamp :  public RbFunction {

    public:
        // Basic utility functions
        Func_unclamp*               clone(void) const;                                  //!< Clone the object
        const VectorString&         getClass(void) const;                               //!< Get class vector
        const TypeSpec&             getTypeSpec(void) const;                            //!< Get language type of the object

        // Regular functions
        bool                        addAsChildOfArguments(void) { return false; }       //!< We do not wish that this function is added as a child of the arguments
        RbPtr<RbLanguageObject>     execute(void);                                      //!< Execute operation
        const RbPtr<ArgumentRules>  getArgumentRules(void) const;                       //!< Get argument rules
        const TypeSpec&             getReturnType(void) const;                          //!< Get type of return value
    
    private:
        static const TypeSpec       typeSpec;
        static const TypeSpec       returnTypeSpec;
};

#endif

