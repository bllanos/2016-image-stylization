/*!
** \file array.h
** \brief Definition and implementation of the Array class.
**
** ### About
** Adapted for: COMP4905A Honours Project\n
** Fall 2016\n
** Bernard Llanos\n
** Supervised by Dr. David Mould\n
** School of Computer Science, Carleton University
**
** ## Primary Basis
** This is a modified version of the Array class from [Open Data Structures](http://opendatastructures.org/)
** - I modified the class by including a second type parameter for the indices
**   of the array and the size of the array.
**
** [Open Data Structures](http://opendatastructures.org/) is an open textbook
** and associated code library started by Pat Morin, distributed under
** the [Creative Commons Attribution License](https://creativecommons.org/licenses/by/2.5/ca/).
**
** The original file comments of the Array class are provided below:
**
** -----------------------------------------------------------------
**  array.h
**
**  Created on: 2011-11-24
**      Author: morin
** -----------------------------------------------------------------
*/

#ifndef ARRAY_H_
#define ARRAY_H_
#include <iostream>
#include <algorithm>

#include <stdlib.h>
#include <assert.h>

namespace ods {

/*!
 * \brief A simple array class that simulates Java's arrays implementation - kind of
 *
 * TODO: Make a reference-counted version so that the = operator doesn't have
 * to destroy its right-hand side.
 *
 * Template parameters:
 * - `T`: The type of elements that the array is storing.
 * - `index_t`: The type used for the array indices and array size.
 */
template<class T, typename index_t>
class array {
protected:
    /*!
     * \brief The array elements
     */
	T *a;
public:
    /*!
     * \brief The length of the array
     */
    index_t length;
    /*!
     * \brief Construct an array with a given length
     * \param [in] len The length of the array
     */
    array(index_t len);
    /*!
     * \brief Construct an array with a given length and initialization value
     * \param [in] len The length of the array
     * \param [in] init An elements to copy to all positions in the array
     */
    array(index_t len, T init);
    /*!
     * \brief Overwrite the contents of the array with copies of a value
     * \param [in] x The value to copy to all positions in the array
     */
	void fill(T x);
	virtual ~array();

    /*!
     * \brief Assignment operator
     * \param [in] b The other array
     * \return A reference to this array
     */
    array<T, index_t>& operator=(array<T, index_t> &b) {
		if (a != NULL) delete[] a;
		a = b.a;
		b.a = NULL;
		length = b.length;
		return *this;
	}

    /*!
     * \brief Subscript operator
     * \param [in] i Array index
     * \return A reference to the element at the given index
     */
    T& operator[](index_t i) {
		assert(i >= 0 && i < length);
		return a[i];
	}

    /*!
     * \brief Pointer offset operator
     * \param [in] i Array index
     * \return The address of the elemnet at the given index
     */
    T* operator+(index_t i) {
		return &a[i];
	}

    /*!
     * \brief Swap two array elements
     * \param [in] i Index of the first element
     * \param [in] j Index of the second element
     */
    void swap(index_t i, index_t j) {
		T x = a[i];
		a[i] = a[j];
		a[j] = x;
	}

    /*!
     * \brief Obtain a slice of the array
     * \param [out] a0 An array overwritten with the slice
     * \param [in] a The array from which to extract a slice
     * \param [in] i The start index of the slice
     * \param [in] j The end index (exclusive) of the slice
     */
    static void copyOfRange(array<T, index_t> &a0, array<T, index_t> &a, index_t i, index_t j);
    /*!
     * \brief Reverse the contents of the array
     */
	virtual void reverse();
};

template<class T, typename index_t>
array<T, index_t>::array(index_t len) {
	length = len;
	a = new T[length];
}

template<class T, typename index_t>
array<T, index_t>::array(index_t len, T init) {
	length = len;
	a = new T[length];
    for (index_t i = 0; i < length; i++)
		a[i] = init;
}

template<class T, typename index_t>
array<T, index_t>::~array() {
	if (a != NULL) delete[] a;
}

template<class T, typename index_t>
void array<T, index_t>::reverse() {
    for (index_t i = 0; i < length/2; i++) {
		swap(i, length-i-1);
	}
}

template<class T, typename index_t>
void array<T, index_t>::copyOfRange(array<T, index_t> &a0, array<T, index_t> &a, index_t i, index_t j) {
    array<T, index_t> b(j-i);
	std::copy(a.a, a.a+j-i, b.a);
	a0 = b;
}

template<class T, typename index_t>
void array<T, index_t>::fill(T x) {
	std::fill(a, a+length, x);
}


} /* namespace ods */
#endif /* ARRAY_H_ */
