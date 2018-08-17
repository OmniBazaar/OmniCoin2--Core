if( ENABLE_ELASTIC_PLUGIN )
    set(APP_NAME witness_node_elastic)
    set(ELASTIC_NAME graphene_elasticsearch)
else()
    set(APP_NAME witness_node)
    set(ELASTIC_NAME "")
endif()

add_executable( ${APP_NAME} main.cpp )
if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling witness_node with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()

# We have to link against graphene_debug_witness because deficiency in our API infrastructure doesn't allow plugins to be fully abstracted #246
target_link_libraries( ${APP_NAME}

PRIVATE graphene_app graphene_delayed_node graphene_account_history ${ELASTIC_NAME} graphene_market_history graphene_witness graphene_chain graphene_debug_witness graphene_egenesis_full graphene_snapshot fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )


install( TARGETS
   ${APP_NAME}

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
