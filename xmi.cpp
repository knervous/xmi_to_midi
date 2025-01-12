/*
Copyright (c) 2009 Peter "Corsix" Cawley

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <memory.h>
#include <stdint.h>
#include <algorithm>
#include <iterator>
#include <new>
#include <vector>

/*!
Utility class for reading or writing to memory as if it were a file.
*/
class MemoryBuffer
{
public:
	MemoryBuffer()
		: m_pData(NULL), m_pPointer(NULL), m_pEnd(NULL), m_pBufferEnd(NULL)
	{
	}

	MemoryBuffer(const unsigned char* pData, size_t iLength)
	{
		m_pData = m_pPointer = (char*)pData;
		m_pEnd = m_pData + iLength;
		m_pBufferEnd = NULL;
	}

	~MemoryBuffer()
	{
		if (m_pBufferEnd != NULL)
			delete[] m_pData;
	}

	unsigned char* takeData(size_t *pLength)
	{
		if (pLength)
			*pLength = m_pEnd - m_pData;
		unsigned char* pResult = (unsigned char*)m_pData;
		m_pData = m_pPointer = m_pEnd = m_pBufferEnd = NULL;
		return pResult;
	}

	size_t tell() const
	{
		return m_pPointer - m_pData;
	}

	bool seek(size_t position)
	{
		if (m_pData + position > m_pEnd)
		{
			if (!_realloc(position))
				return false;
		}
		m_pPointer = m_pData + position;
		return true;
	}

	bool skip(int distance)
	{
		if (distance < 0)
		{
			if (m_pPointer + distance < m_pData)
				return false;
		}
		return seek(m_pPointer - m_pData + distance);
	}

	bool scanTo(const void* pData, size_t iLength)
	{
		for (; m_pPointer + iLength <= m_pEnd; ++m_pPointer)
		{
			if (memcmp(m_pPointer, pData, iLength) == 0)
				return true;
		}
		return false;
	}

	const char* getPointer() const
	{
		return m_pPointer;
	}

	template <class T>
	bool read(T& value)
	{
		return read(&value, 1);
	}

	template <class T>
	bool read(T* values, size_t count)
	{
		if (m_pPointer + sizeof(T)* count > m_pEnd)
			return false;
		memcpy(values, m_pPointer, sizeof(T)* count);
		m_pPointer += sizeof(T)* count;
		return true;
	}

	unsigned int readBigEndianUInt24()
	{
		uint8_t iByte0, iByte1, iByte2;
		if (read(iByte0) && read(iByte1) && read(iByte2))
			return (((iByte0 << 8) | iByte1) << 8) | iByte2;
		else
			return 0;
	}

	unsigned int readUIntVar()
	{
		unsigned int iValue = 0;
		uint8_t iByte;
		for (int i = 0; i < 4; ++i)
		{
			if (!read(iByte))
				return false;
			iValue = (iValue << 7) | static_cast<unsigned int>(iByte & 0x7F);
			if ((iByte & 0x80) == 0)
				break;
		}
		return iValue;
	}

	template <class T>
	bool write(const T& value)
	{
		return write(&value, 1);
	}

	template <class T>
	bool write(const T* values, size_t count)
	{
		if (!skip(static_cast<int>(sizeof(T)* count)))
			return false;
		memcpy(m_pPointer - sizeof(T)* count, values, sizeof(T)* count);
		return true;
	}

	bool writeBigEndianUInt16(uint16_t iValue)
	{
		return write(_byteSwap(iValue));
	}

	bool writeBigEndianUInt32(uint32_t iValue)
	{
		return write(_byteSwap(iValue));
	}

	bool writeUIntVar(unsigned int iValue)
	{
		int iByteCount = 1;
		unsigned int iBuffer = iValue & 0x7F;
		for (; iValue >>= 7; ++iByteCount)
		{
			iBuffer = (iBuffer << 8) | 0x80 | (iValue & 0x7F);
		}
		for (int i = 0; i < iByteCount; ++i)
		{
			uint8_t iByte = iBuffer & 0xFF;
			if (!write(iByte))
				return false;
			iBuffer >>= 8;
		}
		return true;
	}

	bool isEOF() const
	{
		return m_pPointer == m_pEnd;
	}

protected:
	template <class T>
	static T _byteSwap(T value)
	{
		T swapped = 0;
		for (int i = 0; i < static_cast<int>(sizeof(T)) * 8; i += 8)
		{
			swapped |= ((value >> i) & 0xFF) << (sizeof(T)* 8 - 8 - i);
		}
		return swapped;
	}

	bool _realloc(size_t size)
	{
		if (m_pData + size <= m_pBufferEnd)
		{
			m_pEnd = m_pData + size;
			return true;
		}

		char *pNewData = new (std::nothrow) char[size * 2];
		if (pNewData == NULL)
			return false;
		size_t iOldLength = m_pEnd - m_pData;
		memcpy(pNewData, m_pData, size > iOldLength ? iOldLength : size);
		m_pPointer = m_pPointer - m_pData + pNewData;
		if (m_pBufferEnd != NULL)
			delete[] m_pData;
		m_pData = pNewData;
		m_pEnd = pNewData + size;
		m_pBufferEnd = pNewData + size * 2;
		return true;
	}

	char *m_pData, *m_pPointer, *m_pEnd, *m_pBufferEnd;
};

struct midi_token_t
{
	int iTime;
	unsigned int iBufferLength;
	const char *pBuffer;
	uint8_t    iType;
	uint8_t    iData;
};

