/**
 * @file      circularuart.h
 * @author    Atakan S.
 * @date      01/01/2019
 * @version   1.0
 * @brief     Full-duplex UART driver based on circular buffer.
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

// Protection.
#ifndef _H_CIRCULARUART
#define _H_CIRCULARUART

// Includes.
#include <stdio.h>

// Prototypes.
void CircularUART_Init(const uint32_t baud, const uint8_t parity);
void CircularUART_StartTx(uint8_t * const buffer, const uint8_t length_2N);
void CircularUART_StartRx(uint8_t * const buffer, const uint8_t length_2N);
void CircularUART_ClearTx(void);
void CircularUART_ClearRx(void);
uint16_t CircularUART_Send(const uint8_t * data, const uint16_t maxlen);
uint16_t CircularUART_Receive(uint8_t * data, const uint16_t maxlen);
uint16_t CircularUART_GetUnsentCount(void);
uint16_t CircularUART_GetUnreadCount(void);

#endif
