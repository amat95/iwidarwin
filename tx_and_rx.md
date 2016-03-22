# rx #
  * len of args with **inputPacket** **must** the whole packet size( not mbuf\_len ).

# tx #
  * mbuf receive from **outputPacket** is chains. and first mbuf is packet header of mbuf.
  * must copy all packet date  when store mbuf to queue.