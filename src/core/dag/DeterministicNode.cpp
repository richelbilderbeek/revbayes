/**
 * @file
 * This file contains the implementation of some function in DeterministicNode,
 * which is an abstract base class for DAG nodes associated with expressions
 * (equations) determining their value.
 *
 * @brief Partial implementation of DeterministicNode
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date: 2009-12-30 00:19:25 +0100 (Ons, 30 Dec 2009) $
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 * @since 2009-08-16, version 1.0
 * @extends DAGNode
 *
 * $Id: DeterministicNode.cpp 216 2009-12-29 23:19:25Z ronquist $
 */

#include "ArgumentRule.h"
#include "RbBoolean.h"
#include "DAGNode.h"
#include "DeterministicNode.h"
#include "RbException.h"
#include "RbFunction.h"
#include "RbMemoryManager.h"
#include "RbUtil.h"
#include "UserInterface.h"
#include "VectorString.h"
#include "Workspace.h"

#include <algorithm>
#include <cassert>


// Definition of the static type spec member
const TypeSpec DeterministicNode::typeSpec(DeterministicNode_name);

/** Constructor of empty deterministic node */
DeterministicNode::DeterministicNode( const std::string& valType ) : VariableNode( valType ), needsUpdate( true ) {
}

/** Constructor of empty deterministic node */
DeterministicNode::DeterministicNode( RbFunction* func ) : VariableNode(func->getReturnType()), needsUpdate( true ), function( func ) {
    
    // increment the reference count for myself
    RbDagNodePtr::incrementCountForAddress(this);
    
    /* Check for cycles */
    Environment& arguments = func->getArguments();
    std::list<DAGNode*> done;
    for ( size_t i = 0; i < arguments.size(); i++ ) {
        const std::string &name = arguments.getName(i);
        if ( arguments[name].getDagNode() != NULL && arguments[name].getDagNode()->isParentInDAG( RbDagNodePtr(this), done ) )
            throw RbException( "Invalid assignment: cycles in the DAG" );
    }
    
    /* Set parents and add this node as a child node of these */
    for ( size_t i = 0; i < arguments.size(); i++ ) {
        const std::string &name = arguments.getName(i);
        const RbDagNodePtr& theArgument = arguments[name].getDagNode();
        addParentNode( theArgument );
        theArgument->addChildNode( this );
    }
    
    /* Set value and stored value */
    RbLanguageObject* retVal = function->execute();
    
    value           = retVal;
    storedValue     = NULL;
    
    needsUpdate     = false;
    
    // decrement the reference count for myself
    RbDagNodePtr::decrementCountForAddress(this);
}


/** Copy constructor */
DeterministicNode::DeterministicNode( const DeterministicNode& x ) : VariableNode( x ) {
    
    function        = x.function->clone();
    touched         = x.touched;
    needsUpdate     = x.needsUpdate;
    // We do not own the stored value, but the function does
    storedValue     = x.storedValue;
    
    /* Set parents and add this node as a child node of these */
    Environment& args = function->getArguments();
    for ( size_t i = 0; i < args.size(); i++ ) {
        const std::string &name = args.getName(i);
        
        RbDagNodePtr theArgument = args[name].getDagNode();
        addParentNode( theArgument );
        theArgument->addChildNode( this );
    }
}


/** Destructor */
DeterministicNode::~DeterministicNode( void ) {

    delete function;
}

DeterministicNode* DeterministicNode::clone() const {
    return new DeterministicNode(*this);
}


/** Clone the entire graph: clone children, swap parents */
RbDagNodePtr DeterministicNode::cloneDAG( std::map<const DAGNode*, RbDagNodePtr >& newNodes ) const {
    
    if ( newNodes.find( this ) != newNodes.end() )
        return newNodes[ this ];
    
    /* Get pristine copy */
    DeterministicNode* copy = new DeterministicNode( getValueType() );
    newNodes[ this ] = copy;
    
    /* Set the name so that the new node remains identifiable */
    copy->setName(name);
    
    /* Set the copy member variables */
    copy->function      = function->clone();
    copy->touched       = touched;
    copy->needsUpdate   = needsUpdate;
    // We do not own the value so we do not need to clone it
    copy->value         = value;
    copy->storedValue   = storedValue;
    
    /* Set the copy arguments to their matches in the new DAG */
    Environment& args      = function->getArguments();
    Environment& copyArgs  = ( copy->function->getArguments() );
    
    for ( size_t i = 0; i < args.size(); i++ ) {
        const std::string &name = args.getName(i);
        
        // clone the parameter DAG node
        RbDagNodePtr theArgClone( args[name].getDagNode()->cloneDAG(newNodes) );
        copyArgs[name].setVariable(new Variable(theArgClone) );
  
        // this is perhaps not necessary because we already set the parent child relationship automatically
        copy->addParentNode( theArgClone );
        theArgClone->addChildNode( copy );
    }
    
    /* Make sure the children clone themselves */
    for( std::set<VariableNode*>::const_iterator i = children.begin(); i != children.end(); i++ ) {
        (*i)->cloneDAG( newNodes );
    }
    
    return copy;
}


