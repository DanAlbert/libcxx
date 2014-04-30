//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#if defined(__cplusplus) && (__cplusplus >= 201103L)
#define	CXX11
#endif

#include <cassert>

#ifndef CXX11
#include <tr1/unordered_set>
#else
#include <unordered_set>
#endif
#include <iostream>

class myInt {
public:
	myInt ( int x ) : val_(x) { ++construct_count; }
	myInt (const myInt &rhs) : val_(rhs.val_) { ++copy_construct_count; }
	myInt & operator = (const myInt &rhs) { val_ = rhs.val_; ++copy_assignment_count; return *this; }
#ifdef CXX11
#if 1
	myInt (myInt &&rhs) : val_(rhs.val_) { ++move_construct_count; }
	myInt & operator = (myInt &&rhs) { val_ = rhs.val_; ++move_assignment_count;  return *this; }
#endif
#endif

	~myInt() { ++destruct_count; }

	int val_;

	static size_t construct_count;
	static size_t copy_construct_count;
	static size_t move_construct_count;
	static size_t copy_assignment_count;
	static size_t move_assignment_count;
	static size_t destruct_count;

private:
	myInt ();
	};

size_t myInt::construct_count = 0;
size_t myInt::copy_construct_count = 0;
size_t myInt::move_construct_count = 0;
size_t myInt::copy_assignment_count = 0;
size_t myInt::move_assignment_count = 0;
size_t myInt::destruct_count = 0;

#ifdef CXX11
struct myIntHash {
  size_t operator()(const myInt& x) const { return std::hash<int>()(x.val_); }
};
#else
struct myIntHash {
  size_t operator()(const myInt& x) const { return x.val_; }
};
#endif

bool operator==(const myInt &x, const myInt &y) {
	return x.val_ == y.val_;
	}

bool operator<( const myInt &x, const myInt &y) {
	return x.val_ < y.val_;
	}

void doNothing ( const myInt &xxx ) {}
void doNothingMove ( myInt &&xxx ) {}

int main()
{
#ifdef CXX11
    std::unordered_set<myInt, myIntHash> s;
#else
    std::tr1::unordered_set<myInt, myIntHash> s;
#endif

	for(int i=0;i<100 /*000*/;i++) {
		s.insert(myInt(3));
	}

	assert(s.size() == 1);
	assert(s.count(myInt(3)) == 1);
	assert(myInt::construct_count == 101);
	assert(myInt::move_construct_count == 1);
	assert(myInt::destruct_count == 101);
}
