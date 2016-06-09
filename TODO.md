- Message Module
  - [ ] Generic serialization ("meta-cdr")
    - e.g. a "Serializable" concept on types which allows it to be arbitrarily serialized to binary format
  - [ ] Parameters
- Behavior Module
  - [ ] Finish missing functions in writer state machines
  - [ ] Finish missing functions in reader state machines
  - [ ] Writer liveliness protocol
  - [ ] implement all occurrences of calling state transition events

- [ ] Discovery Module
  - [ ] think of a mechanism for natural translation of the SEDP historycache tables (section 8.5.4)
  - [ ] "Interaction with RTPS virtual machine"

- [ ] PIM: Implement UDP/IP (links to generic serialization goal)

- General
  - [ ] Fix the clunky template patterns and figure out which endpoint parameters should be compile-time selectable
  - [ ] Decide on which dynamically sized data structures or pattern for compile-time selection of data structures and their allocators
  - [ ] Decide on patterns for an executor model


Optional behaviors
  - [ ] Fragmentation
  - [ ] Implement DDS QoS features using RTPS (section 8.7)
