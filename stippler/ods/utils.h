/*!
** \file utils.h
** \brief Utility functions for the [Open Data Structures](http://opendatastructures.org/) library
**
** ### About
** Adapted for: COMP4905A Honours Project\n
** Fall 2016\n
** Bernard Llanos\n
** Supervised by Dr. David Mould\n
** School of Computer Science, Carleton University
**
** ## Primary Basis
** This is a copy of the utils.h file from [Open Data Structures](http://opendatastructures.org/)
** - I have not modified this file, other than by adding comments.
**
** [Open Data Structures](http://opendatastructures.org/) is an open textbook
** and associated code library started by Pat Morin, distributed under
** the [Creative Commons Attribution License](https://creativecommons.org/licenses/by/2.5/ca/).
**
** The original comments at the top of the file are provided below:
**
** -----------------------------------------------------------------
**  utils.h
**
**  Created on: 2011-11-23
**      Author: morin
** -----------------------------------------------------------------
*/

#ifndef UTILS_H_
#define UTILS_H_

namespace ods {

/*!
 * \brief Find the minimum value
 * \param [in] a The first value to compare
 * \param [in] b The second value to compare
 * \return The minimum of `a` and `b`, as determined using the `<` operator
 */
template<class T> inline
T min(T a, T b) {
	return ((a)<(b) ? (a) : (b));
}

/*!
 * \brief Find the maximum value
 * \param [in] a The first value to compare
 * \param [in] b The second value to compare
 * \return The maximum of `a` and `b`, as determined using the `>` operator
 */
template<class T> inline
T max(T a, T b) {
	return ((a)>(b) ? (a) : (b));
}

/*!
 * \brief Compare two values using the `<` operator
 * \param [in] x The first value to compare
 * \param [in] y The second value to compare
 * \return A positive number, if `x < y`, a negative number, if `y < x`
 * (i.e. `x > y`) and zero if neither is true (i.e. `x == y`).
 */
template<class T> inline
int compare(T &x, T &y) {
	if (x < y) return -1;
	if (y < x) return 1;
	return 0;
}

/*!
 * \brief Compare two values using the `==` operator
 * \param [in] x The first value to compare
 * \param [in] y The second value to compare
 * \return The result of `x == y`.
 */
template<class T> inline
bool equals(T &x, T &y) {
	return x == y;
}

/*!
 * \brief Convert an integer to an unsigned integer
 * \param [in] x The integer
 * \return The integer cast to an unsigned integer type.
 */
inline
unsigned intValue(int x) {
	return (unsigned)x;
}

/**
 * This is terrible - don't use it
 */
int hashCode(int x);

/*!
 * \brief The XFastTrieNode1 class
 */
template<class T> class XFastTrieNode1;

/*!
 * \brief The hash code of an XFastTrieNode1 object
 * \param [in] u An XFastTrieNode1 object
 * \return The hash code to use for `u`
 */
template<class T>
unsigned hashCode(const XFastTrieNode1<T> *u) {
	return u->prefix;
}

/*!
 * \brief A class presumably for testing ordered data structures
 */
class dodo {
public:
    /*!
     * \brief operator <
     * \param [in] d The other object
     * \return The result of the `<` operator on the addresses of the two objects
     */
	bool operator < (dodo &d) {
		return this < &d;
	}
};

} /* namespace ods */


#endif /* UTILS_H_ */