/** Complete info about object */
std::string DeterministicNode::debugInfo( void ) const {
    
    std::ostringstream o;
    
    o << "Deterministic Node:" << std::endl;
    
    o << "touched     = " << (touched ? "true" : "false") << std::endl;
    o << "needsUpdate = " << (needsUpdate ? "true" : "false") << std::endl;
    
    o << "function    = ";
    function->printValue(o);
    o << std::endl;
    
    o << "value       = ";
    value->printValue(o);
    o << std::endl;
    
    o << "storedValue = ";
    storedValue->printValue(o);
    o << std::endl;
    
    return o.str();
}



/** Get affected nodes: pass through to next stochastic node */
void DeterministicNode::getAffected( std::set<StochasticNode* >& affected ) {

    for ( std::set<VariableNode*>::iterator i = children.begin(); i != children.end(); i++ ) {
        (*i)->getAffected( affected );
    }
    
}


/** Get class vector describing type of DAG node */
const VectorString& DeterministicNode::getClass() const {

    static VectorString rbClass = VectorString( DeterministicNode_name ) + VariableNode::getClass();
    return rbClass;
}


/** Get the type spec of this class. We return a static class variable because all instances will be exactly from this type. */
const TypeSpec& DeterministicNode::getTypeSpec(void) const {
    return typeSpec;
}



const RbFunction& DeterministicNode::getFunction(void) const {
    return *function;
}



/** Get stored value */
const RbLanguageObject& DeterministicNode::getStoredValue( void ) const {

    if ( !touched )
        return *value;
    
    return *storedValue;
}


/** Get value */
const RbLanguageObject& DeterministicNode::getValue( void ) const {
    
    if ( touched && needsUpdate )
        const_cast<DeterministicNode*>(this)->update();
    
    return *value;
}


/** Get value */
RbLanguageObject& DeterministicNode::getValue( void ) {
    
    if ( touched && needsUpdate )
        update();
    
    return *value;
}


///** Get value pointer */
//const RbLanguageObject* DeterministicNode::getValuePtr( void ) const {
//    
//    if ( touched && needsUpdate )
//        const_cast<DeterministicNode*>(this)->update();
//    
//    return value;
//}


/** Keep value of node and affected variable nodes */
void DeterministicNode::keepMe( void ) {
    
    if ( touched ) {
        
        storedValue = NULL;
        
        if ( needsUpdate )
            update();
        
    }
    touched = false;
    
    keepAffected();
    
}


/** Print value for user */
void DeterministicNode::printValue( std::ostream& o ) const {

    if ( touched && needsUpdate )
        const_cast<DeterministicNode*>(this)->update();

    if (value != NULL) 
        value->printValue(o);
}

/** Print struct for user */
void DeterministicNode::printStruct( std::ostream& o ) const {
    
    o << "_DAGClass    = " << getClass() << std::endl;
    o << "_value       = ";
    value->printValue(o);
    o << std::endl;
    if ( touched && !needsUpdate ) {
        o << "_storedValue = ";
        storedValue->printValue(o);
        o << std::endl;
    }
    
    o << "_valueType   = " << getValueType() << std::endl;
    o << "_function    = " << function->getType() << std::endl;
    o << "_touched     = " << ( touched ? RbBoolean( true ) : RbBoolean( false ) ) << std::endl;
    o << "_needsUpdate = " << ( needsUpdate ? RbBoolean( true ) : RbBoolean( false ) ) << std::endl;
    
    o << "_parents     = ";
    printParents(o);
    o << std::endl;
    
    o << "_children    = ";
    printChildren(o);
    o << std::endl;
    
    o << std::endl;
}


/** 
 * Restore value of node
 * We also need to restore the affected variable nodes */
void DeterministicNode::restoreMe( void ) {

    if ( touched ) {
        if (storedValue != NULL) {
//            delete value;
        
            // no matter if this node has been changed we just set it back to its stored value
            value       = storedValue;
        
            storedValue = NULL;
        }
        
        // restore all children
        for ( std::set<VariableNode*>::iterator i = children.begin(); i != children.end(); i++ ) {
            (*i)->restoreMe();
        }
    }
    touched = false;
}


/** Swap parent node */
void DeterministicNode::swapParentNode( const RbDagNodePtr& oldParent, const RbDagNodePtr& newParent ) {

    if ( parents.find( oldParent ) == parents.end() )
        throw RbException( "Node is not a parent" );

    oldParent->removeChildNode( this );
    newParent->addChildNode   ( this );
    removeParentNode( oldParent );
    addParentNode( newParent );

    touch();
}


/** Tell affected nodes that upstream value has been reset */
void DeterministicNode::touchMe( void ) {
    
    // only if this node is not touched we start touching all affected nodes
    // this pervents infinite recursion in statement like "a <- a + b"
    if (!touched) {
        // flag myself as being touched
        touched     = true;
        
//        // store the current value; this should happen only by the first touch unless we change the stored values into a stack
        storedValue = value;
        value = NULL;
    }
        
    // flag myself for an update
    needsUpdate = true;
    
    // touch all my children because they are affected too
    for ( std::set<VariableNode*>::iterator i = children.begin(); i != children.end(); i++ )
        (*i)->touchMe();
//    }
    
}

/** Update value and stored value after node and its surroundings have been touched by a move */
void DeterministicNode::update( void ) {
    
    if ( touched && needsUpdate ) {
        
//        assert( storedValue == NULL );
        
//        // set the stored value and release the old stored value
//        storedValue     = value;
        
        // compute a new value
        value = function->execute();
        
        // mark as changed
        needsUpdate = false;
        
    }
}


