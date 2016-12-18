/*!
** \file BinaryHeap.h
** \brief Definition and implementation of the BinaryHeap class.
**
** ### About
** Adapted for: COMP4905A Honours Project\n
** Fall 2016\n
** Bernard Llanos\n
** Supervised by Dr. David Mould\n
** School of Computer Science, Carleton University
**
** ## Primary Basis
** This is a modified version of the BinaryHeap class from [Open Data Structures](http://opendatastructures.org/)
** - I modified the class by including a second type parameter for the indices
**   of the heap and the size of the heap.
** - Additionally, I added increase-key and decrease-key functionality
**   to the heap. To provide this functionality, I added a mapping
**   between the order in which elements are inserted and their positions
**   in the heap, allowing the client to refer to, and modify, elements in the heap.
** - Some non-essential functions have been removed, such as the static sort
**   function for sorting arrays using a heap sort algorithm.
** - I converted the heap from a min-heap to a max-heap.
**
** [Open Data Structures](http://opendatastructures.org/) is an open textbook
** and associated code library started by Pat Morin, distributed under
** the [Creative Commons Attribution License](https://creativecommons.org/licenses/by/2.5/ca/).
**
** The original file comments of the BinaryHeap class are provided below:
**
** -----------------------------------------------------------------
**  BinaryHeap.h
**
**  Created on: 2011-11-30
**      Author: morin
** -----------------------------------------------------------------
*/

#ifndef BINARYHEAP_H_
#define BINARYHEAP_H_

#include <cstring>
#include "utils.h"
#include "array.h"

/*!
  \brief An index value indicating that the given item is not in the heap
 */
#define BINARYHEAP_INVALID_INDEX (-1)

