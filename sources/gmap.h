//
// FILE: gmap.h -- Declaration of Map container type
//
// $Id$
//

#ifndef GMAP_H
#define GMAP_H

#include <assert.h>
#include <string.h>
#include "basic.h"
#include "output.h"

#ifdef __GNUC__
#define INLINE inline
#elif defined(__BORLANDC__)
#define INLINE
#else
#error Unsupported compiler type
#endif   // __GNUC__, __BORLANDC__

// What is the difference between a gMap and a gSet?
// Essentially that a gMap allows arbitrary (integer) numbering, while a gSet
// only supports sequential numbering
// Note that the gMap is an obsolescent class...  It is preferred to use
// the classes derived from gBaseMap below to implement this type of
// functionality.

template <class T> class gMap  {
  private:
    int length;
    int *numbers;
    T *contents;

  public:
	// CONSTRUCTORS AND DESTRUCTOR
	// initialize an empty gMap
    gMap(void) : length(0), contents(0), numbers(0)   { }
	// copy constructor
    gMap(const gMap<T> &);
	// clean up after a gMap
    ~gMap()   { delete [] contents;  delete [] numbers; }

	// OPERATOR OVERLOADING
	// assignment operator
    gMap<T> &operator=(const gMap<T> &);
	// returns the element with index number 'index'
    T operator[](uint index) const;
	// returns the indexth entry in the gMap
    T operator()(uint index) const;
	// determines the equality of two gMaps
    int operator==(const gMap<T> &) const;
    int operator!=(const gMap<T> &) const;

	// ADDING AND DELETING ELEMENTS
	// add a new element to the map, at the lowest available index
	//  returns the index at which the element is stored
    int Append(const T &new_member);
	// add a new element to the map at a specified index
    int Insert(const T &new_member, uint as_number);
	// remove element number 'number' from the map
	//  returns the element which was removed
    T Remove(uint number);
	// remove element from the map
    T Remove(const T &p);

	// GENERAL INFORMATION
	// returns the number of elements in the map
    uint Length(void) const   { return length; }
	// returns the number corresponding to an element in the map
	//   or zero if the element is not a member
    uint ElNumber(const T &p) const;
	// returns nonzero if the number is in use
    int Contains(int number) const;
	// returns nonzero if the element is in the map
    int Contains(const T &p) const  { return ElNumber(p); }
	// returns lowest unused index greater than zero
    int GetFirstVacancy(void) const;
	// returns index number of the ith element
    int GetIndex(int i) const;

	// DEBUGGING
    void Dump(output&) const;
};


/*****************************************************************************
 *Defining the << operator for class output from class gMap                  *
 *Arguments: class output, class gMap                                        *
 *Returns: class output                                                      *
 *****************************************************************************/

template <class T> inline output &operator<<(output& to, const gMap<T>& A)
{
  A.Dump(to); return to;
}

template <class T> INLINE gMap<T>::gMap(const gMap<T> &m) : length(m.length)
{
  numbers = new int[length];
  contents = new T[length];
  
  for (uint i = 0; i < length; i++)   {
    numbers[i] = m.numbers[i];
    contents[i] = m.contents[i];
  }
}

template <class T> INLINE gMap<T> &gMap<T>::operator=(const gMap<T> &m)
{
  if (this != &m)   {
    delete [] numbers;
    delete [] contents;

    length = m.length;
    numbers = new int[length];
    contents = new T[length];
    
    for (uint i = 0; i < length; i++)   {
      numbers[i] = m.numbers[i];
      contents[i] = m.contents[i];
    }
  }
  return *this;
}

template <class T> inline T gMap<T>::operator[](uint index) const
{
  return contents[Contains(index) - 1];
}

template <class T> inline T gMap<T>::operator()(uint index) const
{
  return contents[index - 1];
}

template <class T> INLINE int gMap<T>::operator==(const gMap<T> &m) const
{
  if (length != m.length)   return 0;

  for (int i = 0; i < length; i++)
    if (numbers[i] != m.numbers[i] || contents[i] != m.contents[i]) return 0;

  return 1;
}

