#ifndef RlBootstrapAnalysis_H
#define RlBootstrapAnalysis_H

#include "BootstrapAnalysis.h"
#include "TypedDagNode.h"
#include "WorkspaceToCoreWrapperObject.h"

#include <ostream>
#include <string>

namespace RevLanguage {
    
    
    /**
     * RevLanguage wrapper class for the posterior predictive analysis object.
     *
     *
     * The wraper class provides the Rev interface to the core class BootstrapAnalysis.
     * See BootstrapAnalysis.h for more details.
     *
     *
     * @copyright Copyright 2009-
     * @author The RevBayes Development Core Team (Sebastian Hoehna)
     *
     */
    class BootstrapAnalysis : public WorkspaceToCoreWrapperObject<RevBayesCore::BootstrapAnalysis> {
        
    public:
        
        BootstrapAnalysis(void);                                                                                                               //!< Default constructor
        
        // Basic utility functions
        virtual BootstrapAnalysis*                  clone(void) const;                                                                      //!< Clone object
        void                                        constructInternalObject(void);                                                          //!< We construct the a new internal PowerPosterior object.
        std::string                                 getConstructorFunctionName(void) const;                                                 //!< Get the name used for the constructor function in Rev.
        static const std::string&                   getClassType(void);                                                                     //!< Get Rev type
        static const TypeSpec&                      getClassTypeSpec(void);                                                                 //!< Get class type spec
        const MemberRules&                          getParameterRules(void) const;                                                          //!< Get member rules (const)
        virtual const TypeSpec&                     getTypeSpec(void) const;                                                                //!< Get language type of the object
        
        // Member method inits
        virtual RevPtr<RevVariable>                 executeMethod(const std::string& name, const std::vector<Argument>& args, bool &f);     //!< Override to map member methods to internal functions
        
    protected:
        
        virtual void                                printValue(std::ostream& o) const;                                                      //!< Print value (for user)
        void                                        setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var);          //!< Set member variable
        
        RevPtr<const RevVariable>                   simulations;
        RevPtr<const RevVariable>                   sampler;
        
    };
    
}

#endif
