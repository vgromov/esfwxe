//----------------------------------------------------------------------------
// ������ � heap by zltigo
// ��������� heap:
// {first_heap_mcb| memory part} {heap_mcb| memory part}...{heap_mcb| memory part}
//  heap_mcb - ��������� �������� ������ (Memory Comtrol Block)
//            memory part - ������� ������, ������� ����������� ��������������� MCB
//  ���� mcb.next ��������� ���������� MCB ������ ���������
//   �� ������ MCB - ����������� ���������.
//  ��������� mcb.prev ������� MCB ��������� ��� �� ����.
//----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>


#include "RTOS.h"
#include "heap_z.h"


#define USE_FULL_SCAN   1   // ������ ������� ��������� ������ � �����
              // ����� ��������� ���� ����������� �� ������� �
              // �������������.

#define HEAP_ALIGN  portBYTE_ALIGNMENT


//---------------------------------------------------------------------------
heap_t system_heap;

//----------------------------------------------------------------------------
void heapadd( heap_t *heap, heap_mcb *addr, int size );
// ��������� � 'heap' ��� ���� �������� ����������� ����.
// *heap - ��������� �� �������� ��� ��������������������� heap
//----------------------------------------------------------------------------
void heapadd( heap_t *heap, heap_mcb *xptr, int size )
{
heap_mcb *tptr = heap->freem;
    // ������������ ������ MCB � �����
  xptr->next = tptr;
    xptr->prev = tptr;
    xptr->ts.size = size-sizeof(heap_mcb);
    xptr->ts.type = MARK_FREE;
    xptr->owner = 0;
    // Reinit Primary MCB
    tptr->next = xptr;
    xptr->prev = xptr;
}

//---------------------------------------------------------------------------
// ������������� Heap - ������������ ����� ����� ��������������
// malloc_z() � free_z()
//---------------------------------------------------------------------------
void init_system_heap(void)
{
  system_heap.start = ...
  system_heap.hsize = ...
  system_heap.freem = system_heap.start;

  heapinit( &system_heap );
  heapadd( &system_heap, ...., .... );

}

//----------------------------------------------------------------------------
// �������������� 'heap'.
// *heap - ��������� �� ��������� ����������� heap
//----------------------------------------------------------------------------
void heapinit( heap_t *heap )
{
heap_mcb *fmcb;
    fmcb = heap->start;
    // ����������� ���������
    fmcb->next = fmcb;
    // ��������� �� ���������� MCB ��������� ��� �� ����
    fmcb->prev = fmcb;
    // ������ ������� ������
    fmcb->ts.size = heap->hsize - sizeof(heap_mcb);
    // ������� ������ ��������
    fmcb->ts.type = MARK_FREE;
    fmcb->owner = 0;

// ����� ������������� heap ������������ ����� ���� ��������� ����,
// ������� ����� ������ heap ����� ������ MCB.

}