template <class T> inline int gMap<T>::operator!=(const gMap<T> &m) const
{
  return !(*this == m);
}

template <class T>
INLINE int gMap<T>::Insert(const T &new_member, uint as_number)
{
  if (Contains(as_number))  {
    contents[Contains(as_number) - 1] = new_member;
    return as_number;
  }

  T *new_contents = new T[++length];
  int *new_numbers = new int[length];

  if (length == 1)   {
    new_contents[0] = new_member;
    new_numbers[0] = as_number;
    contents = new_contents;
    numbers = new_numbers;
    return as_number;
  }

  for (uint i = 0; i < length - 1 && numbers[i] < as_number; i++)  {
    new_contents[i] = contents[i];
    new_numbers[i] = numbers[i];
  }

  new_contents[i] = new_member;
  new_numbers[i] = as_number;

  for (i++; i < length; i++)  {
    new_contents[i] = contents[i - 1];
    new_numbers[i] = numbers[i - 1];
  }

  delete [] contents;
  delete [] numbers;

  contents = new_contents;
  numbers = new_numbers;

  return as_number;
}

template <class T> INLINE int gMap<T>::Append(const T &new_member)
{
  for (uint i = 1; i <= length && numbers[i - 1] == i; i++);
  Insert(new_member, i);
  return i;
}

template <class T> INLINE T gMap<T>::Remove(uint number)
{
  assert(Contains(number));

  T return_value = contents[Contains(number) - 1];

  if (length == 1)  {
    delete [] contents;
    delete [] numbers;
    length--;
    contents = 0;
    numbers = 0;
    return return_value;
  }

  T *new_contents = new T[--length];
  int *new_numbers = new int[length];

  for (uint i = 0; numbers[i] < number; i++)   {
    new_contents[i] = contents[i];
    new_numbers[i] = numbers[i];
  }

  for (; i < length; i++)  {
    new_contents[i] = contents[i + 1];
    new_numbers[i] = numbers[i + 1];
  }

  delete [] contents;
  delete [] numbers;

  contents = new_contents;
  numbers = new_numbers;

  return return_value;
}

template <class T> inline T gMap<T>::Remove(const T &p)
{
  return Remove(ElNumber(p));
}

template <class T> INLINE uint gMap<T>::ElNumber(const T &p) const
{
  for (uint i = 0; i < length; i++)
    if (contents[i] == p)  return numbers[i];
  return 0;
}

template <class T> INLINE int gMap<T>::Contains(int number) const
{
  for (uint i = 0; i < length; i++)
    if (numbers[i] == number)  return i + 1;
  return 0;
}

template <class T> INLINE int gMap<T>::GetFirstVacancy(void) const
{
  for (uint i = 0; i < length; i++)
    if (numbers[i] != i + 1)   return ++i;
  return length + 1;
}

template <class T> inline int gMap<T>::GetIndex(int i) const
{
  return numbers[i - 1];
}



template <class T> INLINE void gMap<T>::Dump(output& to) const
{
  for (uint i = 0; i < length; i++)
    to << "(" << i+1 << ") " << numbers[i] << ": " << contents[i];
  to << "\n";
}

//======================================

//
// The gBaseMap and its derived classes implement associative arrays --
// that is, arrays which are indexed not necessarily by consecutive
// integers but by some arbitrary keys.
//
// The specification and implementation is inspired in part by the AIPS
// library, but the coding is all our own.
//

