file(GLOB HEADERS "include/graphene/net/*.hpp")
file(GLOB OMNIBAZAAR_HEADERS "omnibazaar/*.hpp")

set(SOURCES node.cpp
            stcp_socket.cpp
            core_messages.cpp
            peer_database.cpp
            peer_connection.cpp
            message_oriented_connection.cpp

            omnibazaar/mail_object.cpp
            omnibazaar/mail_sender.cpp)

add_library( graphene_net ${SOURCES} ${HEADERS} ${OMNIBAZAAR_HEADERS} )

target_link_libraries( graphene_net 
  PUBLIC fc graphene_db )
target_include_directories( graphene_net 
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/omnibazaar"
  PRIVATE "${CMAKE_SOURCE_DIR}/libraries/chain/include"
)

if(MSVC)
  set_source_files_properties( node.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

if (USE_PCH)
  set_target_properties(graphene_net PROPERTIES COTIRE_ADD_UNITY_BUILD FALSE)
  cotire(graphene_net)
endif(USE_PCH)

install( TARGETS
   graphene_net

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
install( FILES ${HEADERS} DESTINATION "include/graphene/net" )
install( FILES ${OMNIBAZAAR_HEADERS} DESTINATION "omnibazaar" )