namespace ods {

/*!
 * \brief An array-based binary max-heap
 *
 * Template parameters:
 * - `T`: The type of elements that the heap is storing.
 * - `index_t`: The type used for the heap indices and heap size. This should
 *   be a signed integer type.
 */
template<class T, typename index_t>
class BinaryHeap {
    // Data members
protected:
    /*!
     * \brief The heap itself
     */
    array<T, index_t> a;
    /*!
     * \brief The mapping from insertion order to heap positions
     *
     * Note that this array contains a mapping for all elements inserted over time,
     * with values of #BINARYHEAP_INVALID_INDEX for elements no longer in the heap.
     */
    array<index_t, index_t> indexToA;
    /*!
     * \brief The mapping from heap positions to insertion order
     */
    array<index_t, index_t> aToIndex;
    /*!
     * \brief The number of elements currently stored in the heap
     */
    index_t n;
    /*!
     * \brief The number of elements that have been inserted into the heap
     * since its creation
     *
     * This value monotonically increases over the lifetime of the object.
     */
    index_t nIndex;

protected:
    /*!
     * \brief Adjust the sizes of the arrays used by the heap
     * \param [in] grow If true, the BinaryHeap::indexToA array will be resized.
     * Otherwise, only the BinaryHeap::a and BinaryHeap::aToIndex arrays will
     * be resized, as they are the only arrays that can shrink in size.
     */
    void resize(bool grow = true);
    /*!
     * \brief Move an element to the correct position in the heap
     *
     * This function is used to move elements that have increased in priority.
     * \param [in] i The index of the element in the heap (BinaryHeap::a)
     */
    void bubbleUp(index_t i);
    /*!
     * \brief Move an element to the correct position in the heap
     *
     * This function is used to move elements that have decreased in priority
     * \param [in] i The index of the element in the heap (BinaryHeap::a)
     */
    void trickleDown(index_t i);
    /*!
     * \brief Find the left child of an element
     * \param [in] i The index of the element in the heap (BinaryHeap::a)
     * \return The index of the element's left child in the heap
     */
    static index_t left(index_t i) {
		return 2*i + 1;
	}
    /*!
     * \brief Find the right child of an element
     * \param [in] i The index of the element in the heap (BinaryHeap::a)
     * \return The index of the element's right child in the heap
     */
    static index_t right(index_t i) {
		return 2*i + 2;
	}
    /*!
     * \brief Find the parent of an element
     * \param [in] i The index of the element in the heap (BinaryHeap::a)
     * \return The index of the element's parent in the heap
     */
    static index_t parent(index_t i) {
		return (i-1)/2;
	}

public:
    /*!
     * \brief Construct an empty heap
     */
	BinaryHeap();
	virtual ~BinaryHeap();
    /*!
     * \brief Insert an element into the heap
     * \param [in] x The element to insert
     * \return A handle used to refer to the element later, which is equal
     * to the index of the element's insertion in the sequence of insertions
     * performed over the lifetime of the heap.
     */
    index_t add(T x);
    /*!
     * \brief Find the highest-priority element of the heap
     *
     * Note that this is a max-heap.
     * \return A reference to the highest-priority element
     */
    T& findMax() {
		return a[0];
	}
    /*!
     * \brief Extract the highest-priority element of the heap
     *
     * Note that this is a max-heap.
     *
     * After this function has been called, the item is no longer in the heap,
     * and attempting to refer to it later, using the handle returned by add(), will
     * trigger an assertion error.
     * \return A copy of the highest-priority element, which is now no longer
     * in the heap.
     */
	T remove();
    /*!
     * \brief The size of the heap
     * \return The number of elements currently in the heap
     */
    index_t size() {
		return n;
	}
    /*!
     * \brief Subscript operator
     *
     * Note that the input index is **not** the position of the element
     * in the array used internally to store heap elements. In particular,
     * `i` may be greater than the size of the heap. However, if the element
     * is no longer in the heap, an assertion error will be raised.
     * \param [in] i The handle of the heap element, returned by add()
     * \return A reference to the heap element
     */
    T& operator[](index_t i);
    /*!
     * \brief Update the heap following an increase in the priority of an element
     * \param [in] i The handle of the heap element, returned by add()
     */
    void increase(index_t i);
    /*!
     * \brief Update the heap following a decrease in the priority of an element
     * \param [in] i The handle of the heap element, returned by add()
     */
    void decrease(index_t i);
};


template<class T, typename index_t>
void BinaryHeap<T, index_t>::resize(bool grow) {
    array<T, index_t> b(max(2*n, 1));
	std::copy(a+0, a+n, b+0);
	a = b;
    array<index_t, index_t> aToIndexb(max(2*n, 1));
    std::copy(aToIndex+0, aToIndex+n, aToIndexb+0);
    aToIndex = aToIndexb;
    if(grow) {
        array<index_t, index_t> indexToAb(max(2*nIndex, 1));
        std::copy(indexToA+0, indexToA+nIndex, indexToAb+0);
        indexToA = indexToAb;
    }
}

template<class T, typename index_t>
index_t BinaryHeap<T, index_t>::add(T x) {
	if (n + 1 > a.length) resize();
    a[n++] = x;
    indexToA[nIndex] = n - 1;
    aToIndex[n - 1] = nIndex;
    bubbleUp(n - 1);
    nIndex += 1;
    return nIndex - 1;
}



template<class T, typename index_t>
void BinaryHeap<T, index_t>::bubbleUp(index_t i) {
    index_t p = parent(i);
    while (i > 0 && compare(a[i], a[p]) > 0) {
		a.swap(i,p);
        indexToA.swap(aToIndex[i], aToIndex[p]);
        aToIndex.swap(i, p);
		i = p;
		p = parent(i);
	}
}



template<class T, typename index_t>
T BinaryHeap<T, index_t>::remove() {
	T x = a[0];
	a[0] = a[--n];
    indexToA[aToIndex[n]] = 0;
    indexToA[aToIndex[0]] = BINARYHEAP_INVALID_INDEX; // No longer in heap
    aToIndex[0] = aToIndex[n];
    aToIndex[n] = BINARYHEAP_INVALID_INDEX;
	trickleDown(0);
    if (3*n < a.length) resize(false);
	return x;
}



template<class T, typename index_t>
void BinaryHeap<T, index_t>::trickleDown(index_t i) {
	do {
        index_t j = -1;
        index_t r = right(i);
        if (r < n && compare(a[r], a[i]) > 0) {
            index_t l = left(i);
            if (compare(a[l], a[r]) > 0) {
				j = l;
			} else {
				j = r;
			}
		} else {
            index_t l = left(i);
            if (l < n && compare(a[l], a[i]) > 0) {
				j = l;
			}
		}
        if (j >= 0) {
            a.swap(i, j);
            indexToA.swap(aToIndex[i], aToIndex[j]);
            aToIndex.swap(i, j);
        }
		i = j;
	} while (i >= 0);
}

template<class T, typename index_t>
T& BinaryHeap<T, index_t>::operator[](index_t i) {
    index_t heapIndex = indexToA[i];
    assert(heapIndex != BINARYHEAP_INVALID_INDEX);
    return a[heapIndex];
}

template<class T, typename index_t>
void BinaryHeap<T, index_t>::increase(index_t i) {
    index_t heapIndex = indexToA[i];
    assert(heapIndex != BINARYHEAP_INVALID_INDEX);
    bubbleUp(heapIndex);
}

template<class T, typename index_t>
void BinaryHeap<T, index_t>::decrease(index_t i) {
    index_t heapIndex = indexToA[i];
    assert(heapIndex != BINARYHEAP_INVALID_INDEX);
    trickleDown(heapIndex);
}

template<class T, typename index_t>
BinaryHeap<T, index_t>::BinaryHeap() : a(1), indexToA(1), aToIndex(1) {
	n = 0;
    nIndex = 0;
}



template<class T, typename index_t>
BinaryHeap<T, index_t>::~BinaryHeap() {
	// nothing to do
}

} /* namespace ods */
#endif /* BINARYHEAP_H_ */
