/*
 heap.h
 Copyright (C) 2010 Stefano Ciccarelli (step76)

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef HEAP_H
#define HEAP_H

class heap {
	private:
		struct HeapElement {
			HeapElement* pNext;
			HeapElement* pPrev;
			void* pData;
			
			HeapElement() {
				pNext = NULL;
				pPrev = NULL;
				pData = NULL;
			}
		} *pHeapHead, *pHeapTail;

	public:
		void* pop() {
			// if the heap is empty return NULL
			if (pHeapHead == NULL)
				return NULL;
			
			// save a pointer to data
			void* pData = pHeapHead->pData;
			
			if (pHeapHead == pHeapTail) {
				// if head and tail are the same, empty the heap
				delete pHeapHead;
				pHeapHead = NULL;
				pHeapTail = NULL;
			} else {
				// remove the head
				pHeapHead->pNext->pPrev = NULL;
				HeapElement* pOldHead = pHeapHead;
				pHeapHead = pHeapHead->pNext;
				delete pOldHead;
				pOldHead = NULL;
			}
			
			return pData;
		};
		
		void push(void* pData) {
			// create the new element
			HeapElement* pElement = new HeapElement;
			pElement->pData = pData;
			
			if (pHeapHead == NULL && pHeapTail == NULL) {
				// the heap is empty
				pHeapHead = pElement;
				pHeapTail = pElement;
			} else {
				// the element is the new tail
				pHeapTail->pNext = pElement;
				pElement->pPrev = pHeapTail;
				pHeapTail = pElement;
			}
		};
		
		void* front() {
			if (pHeapHead == NULL)
				return NULL;
			else
				return pHeapHead->pData;
		};
};

#endif
