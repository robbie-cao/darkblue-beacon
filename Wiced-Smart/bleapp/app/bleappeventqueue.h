#ifndef _BLE_APP_EVENT_QUEUE_H_
#define _BLE_APP_EVENT_QUEUE_H_
/*******************************************************************************
* THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP 
*
* ------------------------------------------------------------------------------
*
* Copyright (c) 2011 Broadcom Corp.
*
*          ALL RIGHTS RESERVED
*
********************************************************************************
*
* File Name: blekb.h
*
* Abstract: This file implements the BLE keyboard profile ,service and application
*
* Functions:
*
*******************************************************************************/
#include "bleprofile.h"
#include "blehidevent.h"

/// Internal structure of the application event queue.
typedef struct
{
    HidEvent eventFifoOverflow;

    /// Maximum size of elements. Provided during construction.
    BYTE bleappevtq_elementSize;
    
    /// Maximum number of elements that can be placed in the queue. Provided during construction.
    BYTE bleappevtq_maxNumElements;
    
    /// Location where the queue starts. Provided during construction.
    BYTE *bleappevtq_bufStart;

    /// Number of elements currently in the queue
    BYTE bleappevtq_curNumElements;

    /// Read index into the queue
    BYTE bleappevtq_readIndex;

    /// Write index into the queue
    BYTE bleappevtq_writeIndex;
} BleAppEventQueue;

/// Initialize the application event (circular) queue. The queue will be empty upon creation.
/// Here is a sample usage:
/// \verbatim
/// #include "cfa.h"
/// #include "bleappeventqueue.j"
/// BleAppEventQueue application_event_queue;
/// void application_create(void)
///	{
///		// .... All other application initialization.
///		// Initialize the application event queue using application_event_queue as the storage area
///		// for the queue state, permanently allocate APP_EVENT_NUM_MAX * APP_EVENT_SIZE_MAX bytes
///		// for storage.
///		bleappevtq_init(&application_event_queue, cfa_mm_Sbrk(APP_EVENT_NUM_MAX * APP_EVENT_SIZE_MAX),
///						APP_EVENT_SIZE_MAX, APP_EVENT_NUM_MAX);
///	}
/// \endverbatim
/// \param theq The storage area for the queue state.
/// \param bufStart_c pointer to buffer where queue data will be stored. Must
///   have enough space to store elementSize_c*maxNumElements_c bytes
/// \param elementSize_c maximum size of each element
/// \param maxNumElements_c size of the queue, i.e. the maximum number of elements
///    that can be kept in the queue. This number must 2 or more. One of the
///    elements will be used to provide an overflow slot functionality.
void bleappevtq_init(BleAppEventQueue* theq, void *bufStart_c, BYTE elementSize_c, BYTE maxNumElements_c);

/// Discards all elements in the queue, including any elements in the overflow
/// slot.
/// \param theq The queue to operate on.
void bleappevtq_flush(BleAppEventQueue* theq);

/// Gets the number of elements currently in the queue
/// \param theq The queue to operate on.
/// \return Number of elements in the queue
BYTE bleappevtq_getCurNumElements(BleAppEventQueue* theq);

/// Puts an element into the queue. Does not perform any bound checking.
/// \param theq The queue to put this element into.
/// \param elm pointer to the element.
/// \param len number of bytes in element. This number of bytes is copied into the
///    internal storage of the queue. This must be <= the maximum element size
///    specified when the queue was constructed, otherwise the results are undefined.
/// \return TRUE always
BOOLEAN bleappevtq_put(BleAppEventQueue* theq, void *elm, BYTE len);

/// Puts an element into the queue if the queue has room for 2 or more elements, i.e.
/// it will not put an element in the overflow slot.
/// \param theq The queue to put this element into.
/// \param elm pointer to the element.
/// \param len number of bytes in element. This number of bytes is copied into the
///    internal storage of the queue. This must be <= the maximum element size
///    specified when the queue was constructed, otherwise the results are undefined.
/// \return TRUE if the element was put into the queue; FALSE otherwise
BOOLEAN bleappevtq_putExcludeOverflowSlot(BleAppEventQueue* theq, void *elm, BYTE len);

/// Puts an element into the queue as long as there is room for 1 element or more, i.e.
/// this method will fill in the overflow slot if that is the
/// last slot left. Should be used if (a) overflow slots are not needed (b)
/// for queuing an overflow event.
/// \param theq The queue to put this element into.
/// \param elm pointer to the element.
/// \param len number of bytes in element. This number of bytes is copied into the
///    internal storage of the queue. This must be <= the maximum element size
///    specified when the queue was constructed, otherwise the results are undefined.
/// \return TRUE if the element was put into the queue; FALSE otherwise
BOOLEAN bleappevtq_putIncludeOverflowSlot(BleAppEventQueue* theq, void *elm, BYTE len);

/// Returns pointer to the first element in the queue. If the queue is empty
/// returns NULL. Does not remove the element from the queue.
/// \param theq The queue to get from.
/// \return pointer to the next element in the queue if the queue is not empty
///   NULL if the queue is empty
void *bleappevtq_getCurElmPtr(BleAppEventQueue* theq);

/// Removes the current element from the queue. Does nothing if the queue is
/// empty.
/// \param theq The queue to remove from.
void bleappevtq_removeCurElement(BleAppEventQueue* theq);

/// This function queues the given event into the event fifo after adding
/// the current poll SEQN. If the event fifo is full, it queues an
/// overflow event.
/// \param theq The queue to add to.
/// \param event pointer to copy into queue. NOTE: This is a HID event type.
/// \param len length of event
/// \param pollSeqn Any application defined sequence number.
void bleappevtq_queueEventWithOverflow(BleAppEventQueue* theq, HidEvent *event, BYTE len, BYTE pollSeqn);

#endif
