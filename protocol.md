# Gambezi: A Lightweight Hierarchical Key Value Pair Protocol
Yixu (Tiger) Huang<br>
tigerrun1998@gmail.com



## Introduction
This protocol is designed to be a cross platform way to communicate between different programs or sections of code.



## Requirements
A packet based transport layer is preferred but not necessary. Websockets works nicely because of low overhead and a large number of platforms supported, including browsers.

All multi-byte values are big endian (the most significant byte occurs first, the zeroth element of the array).

Each packet must consist of a header and one or more sections.
Multiple protocol packets may be packed into a single packet of the transport layer (currently unimplemented).



## Tree Structure
The protocol is arranged as a hierarchical database. There are a maximum of 256 child nodes per node, and a maximum tree depth of 255. The root node is not counted in the depth number.



## General Formats

### Data Values Format
Values are stored as a two byte length followed by the data.
| Byte    | Contents        |
|---------|-----------------|
| 0 - 1   | *n* (length)    |
| 2       | 1st data byte   |
| ...     | ...             |
| 2 + *n* | *nth* data byte |

### String and Name Format
Names can be up to 255 bytes long (excluding the length byte).
| Byte    | Contents        |
|---------|-----------------|
| 0       | *n* (length)    |
| 1       | 1st character   |
| ...     | ...             |
| 1 + *n* | *nth* character |

### Key Format
Keys can be up to 255 bytes long (excluding the length byte).
| Byte    | Contents          |
|---------|-------------------|
| 0       | *n* (length)      |
| 1       | 1st level index   |
| ...     | ...               |
| 1 + *n* | *nth* level index |



## Client->Server Packet Structure


### Request Node ID
#### Header
| Byte | Contents | Function                       |
|------|----------|--------------------------------|
| 0    | 0x00     | Signifies this type of packet  |
| 1    | *flags*  | Flags related to subscriptions |
##### Flags
| Bit      | 7 - 1      | 0                          |
|----------|------------|----------------------------|
| Function | *reserved* | Set to get all child nodes |
#### Sections
| Section Type | Description                          |
|--------------|--------------------------------------|
| Header       | Header belonging to this packet type |
| Key          | Key of the parent node               |
| Name         | Name of this node                    |


### Set Node Value
#### Header
| Byte | Contents | Function                      |
|------|----------|-------------------------------|
| 0    | 0x01     | Signifies this type of packet |
#### Sections
| Section Type | Description                          |
|--------------|--------------------------------------|
| Header       | Header belonging to this packet type |
| Key          | Key of this node                     |
| Value        | Value of this node                   |


### Set Client Refresh Rate
#### Header
| Byte  | Contents       | Function                      |
|-------|----------------|-------------------------------|
| 0     | 0x02           | Signifies this type of packet |
| 1 - 2 | *refresh_rate* | Client refresh rate (ms)      |
#### Sections
| Section Type | Description                          |
|--------------|--------------------------------------|
| Header       | Header belonging to this packet type |


### Update Subscription to a Node
#### Header
| Byte  | Contents       | Function                          |
|-------|----------------|-----------------------------------|
| 0     | 0x03           | Signifies this type of packet     |
| 1     | *flags*        | Flags related to subscriptions    |
| 2 - 3 | *refresh_skip* | Update every *nth* client refresh |
##### Flags
| Bit      | 7 - 1      | 0                                   |
|----------|------------|-------------------------------------|
| Function | *reserved* | Set to subscribe to all child nodes |
##### Refresh Skip Special Values
| Value  | Meaning                            |
|--------|------------------------------------|
| 0x0000 | Get updates as soon as they arrive |
| 0xFFFF | Unsubscribe from node              |
#### Sections
| Section Type | Description                          |
|--------------|--------------------------------------|
| Header       | Header belonging to this packet type |
| Key          | Key of this node                     |


### Request Node Value
#### Header
| Byte  | Contents       | Function                      |
|-------|----------------|-------------------------------|
| 0     | 0x04           | Signifies this type of packet |
| 1     | *flags*        | Flags related to request      |
##### Flags
| Bit      | 7 - 1      | 0                             |
|----------|------------|-------------------------------|
| Function | *reserved* | Set to get to all child nodes |
#### Sections
| Section Type | Description                          |
|--------------|--------------------------------------|
| Header       | Header belonging to this packet type |
| Key          | Key of this node                     |



## Server->Client Packet Structure


### Return node ID
#### Header
| Byte | Contents | Function                      |
|------|----------|-------------------------------|
| 0    | 0x00     | Signifies this type of packet |
#### Sections
| Section Type | Description                          |
|--------------|--------------------------------------|
| Header       | Header belonging to this packet type |
| Key          | Key of this node                     |
| Name         | Name of this node                    |


### Return node value
#### Header
| Byte | Contents | Function                      |
|------|----------|-------------------------------|
| 0    | 0x01     | Signifies this type of packet |
#### Sections
| Section Type | Description                          |
|--------------|--------------------------------------|
| Header       | Header belonging to this packet type |
| Key          | Key of this node                     |
| Value        | Value of this node                   |


### Error with request
#### Header
| Byte | Contents | Function                      |
|------|----------|-------------------------------|
| 0    | 0x02     | Signifies this type of packet |
#### Sections
| Section Type | Description                          |
|--------------|--------------------------------------|
| Header       | Header belonging to this packet type |
| Name         | The error message                    |



## Glossory
| Word         | Definition                                                                                          |
|--------------|-----------------------------------------------------------------------------------------------------|
| Data Tree    | Holds all data for this protocol and contains multiple nodes in a tree structure.                   |
| Node         | A location within the data tree where data can be stored. Each node can hold up to 255 child nodes. |
| (Binary) Key | A numerical sequence that denotes a node's location in the data tree.                               |
| String Key   | A string sequence that denotes a node's location in the data tree.                                  |
| Name         | A single string from a string key that operates on a single level of the data tree.                 |
| ID           | A single number from a (binary) key that operates on a single level of the data tree.               |

