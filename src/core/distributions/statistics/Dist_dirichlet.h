/**
 * @file
 * This file contains the declaration of Dist_dirichlet, which is used to hold
 * parameters and functions related to a Dirichlet distribution.
 *
 * @brief Declaration of Dist_dirichlet
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

#ifndef Dist_dirichlet_H
#define Dist_dirichlet_H

#include "DistributionContinuous.h"
#include "Simplex.h"

#include <ostream>
#include <string>

class DAGNode;
class Real;
class Simplex;
class StochasticNode;
class VectorString;

const std::string Dist_dirichlet_name = "Dist_dirichlet";

class Dist_dirichlet: public DistributionContinuous {

    public:
                                    Dist_dirichlet(void);                                               //!< Parser constructor

        // Basic utility functions
        Dist_dirichlet*             clone(void) const;                                                  //!< Clone object
        const VectorString&         getClass(void) const;                                               //!< Get class vector
        const TypeSpec&             getTypeSpec(void) const;                                            //!< Get language type of the object

        // Member variable setup
        RbPtr<const MemberRules>    getMemberRules(void) const;                                         //!< Get member variable rules

        // Real-valued distribution functions
        double                      cdf(RbPtr<const RbLanguageObject> value);                           //!< Cumulative density
        const TypeSpec&             getVariableType(void) const;                                        //!< Get random variable type (Simplex)
        double                      lnPdf(const RbLanguageObject *value) const;                         //!< Ln probability density
        double                      pdf(const RbLanguageObject *value) const;                           //!< Probability density
        RbPtr<Real>                 quantile(const double p);                                           //!< Quantile
        RbPtr<RbLanguageObject>     rv(void);                                                           //!< Generate random variable
    
    private:
        static const TypeSpec       typeSpec;
        static const TypeSpec       varTypeSpec;
};

#endif

