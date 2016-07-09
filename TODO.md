

- Message Module
  - [ ] Generic serialization ("meta-cdr")
  - [x] Deserialization: implement std::forward(callback) in deserialize as suggested by Arthur
    - benchmark if this is actually useful
  - [ ] Fix wrapping of messages in send (I'm still not sure about message "glomming")
  - [ ] Parameters
- Behavior Module
  - [x] Finish missing functions in writer state machines
  - [x] Finish missing functions in reader state machines
  - [ ] Writer liveliness protocol
  - [x] implement all occurrences of calling state transition events

- [ ] Discovery Module
  - [ ] think of a mechanism for natural translation of the SEDP historycache tables (section 8.5.4)
  - [ ] "Interaction with RTPS virtual machine"
  - [ ] 4 builtin topics:
    - DCPSParticipant
    - DCPSSubscription
    - DCPSPublication
    - DCPSTopic
  - [ ] flesh out custom messages interface
  - [x] How to assign GuidPrefix for new participants?

- [ ] PIM: Implement UDP/IP (links to generic serialization goal)

- General
  - [ ] Fix the clunky template patterns and figure out which endpoint parameters should be compile-time selectable
  - [ ] Decide on which dynamically sized data structures or pattern for compile-time selection of data structures and their allocators
  - [x] Decide on patterns for an executor model
    - [ ] Multithreaded executor
  - [ ] Guard conditions
  - [ ] Error handling and console output mechanisms
  - [ ] Put constants in one header (?) and use std::integer_sequence for compile-time integer seqs
  - [ ] header reorg

- Unit test sockets module

Optional behaviors
  - [ ] Fragmentation
  - [ ] Implement DDS QoS features using RTPS (section 8.7)

- [ ] Minimum needed DDS api for an rmw implementation


- [ ] Fix Context injection
  - Maybe Context should probably instead be something like "TransportManager" and be a singleton

