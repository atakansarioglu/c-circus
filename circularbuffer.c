/**
 * @file      circularbuffer.c
 * @author    Atakan S.
 * @date      01/01/2019
 * @version   1.0
 * @brief     Lightweight Circular Buffer implementation for ARM Cortex-M.
 *
 * @copyright Copyright (c) 2018 Atakan SARIOGLU ~ www.atakansarioglu.com
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */

#include "circularbuffer.h"
#include <string.h>
#include <assert.h>

/*
 * @brief Initializes a circular buffer object using the provided memory space.
 * @param bufferObject The buffer object handler.
 * @param bufferMemory The memory sppace for the buffer.
 * @param length_2N Size of the buffer memory, i.e. 8 indicates 2^8=256 bytes.
 */
void CircularBuffer_init(CircularBufferObject_t * const bufferObject, uint8_t * const bufferMemory, const uint8_t length_2N) {
	// Buffer check.
	assert(bufferObject);

	// Size is limited to 2^16.
	assert(length_2N <= 16);

	// Initialize the struct.
	bufferObject->memory = (uint8_t *)bufferMemory;
	bufferObject->lengthMask = (0x0001UL << length_2N) - 1;
	bufferObject->length = length_2N ? (uint32_t)bufferObject->lengthMask + 1 : 0;
	bufferObject->faultFlag = false;
	bufferObject->front = 0;
	bufferObject->back = 0;
}

/*
 * @brief Gets the size of available data in the buffer.
 * @param bufferObject The buffer object handler.
 * @return Unread data size in bytes.
 */
uint16_t CircularBuffer_getUnreadSize(const CircularBufferObject_t * const bufferObject) {
	// Buffer check.
	assert(bufferObject && bufferObject->memory);

	// Get snapshot.
	CircularBufferPointers_t cachedPointers;
	*((uint32_t *)&cachedPointers) = *((uint32_t *)bufferObject);

	// Return the difference.
	return (cachedPointers.back - cachedPointers.front) & bufferObject->lengthMask;
}

/*
 * @brief Checks for fault (i.e. data loss) and clears the buffer on request.
 * @param bufferObject The buffer object handler.
 * @param clearBuffer Set true to clear/discard the buffer.
 * @return Returns true if a fault occured and clears the fault before return.
 */
bool CircularBuffer_checkAndClearFault(CircularBufferObject_t * const bufferObject, const bool clearBuffer) {
	// Buffer check.
	assert(bufferObject && bufferObject->memory);

	// Clear the buffer.
	if(clearBuffer){
		// New front is back.
		bufferObject->front = bufferObject->back;
	}

	// Check fault.
	if (bufferObject->faultFlag) {
		// Clear the flag.
		bufferObject->faultFlag = false;

		// There was fault.
		return true;
	}

	// No fault.
	return false;
}

/*
 * @brief Push-back a byte into the buffer.
 * @param bufferObject The buffer object handler.
 * @param data The byte data to be pushed.
 * @return Returns true on success, false if no space is left.
 */
bool CircularBuffer_pushBackByte(CircularBufferObject_t * const bufferObject, const uint8_t data) {
	// Buffer check.
	assert(bufferObject && bufferObject->memory);

	// Get unread byte count.
	uint16_t unread = CircularBuffer_getUnreadSize(bufferObject);

	// Buffer space is available.
	if (unread < bufferObject->lengthMask) {
		// Write to back.
		bufferObject->memory[bufferObject->back] = data;

		// Advance the back pointer.
		bufferObject->back = (bufferObject->back + 1) & bufferObject->lengthMask;

		// Success.
		return true;
	}

	// No space left.
	else {
		// Set fault flag and result.
		bufferObject->faultFlag = true;
	}

	// Failure.
	return false;
}

/*
 * @brief Pop-front a byte from buffer .
 * @param bufferObject The buffer object handler.
 * @param data Pointer to byte to write the popped data.
 * @return Returns true if data was popped, false if no data available.
 */
bool CircularBuffer_popFrontByte(CircularBufferObject_t * const bufferObject, uint8_t * const data) {
	// Buffer check.
	assert(bufferObject && bufferObject->memory);

	// Check data availability.
	if(bufferObject->back != bufferObject->front){
		// Read from front.
		*data = bufferObject->memory[bufferObject->front];

		// Advance the back pointer.
		bufferObject->front = (bufferObject->front + 1) & bufferObject->lengthMask;

		// Success.
		return true;
	}

	// Failure.
	return false;
}

/*
 * @brief Push-back multiple data to the buffer until maxlen is reached or the buffer is full.
 * @param bufferObject The buffer object handler.
 * @param data Pointer to the data source.
 * @param maxlen Size of the source data.
 * @return Actual bytes pushed to the buffer.
 */
uint16_t CircularBuffer_pushBack(CircularBufferObject_t * const bufferObject, const uint8_t * data, const uint16_t maxlen) {
	// Buffer check.
	assert(bufferObject && bufferObject->memory);

	// Get the free size.
	uint16_t lenTotal = bufferObject->lengthMask - CircularBuffer_getUnreadSize(bufferObject);

	// Limit the total count by client buffer size.
	if(lenTotal > maxlen){
		lenTotal = maxlen;
	}

	// Actual number of bytes to write.
	uint16_t actualLen = lenTotal;

	// Copy in 1 or 2 parts [OOoooOOO] -> [oooooOOO] + [OOoooooo].
	while(lenTotal > 0){
		// Limit the transfer by the end of the buffer.
		uint16_t partialLen = lenTotal;
		if((bufferObject->length - bufferObject->back) < partialLen) {
			partialLen = bufferObject->length - bufferObject->back;
		}

		// Copy actual bytes.
		memcpy(&bufferObject->memory[bufferObject->back], data, partialLen);

		// Move the back pointer forward.
		bufferObject->back = (bufferObject->back + partialLen) & bufferObject->lengthMask;

		// Substract the read bytes.
		lenTotal -= partialLen;

		// Iterate our buffer pointer;
		data += partialLen;
	}

	// Return count of actual written bytes.
	return actualLen;
}

/*
 * @brief Pop-front multiple data from the buffer until maxlen is reached or the buffer is empty.
 * @param bufferObject The buffer object handler.
 * @param data Pointer to the output memory.
 * @param maxlen Size of the output memory.
 * @return Actual bytes popped from the buffer.
 */
uint16_t CircularBuffer_popFront(CircularBufferObject_t * const bufferObject, uint8_t * data, const uint16_t maxlen) {
	// Buffer check.
	assert(bufferObject && bufferObject->memory);

	// Get available count.
	uint16_t lenTotal = CircularBuffer_getUnreadSize(bufferObject);

	// Limit the total count by client buffer size.
	if(lenTotal > maxlen){
		lenTotal = maxlen;
	}

	// Actual number of bytes to read.
	uint16_t actualLen = lenTotal;

	// Copy in 1 or 2 parts [OOoooOOO] -> [oooooOOO] + [OOoooooo].
	while(lenTotal > 0){
		// Limit the transfer by the end of the buffer.
		uint16_t partialLen = lenTotal;
		if((bufferObject->length - bufferObject->front) < partialLen) {
			partialLen = bufferObject->length - bufferObject->front;
		}

		// Copy actual bytes.
		memcpy(data, &bufferObject->memory[bufferObject->front], partialLen);

		// Move the front pointer forward.
		bufferObject->front = (bufferObject->front + partialLen) & bufferObject->lengthMask;

		// Substract the read bytes.
		lenTotal -= partialLen;

		// Iterate our buffer pointer;
		data += partialLen;
	}

	// Return count of actual read bytes.
	return actualLen;
}
