/*-
 * Copyright (c) 2013 HHS.GOV
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file SimplyGo Battery Library
 */

#include "simplygo.h"

/* For free memory check */
extern unsigned int __bss_end;
extern unsigned int __heap_start;
extern void *__brkval;

#undef DEBUG

#ifdef DEBUG
#define DPRINT(item) debug.print(item)
#define DPRINTLN(item) debug.println(item)
#else
#define DPRINT(item)
#define DPRINTLN(item)
#endif

/* From Tom */

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
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



struct SimplyGoBatteryPacket
{
	/*0, 2*/ word headerSignature;
	/*2, 1*/ byte packetSequenceNo;
	/*3, 31*/ byte unused1[31];
	/*34, 2*/ word batteryData;
	/*36, 80*/ byte unused2[80];
	/*116, 2*/ word checksumLow;
	/*118, 2*/ word checksumHigh;
	/*120, 8*/ byte unused3[8];
};


class SimplyGoReader
{
public:
	SimplyGoReader()

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
			    byte *b = packet;
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
	bool m_fAvailable;
	RingBuffer<byte,256> m_ringBuffer;
    };

/**
 * Convert a unsigned int to a string
 * @param val the value to convert to a string
 * @param base format for string; either DEC for decimal or
 *             HEX for hexidecimal
 * @param buf the buffer to write the string to
 * @param size the size of the buffer
 * @returns number of characters written to the buffer
 *          not including the null terminator (i.e. size of the string)
 **/
static int simple_utoa(uint32_t val, uint8_t base, char *buf, int size)
{
    char tmpbuf[16];
    int ind=0;
    uint32_t nval;
    int fsize=0;

    if (base == DEC) {
	do {
	    nval = val / 10;
	    tmpbuf[ind++] = '0' + val - (nval * 10);
	    val = nval;
	} while (val);
    } else {
	do {
	    nval = val & 0x0F;
	    tmpbuf[ind++] = nval + ((nval < 10) ? '0' : 'A');
	    val >>= 4;
	} while (val);
	tmpbuf[ind++] = 'x';
	tmpbuf[ind++] = '0';
    }

    ind--;

    do {
	buf[fsize++] = tmpbuf[ind];
    } while ((ind-- > 0) && (fsize < (size-1)));
    buf[fsize] = '\0';

    return fsize;
}

/** Simple hex string to uint32_t */
static uint32_t atoh(char *buf)
{
    uint32_t res=0;
    char ch;
    bool gotX = false;

    while ((ch=*buf++) != 0) {
	if (ch >= '0' && ch <= '9') {
	    res = (res << 4) + ch - '0';
	} else if (ch >= 'a' && ch <= 'f') {
	    res = (res << 4) + ch - 'a' + 10;
	} else if (ch >= 'A' && ch <= 'F') {
	    res = (res << 4) + ch - 'A' + 10;
	} else if ((ch == 'x') && !gotX) {
	    /* Ignore 0x at start */
	    gotX = true;
	} else {
	    break;
	}
    }

    return res;
}

/** Simple ASCII to unsigned int */
static uint32_t atou(const char *buf)
{
    uint32_t res=0;

    while (*buf) {
	if ((*buf < '0') || (*buf > '9')) {
	    break;
	}
	res = res * 10 + *buf - '0';
	buf++;
    }

    return res;
}

SimplyGO::SimplyGo()
{
	//set initial state


#ifdef DEBUG
    debugOn = true;
#else
    debugOn = false;
#endif

    dbgBuf = NULL;
    dbgInd = 0;
    dbgMax = 0;

}
/**
 * Get ready to handle commands, Clear Ring, and determine
 * some initial status.
 */
void SimplyGO::init()
{


}

/**
 * Flush the incoming data from the SimplyGo.
 * @param timeout - the number of milliseconds to wait for additional data to flush. Default is 500msecs.
 */
void SimplyGO::flushRx(int timeout)
{
    char ch;
    DPRINT(F("flush\n\r"));
    while (readTimeout(&ch,timeout));
    DPRINT(F("flushed\n\r"));
}


/** Check to see if data is available to be read.
 * @returns the number of bytes that are available to read.
 * @retval 0 - no data available
 * @retval -1 - active serial closed,
 */
int SimplyGo::available()
{
    int count;

    count = serial->available();
    if (count > 0) {
	if (debugOn) {
	    debug.print(F("available: peek = "));
	    debug.println((char)serial->peek());
	}
	
	}

    return count+peekCount;
}

void SimplyGo::flush()
{
   serial->flush();
}
  

/** Hex dump a string */
void SimplyGo::dump(const char *str)
{
    while (*str) {
	debug.print(*str,HEX);
	debug.print(' ');
	str++;
    }
    debug.println();
}

/**
 * Start a capture of all the characters recevied from the Serial Device.
 * @param size - the size of the capture buffer. This will be malloced.
 */
void SimplyGo::dbgBegin(int size)
{
    if (dbgBuf != NULL) {
	free(dbgBuf);
    }
    dbgBuf = (char *)malloc(size);
    dbgInd = 0;
    dbgMax = size;
}

/** Stop debug capture and free buffer */
void SimplyGo::dbgEnd()
{
    if (dbgBuf != NULL) {
	free(dbgBuf);
	dbgBuf = NULL;
    }
    dbgInd = 0;
    dbgMax = 0;
}

/** Do a hex and ASCII dump of the capture buffer, and free the buffer.  */
void SimplyGo::dbgDump()
{
    int ind;

    if (dbgBuf == NULL) {
	return;
    }

    if (dbgInd > 0) {
	debug.println(F("debug dump"));
	for (ind=0; ind<dbgInd; ind++) {
	    debug.print(ind);
	    debug.print(F(": "));
	    debug.print(dbgBuf[ind],HEX);
	    if (isprint(dbgBuf[ind])) {
		debug.print(' ');
		debug.print(dbgBuf[ind]);
	    }
	    debug.println();
	}
    }
    free(dbgBuf);
    dbgBuf = NULL;
    dbgMax = 0;
}

