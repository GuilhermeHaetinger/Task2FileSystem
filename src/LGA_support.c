#include <stdlib.h>

#include "../include/LGA_logger.h"
#include "../include/LGA_support.h"
#include "../include/cdata.h"

/*
  Verify if exists an element that has the given tid
  in the given queue.
  Return 0 SUCCEEDED
  Return -1 FAILED
 */
int LGA_tid_inside_of_fila (PFILA2 pFila, int tid) {
  TCB_t *tcb_temp;

  if (pFila == NULL) {
    LGA_LOGGER_WARNING("[LGA_tid_inside_of_fila] The queue is empty");
    return FAILED;
  }

  if (FirstFila2(pFila) != SUCCEEDED) {
    LGA_LOGGER_WARNING("[LGA_tid_inside_of_fila] Couldnt set the first element");
    return FAILED;
  }

  tcb_temp = (TCB_t *) GetAtIteratorFila2(pFila);

  if (tcb_temp == NULL) {
    LGA_LOGGER_ERROR("[LGA_tid_inside_of_fila] The tcb is NULL");
    return FAILED;
  }

  if (tcb_temp->tid == tid) {
    LGA_LOGGER_LOG("[LGA_tid_inside_of_fila] The tid was found");
    return SUCCEEDED;
  }
  while(NextFila2(pFila) == SUCCEEDED) {
    tcb_temp = (TCB_t *) GetAtIteratorFila2(pFila);

    if (tcb_temp->tid == tid) {
      LGA_LOGGER_LOG("[LGA_tid_inside_of_fila] The tid was found");
      return SUCCEEDED;
    }
  }
  LGA_LOGGER_WARNING("[LGA_tid_inside_of_fila] The tid wasnt found");
  return FAILED;
}

/*
  Remove from the queue the element that has the given tid
  Return 0 - SUCCEEDED
  Return -1 - FAILED
 */
int LGA_tid_remove_from_fila (PFILA2 pFila, int tid) {
  TCB_t *tcb_temp;

  if (LGA_tid_inside_of_fila(pFila, tid) == SUCCEEDED) {
    if (FirstFila2(pFila) != SUCCEEDED) {
      LGA_LOGGER_ERROR("[LGA_tid_remove_from_fila] Couldnt set the first element");
      return FAILED;
    }

    tcb_temp = (TCB_t *) GetAtIteratorFila2(pFila);
    if (tcb_temp->tid == tid) {
      LGA_LOGGER_LOG("[LGA_tid_remove_from_fila] The tid was found");
      if (DeleteAtIteratorFila2(pFila) == SUCCEEDED) {
        LGA_LOGGER_LOG("[LGA_tid_remove_from_fila] The tid was removed");
        return SUCCEEDED;
      } else {
        LGA_LOGGER_ERROR("[LGA_tid_remove_from_fila] The tid wasnt removed");
        return FAILED;
      }
    }
    while(NextFila2(pFila) == SUCCEEDED) {
      tcb_temp = (TCB_t *) GetAtIteratorFila2(pFila);

      if (tcb_temp->tid == tid) {
        LGA_LOGGER_LOG("[LGA_tid_remove_from_fila] The tid was found");
        if (DeleteAtIteratorFila2(pFila) == SUCCEEDED) {
          LGA_LOGGER_LOG("[LGA_tid_remove_from_fila] The tid was removed");
          return SUCCEEDED;
        } else {
          LGA_LOGGER_ERROR("[LGA_tid_remove_from_fila] The tid wasnt removed");
          return FAILED;
        }
      }
    }
    return FAILED;
  }
}

/*
  Get the TCB referation from the queue
  You must cast to (TCB_t *) before use it
  Return Valid Pointer - SUCCEEDED
  Return NULL - FAILED
 */
void* LGA_tid_get_from_fila (PFILA2 pFila, int tid) {
  TCB_t *tcb_temp;

  if (LGA_tid_inside_of_fila(pFila, tid) != SUCCEEDED) {
    LGA_LOGGER_WARNING("[LGA_tid_inside_of_fila] The element is not inside the queue");
    return NULL;
  }

  if (FirstFila2(pFila) != SUCCEEDED) {
    LGA_LOGGER_ERROR("[LGA_tid_inside_of_fila] Cant set the first element");
    return NULL;
  }

  tcb_temp = (TCB_t *) GetAtIteratorFila2(pFila);

  if (tcb_temp->tid == tid) {
    LGA_LOGGER_LOG("[LGA_tid_inside_of_fila] Get the element of the queue");
    return (void *) tcb_temp;
  }

  while(NextFila2(pFila) == SUCCEEDED) {
    tcb_temp = (TCB_t *) GetAtIteratorFila2(pFila);

    if (tcb_temp->tid == tid) {
      LGA_LOGGER_LOG("[LGA_tid_inside_of_fila] Get the element of the queue");
      return (void *) tcb_temp;
    }
  }
  return NULL;
}
