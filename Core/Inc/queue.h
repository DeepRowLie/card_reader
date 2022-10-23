#ifndef __QUEUE_H
#define __QUEUE_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define QUEUE_OK  0
#define QUEUE_ERR 1

typedef uint32_t *array;

typedef struct
{
    uint16_t capacity;
    uint8_t  front;
    uint8_t  rear;
    array    queue;
    
}queue_handler;

void initQueue(queue_handler *ptr_queue, uint32_t *array, uint16_t MaxElements);
void makeEmpty(queue_handler *ptr_queue);
bool isEmpty(queue_handler *ptr_queue);
bool isFull(queue_handler *ptr_queue);
bool enQueue(queue_handler *ptr_queue, uint32_t element);
bool deQueue(queue_handler *ptr_queue, uint32_t *element);

#endif /*__QUEUE_H*/
