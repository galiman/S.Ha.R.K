/**
   \mainpage FSF reference

   This reference manual contains the description of the API provided
   by the FSF library.

   \section intro Introduction to FSF

   The First Scheduling Framework (FSF) is the result of a joint
   effort from four european research groups to propose an API for
   flexible real-time scheduling in Real Time operating systems.  See
   the FIRST web page <a
   href="http://130.243.76.81:8080/salsart/first/">
   (http://130.243.76.81:8080/salsart/first/)</a> for more details in
   the project.

   \section lib Objectives 

Scheduling theory generally assumes that real-time systems are mostly
composed of activities with hard real-time requirements. 

Many systems are built today by composing different application or
components in the same system, leading to a mixture of many different
kinds of requirements with small parts of the system having hard
real-time requirements and other larger parts with requirements for
more flexible scheduling, taking into account quality of service. Hard
real-time scheduling techniques are extremely pessimistic for the
latter part of the application, and consequently it is necessary to
use techniques that let the system resources be fully utilized to
achieve the highest possible quality.

The FIRST project aims at developing a framework for a scheduling
architecture that provides the ability to compose several applications
or components into the system, and to flexibly schedule the available
resources while guaranteeing hard real-time requirements. The FIRST
Scheduling Framework (FSF) is independent of the underlying
implementation, and can run on different underlying scheduling
strategies. It is based on establishing service contracts that
represent the complex and flexible requirements of the application,
and which are managed by the underlying system to provide the required
level of service.

FSF provides a generalized architecture framework that combines
different kinds of requirements:

 - co-operation and coexistence of standard real-time scheduling
   schemes, time triggered and event triggered, dynamic and fixed
   priority based, as well as off-line based through a common
   architecture that is independent on the underlying scheduling
   mechanism

- integration of different timing requirements such as hard and soft,
  and more flexible notions

- temporal encapsulation of subsystems in order to support the
  composability and reusability of available components including
  legacy subsystems.

FSF has been implemented in two POSIX compliant real-time operating
systems, <a href="http://marte.unican.es">MaRTE</a> and <a
href="http://shark.sssup.it/">SHARK</a>, which are based on FP and EDF
scheduling schemes, respectively, thus illustrating the platform
independence of the presented approach.

   \section Service Contracts

The service contract is the mechanism that we have chosen for the
application to dynamically specify its own set of complex and flexible
execution requirements. From the application's perspective, the
requirements of an application or application component are written as
a set of service contracts, which are negotiated with the underlying
implementation. To accept a set of contracts, the system has to check
as part of the negotiation if it has enough resources to guarantee all
the minimum requirements specified, while keeping guarantees on all
the previously accepted contracts negotiated by other application
components. If as a result of this negotiation the set of contracts is
accepted, the system will reserve enough capacity to guarantee the
minimum requested resources, and will adapt any spare capacity
available to share it among the different contracts that have
specified their desire or ability for using additional capacity.


As a result of the negotiation process, if a contract is accepted, a
server is created for it. The server is a software object that is the
run-time representation of the contract; it stores all the information
related to the resources currently reserved for that contract, the
resources already consumed, and the resources required to handle the
budget consumption and replenishment events in the particular
operating system being used.

Because there are various application requirements specified in the
contract, they are divided into several groups, also allowing the
underlying implementation to give different levels of support trading
them against implementation complexity. This gives way to a modular
implementation of the framework, with each module addressing specific
application requirements. The minimum resources required by the
application to be reserved by the system are specified in the core
module. The requirements for mutual exclusive synchronization among
parts of the application being scheduled by different servers or among
different applications are specified in the shared objects
module. Flexible resource usage is associated with the spare capacity
and dynamic reclamation modules. The ability to compose applications
or application components with several threads of control, thus
requiring hierarchical scheduling of several threads inside the same
server are supported by the hierarchical scheduling module. Finally,
the requirements of distributed applications are supported by the
distributed and the distributed spare capacity modules. We will now
explain these modules together with their associated application
requirements.


 */

