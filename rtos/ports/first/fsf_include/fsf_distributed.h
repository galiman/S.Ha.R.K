
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/**
   @file Distributed.
 */

//fsf_distributed.h
//=====================================================================
//       FFFFFFIII   RRRRR      SSTTTTTTT
//      FF         IIR   RR    SS
//     FF           IR        SS
//    FFFFFF         RRRR    SSSSST
//   FF       FI       RRR  SS
//  FF         II     RRR  SS
// FF           IIIIIR    RS
//
// FSF(FIRST Scheduling Framework)
// distributed services functionality
//=====================================================================

#include "fsf_core.h"

#ifndef _FSF_DISTRIBUTED_H_
#define _FSF_DISTRIBUTED_H_

#define FSF_DISTRIBUTED_MODULE_SUPPORTED       1


/**

   \ingroup distjmodule

   This operation sets the network id attribute in the contract
   pointed to by parameter contract, to the value specified by
   network_id.  When a contract is negotiated, if network_id is
   FSF_NULL_NETWORK_ID (which is the default value) the contract is
   negotiated for the processing capacity of the node making the
   call. If network_id represents a valid network identifier, the
   negotiation is carried out for the specified network.
   
   @param contract          the pointer to the contract object
   @param [in] network_id   the network identifier
   
   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if contract is null or the network
     id is not valid.
*/
int
fsf_set_contract_network_id
  (fsf_contract_parameters_t *contract,
   fsf_network_id_t           network_id);

/**

   \ingroup distjmodule
  
   This operation puts the network identification
   corresponding to the contract parameters object
   pointed to by contract in the variable pointed
   to by network_id. If the contract is not a network
   one and therefore has not a network_id set, it
   puts the FSF_NULL_NETWORK_ID constant instead.

   @param [in] contract     the pointer to the contract object
   @param [out] network_id  pointer to the variable that will contain
     the network identifier
   
   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if any of the pointers is null.
*/
int
fsf_get_contract_network_id
  (const fsf_contract_parameters_t *contract,
   fsf_network_id_t                *network_id);


//Transmission services:

/**

   \ingroup distjmodule

   This operation creates a unidirectional input
   data endpoint through which, after the
   corresponding binding, it is possible to send
   data.  network_id identifies the network to use,
   receiver specifies the communication protocol
   dependent information that is necessary to
   address the receiving node, and port specifies
   the communication protocol dependent information
   that is necessary to get in contact with the
   desired destination.  

   @param [in] network_id the identifier ot the network to use
   @param [in] receiver   protocol dependent information used
     to address the receiving node
   @param [in] port       protocol dependent information used
     to get in contact with the desired destination process
   @param [out] endpoint  pointer to the variable that will
     contain the send endpoint
   
   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if endpoint is null, netwok_id
     is not valid or if the receiver or the port do not conform
     to their expected formats
*/
int
fsf_create_send_endpoint
  (fsf_network_id_t      network_id,
   fsf_node_address_t    receiver,
   fsf_port_t            port,
   fsf_send_endpoint_t  *endpoint);

/**
   \ingroup distjmodule

   This operation eliminates any resources reserved
   for the referenced endpoint. If the endpoint is
   bound to a network server, it is unbound from it
   and can not be further used to invoke send
   operations on it.  

   @param endpoint        pointer to the endpoint to eliminate

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if endpoint is null, or not valid
*/
int
fsf_destroy_send_endpoint
  (fsf_send_endpoint_t  *endpoint);

/**
   \ingroup distjmodule

   This operation returns (except for those NULL
   arguments) in the variables pointed to by
   network_id, receiver, or port, the corresponding
   parameters used in the creation of the given
   send endpoint.
   
   @param [in] endpoint     pointer to the endpoint
   @param [out] network_id  pointer to variable the will contain
     the network id
   @param [out] receiver    pointer to variable the will contain
     the protocol dependent information used to address the
     receiving node 
   @param [out] port        pointer to variable the will contain
     the protocol dependent information used to get in contact
     with the desired destination process

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if endpoint is null, or not valid
     or all the other pointers are NULL.

*/
int
fsf_get_send_endpoint_parameters
  (const fsf_send_endpoint_t  *endpoint,
   fsf_network_id_t           *network_id,
   fsf_node_address_t         *receiver,
   fsf_port_t                 *port);

