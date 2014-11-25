
#ifndef DETH_H_
#define DETH_H_

enum message_type
{
    MULTICAST_ADDRESS_CHANGE,
    PACKET,
    LINK_CHANGE,
	LINK_STATS
};

/*
	Packets format:

			4 bytes

	MULTICAST_ADDRESS_CHANGE
	____________________________ 
	|                           |
    |     interface index       |
    |___________________________|


	PACKET
	____________________________ 
	|                           |
    |     interface index       |
    |___________________________|
    |                           |
    |         packet            |
	|           ...             |
	|           ...             |
	|           ...             |
    |___________________________|


	LINK_CHANGE
	____________________________ 
	|                           |
    |     interface index       |
    |___________________________|
    |                           |
    |          value            |
    |___________________________|


	LINK_STATS
    ____________________________ 
    |                           |
    |     interface index       |
    |___________________________|
    |                           |
    |        RX packets         |
    |                           |
	|         64 bits           |
    |___________________________|
    |                           |
    |         RX bytes          | 
    |                           |
	|         64 bits           | 
    |___________________________|
    |                           |
    |        TX packets         |
    |                           |
	|         64 bits           |
    |___________________________|
    |                           |
    |         TX bytes          |
    |                           |
	|         64 bits           |
    |___________________________|
*/

#endif