static bool operator < (const midi_token_t& oLeft, const midi_token_t& oRight)
{
	return oLeft.iTime < oRight.iTime;
}

struct midi_token_list_t : std::vector<midi_token_t>
{
	midi_token_t* append(int iTime, uint8_t iType)
	{
		push_back(midi_token_t());
		midi_token_t* pToken = &back();
		pToken->iTime = iTime;
		pToken->iType = iType;
		return pToken;
	}
};
std::vector<unsigned char*> TranscodeXmiToMid(const unsigned char* pXmiData, size_t iXmiLength, std::vector<size_t>& midLengths)
{
    MemoryBuffer bufInput(pXmiData, iXmiLength);
    std::vector<unsigned char*> midiBuffers;

    while (bufInput.scanTo("EVNT", 4)) {  // Continue scanning for "EVNT" chunks
        if (!bufInput.skip(8)) {
            break;  // Skip if the chunk is too short
        }

        MemoryBuffer bufOutput;
        midi_token_list_t lstTokens;
        midi_token_t* pToken;
        int iTokenTime = 0;
        int iTempo = 500000;
        bool bTempoSet = false;
        bool bEnd = false;
        uint8_t iTokenType, iExtendedType;

        while (!bufInput.isEOF() && !bEnd) {
            while (true) {
                if (!bufInput.read(iTokenType))
                    break;

                if (iTokenType & 0x80)
                    break;
                else
                    iTokenTime += static_cast<int>(iTokenType) * 3;
            }
            pToken = lstTokens.append(iTokenTime, iTokenType);
            pToken->pBuffer = bufInput.getPointer() + 1;
            switch (iTokenType & 0xF0) {
                case 0xC0:
                case 0xD0:
                    if (!bufInput.read(pToken->iData))
                        break;
                    pToken->pBuffer = NULL;
                    break;
                case 0x80:
                case 0xA0:
                case 0xB0:
                case 0xE0:
                    if (!bufInput.read(pToken->iData))
                        break;
                    if (!bufInput.skip(1))
                        break;
                    break;
                case 0x90:
                    if (!bufInput.read(iExtendedType))
                        break;
                    pToken->iData = iExtendedType;
                    if (!bufInput.skip(1))
                        break;
                    pToken = lstTokens.append(iTokenTime + bufInput.readUIntVar() * 3, iTokenType);
                    pToken->iData = iExtendedType;
                    pToken->pBuffer = "\0";
                    break;
                case 0xF0:
                    iExtendedType = 0;
                    if (iTokenType == 0xFF) {
                        if (!bufInput.read(iExtendedType))
                            break;
                        if (iExtendedType == 0x2F) {
                            bEnd = true;
                        } else if (iExtendedType == 0x51) {
                            if (!bTempoSet) {
                                bufInput.skip(1);
                                iTempo = bufInput.readBigEndianUInt24() * 3;
                                bTempoSet = true;
                                bufInput.skip(-4);
                            } else {
                                lstTokens.pop_back();
                                if (!bufInput.skip(bufInput.readUIntVar()))
                                    break;
                                continue;
                            }
                        }
                    }
                    pToken->iData = iExtendedType;
                    pToken->iBufferLength = bufInput.readUIntVar();
                    pToken->pBuffer = bufInput.getPointer();
                    if (!bufInput.skip(pToken->iBufferLength))
                        break;
                    break;
            }
        }

        if (lstTokens.empty()) {
            continue;
        }

        // Write MIDI Header
        if (!bufOutput.write("MThd\0\0\0\x06\0\0\0\x01", 12))
            continue;
        if (!bufOutput.writeBigEndianUInt16((iTempo * 3) / 25000))
            continue;
        if (!bufOutput.write("MTrk\xBA\xAD\xF0\x0D", 8))
            continue;

        std::sort(lstTokens.begin(), lstTokens.end());

        iTokenTime = 0;
        iTokenType = 0;
        bEnd = false;

        // Write the MIDI tokens
        for (auto itr = lstTokens.begin(), itrEnd = lstTokens.end(); itr != itrEnd && !bEnd; ++itr) {
            if (!bufOutput.writeUIntVar(itr->iTime - iTokenTime))
                break;
            iTokenTime = itr->iTime;
            if (itr->iType >= 0xF0) {
                if (!bufOutput.write(iTokenType = itr->iType))
                    break;
                if (iTokenType == 0xFF) {
                    if (!bufOutput.write(itr->iData))
                        break;
                    if (itr->iData == 0x2F)
                        bEnd = true;
                }
                if (!bufOutput.writeUIntVar(itr->iBufferLength))
                    break;
                if (!bufOutput.write(itr->pBuffer, itr->iBufferLength))
                    break;
            } else {
                if (itr->iType != iTokenType) {
                    if (!bufOutput.write(iTokenType = itr->iType))
                        break;
                }
                if (!bufOutput.write(itr->iData))
                    break;
                if (itr->pBuffer) {
                    if (!bufOutput.write(itr->pBuffer, 1))
                        break;
                }
            }
        }

        // Finalize MIDI track length
        uint32_t iLength = static_cast<uint32_t>(bufOutput.tell() - 22);
        bufOutput.seek(18);
        bufOutput.writeBigEndianUInt32(iLength);

        // Save result for this sequence
        size_t midiLength = 0;
        unsigned char* midiData = bufOutput.takeData(&midiLength);
        midiBuffers.push_back(midiData);
        midLengths.push_back(midiLength);
    }

    return midiBuffers;
}

