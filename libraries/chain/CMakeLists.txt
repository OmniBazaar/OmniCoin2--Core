
add_custom_target( build_hardfork_hpp
                   COMMAND cat-parts "${CMAKE_CURRENT_SOURCE_DIR}/hardfork.d" "${CMAKE_CURRENT_BINARY_DIR}/include/graphene/chain/hardfork.hpp" )
set_source_files_properties( "${CMAKE_CURRENT_BINARY_DIR}/include/graphene/chain/hardfork.hpp" PROPERTIES GENERATED TRUE )

add_dependencies( build_hardfork_hpp cat-parts )

file(GLOB HEADERS "include/graphene/chain/*.hpp")
file(GLOB PROTOCOL_HEADERS "include/graphene/chain/protocol/*.hpp")
file(GLOB OMNIBAZAAR_HEADERS "omnibazaar/*.hpp")

if( GRAPHENE_DISABLE_UNITY_BUILD )
   set( GRAPHENE_DB_FILES
        db_balance.cpp
        db_block.cpp
        db_debug.cpp
        db_getter.cpp
        db_init.cpp
        db_maint.cpp
        db_management.cpp
        db_market.cpp
        db_update.cpp
        db_witness_schedule.cpp
        db_bonus.cpp
        db_escrow.cpp
      )
   message( STATUS "Graphene database unity build disabled" )
else( GRAPHENE_DISABLE_UNITY_BUILD )
   set( GRAPHENE_DB_FILES
        database.cpp )
   message( STATUS "Graphene database unity build enabled" )
endif( GRAPHENE_DISABLE_UNITY_BUILD )

## SORT .cpp by most likely to change / break compile
add_library( graphene_chain

             # As database takes the longest to compile, start it first
             ${GRAPHENE_DB_FILES}
             fork_database.cpp

             omnibazaar/welcome_bonus.cpp
             omnibazaar/welcome_bonus_evaluator.cpp
             omnibazaar/referral_bonus.cpp
             omnibazaar/referral_bonus_evaluator.cpp
             omnibazaar/sale_bonus.cpp
             omnibazaar/sale_bonus_evaluator.cpp
             omnibazaar/founder_bonus.cpp
             omnibazaar/founder_bonus_evaluator.cpp
             omnibazaar/witness_bonus.cpp
             omnibazaar/witness_bonus_evaluator.cpp
             omnibazaar/multisig_transfer.cpp
             omnibazaar/multisig_transfer_evaluator.cpp

             omnibazaar/referrer_component.cpp
             omnibazaar/publisher_component.cpp

             omnibazaar/escrow_object.cpp
             omnibazaar/escrow.cpp
             omnibazaar/escrow_evaluator.cpp

             omnibazaar/listing_object.cpp
             omnibazaar/listing.cpp
             omnibazaar/listing_evaluator.cpp

             omnibazaar/omnibazaar_fee_type.cpp

             omnibazaar/verification.cpp
             omnibazaar/verification_evaluator.cpp

             omnibazaar/exchange_object.cpp
             omnibazaar/exchange.cpp
             omnibazaar/exchange_evaluator.cpp

             omnibazaar/pop_weights.cpp

             omnibazaar/reserved_names_object.cpp
             omnibazaar/reserved_names.cpp
             omnibazaar/reserved_names_evaluator.cpp

             protocol/types.cpp
             protocol/address.cpp
             protocol/authority.cpp
             protocol/asset.cpp
             protocol/assert.cpp
             protocol/account.cpp
             protocol/transfer.cpp
             protocol/committee_member.cpp
             protocol/witness.cpp
             protocol/market.cpp
             protocol/proposal.cpp
             protocol/withdraw_permission.cpp
             protocol/asset_ops.cpp
             protocol/memo.cpp
             protocol/worker.cpp
             protocol/custom.cpp
             protocol/operations.cpp
             protocol/transaction.cpp
             protocol/block.cpp
             protocol/fee_schedule.cpp
             protocol/confidential.cpp
             protocol/vote.cpp
             protocol/base.cpp

             genesis_state.cpp
             get_config.cpp

             pts_address.cpp

             evaluator.cpp
             balance_evaluator.cpp
             account_evaluator.cpp
             assert_evaluator.cpp
             witness_evaluator.cpp
             committee_member_evaluator.cpp
             asset_evaluator.cpp
             transfer_evaluator.cpp
             proposal_evaluator.cpp
             market_evaluator.cpp
             vesting_balance_evaluator.cpp
             withdraw_permission_evaluator.cpp
             worker_evaluator.cpp
             confidential_evaluator.cpp
             special_authority.cpp
             buyback.cpp

             account_object.cpp
             asset_object.cpp
             fba_object.cpp
             proposal_object.cpp
             vesting_balance_object.cpp

             block_database.cpp

             is_authorized_asset.cpp

             ${HEADERS}
             ${PROTOCOL_HEADERS}
             ${OMNIBAZAAR_HEADERS}
             "${CMAKE_CURRENT_BINARY_DIR}/include/graphene/chain/hardfork.hpp"
           )

add_dependencies( graphene_chain build_hardfork_hpp )
target_link_libraries( graphene_chain fc graphene_db graphene_app )
target_include_directories( graphene_chain
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include"
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/omnibazaar" )

if(MSVC)
  set_source_files_properties( db_init.cpp db_block.cpp database.cpp block_database.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

INSTALL( TARGETS
   graphene_chain

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/graphene/chain" )
INSTALL( FILES ${PROTOCOL_HEADERS} DESTINATION "include/graphene/chain/protocol" )
INSTALL( FILES ${OMNIBAZAAR_HEADERS} DESTINATION "omnibazaar" )
