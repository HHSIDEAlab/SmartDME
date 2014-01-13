/* From Tom */

#include <SoftwareSerial.h> // Serial communications

/* setup debug mode */
#define DEBUG true  //turn debugging on / off
#define DEBUGSD false  //debug to sd card verse serial terminal

#define ARDUINO_RX_PIN  2
#define ARDUINO_TX_PIN  3

SoftwareSerial goSerial(ARDUINO_RX_PIN, ARDUINO_TX_PIN);

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

#ifndef null
#define null 0
#endif

template <class T, int RINGBUFFERSIZE>
class RingBuffer
{
public:
	RingBuffer()
	{
	    clear();
	}
	
	bool write(/*[in]*/ T t)
	{
	    bool fRet = false;
	    if (m_nCount < RINGBUFFERSIZE)
	    {
		m_rgBuffer[m_nTail] = t;
		m_nCount++;
		if (+++m_nTail == RINGBUFFERSIZE)
		    m_nTail = 0;
	    }
	    return fRet;
	}

	bool peek(/*[out]*/ T *t, /*[in,opt]*/ int index = 0)
	{
	    bool fRet = false;
	    if (m_nCount > index)
	    {
		int nHead = m_nHead;
		for (int i = 0; i < index; i++)
		{
		    nHead = (nHead + 1) == RINGBUFFERSIZE ? 0 : nHead + 1;
		}
		if (t)
		     *t = m_rgBuffer[m_nHead];
		fRet = true;
	    }
	    return fRet;
	}

	bool read(/*[out]*/ T *t)
	{
	    bool fRet = false;
	    if (m_nCount)
	    {
		if (t)
		    *t = m_rgBuffer[m_nHead];
		m_nCount--;
		if (++m_nHead == RINGBUFFERSIZE)
		    m_nHead = RINGBUFFERSIZE-1;
		fRet = true;
	    }
            return fRet;
	}

	bool unread(/*[in]*/ T t)
	{
	    bool fRet = false;
	    if (m_nCount < RINGBUFFERSIZE)
	    {
		m_nHead = (m_nHead == 0) ? RINGBUFFERSIZE-1 : m_nHead - 1;
		m_rgBuffer[m_nHead] = t;
		m_nCount++;
		fRet = true;              
            }
            return fRet;		
	}

	void clear()
	{
	    m_nHead = m_nTail = m_nCount = 0;
	}

	bool discard(/*[in]*/ int count)
	{
	    bool fRet = true;
	    for (int i = 0; i < count; i++)
		fRet = fRet & read(null);
	    return fRet;
	}

	int count()
	{
	    return m_nCount;
	}

private:
	int m_nHead;
	int m_nTail;
	int m_nCount;
	T m_rgBuffer[RINGBUFFERSIZE];
};

typedef struct SimplyGoPacket
{
	/*0, 2*/ word headerSignature;
	/*2, 1*/ byte packetSequenceNo;
	/*3, 31*/ byte unused1[31];
	/*34, 2*/ word batteryData;
	/*36, 80*/ byte unused2[80];
	/*116, 2*/ word checksumLow;
	/*118, 2*/ word checksumHigh;
	/*120, 8*/ byte unused3[8];
} SimplyGoPacket;


class SimplyGoReader
{
public:
	SimplyGoReader() {}

	bool Read(/*[in]*/ SoftwareSerial *device, /*[out]*/ SimplyGoPacket *packet)
	{
            bool fRet = false;
	    if (device->available() > 0)
	    {
		byte ch = (byte)device->read();
		int count = m_ringBuffer.count();

		switch(count)
		{
		    case 0:
		    {
 		        if (ch == 0xFE) /*first byte of signature?*/
		            m_ringBuffer.write(ch);
                    }
		    break;

		    case 1:
		    {
		        if (ch == 0x01) /*second byte of signature?*/
		            m_ringBuffer.write(ch);
		        else
			    m_ringBuffer.read(null); /*throw it away--it's not the beginning of a packet*/
		    }
                    break;

		    default:
		    {
			/*if this fails, the buffer isn't big enough. make sure this never fails*/
			bool fwrote = m_ringBuffer.write(ch);
			/*assert(fwrote);*/

			/*if we've filled enough for a single packet, process it now*/
			if (m_ringBuffer.count() >= sizeof(SimplyGoPacket))
			{
			    byte *b = (byte*)packet;
			    word checksum = 0;
			    int nNextHeader = 0;
                            for (int i = 0; i < sizeof(SimplyGoPacket); i++, b++)
			    {
				m_ringBuffer.peek(b, i);
				if (i < 116 || i > 119) /*we don't want to include the checksum value, itself*/
				    checksum = checksum + (*b);
				if (*b == 0xFE && nNextHeader == 0) /*figure out where the next header might begin*/
				    nNextHeader = i;
			    }
			    if (checksum == packet->checksumLow) /*it's a correctly-formed packet*/
			    {
				m_ringBuffer.discard(sizeof(SimplyGoPacket));
				fRet = true;
			    }
			    else
                            if (nNextHeader != 0) /*skip to the next possible header in the packet*/
			    {
			     	m_ringBuffer.discard(nNextHeader);   
			    }
			    else
			    {
				/*the current packet is not correctly formed and there is no header embedded, either*/
				/*so just throw away the entire packet*/
				m_ringBuffer.discard(sizeof(SimplyGoPacket)); 
			    }    
			}
                    }
	            break;
	    }
	    return fRet;
	}

private:
	RingBuffer<byte,256> m_ringBuffer;
};