//
// This is the abstract class from which all Map classes are derived
//
template <class K, class T> class gBaseMap   {
  protected:
    int length;
    T _default;
    K *keys;
    T *values;

    T &Insert(const K &key, int where, const T &value);
    T Delete(int where);

  public:
//
// This is the basic map constructor.  It initializes the map to be the
// empty map with no relations defined.
//
    gBaseMap(const T &d) : _default(d), length(0), keys(0), values(0)   { }
    gBaseMap(const gBaseMap<K, T> &);

//
// This is the map destructor.  It deletes all allocated memory, and calls
// the destructors for the keys and values which remain in the map at the
// time of its deallocation.
//
    virtual ~gBaseMap()  
      { delete [] keys;  delete [] values; }

//
// These implement the mapping function which maps a key to a value.  If
// the map from a key to a value is not defined, a mapping will be defined
// from the key to the default value.  The IsDefined() member can be used
// to determine whether a mapping is defined.
//
// <note> If the mapping is not defined for the key in the const map case,
//        the mapping returns the default value and no entry is created in
//        the map for that key.
//+grp
    virtual T &operator()(const K &key) = 0;
    virtual T operator()(const K &key) const = 0;
//-grp

//
// Returns the default value for the map
//+grp    
    T &Default(void)     { return _default; }
    const T &Default(void) const   { return _default; }
//-grp

//
// Returns the number of mappings defined in the map
//
    int Length(void) const   { return length; }

//
// Returns nonzero if the key has a mapping defined in the map
//
    virtual int IsDefined(const K &key) const = 0;

//
// These member functions implement adding and removing mapping from the map
//+grp
    virtual void Define(const K &key, const T &value) = 0;
    virtual T Remove(const K &key) = 0;
//-grp

//
// Prints out the contents of a map (for debugging purposes)
//
    void Dump(output &) const;
};

template <class K, class T> INLINE 
gBaseMap<K, T>::gBaseMap(const gBaseMap<K, T> &m) :
length(m.length), _default(m._default)
{
  keys = new K[length];
  memcpy(keys, m.keys, length * sizeof(K));

  values = new T[length];
  memcpy(values, m.values, length * sizeof(T));
}

template <class K, class T> INLINE
T &gBaseMap<K, T>::Insert(const K &key, int entry, const T &value)
{
  K *new_keys = new K[length + 1];
  T *new_values = new T[length + 1];
  
  if (length > 0)   {
    memcpy(new_keys, keys, entry * sizeof(K));
    memcpy(new_values, values, entry * sizeof(T));

    memcpy(new_keys + entry + 1, keys + entry, (length - entry) * sizeof(K));
    memcpy(new_values + entry + 1, values + entry, (length - entry) * sizeof(T));
  }

  new_keys[entry] = key;
  new_values[entry] = value;

  if (length > 0)   {
    delete keys;
    delete values;
  }

  keys = new_keys;
  values = new_values;
  length++;
  return values[entry];
}

template <class K, class T> INLINE T gBaseMap<K, T>::Delete(int where)
{
  if (length == 1)  {
    T ret = values[0];
    delete keys;
    delete values;
    keys = 0;
    values = 0;
    return ret;
  }

  T ret = values[where];
    
  K *new_keys = new K[length - 1];
  T *new_values = new T[length - 1];
    
  memcpy(new_keys, keys, where * sizeof(K));
  memcpy(new_values, values, where * sizeof(T));

  memcpy(new_keys + where, keys + where + 1,
	 (length - where - 1) * sizeof(K));
  memcpy(new_values + where, new_values + where + 1,
	 (length - where - 1) * sizeof(T));

  delete keys;
  delete values;
    
  keys = new_keys;
  values = new_values;

  length--;
  return ret;
}

template <class K, class T> INLINE void gBaseMap<K, T>::Dump(output &f) const
{
  for (int i = 0; i < length; i++)
    f << keys[i] << " --> " << values[i] << '\n';
}

  

//
// The gOrdMap is an ordered map.  That is, the index class has all the
// usual ordering operators defined upon it (==, !=, <, <=, >, >=).  These
// are used to sort the map by keys, thus making search-type operations
// logarithmic instead of linear.  This is a particularly large improvement
// when using keys which are costly to compare
//
template <class K, class T> class gOrdMap : public gBaseMap<K, T>  {
  private:
    int Locate(const K &key) const;

  public:
    gOrdMap(const T &d) : gBaseMap<K, T>(d)   { }
    gOrdMap(const gOrdMap<K, T> &m) : gBaseMap<K, T>(m)  { }

    T &operator()(const K &key);
    T operator()(const K &key) const;

    int IsDefined(const K &key) const;

    void Define(const K &key, const T &value);
    T Remove(const K &key);
};