/**
   \defgroup coremodule Core module

The core module contains the service contract information related to
the application minimum resource requirements, the operations required
to create contracts and negotiate them, and the underlying
implementation of the servers with a resource reservation mechanism
that allows the system to guarantee the resources granted to each
server.

   This module includes the basic functions and services that are
   provided by any FSF implementation. This module includes basic type
   definitions, and functions to

   - create a contract and initialize it
   - set/get the basic parameters of a contract
   - negotiate a service contract, obtaining a server id
   - create and bind threads to servers
   - create/destroy a synchronization object
   - manage bounded workloads

   A round-robin background scheduling policy is available for those
   threads that do not have real-time requirements. Because some of
   these threads may require sharing information with other threads
   run by regular servers, special background contracts may be created
   for specifying the synchronization requirements.

   The way of specifying a background contract is by setting
   budget_min = period_max = 0. Negotiation may fail if the contract
   uses shared_objects. If the contract has no shared_objects the
   returned server id represents the background and may be used to
   bind more than one thread. If the contract has shared objects a
   server is created to keep track of them, but the associated threads
   are executed in the background, together with the other background
   threads.

   An abstract synchronization object is defined by the application.
   This object can be used by an application to wait for an event to
   arrive by invoking the fsf_schedule_triggered_job() operation.  It
   can also be used to signal the event either causing a waiting
   server to wake up, or the event to be queued if no server is
   waiting for it.

   These objects are used to synchronize threads belonging to bounded
   workload servers.

   In the future we may add a broadcast operation that would signal a
   group of synchronization objects. We have not included a broadcast
   service in this version because it can be easily created by the
   user by signalling individual synchronization objects inside a
   loop.

   Notice that for synchronization objects there is no naming service
   like in shared objects because tasks that use synchronization are
   not developed independently, as they are closely coupled.


*/

/**
   \defgroup sparemodule Spare capacity sharing module

Many applications have requirements for flexibility regarding the
amount of resources that can be used. The spare capacity module allows
the system to share the spare capacity that may be left over from the
negotiation of the service contracts, in a static way. During the
negotiation, the minimum requested resources are granted to each
server, if possible. Then, if there is any extra capacity left, it is
distributed among those applications that have expressed their ability
to take advantage of it.

   This module includes functions for sharing the spare capacity in the
   system between the servers. It allows to mainly to specify
   additional contract parameters to allow the sharing of the spare
   capacity.

   The features provided by this module are different from the
   services provided by the Dynamic reclamation module. This module
   influences the negotiation procedure (see the \ref coremodule
   module).
     
 */

/**
   \defgroup hiermodule Hierarchical scheduling module

One of the application requirements that FSF addresses is the ability
to compose different applications, possibly using different scheduling
policies, into the same system. This can be addressed with support in
the system for hierarchical scheduling. The lower level is the
scheduler that takes care of the service contracts, using an
unspecified scheduling policy (for instance, a CBS on top of EDF, or a
sporadic server on top of fixed priorities). The top level is a
scheduler running inside one particular FSF server, and scheduling the
application threads with whatever scheduling policy they were
designed. In this way, it is possible to have in the same system one
application with, for example, fixed priorities, and another one
running concurrently with an EDF scheduler.

We are currently providing four top-level schedulers: fixed
priorities, EDF, round robin and table-driven.
 */

/**
   \defgroup shobjmodule Shared Objects module

The shared objects module of FSF allows the application to specify in
the contract attributes all the information related to the mutually
exclusive use of shared resources that is required to do the
schedulability analysis.

The set of shared objects present in the system together with the
lists of critical sections specified for each contract are used for
schedulability analysis purposes only. A run-time mechanism for mutual
exclusion is not provided in FSF for two important reasons. One of
them is upward compatibility of previous code using regular primitives
such as mutexes or protected objects (in Ada); this is a key issue if
we want to persuade application developers to switch their systems to
the FSF environment. The second reason is that enforcing worst case
execution time for critical sections is expensive. The number of
critical sections in real pieces of code may be very high, in the tens
or in the hundreds per task, and monitoring all of them would require
a large amount of system resources.

The FSF application does not depend on any particular synchronization
protocol, but there is a requirement that a budget expiration cannot
occur inside a critical section, because otherwise the blocking delays
could be extremely large. This implies that the application is allowed
to overrun its budget for the duration, at most, of the critical
section, and this extra budget is taken into account in the
schedulability analysis.

   Because shared objects are meant to be used by independent
   applications, there needs to be a way of identifying them through
   some global name. We use the type fsf_shared_object_id_t to
   represent these identifiers. To avoid potential name or identity
   conflicts between different independent applications we use a
   null-character-terminated string.

   For efficiency purposes, the contracts have smaller identifiers for
   the shared objects, that can be implemented with a pointer or an
   index to an array element.  Therefore conversion functions are
   necessary to convert a long identifier of type
   fsf_shared_object_id_t into a short handle to the object, of the
   type fsf_shared_obj_handle_t.  This conversion is done at the
   creation of the object (with fsf_init_shared_object), and there is
   also a function to obtain the handle after the object has been
   created (fsf_get_shared_object_handle).

   To make the use of shared objects in FSF independent of the
   underlying synchronization protocol, the function that initializes
   a shared object (fsf_init_shared_object) also initializes the mutex
   specified by the user with the appropriate, implementation-defined,
   attributes. Therefore, the application should not initialize the
   mutex itself, but should pass the mutex to this function call, and
   then use it with the regular POSIX interfaces to enter and leave
   critical sections. It is possible to obtain a pointer to the mutex
   from the shared object with the function
   fsf_get_shared_object_mutex.
 */

