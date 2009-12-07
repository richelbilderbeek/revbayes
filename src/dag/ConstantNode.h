/**
 * @file
 * This file contains the declaration of ConstantNode, which is derived
 * from DAGNode. ConstantNode is used for DAG nodes holding constant
 * values.
 *
 * @brief Declaration of ConstantNode
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date: 2009-11-19 17:29:33 +0100 (Tor, 19 Nov 2009) $
 * @author The RevBayes development core team
 * @license GPL version 3
 * @version 1.0
 * @since 2009-08-16, version 1.0
 * @extends DAGNode
 *
 * $Id: ConstantNode.h 69 2009-11-19 16:29:33Z ronquist $
 */

#ifndef ConstantNode_H
#define ConstantNode_H

#include "RbObject.h"
#include "DAGNode.h"
#include "StringVector.h"

class ConstantNode : public DAGNode {

    public:
	     ConstantNode(RbObject* val);            //!< Constructor from value
	     ConstantNode(const ConstantNode& c);    //!< Copy constructor

	     RbObject*               clone(void) const;                      //!< Clone this object
	     bool                    equals(const RbObject* obj) const;  //!< Object comparison
	     const StringVector&    getClass(void) const;                   //!< Get class vector
	     void                   printValue(std::ostream& o) const;                    //!< Print value (for user)
         std::string            toString(void) const;                                 //!< General info on object


        // overloaded functions from DAGNode
        double                  getLnProbabilityRatio(void);
        double                  getLnProbability(void);

        // overloaded operators
        RbObject&               operator=(const RbObject& o);
        DAGNode&            	operator=(const DAGNode& o);
        ConstantNode&           operator=(const ConstantNode& o);
        
        void    	            keepAffectedChildren(void);                         //!< Keep value of affected nodes recursively
        void	                keepAffectedParents(void);                         //!< Keep value of affected nodes recursively
        void         	  		restoreAffectedChildren(void);                      //!< Restore affected nodes recursively
        void         	  		restoreAffectedParents(void);                      //!< Restore affected nodes recursively
        void    	       		touchAffectedChildren(void);                        //!< Mark affected nodes recursively
        void	           		touchAffectedParents(void);                        //!< Mark affected nodes recursively

    protected:
    	
};

#endif