template <class K, class T> INLINE
int gOrdMap<K, T>::Locate(const K &key) const
{
  int low = 0, high = length - 1, mid = 0;
  
  while (low <= high)   {
    mid = (low + high) / 2;
    if (key < keys[mid])     high = mid - 1;
    else if (key > keys[mid])    low = mid + 1;
    else    return mid;
  }

  return mid;
}


template <class K, class T> INLINE T &gOrdMap<K, T>::operator()(const K &key)
{
  int where = Locate(key);

  if (keys[where] == key)    return values[where];
  else return Insert(key, ((key < keys[where]) ? where : where + 1),
		    _default);
}

template <class K, class T> INLINE
T gOrdMap<K, T>::operator()(const K &key) const
{
  int where = Locate(key);

  if (keys[where] == key)    return values[where];
  else   return _default;
}

template <class K, class T> INLINE
int gOrdMap<K, T>::IsDefined(const K &key) const
{
  if (length == 0)   return 0;
  return (keys[Locate(key)] == key);
}

template <class K, class T> INLINE
void gOrdMap<K, T>::Define(const K &key, const T &value)
{
  if (length == 0)  {
    Insert(key, 0, value);
    return;
  }

  int where = Locate(key);

  if (keys[where] == key)   values[where] = value;
  else Insert(key, ((key < keys[where]) ? where : where + 1), value);
}

template <class K, class T> INLINE T gOrdMap<K, T>::Remove(const K &key)
{
  int where = Locate(key);

  if (where >= 0)    return Delete(where);
  return _default;
}

//
// The gSparseSet implements a gSet in which the elements need not be
// sequentially numbered.  It is implemented as a gOrdMap in which the
// key is an int.
//
// <note> This class implements functionality similar to the (now obsolescent)
//        gMap class
//
template <class T> class gSparseSet : public gOrdMap<int, T>  {
  public:
    gSparseSet(const T &d) : gOrdMap<int, T>(d)  { }
    gSparseSet(const gSparseSet<T> &s) : gOrdMap<int, T>(s)  { }
};


//
// The gUnordMap implements a map in which no ordering is defined on the
// key class.
//
template <class K, class T> class gUnordMap : public gBaseMap<K, T>  {
  public:
    gUnordMap(const T &d) : gBaseMap<K, T>(d)  { }
    gUnordMap(const gUnordMap<K, T> &m) : gBaseMap<K, T>(m)  { } 

    T &operator()(const K &key);
    T operator()(const K &key) const;

    int IsDefined(const K &key) const;

    void Define(const K &key, const T &value);
    T Remove(const K &key);
};

template <class K, class T> INLINE T &gUnordMap<K, T>::operator()(const K &key)
{
  for (int i = 0; i < length; i++)
    if (key == keys[i])    return values[i];
  return Insert(key, length, _default);
}

template <class K, class T> INLINE
T gUnordMap<K, T>::operator()(const K &key) const
{
  for (int i = 0; i < length; i++)
    if (key == keys[i])    return values[i];
  return _default;
}

template <class K, class T> INLINE
int gUnordMap<K, T>::IsDefined(const K &key) const
{
  for (int i = 0; i < length; i++)
    if (key == keys[i])    return 1;
  return 0;
}

template <class K, class T> inline
void gUnordMap<K, T>::Define(const K &key, const T &value)
{
  Insert(key, length, value);
}

template <class K, class T> INLINE T gUnordMap<K, T>::Remove(const K &key)
{
  for (int i = 0; i < length; i++)
    if (key == keys[i])   return Delete(i);
  return _default;
}


#endif   // GMAP_H


