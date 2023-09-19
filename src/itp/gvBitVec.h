/****************************************************************************
  FileName     [ GVBitVec.h ]
  PackageName  [ GV/src/adt ]
  Synopsis     [ Bit-Vector Data Structure Implementation. ]
  Author       [ Cheng-Yin Wu ]
  Copyright    [ Copyright(c) 2012-2014 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef GV_BIT_VEC_H
#define GV_BIT_VEC_H

#include "gvIntType.h"

#include <cstdlib>
#include <iostream>

using namespace std;

class GVBitVec
{
    public:
        // Constructors and Destructor
        GVBitVec(const unsigned& = 8);
        GVBitVec(const char*);
        GVBitVec(const GVBitVec&);
        ~GVBitVec();
        // Member Functions
        inline const unsigned size() const { return _size; }
        void                  set1(const unsigned&);
        void                  set0(const unsigned&);
        const bool            all1() const;
        const bool            all0() const;
        const unsigned        ones() const;
        const unsigned        value() const;
        void                  clear();
        void                  random();
        const string          toExp() const;
        const string          regEx() const;
        void                  copy(GVBitVec&) const;
        void                  resize(const unsigned&, const bool& = true);
        // Operator Overloads
        const bool            operator[](const unsigned&) const;
        GVBitVec&             operator&=(const GVBitVec&);
        GVBitVec&             operator|=(const GVBitVec&);
        GVBitVec&             operator=(const GVBitVec&);
        const bool            operator==(const GVBitVec&) const;
        friend ostream&       operator<<(ostream&, const GVBitVec&);

    private:
        void           set(const unsigned&, const bool& = false);
        void           set(const char*);
        unsigned       _size;
        unsigned char* _data;
};

// GVBitVec with Don't Care Supports (00 for X, 01 for 0, 10 for 1, 11 illegal)
class GVBitVecS;
class GVBitVecX
{
    public:
        friend class GVBitVecS;
        // Constructors and Destructors
        GVBitVecX(const unsigned& = 8);
        GVBitVecX(const GVBitVec&);
        GVBitVecX(const GVBitVecS&);
        GVBitVecX(const GVBitVecX&);
        ~GVBitVecX();
        // Inline Member Functions
        inline const unsigned size() const { return _size; }
        void                  set1(const unsigned&);
        void                  set0(const unsigned&);
        void                  setX(const unsigned&);
        void                  restrict0();
        const bool            all1() const;
        const bool            all0() const;
        const bool            allX() const;
        const bool            exist1() const;
        const bool            exist0() const;
        const unsigned        value() const;
        const unsigned        first1() const;
        const unsigned        first0() const;
        const unsigned        firstX() const;
        const unsigned        dcBits() const;
        void                  clear();
        void                  random();
        const string          toExp() const;
        const string          regEx() const;
        void                  copy(GVBitVecX&) const;
        void                  resize(const unsigned&, const bool& = true);
        // Logic Operators
        GVBitVecX&            operator&=(const GVBitVecX&);
        GVBitVecX&            operator|=(const GVBitVecX&);
        GVBitVecX&            operator^=(const GVBitVecX&);
        // Arithmetic Operators
        GVBitVecX&            operator+=(const GVBitVecX&);
        GVBitVecX&            operator-=(const GVBitVecX&);
        GVBitVecX&            operator*=(const GVBitVecX&);
        GVBitVecX&            operator/=(const GVBitVecX&);
        GVBitVecX&            operator%=(const GVBitVecX&);
        // Shifter Operators
        GVBitVecX&            operator>>=(const GVBitVecX&);
        GVBitVecX&            operator<<=(const GVBitVecX&);
        // Symbolic Comparative Operators
        const bool            operator==(const GVBitVecX&) const;
        const bool            operator!=(const GVBitVecX&) const;
        // Assignments
        GVBitVecX&            operator=(const GVBitVecX&);
        const char            operator[](const unsigned&) const;
        friend ostream&       operator<<(ostream&, const GVBitVecX&);
        // Ternary Comparative Operators
        const GVBitVecX       bv_equal(const GVBitVecX&) const;
        const GVBitVecX       bv_geq(const GVBitVecX&) const;
        const GVBitVecX       bv_leq(const GVBitVecX&) const;
        const GVBitVecX       bv_gt(const GVBitVecX&) const;
        const GVBitVecX       bv_lt(const GVBitVecX&) const;
        // Bit-wise Operators
        const GVBitVecX       operator~() const;
        const GVBitVecX       bv_red_and() const;
        const GVBitVecX       bv_red_or() const;
        const GVBitVecX       bv_red_xor() const;
        // Partial Manipulating Operators
        const GVBitVecX       bv_slice(const unsigned&, const unsigned&) const;
        const GVBitVecX       bv_concat(const GVBitVecX&) const;                   // {a = this, b};
        const GVBitVecX       bv_select(const GVBitVecX&, const GVBitVecX&) const; // (s = this);
        // Special Operators
        const bool            bv_intersect(const GVBitVecX&) const;
        const bool            bv_cover(const GVBitVecX&) const;

    private:
        void           set(const unsigned&, const bool& = false);
        void           set_value(const unsigned&, const char&);
        // Bit Operators
        const char     op_inv(const char&) const;
        const char     op_and(const char&, const char&) const;
        const char     op_or(const char&, const char&) const;
        const char     op_xor(const char&, const char&) const;
        const char     op_add(const char&, const char&, const char&) const;
        const char     op_carry(const char&, const char&, const char&) const;
        const bool     op_eq(const char&, const char&) const;
        const bool     op_ge(const char&, const char&) const;
        const bool     op_le(const char&, const char&) const;
        // Ternary Comparative Operators
        const char     op_bv_equal(const GVBitVecX&) const;
        const char     op_bv_geq(const GVBitVecX&) const;
        const char     op_bv_leq(const GVBitVecX&) const;
        // Divider / Modular Operators
        void           bv_divided_by(const GVBitVecX&, const bool&);
        // Data Members
        unsigned       _size;
        unsigned char* _data0;
        unsigned char* _data1;
};

// Simplified GVBitVecX for Only AND, INV Usage
class GVBitVecS
{
    public:
        friend class GVBitVecX;
        // Constructors and Destructors
        GVBitVecS(const uint64_t&, const uint64_t&);
        GVBitVecS(const GVBitVecS&);
        GVBitVecS(const GVBitVecX&);
        GVBitVecS();
        ~GVBitVecS();
        // Inline Member Functions
        inline void setZeros(const uint64_t& v) { _data0 = v; }
        inline void setOnes(const uint64_t& v) { _data1 = v; }
        inline void set0(const unsigned& i) {
            _data0 |= (1ul << i);
            _data1 &= ~(1ul << i);
        }
        inline void set1(const unsigned& i) {
            _data0 &= ~(1ul << i);
            _data1 |= (1ul << i);
        }
        inline void setX(const unsigned& i) {
            _data0 &= ~(1ul << i);
            _data1 &= ~(1ul << i);
        }
        inline void clear() { _data0 = _data1 = 0; }
        inline void random() {
            _data0 = rand();
            _data1 = ~_data0;
        }
        inline const bool      allX() const { return !_data0 && !_data1; }
        // Operators Overloads
        inline const GVBitVecS operator~() const {
            const GVBitVecS v(_data1, _data0);
            return v;
        }
        inline GVBitVecS& operator&=(const GVBitVecS& v) {
            _data0 |= v._data0;
            _data1 &= v._data1;
            return *this;
        }
        inline const bool operator==(const GVBitVecS& v) const { return (_data0 == v._data0) && (_data1 == v._data1); }
        inline const bool operator!=(const GVBitVecS& v) const { return !(*this == v); }
        const char        operator[](const unsigned&) const;
        GVBitVecS&        operator^=(const GVBitVecS&);
        friend ostream&   operator<<(ostream&, const GVBitVecS&);
        // Special Operators
        void              bv_and(const GVBitVecS&, const bool&, const GVBitVecS&, const bool&);
        inline const bool bv_cover(const GVBitVecS& v) const { return !((_data0 & ~v._data0) | (_data1 & ~v._data1)); }
        inline const bool bv_full() const {
            return (_data0 == 0ul || _data0 == ~0ul) && (_data1 == 0ul || _data1 == ~0ul);
        }

    private:
        uint64_t _data0;
        uint64_t _data1;
};

#endif
