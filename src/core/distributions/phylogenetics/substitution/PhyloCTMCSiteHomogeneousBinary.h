#ifndef PhyloCTMCSiteHomogeneousBinary_H
#define PhyloCTMCSiteHomogeneousBinary_H

#include "BinaryState.h"
#include "PhyloCTMCSiteHomogeneousConditional.h"

namespace RevBayesCore {

    struct BinaryAscertainmentBias {
        
      enum Coding { ALL                 = 0x00,
                    NOABSENCESITES      = 0x01,
                    NOPRESENCESITES     = 0x02,
                    VARIABLE            = 0x03,
                    NOSINGLETONPRESENCE = 0x04,
                    NOSINGLETONABSENCE  = 0x08,
                    NOSINGLETONS        = 0x0C,
                    INFORMATIVE         = 0x0F
                  };
    };

    class PhyloCTMCSiteHomogeneousBinary : public PhyloCTMCSiteHomogeneousConditional<BinaryState> {

        public:
            PhyloCTMCSiteHomogeneousBinary(const TypedDagNode< Tree > *t, bool c, size_t nSites, bool amb, BinaryAscertainmentBias::Coding cod = BinaryAscertainmentBias::ALL, bool internal = false, bool gapmatch = true);

            // public member functions
            PhyloCTMCSiteHomogeneousBinary*                     clone(void) const;

        protected:

            double                                              sumRootLikelihood( void );
            bool                                                isSitePatternCompatible(std::map<size_t, size_t>);
            bool                                                isSitePatternCompatible(std::map<RbBitSet, size_t>);
        };

}

#endif
