/**
 * @file
 * This file contains the declaration of Func_normalize,
 * which is used to normalize vectors.
 *
 * @brief Declaration of Func_normalize
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date: 2012-06-12 10:25:58 +0200 (Tue, 12 Jun 2012) $
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 *
 * $Id: Func_normalize.h 1626 2012-06-12 08:25:58Z hoehna $
 */

#ifndef Func_normalize_H
#define Func_normalize_H

#include "RlFunction.h"
#include <map>
#include <string>

namespace RevLanguage {
    
    class Func_normalize :  public Function {
        
    public:
        Func_normalize();
        
        // Basic utility functions
        Func_normalize*             clone(void) const;                                          //!< Clone the object
        static const std::string&   getClassName(void);                                         //!< Get class name
        static const TypeSpec&      getClassTypeSpec(void);                                     //!< Get class type spec
        const TypeSpec&             getTypeSpec(void) const;                                    //!< Get language type of the object
        
        // Regular functions
        const ArgumentRules&        getArgumentRules(void) const;                               //!< Get argument rules
        const TypeSpec&             getReturnType(void) const;                                  //!< Get type of return value
        
        
        RevObject*                  execute(void);                                              //!< Execute function
        
    };
    
}


#endif


