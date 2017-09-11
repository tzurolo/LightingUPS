//
// UART interface
//

#include "UART_polled.h"

#include "ByteQueue.h"
#include "device.h"
#include <avr/io.h>
#include <string.h>

static ByteQueue tx_queue;
static ByteQueue rx_queue;

void UART_init ()
   {
   // initialize transmit and receive queues
   ByteQueue_init(&tx_queue);
   ByteQueue_init(&rx_queue);

   // set default baud rate
   UART_set_baud_rate(9600);

   // enable receiver and transmitter
   UCSR0B = (1<<RXEN0)|(1<<TXEN0);
   // set frame format to 8-N-1
   UCSR0C = (3<<UCSZ00);
   }

void UART_set_baud_rate (
   const uint16_t new_baud_rate)
   {
   uint16_t ubrr0 = ((F_CPU >> 4) / new_baud_rate) - 1;
   UBRR0L = (uint8_t)ubrr0;
   UBRR0H = (uint8_t)((ubrr0 >> 8) & 0x0F);
   }

void UART_update ()
   {
   // check if anything has come in from the uart
   if ((UCSR0A & (1<<RXC0)) != 0)
      ByteQueue_push(UDR0, &rx_queue);

   // check if the tranmitter is ready and there's something to write 
   char tx_byte;
   if (((UCSR0A & (1<<UDRE0)) != 0) &&     // check transmitter ready first
       ByteQueue_pop(&tx_queue, &tx_byte)) // before popping the byte
      UDR0 = tx_byte;
   }

Bool UART_read_byte (
   char *byte)
   {
   return ByteQueue_pop(&rx_queue, byte);
   }

Bool UART_write_byte (
   const char byte)
   {
   return ByteQueue_push(byte, &tx_queue);
   }

Bool UART_tx_queue_is_empty ()
   {
   return ByteQueue_is_empty(&tx_queue);
   }

Bool UART_write_string (
   const char *string)
   {
   Bool successful = false;

   // determine how much space is left in the tx queue
   uint8_t space_remaining = BYTEQUEUE_CAPACITY - ByteQueue_length(&tx_queue);
   if (strlen(string) <= space_remaining)
      {  // there is enough space in the queue
      // push all bytes onto the queue
      const char *byte_ptr = string;
      while ((*byte_ptr) != 0)
         ByteQueue_push(*byte_ptr++, &tx_queue);

      successful = true;
      }

   return successful;
   }

Bool UART_read_string (
   StringBuffer *str_buf)
   {
   Bool string_is_complete = false;

   char str_byte;
   if (UART_read_byte(&str_byte))
      {
      if (str_byte == 0x0A)
         string_is_complete = true;
      else
         StringBuffer_append_byte(str_byte, str_buf);
      }

   return string_is_complete;
   }