/**
   \defgroup distjmodule Distributed module

   FSF is designed to support applications with requirements for
   distribution. The first step towards distribution is the ability to
   support service contracts for the network or networks used to
   interconnect the different processing nodes in the system. Similar
   to the core FSF module, the contracts on the network allow the
   application to specify its minimum utilization (bandwidth)
   requirements, so that the implementation can make guarantees or
   reservations for that minimum utilization. We use the same contract
   that is used for processing nodes, and thus the core attributes for
   distribution are the same as for the core FSF with the addition of
   the network id attribute, that identifies the contract as a network
   contract for the specified network. The default value for the
   network id is null, which means that the contract applies to the
   processing node where the contact is negotiated.

   For the FSF implementation to keep track of consumed network
   resources and to enforce the budget guarantees it is necessary that
   the information is sent and received through specific FSF
   services. To provide communication in this context we need to
   create objects similar to the sockets used in most operating
   systems to provide message communication services. We call these
   objects communication endpoints, and we distinguish send and
   receive endpoints.

   A send endpoint contains information about the network to use, the
   destination node, and the port that identifies a reception
   endpoint. It is bound to a network server that specifies the
   scheduling parameters of the messages sent through that endpoint,
   keeps track of the resources consumed, and limits the bandwidth to
   the amount reserved for it by the system. It provides message
   buffering for storing messages that need to be sent.

   A receive endpoint contains information about the network and port
   number to use. It provides message buffering for storing the
   received messages until they are retrieved by the application. A
   receive endpoint may get messages sent from different send
   endpoints, possibly located in different processing nodes.

   This module provides communication and negotiation services to
   support the negotiation of independent servers on networks, as well
   as to offer the simple communications primitives that allow for
   accounting and limiting the network bandwidth used.

   The concrete protocol and communications hardware and software
   interfaces to use and whether the communication strategy over the
   real-time network is connection-oriented or not, are implementation
   dependent. The creation of endpoints will set them as
   appropriate.

   A node may negotiate contracts to reserve bandwidth for the sending
   of messages through the networks to which it is connected. It does
   not have to reserve bandwidth for incoming messages, just for
   outgoing messages.  A node cannot reserve bandwidth in networks to
   which it has no direct connection.

   As mentioned above, specific send and receive communication
   primitives are included to transfer messages while managing the
   bandwidth budget consumption. The send operation is non-blocking:
   it just copies the message into an internal buffer and returns
   immediately. The FSF network scheduler of the sender processor
   takes responsibility on accounting the bandwidth fraction used
   through each send endpoint, using the bandwidth reservation made by
   the network server that is bound to it.

   Considering that bandwidth reservations should be done in the form
   of number of bytes per time unit, that the speed of the network is
   quite variable from one platform to another, and that the regular
   FSF contract negotiation operations work with budgets expressed as
   CPU time, a function is added to convert the size of messages to
   the amount of time that is necessary to be used as the budget in
   the FSF contract.

 */

/**
   \defgroup distsparemodule Distributed spare capacity module

   In the distributed FSF we have chosen to give a minimum support for
   spare capacity distribution inside FSF, and leave the consensus
   problem related to the negotiation of end-to-end transactions to
   some higher-level manager that would make the negotiations for the
   application.

   There is a new attribute in the service contract called the granted
   capacity flag which, when set, has the implication that the period
   or budget of the server can only change if a renegotiation or a
   change of quality and importance is requested for it; it may not
   change automatically, for instance because of negotiations for
   other servers. This provides a stable framework while performing
   the distributed negotiation.

   For a server with the granted capacity flag set, there is an
   operation to return spare capacity that cannot be used due to
   restrictions in other servers of a distributed transaction.
 */