//----------------------------------------------------------------------------
// malloc()
//----------------------------------------------------------------------------
void *malloc_z( heap_t *heap, size_t size, int type, void *owner )
{
  vTaskSuspendScheduler();

heap_mcb *tptr = heap->freem; // ����� ���������� � ������� ����������
#if( USE_FULL_SCAN )
heap_mcb *xptr = NULL;
#else
heap_mcb *xptr;
#endif
void *fptr;
int free_cnt = 0;

#ifdef HEAP_ALIGN
    if( size &( HEAP_ALIGN - 1 ) )
      size = size + ( HEAP_ALIGN - ( size &( HEAP_ALIGN - 1 )) );
#endif
  for( ; ; )
    {   if( tptr->ts.type == MARK_FREE )
        {
#if( USE_FULL_SCAN )
#else
      ++free_cnt;
#endif
      if( tptr->ts.size == size )          // ��������� � ��������� ������� ������ �����?
            {  tptr->owner = owner;
              tptr->ts.type = type;                  // ����������� ����
                fptr = (esU8 *)tptr+sizeof(heap_mcb);
#if( USE_FULL_SCAN )
        ++free_cnt;
#endif
        break;
      }
#if( USE_FULL_SCAN )
      else if( xptr == NULL )
      {   if( tptr->ts.size >= ( size + sizeof(heap_mcb) ) ) // ������ ���������� ��� ���������� ����� � ��� MCB?
          xptr = tptr;
          ++free_cnt;
      }
#else
            else if( tptr->ts.size >= ( size + sizeof(heap_mcb) ) ) // ������ ���������� ��� ���������� ����� � ��� MCB?
            {   // Create new free MCB in parent's MCB tail
        xptr = (heap_mcb *)( (esU8 *)tptr + sizeof(heap_mcb) + size );
                xptr->next = tptr->next;
                xptr->prev = tptr;
                xptr->ts.size = ( tptr->ts.size - size - sizeof(heap_mcb) );
                xptr->ts.type = MARK_FREE;
                // Reinit curent MCB
                tptr->next = xptr;
                tptr->ts.size = size;
                tptr->ts.type = type;     // Mark block as used
                tptr->owner = owner;
                // ���� ��������� MCB �� ���������, �� mcb.prev ���������� �� ���
                // ������ ������ ��������� �� ���������� (xptr) MCB
                if( xptr->next != heap->start )
                  ( xptr->next )->prev = xptr;
                fptr = (esU8 *)tptr + sizeof(heap_mcb);   // Valid pointer
             break;
      }
#endif
        }
        // Get ptr to next MCB
        tptr = tptr->next;
    if( tptr == heap->start )  // End of heap?
    {
#if( USE_FULL_SCAN )
      if( xptr != NULL )
      {  tptr = xptr;
        // Create new free MCB in parent's MCB tail
        xptr = (heap_mcb *)( (esU8 *)tptr + sizeof(heap_mcb) + size );
                xptr->next = tptr->next;
                xptr->prev = tptr;
                xptr->ts.size = ( tptr->ts.size - size - sizeof(heap_mcb) );
                xptr->ts.type = MARK_FREE;
                // Reinit curent MCB
                tptr->next = xptr;
                tptr->ts.size = size;
                tptr->ts.type = type;     // Mark block as used
                tptr->owner = owner;
                // ���� ��������� MCB �� ���������, �� mcb.prev ���������� �� ���
                // ������ ������ ��������� �� ���������� (xptr) MCB
                if( xptr->next != heap->start )
                  ( xptr->next )->prev = xptr;
                fptr = (esU8 *)tptr + sizeof(heap_mcb);   // Valid pointer
             break;
      }
      else
#endif
      {  fptr = NULL;       // No Memory
              break;
      }
        }
    }

   if( ( free_cnt == 1 )&&( tptr ) )  // ��� ����� ������ ��������� ���� ������?
     heap->freem = tptr->next;     // ��������� '������ ���������' �� ��������� MCB
                    // �� ��� �������� ��� �� ������� ���� ����� � ���������� ����������
    xTaskResumeScheduler();

    return( fptr );
}

//----------------------------------------------------------------------------
// free()
//----------------------------------------------------------------------------
void free_z( heap_t *heap, void *mem_ptr )
{
  vTaskSuspendScheduler();
heap_mcb *xptr;
heap_mcb *tptr = (heap_mcb *)( (esU8 *)mem_ptr - sizeof(heap_mcb) );

  // � ����� ���� �������������� _���_ :( ��������� �� ��������� � RAM, ����� ����� exception :(
  // ��� ������������ ����� ������� MCB � ���������� � mem_ptr
  // ����? ������ mem_ptr � �� �� ����� �������.
  // ������������ �������� ��� ����������� ����������
  xptr = tptr->prev;
  if( ( xptr->next != tptr )||( mem_ptr < heap->start ) )
  {   xTaskResumeScheduler();
     return;
  }
  // Valid pointer present ------------------------------------------------
    tptr->ts.type = MARK_FREE;     // Mark as "free"
  // Check Next MCB
    xptr = tptr->next;
    // ���� ��������� MCB �������� � �� ������ � heap..
  if( ( xptr->ts.type == MARK_FREE )&&( xptr != heap->start ) )
    {   // ���������� ������� (tptr) � ��������� (xptr) MCB
    tptr->ts.size = tptr->ts.size + xptr->ts.size + sizeof(heap_mcb);
    tptr->next = xptr->next;
    // ���� ��������� MCB �� ���������, �� ������ � ��� mcb.prev �� �������
    xptr = xptr->next;
        if( xptr != heap->start )
            xptr->prev = tptr;
  }
  // Check previous MCB
    xptr = tptr->prev;
    // ���� ���������� MCB �������� � ������� �� ������ � heap...
    if( ( xptr->ts.type == MARK_FREE )&&( tptr != heap->start ) )
    {  // ���������� ������� (tptr) � ���������� (xptr) MCB
     xptr->ts.size = tptr->ts.size + xptr->ts.size + sizeof(heap_mcb);
       xptr->next = tptr->next;
    // ���� ��������� MCB �� ���������, �� ������ � ��� mcb.prev �� �������
    tptr = tptr->next;
    if( tptr != heap->start )
          tptr->prev = xptr;
    tptr = xptr;      // tptr ������ �� �������������� ����.
  }
  // ��������� heap->freem ��� ����� �������� ��������
   if( tptr < heap->freem )   // ������������ ���� ��������� ����� ����������� ������ ���������?
     heap->freem = tptr;   // ����� ��������� �� ������ 'free'

  xTaskResumeScheduler();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void heaplist_z( heap_t *heap )
{

.......
}
