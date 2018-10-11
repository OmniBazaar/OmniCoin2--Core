#include <graphene/chain/protocol/base.hpp>
#include <../omnibazaar/omnibazaar_fee_type.hpp>

namespace graphene { namespace chain {

        omnibazaar::omnibazaar_fee_type base_operation::calculate_omnibazaar_fee(const database& db)const
        {
            return omnibazaar::omnibazaar_fee_type();
        }
}}