/**
   \ingroup distjmodule

   This operation associates a send endpoint with a
   server, which means that messages sent through
   that endpoint will consume the server's reserved
   bandwidth and its packets will be sent according
   to the contract established for that server.  If
   the endpoint is already bound to another server,
   it is effectively unbound from it and bound to
   the specified one.  

   @param [in] server       server id
   @param endpoint          pointer to the endpoint
 
   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if endpoint is null, or not valid
     or server is not valid.
   @retval FSF_ERR_ALREADY_BOUND if the server is already
     bound to some other send endpoint
   @retval FSF_ERR_WRONG_NETWORK if the server network id
     is not the same as the one in the endpoint
*/ 
int
fsf_bind_endpoint_to_server 
  (fsf_server_id_t server,
   fsf_send_endpoint_t *endpoint);

/**
   \ingroup distjmodule

   This operation unbinds a send endpoint from a
   server. Endpoints with no server associated
   cannot be used to send data, and they stay in 
   that state  until they are either eliminated or
   bound again.  
   
   @param endpoint        pointer to the endpoint to unbind

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if endpoint is null, or not valid
   @retval FSF_ERR_NOT_BOUND if the endpoint has no server
     bound
 */
int
fsf_unbind_endpoint_from_server
  (fsf_send_endpoint_t  *endpoint);

/**
   \ingroup distjmodule

   This operation copies the id of the server that
   is bound to the specified send endpoint into the
   variable pointed to by server. 

   @param [in] endpoint      pointer to the endpoint
   @param [out] server       pointer to variable the will contain
     the server id
 
   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if endpoint is null, or not valid
     or server is NULL.
*/
int
fsf_get_endpoint_server
  (const fsf_send_endpoint_t  *endpoint,
   fsf_server_id_t            *server);


/**
   \ingroup distjmodule

   This operation sends a message stored in msg and of length size
   through the given endpoint. The operation is non-blocking and
   returns immediately. An internal fsf service will schedule the
   sending of messages and implement the communications sporadic server
   corresponding to the network server bound to the given endpoint.
   Messages sent through the same endpoint are received in the same
   order in which they were sent

   @param endpoint      pointer to the endpoint
   @param [in] msg      pointer to the message
   @param [in] size     number of bytes to transmitt
   
   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if endpoint is null, or not valid
     or msg is NULL
   @retval FSF_ERR_NOT_BOUND if endpoint is not bound to a valid
     server;
   @retval FSF_ERR_TOO_LARGE if the message is too large for the
     network protocol
   @retval FSF_ERR_BUFFER_FULL if the sending queue is full
*/
int
fsf_send
  (const fsf_send_endpoint_t  *endpoint,
   void                       *msg,
   size_t                      size);

/**
   \ingroup distjmodule

   This operation creates a receive endpoint with all the information
   that is necessary to receive information from the specified network
   and port
   
   @param [in] network_id the identifier ot the network to use
   @param [in] port       protocol dependent information used
     to identify the calling process as an expecting destination
   @param [out] endpoint  pointer to the variable that will
     contain the receive endpoint
   
   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if endpoint is null, network_id 
     is not valid or if port do not conform to the expected format
*/
int
fsf_create_receive_endpoint
  (fsf_network_id_t         network_id,
   fsf_port_t               port,
   fsf_receive_endpoint_t  *endpoint);

/**
   \ingroup distjmodule

   This operation eliminates any resources reserved for the given
   endpoint. After callling this operation the endpoint can not be
   further used to invoke receive operations on it.

   @param endpoint         pointer to the endpoint
   
   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if endpoint is null, or not valid
*/
int
fsf_destroy_receive_endpoint
  (fsf_receive_endpoint_t  *endpoint);


/**
   \ingroup distjmodule

   This operation returns in the variables network_id,
   and port, the corresponding parameters used in the
   creation of the given receive endpoint.
   
   @param [in] endpoint     pointer to the endpoint
   @param [out] network_id  pointer to variable the will contain
     the network id
   @param [out] port        pointer to variable the will contain
     the protocol dependent information used to get in contact
     with the desired destination process

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if endpoint is null, or not valid
     or all the other pointers are NULL.
*/
int
fsf_get_receive_endpoint_parameters
  (fsf_receive_endpoint_t  *endpoint,
   fsf_network_id_t        *network_id,
   fsf_port_t              *port);


