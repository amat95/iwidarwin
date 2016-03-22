# methods #
  * allocatePacket
> > allocate packet with **data** sepecified length.
> > packet is the whole of chains.
> > first packet has MBUF\_PKTHDR.


  * mbuf\_pkthdr\_len
> > return length of packet( data of the whole chaines)

  * mbuf\_pkthdr\_setlen
> > set length of packet (data of the whole chaines).
> > if use this function, should chage other mbuf\_len of same chains?
  * mbuf\_len
> > data size of signle mbuf
  * mbuf\_leadingspace
> > area length before current pointer.( only single mbuf).
  * mbuf\_trailingspace
> > free area length following current data. (only single mbuf).

  * mbuf\_prepend
> > reverse data pointer (m\_data ) with specified size .
> > this is the same of **skb\_push**

  * mbuf\_adj
> > decrese m\_len(if MBUF\_PKTHDR, with m\_pktdr.len ) and increate data pointer(m\_data) and
> > this is the same **skb\_pull**. but ( skb\_pull is return old data pointer ).
> > if size argument < 0 , this is the same **skb\_trim**

  * mbuf\_datastart
> > beging point of data. if MBUF\_FLAGS != M\_EXT, m\_dat,
> > if MBUF\_FRAGS == M\_EXT, m\_ext is m\_ext.ext\_buf ( begining point of extrac buffer )
  * mbuf\_data
> > current data point ( m\_data )

  * mbuf\_maxlen
> > current mbuf's maxsize.

  * mbuf\_setdata
> > set position of current data pointer and set mbuf->m\_len.
> > warning: this function dont think entire packet.
> > > if mbuf\_flags(mbuf) is MBUF\_PKTHDR|MBUF\_EXT, mbuf->m\_pkghdr.len is old size.


  * mbuf\_align\_32

> > move current data pointer to m\_datastart +


## wraping skb ##

**skb\_reserve
```
    void skb_reserve(struct mbuf_t skb, unsigned int len)
    {
         void *data = (Uint8*)mbuf_data(skb)+len; 
         mbuf_setdata(skb,data,mbuf_len(skb));// m_len is not changed.
    }
```** skb\_put
```
    inline void *skb_put(mbuf_t skb, unsigned int len)
    {
        
        void *data=(UInt8*)mbuf_data(skb)+mbuf_len(skb);
        //mbuf_prepend(&skb,len,1); /* no prepend work */
        if(mbuf_trailingspace(skb) > len )
          mbuf_setlen(skb,mbuf_len(skb)+len);
          
          if(mbuf_flags(skb) & MBUF_PKTHDR){
              mbuf_pkthdr_setlen(skb,mbuf_pkthdr_len(skb)+len); 
          }
        }
        return data;
     }

```




# function #
from bsd/sys/kpi\_mbuf.h
  * mbuf\_data
  * mbuf\_datastart
  * mbuf\_setdata
  * mbuf\_align\_32
  * mbuf\_data\_to\_physical
  * mbuf\_get
  * mbuf\_getcluster
  * mbuf\_mclget
  * mbuf\_mclget
  * etc