/**
   \ingroup distjmodule

   If there are no messages available in the specified receive endpoint
   this operation blocks the calling thread waiting for a message to be
   received. When a message is available, if its size is less than or
   equal to the buffersize, the function stores it in the variable
   pointed to by buffer and puts the number of bytes received in the
   variable pointed to by messagesize. Messages arriving at a receiver
   buffer that is full will be silently discarded. The application is
   responsible of reading the receive endpoints with appropriate
   regularity, or of using a sequence number or some other mechanism
   to detect any lost messages.
   
   @param endpoint          pointer to the endpoint
   @param [out] buffer      pointer to the place that will contain
     the message received
   @param [in] buffersize   size of the buffer space
   @param [out] messagesize pointer to the variable that will contain
     the number of bytes received

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if endpoint is null, or not valid
     or if buffer or messagesize are NULL.
   @retval FSF_ERR_NO_SPACE if the buffersize is too small for the
     message received (in which case the message is lost).  
*/
int
fsf_receive
  (const fsf_receive_endpoint_t  *endpoint,
   void                          *buffer,
   size_t                         buffersize,
   size_t                        *messagesize);

/**
   \ingroup distjmodule

   This operation is the same as fsf_receive, except
   that if there are no messages available in the
   specified receive endpoint at the time of the call
   the operation returns with an error

   @param endpoint          pointer to the endpoint
   @param [out] buffer      pointer to the place that will contain
     the message received
   @param [in] buffersize   size of the buffer space
   @param [out] messagesize pointer to the variable that will contain
     the number of bytes received

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if endpoint is null, or not valid
     or if buffer or messagesize are NULL.
   @retval FSF_ERR_NO_SPACE if the buffersize is too small for the
     message received (in which case the message is lost).  
   @retval FSF_ERR_NO_MESSAGES if there are no messages available
     in the specified receive endpoint at the time of the call
*/
int
fsf_try_receive
  (const fsf_receive_endpoint_t  *endpoint,
   void                          *buffer,
   size_t                         buffersize,
   size_t                        *messagesize);

/**
   \ingroup distjmodule

   This operation is used to calculate a budget represented as
   a time magnitude, for a specified network. If succesful, the
   operation puts in the variable pointed to by budget the
   transmission time that it takes to send a packet through the
   network designated by network_id, when there is no contention,
   but including any network overheads. 

   @param [in] network_id  the identifier ot the network to use
   @param [out] budget     pointer to the variable tha will contain
     the time it takes to transmitt a packet

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if network_id is not a valid
     network identifier of if budget is a NULL pointer
*/
int
fsf_packet_tx_time
  (fsf_network_id_t      network_id,
   struct timespec       *budget);

/**
   \ingroup distjmodule

   This operation puts in the variable pointed to by 
   packet_size the maximum number of bytes that can be sent 
   in a packet through the network designated by network_id. 
   It is usually a configuration value and it helps the user 
   application to calculate the number of packets it will 
   need to reserve for the periodic transmision of its 
   messages and prepare the corresponding contracts. 

   @param [in] network_id   the identifier ot the network to use
   @param [out] packet_size pointer to the variable tha will contain
     the maximum size in bytes of a packet

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if network_id is not a valid
     network identifier of if packet_size is a NULL pointer
*/
int
fsf_packet_size
  (fsf_network_id_t      network_id,
   size_t               *packet_size);


/**
   \ingroup distjmodule

   This operation is used to obtain the maximum number of 
   packets of which a message can be formed, for the 
   specified network. A message is defined as the piece of 
   information used in a send operation. Since the value 
   returned by this operation is measured in packet units, 
   the effective size can be calculated multiplying this 
   value by the size of a packet.  When the value returned by 
   this operation is larger than 1 it means the 
   implementation will make the partition of messages into 
   packets and its recomposition at the receiving node.

   @param [in] network_id    the identifier ot the network to use
   @param [out] max_msg_size pointer to the variable tha will contain
     the maximum number of packets sent in a single send operation

   @retval 0 if the operation is succesful
   @retval FSF_ERR_BAD_ARGUMENT if network_id is not a valid
     network identifier of if max_msg_size is a NULL pointer
*/
int
fsf_max_message_size
  (fsf_network_id_t      network_id,
   size_t                *max_msg_size);



#endif // _FSF_DISTRIBUTED_H_
