/****************************************************************************
  FileName     [ GVBitVec.cpp ]
  PackageName  [ GV/src/adt ]
  Synopsis     [ Bit-Vector Data Structure Implementation. ]
  Author       [ Cheng-Yin Wu ]
  Copyright    [ Copyright(c) 2012-2014 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef GV_BIT_VEC_C
#define GV_BIT_VEC_C

#include "gvBitVec.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "gmpxx.h"

// Constants
static const unsigned char MOD_WORD   = 0x07;
static const unsigned char IDX_MASK[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

static const unsigned char GV_BV0 = '0';
static const unsigned char GV_BV1 = '1';
static const unsigned char GV_BVX = 'X';

// Helper Function
unsigned
words(const unsigned s) {
    return (s >> 3) + ((MOD_WORD & s) ? 1 : 0);
}

/* -------------------------------------------------- *\
 * Class GVBitVec Implementations
\* -------------------------------------------------- */
GVBitVec::GVBitVec(const unsigned& s) {
    assert(s);
    set(s, true);
}

GVBitVec::GVBitVec(const char* exp) {
    assert(exp);
    set(exp);
}

GVBitVec::GVBitVec(const GVBitVec& b) {
    assert(b._size);
    set(b._size);
    memcpy(_data, b._data, sizeof(unsigned char) * words(_size));
}

GVBitVec::~GVBitVec() {
    if (_data) delete[] _data;
}

void
GVBitVec::set1(const unsigned& i) {
    _data[i >> 3] |= IDX_MASK[MOD_WORD & i];
}

void
GVBitVec::set0(const unsigned& i) {
    _data[i >> 3] &= ~IDX_MASK[MOD_WORD & i];
}

const bool
GVBitVec::all1() const {
    for (unsigned i = 0; i < _size; ++i)
        if (!(*this)[i]) return false;
    return true;
}

const bool
GVBitVec::all0() const {
    for (unsigned i = 0, j = words(_size); i < j; ++i)
        if (_data[i]) return false;
    return true;
}

const unsigned
GVBitVec::ones() const {
    unsigned j = 0;
    for (unsigned i = 0; i < _size; ++i)
        if ((*this)[i]) ++j;
    return j;
}

const unsigned
GVBitVec::value() const {
    assert(_size < (sizeof(unsigned) << 3));
    unsigned value = 0, sum = 0;
    unsigned shift = sizeof(_data[0]) << 3;
    for (int i = (int)(words(_size)) - 1; i >= 0; --i) {
        sum <<= shift;
        value = _data[i];
        sum += value;
    }
    return sum;
}

void
GVBitVec::clear() {
    for (unsigned i = 0, j = words(_size); i < j; ++i) _data[i] = 0;
}

void
GVBitVec::random() {
    const unsigned j = words(_size) - 1;
    for (unsigned i = 0; i <= j; ++i) _data[i] = rand();
    const unsigned idx = MOD_WORD & (unsigned char)_size;
    if (idx) _data[j] &= (IDX_MASK[idx] - 1);
}

const string
GVBitVec::toExp() const {
    string str = "";
    for (unsigned i = 0, j = words(_size); i < j; ++i) str += _data[i];
    return str;
}

const string
GVBitVec::regEx() const {
    unsigned i   = _size;
    string   str = "";
    while (i--) str += ((*this)[i] ? "1" : "0");
    return str;
}

void
GVBitVec::copy(GVBitVec& b) const {
    assert(_size == b.size());
    memcpy(b._data, _data, sizeof(unsigned char) * words(_size));
}

void
GVBitVec::resize(const unsigned& s, const bool& init) {
    assert(_data);
    delete[] _data;
    set(s, init);
}

const bool
GVBitVec::operator[](const unsigned& i) const {
    return (_data[i >> 3] & IDX_MASK[MOD_WORD & i]);
}

GVBitVec&
GVBitVec::operator&=(const GVBitVec& b) {
    assert(_size == b._size);
    for (unsigned i = 0, j = words(_size); i < j; ++i) _data[i] &= b._data[i];
    return *this;
}

GVBitVec&
GVBitVec::operator|=(const GVBitVec& b) {
    assert(_size == b._size);
    for (unsigned i = 0, j = words(_size); i < j; ++i) _data[i] |= b._data[i];
    return *this;
}

GVBitVec&
GVBitVec::operator=(const GVBitVec& b) {
    if (words(_size) != words(b._size)) resize(b._size, false);
    memcpy(_data, b._data, sizeof(unsigned char) * words(_size));
    _size = b._size;
    return *this;
}

const bool
GVBitVec::operator==(const GVBitVec& b) const {
    assert(_size == b._size);
    for (unsigned i = 0, j = words(_size); i < j; ++i)
        if (_data[i] != b._data[i]) return false;
    return true;
}

ostream&
operator<<(ostream& os, const GVBitVec& b) {
    unsigned i = b._size;
    os << b._size << "'b";
    while (i--) {
        os << b[i];
        if (i && (i % 8 == 0)) os << "_";
    }
    return os;
}

void
GVBitVec::set(const unsigned& s, const bool& init) {
    unsigned i = words(_size = s);
    _data      = new unsigned char[i + 1];
    if (init)
        while (i--) {
            _data[i] = 0;
        }
}

void
GVBitVec::set(const char* exp) {
    // Get width in exp
    char* src = new char[strlen(exp) + 1];
    assert(src);
    strcpy(src, exp);
    assert(src);
    assert(!strcmp(src, exp));
    size_t ptr = strcspn(src, "'");
    assert(ptr < (strlen(src)));
    src[ptr] = '\0';
    _size    = atoi(src);
    assert(_size);
    _data = new unsigned char[words(_size) + 1];
    assert(_data);
    for (unsigned i = 0; i < words(_size); ++i) _data[i] = 0;
    // Get Value in exp
    mpz_class x;
    src[ptr] = 0;
    ++ptr;
    switch (src[ptr]) {
    case 'b':
    case 'B':
        x.set_str(&src[++ptr], 2);
        break;
    case 'd':
    case 'D':
        x.set_str(&src[++ptr], 10);
        break;
    case 'h':
    case 'H':
        x.set_str(&src[++ptr], 16);
        break;
    default:
        assert(0);
        return;
    }
    // Set Value
    for (unsigned i = 0, pos = 0, w_pos = 0; i < _size; ++i) {
        assert(w_pos < words(_size));
        if (mpz_tstbit(x.get_mpz_t(), i)) _data[w_pos] |= IDX_MASK[pos];
        if (++pos < 8) continue;
        pos = 0;
        ++w_pos;
    }
    _data[words(_size)] = '\0';
}

/* -------------------------------------------------- *\
 * Class GVBitVecX Implementations
\* -------------------------------------------------- */
GVBitVecX::GVBitVecX(const unsigned& s) {
    assert(s);
    set(s, true);
}

GVBitVecX::GVBitVecX(const GVBitVec& b) {
    assert(b.size());
    set(b.size(), true);
    assert(_size == b.size());
    for (unsigned i = 0, pos = 0, w_pos = 0; i < _size; ++i) {
        assert(w_pos < words(_size));
        if (b[i]) _data1[w_pos] |= IDX_MASK[pos];
        else _data0[w_pos] |= IDX_MASK[pos];
        if (++pos < 8) continue;
        pos = 0;
        ++w_pos;
    }
}

GVBitVecX::GVBitVecX(const GVBitVecS& b) {
    unsigned s = sizeof(b._data0);
    set(s << 3);
    memcpy(_data0, &(b._data0), s);
    memcpy(_data1, &(b._data1), s);
}

GVBitVecX::GVBitVecX(const GVBitVecX& b) {
    assert(b._size);
    set(b._size);
    unsigned s = sizeof(unsigned char) * words(_size);
    memcpy(_data0, b._data0, s);
    memcpy(_data1, b._data1, s);
}

GVBitVecX::~GVBitVecX() {
    if (_data0) delete[] _data0;
    if (_data1) delete[] _data1;
}

void
GVBitVecX::set1(const unsigned& i) {
    _data0[i >> 3] &= ~IDX_MASK[MOD_WORD & i];
    _data1[i >> 3] |= IDX_MASK[MOD_WORD & i];
    assert(i < _size);
    assert(GV_BV1 == (*this)[i]);
}

void
GVBitVecX::set0(const unsigned& i) {
    _data0[i >> 3] |= IDX_MASK[MOD_WORD & i];
    _data1[i >> 3] &= ~IDX_MASK[MOD_WORD & i];
    assert(i < _size);
    assert(GV_BV0 == (*this)[i]);
}

void
GVBitVecX::setX(const unsigned& i) {
    _data0[i >> 3] &= ~IDX_MASK[MOD_WORD & i];
    _data1[i >> 3] &= ~IDX_MASK[MOD_WORD & i];
    assert(i < _size);
    assert(GV_BVX == (*this)[i]);
}

void
GVBitVecX::restrict0() {
    for (unsigned i = 0, j = words(_size); i < j; ++i) _data0[i] |= ~(_data0[i] | _data1[i]);
}

const bool
GVBitVecX::all1() const {
    for (unsigned i = 0; i < _size; ++i)
        if (GV_BV1 != (*this)[i]) return false;
    return true;
}

const bool
GVBitVecX::all0() const {
    for (unsigned i = 0; i < _size; ++i)
        if (GV_BV0 != (*this)[i]) return false;
    return true;
}

const bool
GVBitVecX::allX() const {
    for (unsigned i = 0, j = words(_size); i < j; ++i)
        if (_data0[i] || _data1[i]) return false;
    return true;
}

const bool
GVBitVecX::exist1() const {
    for (unsigned i = 0, j = words(_size); i < j; ++i)
        if (_data1[i]) return true;
    return false;
}

const bool
GVBitVecX::exist0() const {
    for (unsigned i = 0, j = words(_size); i < j; ++i)
        if (_data0[i]) return true;
    return false;
}

const unsigned
GVBitVecX::value() const {
    assert(_size == firstX());
    assert(_size < (sizeof(unsigned) << 3));
    unsigned value = 0, sum = 0;
    unsigned shift = sizeof(_data1[0]) << 3;
    for (int i = (int)(words(_size)) - 1; i >= 0; --i) {
        sum <<= shift;
        value = _data1[i];
        sum += value;
    }
    return sum;
}

const unsigned
GVBitVecX::first1() const {
    unsigned i = 0;
    for (; i < _size; ++i)
        if (GV_BV1 == (*this)[i]) break;
    return i;
}

const unsigned
GVBitVecX::first0() const {
    unsigned i = 0;
    for (; i < _size; ++i)
        if (GV_BV0 == (*this)[i]) break;
    return i;
}

const unsigned
GVBitVecX::firstX() const {
    unsigned i = 0;
    for (; i < _size; ++i)
        if (GV_BVX == (*this)[i]) break;
    return i;
}

const unsigned
GVBitVecX::dcBits() const {
    unsigned j = 0;
    for (unsigned i = 0; i < _size; ++i)
        if (GV_BVX == (*this)[i]) ++j;
    return j;
}

void
GVBitVecX::clear() {
    for (unsigned i = 0, j = words(_size); i < j; ++i) _data0[i] = _data1[i] = 0;
}

void
GVBitVecX::random() {
    const unsigned j = words(_size) - 1;
    for (unsigned i = 0; i <= j; ++i) {
        _data0[i] = rand();
        _data1[i] = ~_data0[i];
    }
    const unsigned idx = MOD_WORD & (unsigned char)_size;
    if (idx) {
        _data0[j] &= (IDX_MASK[idx] - 1);
        _data1[j] &= (IDX_MASK[idx] - 1);
    }
}

const string
GVBitVecX::toExp() const {
    string str = "";
    for (unsigned i = 0, j = words(_size); i < j; ++i) {
        str += _data0[i];
        str += _data1[i];
    }
    return str;
}

const string
GVBitVecX::regEx() const {
    unsigned i   = _size;
    string   str = "";
    while (i--) str += (*this)[i];
    return str;
}

void
GVBitVecX::resize(const unsigned& s, const bool& init) {
    assert(_data0);
    delete[] _data0;
    assert(_data1);
    delete[] _data1;
    set(s, init);
}

GVBitVecX&
GVBitVecX::operator&=(const GVBitVecX& b) {
    assert(size() == b.size());
    for (unsigned i = 0, j = words(_size); i < j; ++i) {
        _data0[i] |= b._data0[i];
        _data1[i] &= b._data1[i];
    }
    return *this;
}

GVBitVecX&
GVBitVecX::operator|=(const GVBitVecX& b) {
    assert(size() == b.size());
    for (unsigned i = 0, j = words(_size); i < j; ++i) {
        _data0[i] &= b._data0[i];
        _data1[i] |= b._data1[i];
    }
    return *this;
}

GVBitVecX&
GVBitVecX::operator^=(const GVBitVecX& b) {
    assert(size() == b.size());
    GVBitVecX temp = *this;
    assert(temp == *this);
    for (unsigned i = 0, j = words(_size); i < j; ++i) {
        _data0[i] = (temp._data0[i] & b._data0[i]) | (temp._data1[i] & b._data1[i]);
        _data1[i] = (temp._data0[i] & b._data1[i]) | (temp._data1[i] & b._data0[i]);
    }
    return *this;
}

GVBitVecX&
GVBitVecX::operator+=(const GVBitVecX& b) {
    assert(size() == b.size());
    char sum = GV_BV0, carry = GV_BV0;
    for (unsigned i = 0; i < _size; ++i) {
        sum   = op_add((*this)[i], b[i], carry);
        carry = op_carry((*this)[i], b[i], carry);
        set_value(i, sum);
    }
    return *this;
}

GVBitVecX&
GVBitVecX::operator-=(const GVBitVecX& b) {
    assert(size() == b.size());
    char sum = GV_BV0, carry = GV_BV1;
    for (unsigned i = 0; i < _size; ++i) {
        sum   = op_add((*this)[i], op_inv(b[i]), carry);
        carry = op_carry((*this)[i], op_inv(b[i]), carry);
        set_value(i, sum);
    }
    return *this;
}

GVBitVecX&
GVBitVecX::operator*=(const GVBitVecX& b) {
    assert(size() == b.size());
    char            carry[((_size * (_size + 1)) >> 1) + 1], prev_result[_size + 1];
    const GVBitVecX value(*this);
    assert(*this == value);
    // 1. AND results for corresponding ai * bj
    set_value(0, op_and(value[0], b[0]));
    unsigned index = 1;
    for (unsigned i = 0; i < _size; ++i)
        for (unsigned j = (i == 0) ? i + 1 : i; j < _size; ++j) carry[index++] = op_and(value[j - i], b[i]);
    assert(index == (_size * (_size + 1)) >> 1);
    // 2. Adders for sum_out and carry
    for (unsigned i = 1; i < _size; ++i) prev_result[i] = carry[i];
    char xor_temp, carry_temp = 0;
    index = _size;
    for (unsigned i = 1; i < _size; ++i) {
        set_value(i, op_xor(prev_result[i], carry[index]));
        if (i < _size - 1) carry_temp = op_carry(prev_result[i], carry[index], GV_BV0);
        ++index;
        for (unsigned j = i + 1; j < _size; ++j) {
            xor_temp = op_xor(carry_temp, op_xor(prev_result[j], carry[index]));
            if (j < _size - 1) carry_temp = op_carry(prev_result[j], carry[index], carry_temp);
            prev_result[j] = xor_temp;
            ++index;
        }
    }
    assert(index == (_size * (_size + 1) >> 1));
    return *this;
}

GVBitVecX&
GVBitVecX::operator/=(const GVBitVecX& b) {
    assert(_size == b._size);
    bv_divided_by(b, false);
    return *this;
}

GVBitVecX&
GVBitVecX::operator%=(const GVBitVecX& b) {
    assert(_size == b._size);
    bv_divided_by(b, true);
    return *this;
}

GVBitVecX&
GVBitVecX::operator>>=(const GVBitVecX& b) {
    bool     hasX  = false;
    unsigned shift = 0, j = 0;
    for (unsigned i = 0; i < b.size(); ++i) {
        if (b[i] == GV_BV1) {
            j = (1ul << i);
            assert(1ul == (j >> i));
            // j = (unsigned)(pow(2, (double)i));
            if (_size <= j) {
                shift = _size;
                break;
            }
            // else if (i >= sizeof(unsigned)) assert (0);
            else
                shift += j;
        } else if (b[i] == GV_BVX) hasX = true;
    }
    if (shift >= _size)
        for (unsigned i = 0; i < _size; ++i) set0(i);
    else if (shift) {
        for (unsigned i = 0, j = _size - shift; i < j; ++i) set_value(i, (*this)[shift + i]);
        for (unsigned i = _size - shift, j = _size; i < j; ++i) set0(i);
    }
    if (hasX && !all0()) {
        for (unsigned i = 0; i < b.size(); ++i) {
            if (b[i] == GV_BVX) {
                // Ex : 101010 >> X0 will be X01010
                shift = (1ul << i);
                assert(1ul == (shift >> i));
                // shift = (unsigned)(pow(2, (double)i));
                if (_size > shift)
                    for (unsigned i = 0, j = _size - shift; i < j; ++i)
                        if (!op_eq((*this)[i], (*this)[shift + i])) setX(i);
                for (unsigned i = _size - shift, j = _size; i < j; ++i)
                    if (GV_BV1 == (*this)[i]) setX(i);
                if (_size <= shift) break;
            }
        }
    }
    return *this;
}

GVBitVecX&
GVBitVecX::operator<<=(const GVBitVecX& b) {
    bool     hasX  = false;
    unsigned shift = 0, j = 0;
    for (unsigned i = 0; i < b.size(); ++i) {
        if (b[i] == GV_BV1) {
            j = (1ul << i);
            assert(1ul == (j >> i));
            // j = (unsigned)(pow(2, (double)i));
            if (_size <= j) {
                shift = _size;
                break;
            }
            // else if (i >= sizeof(unsigned)) assert (0);
            else
                shift += j;
        } else if (b[i] == GV_BVX) hasX = true;
    }
    if (shift >= _size)
        for (unsigned i = 0; i < _size; ++i) set0(i);
    else if (shift) {
        unsigned i = _size - shift;
        while (i--) set_value(shift + i, (*this)[i]);
        for (i = 0, j = shift; i < j; ++i) set0(i);
    }
    if (hasX && !all0()) {
        for (unsigned i = 0; i < b.size(); ++i) {
            if (b[i] == GV_BVX) {
                // Ex : 101010 >> X0 will be X01010
                shift = (1ul << i);
                assert(1ul == (shift >> i));
                // shift = (unsigned)(pow(2, (double)i));
                if (_size > shift) {
                    unsigned j = _size - shift;
                    while (j--)
                        if (!op_eq((*this)[j], (*this)[shift + j])) setX(shift + j);
                }
                for (unsigned i = 0, j = shift; i < j; ++i)
                    if ((i < _size) && (GV_BV1 == (*this)[i])) setX(i);
                if (_size <= shift) break;
            }
        }
    }
    return *this;
}

const bool
GVBitVecX::operator==(const GVBitVecX& b) const {
    assert(b.size() == size());
    for (unsigned i = 0, j = words(_size); i < j; ++i)
        if ((_data0[i] != b._data0[i]) || (_data1[i] != b._data1[i])) return false;
    return true;
}

const bool
GVBitVecX::operator!=(const GVBitVecX& b) const {
    return !(b == *this);
}

GVBitVecX&
GVBitVecX::operator=(const GVBitVecX& b) {
    if (words(_size) != words(b._size)) resize(b._size, false);
    unsigned s = sizeof(unsigned char) * words(_size);
    memcpy(_data0, b._data0, s);
    memcpy(_data1, b._data1, s);
    _size = b._size;
    return *this;
}

const char
GVBitVecX::operator[](const unsigned& i) const {
    if (_data0[i >> 3] & IDX_MASK[MOD_WORD & i]) return GV_BV0;
    else if (_data1[i >> 3] & IDX_MASK[MOD_WORD & i]) return GV_BV1;
    else return GV_BVX;
}

ostream&
operator<<(ostream& os, const GVBitVecX& b) {
    unsigned i = b._size;
    os << b._size << "'b";
    while (i--) {
        os << b[i];
        if (i && (i % 8 == 0)) os << "_";
    }
    return os;
}

void
GVBitVecX::copy(GVBitVecX& b) const {
    assert(_size == b.size());
    unsigned s = sizeof(unsigned char) * words(_size);
    memcpy(b._data0, _data0, s);
    memcpy(b._data1, _data1, s);
}

const bool
GVBitVecX::bv_intersect(const GVBitVecX& b) const {
    assert(size() == b.size());
    for (unsigned i = 0, j = words(_size); i < j; ++i) {
        if ((_data0[i] | b._data0[i]) & (_data1[i] | b._data1[i])) return false;
    }
    return true;
}

const bool
GVBitVecX::bv_cover(const GVBitVecX& b) const {
    assert(size() == b.size());
    for (unsigned i = 0, j = words(_size); i < j; ++i) {
        if ((_data0[i] & ~b._data0[i]) | (_data1[i] & ~b._data1[i])) return false;
    }
    return true;
}

const GVBitVecX
GVBitVecX::bv_equal(const GVBitVecX& b) const {
    GVBitVecX bv_comp(1);
    bv_comp.set_value(0, op_bv_equal(b));
    return bv_comp;
}

const GVBitVecX
GVBitVecX::bv_geq(const GVBitVecX& b) const {
    GVBitVecX bv_comp(1);
    bv_comp.set_value(0, op_bv_geq(b));
    return bv_comp;
}

const GVBitVecX
GVBitVecX::bv_leq(const GVBitVecX& b) const {
    GVBitVecX bv_comp(1);
    bv_comp.set_value(0, op_bv_leq(b));
    return bv_comp;
}

const GVBitVecX
GVBitVecX::bv_gt(const GVBitVecX& b) const {
    GVBitVecX bv_comp(1);
    bv_comp.set_value(0, op_inv(op_bv_leq(b)));
    return bv_comp;
}

const GVBitVecX
GVBitVecX::bv_lt(const GVBitVecX& b) const {
    GVBitVecX bv_comp(1);
    bv_comp.set_value(0, op_inv(op_bv_geq(b)));
    return bv_comp;
}

const GVBitVecX
GVBitVecX::operator~() const {
    GVBitVecX bv_inv(_size);
    for (unsigned i = 0, j = words(_size); i < j; ++i) {
        bv_inv._data0[i] = _data1[i];
        bv_inv._data1[i] = _data0[i];
    }
    return bv_inv;
}

const GVBitVecX
GVBitVecX::bv_red_and() const {
    GVBitVecX bv_red(1);
    bv_red.set1(0);
    bool exist_X = false;
    for (unsigned i = 0; i < _size; ++i)
        if (GV_BV0 == (*this)[i]) {
            bv_red.set0(0);
            return bv_red;
        } else if (GV_BVX == (*this)[i]) exist_X = true;
    if (exist_X) bv_red.setX(0);
    return bv_red;
}

const GVBitVecX
GVBitVecX::bv_red_or() const {
    GVBitVecX bv_red(1);
    bv_red.set0(0);
    bool exist_X = false;
    for (unsigned i = 0; i < _size; ++i)
        if (GV_BV1 == (*this)[i]) {
            bv_red.set1(0);
            return bv_red;
        } else if (GV_BVX == (*this)[i]) exist_X = true;
    if (exist_X) bv_red.setX(0);
    return bv_red;
}

const GVBitVecX
GVBitVecX::bv_red_xor() const {
    GVBitVecX bv_red(1);
    bv_red.set0(0);
    unsigned count_1 = 0;
    for (unsigned i = 0; i < _size; ++i)
        if (GV_BV1 == (*this)[i]) ++count_1;
        else if (GV_BVX == (*this)[i]) {
            bv_red.setX(0);
            return bv_red;
        }
    if (count_1 & 1ul) bv_red.set1(0);
    return bv_red;
}

const GVBitVecX
GVBitVecX::bv_slice(const unsigned& msb, const unsigned& lsb) const {
    assert(msb < _size);
    assert(lsb < _size);
    const unsigned size = 1 + (unsigned)abs((int)msb - (int)lsb);
    if (_size == size) return *this;
    GVBitVecX slice(size);
    assert(_size > size);
    if (msb >= lsb)
        for (unsigned i = 0; i < size; ++i) slice.set_value(i, (*this)[lsb + i]);
    else
        for (unsigned i = 0; i < size; ++i) slice.set_value(i, (*this)[lsb - i]);
    return slice;
}

const GVBitVecX
GVBitVecX::bv_concat(const GVBitVecX& a) const {
    GVBitVecX concat(_size + a.size());
    for (unsigned i = 0; i < a.size(); ++i) concat.set_value(i, a[i]);
    for (unsigned i = 0; i < _size; ++i) concat.set_value(a.size() + i, (*this)[i]);
    return concat;
}

const GVBitVecX
GVBitVecX::bv_select(const GVBitVecX& f, const GVBitVecX& t) const {
    assert(f.size() == t.size());
    assert(_size == 1);
    char select = (*this)[0];
    if (GV_BV1 == select) return t;
    else if (GV_BV0 == select) return f;
    GVBitVecX bv_sel = t;
    assert(t == bv_sel);
    char t_value;
    for (unsigned i = 0; i < f._size; ++i) {
        t_value = t[i];
        if (GV_BVX == t_value || !op_eq(t_value, f[i])) bv_sel.setX(i);
    }
    return bv_sel;
}

void
GVBitVecX::bv_divided_by(const GVBitVecX& b, const bool& setRemainder) {
    assert(_size == b.size());
    GVBitVecX a(*this), q(_size), r(_size);
    r.restrict0();
    unsigned i = _size, j;
    char     sum, carry;
    while (i--) {
        // Set current dividen r = {(r << 1), a[i]}
        j = _size;
        while (--j) r.set_value(j, r[j - 1]);
        r.set_value(0, a[i]);
        // Compute r - b and the final carry
        sum = GV_BV0, carry = GV_BV1;
        for (j = 0; j < _size; ++j) {
            sum   = op_add(r[j], op_inv(b[j]), carry);
            carry = op_carry(r[j], op_inv(b[j]), carry);
            set_value(j, sum);
        }
        // If carry == '1', set quotient = '1', set remainder r = *this
        // If carry == '0', set quotient = '0'
        if (GV_BV1 == carry) {
            q.set1(i);
            r = *this;
        } else if (GV_BV0 == carry) q.set0(i);
        else { // General case: r = carry ? *this : r
            for (j = 0; j < _size; ++j) {
                carry = (*this)[j];
                if (GV_BVX == carry || !op_eq(carry, r[j])) r.setX(j);
                else r.set_value(j, carry);
            }
        }
    }
    if (!setRemainder) {
        unsigned s = sizeof(unsigned char) * words(_size);
        memcpy(_data0, q._data0, s);
        memcpy(_data1, q._data1, s);
    } else {
        unsigned s = sizeof(unsigned char) * words(_size);
        memcpy(_data0, r._data0, s);
        memcpy(_data1, r._data1, s);
    }
}

void
GVBitVecX::set(const unsigned& s, const bool& init) {
    unsigned i = words(_size = s);
    _data0     = new unsigned char[i + 1];
    assert(_data0);
    _data1 = new unsigned char[i + 1];
    assert(_data1);
    if (init)
        while (i--) {
            _data0[i] = _data1[i] = 0;
        }
}

void
GVBitVecX::set_value(const unsigned& i, const char& v) {
    assert(v == GV_BV1 || v == GV_BV0 || v == GV_BVX);
    switch (v) {
    case GV_BV1:
        set1(i);
        break;
    case GV_BV0:
        set0(i);
        break;
    default:
        setX(i);
        break;
    }
}

const char
GVBitVecX::op_inv(const char& a) const {
    if (GV_BV0 == a) return GV_BV1;
    if (GV_BV1 == a) return GV_BV0;
    return GV_BVX;
}

const char
GVBitVecX::op_and(const char& a, const char& b) const {
    if (GV_BV0 == a || GV_BV0 == b) return GV_BV0;
    if (GV_BV1 == a && GV_BV1 == b) return GV_BV1;
    return GV_BVX;
}

const char
GVBitVecX::op_or(const char& a, const char& b) const {
    if (GV_BV1 == a || GV_BV1 == b) return GV_BV1;
    if (GV_BV0 == a && GV_BV0 == b) return GV_BV0;
    return GV_BVX;
}

const char
GVBitVecX::op_xor(const char& a, const char& b) const {
    return op_or(op_and(a, op_inv(b)), op_and(op_inv(a), b));
}

const char
GVBitVecX::op_add(const char& a, const char& b, const char& c) const {
    return op_xor(a, op_xor(b, c));
}

const char
GVBitVecX::op_carry(const char& a, const char& b, const char& c) const {
    return op_or(op_and(a, b), op_or(op_and(b, c), op_and(c, a)));
}

const bool
GVBitVecX::op_eq(const char& a, const char& b) const {
    return (a == b);
}

const bool
GVBitVecX::op_ge(const char& a, const char& b) const {
    if (GV_BV1 == a) return true;
    if (GV_BVX == a && GV_BV0 == b) return true;
    return false;
}

const bool
GVBitVecX::op_le(const char& a, const char& b) const {
    if (GV_BV0 == a) return true;
    if (GV_BVX == a && GV_BV1 == b) return true;
    return false;
}

const char
GVBitVecX::op_bv_equal(const GVBitVecX& b) const {
    assert(size() == b.size());
    char result = GV_BV1;
    for (unsigned i = 0; i < _size; ++i) {
        result = op_and(result, op_inv(op_xor((*this)[i], b[i])));
        if (result == GV_BV0) break;
    }
    return result;
}

const char
GVBitVecX::op_bv_geq(const GVBitVecX& b) const {
    assert(size() == b.size());
    unsigned j  = _size - 1;
    bool     eq = true, gt = false;
    char     aValue, bValue;
    for (unsigned i = 0; i < _size; ++i, --j) {
        aValue = (*this)[j], bValue = b[j];
        if (op_eq(aValue, bValue)) {
            if ((aValue != GV_BV0) && (aValue != GV_BV1)) return GV_BVX;
            else continue;
        }
        if (eq) {
            eq = false;
            if (op_ge(aValue, bValue)) {
                if ((aValue == GV_BV1) && (bValue == GV_BV0)) return GV_BV1;
                else gt = true;
            } else if (op_le(aValue, bValue)) {
                if ((aValue == GV_BV0) && (bValue == GV_BV1)) return GV_BV0;
                else gt = false;
            }
        } else if (gt) {
            if (op_ge(aValue, bValue)) {
                if ((aValue == GV_BV1) && (bValue == GV_BV0)) return GV_BV1;
            } else if (op_le(aValue, bValue)) return GV_BVX;
        } else {
            if (op_le(aValue, bValue)) {
                if ((aValue == GV_BV0) && (bValue == GV_BV1)) return GV_BV0;
            } else if (op_ge(aValue, bValue)) return GV_BVX;
        }
    }
    return ((eq || gt) ? GV_BV1 : GV_BVX);
}

const char
GVBitVecX::op_bv_leq(const GVBitVecX& b) const {
    assert(size() == b.size());
    unsigned j  = _size - 1;
    bool     eq = true, lt = false;
    char     aValue, bValue;
    for (unsigned i = 0; i < _size; ++i, --j) {
        aValue = (*this)[j], bValue = b[j];
        if (op_eq(aValue, bValue)) {
            if ((aValue != GV_BV0) && (aValue != GV_BV1)) return GV_BVX;
            else continue;
        }
        if (eq) {
            eq = false;
            if (op_le(aValue, bValue)) {
                if ((aValue == GV_BV0) && (bValue == GV_BV1)) return GV_BV1;
                else lt = true;
            } else if (op_ge(aValue, bValue)) {
                if ((aValue == GV_BV1) && (bValue == GV_BV0)) return GV_BV0;
                else lt = false;
            }
        } else if (lt) {
            if (op_le(aValue, bValue)) {
                if ((aValue == GV_BV0) && (bValue == GV_BV1)) return GV_BV1;
            } else if (op_ge(aValue, bValue)) return GV_BVX;
        } else {
            if (op_ge(aValue, bValue)) {
                if ((aValue == GV_BV1) && (bValue == GV_BV0)) return GV_BV0;
            } else if (op_le(aValue, bValue)) return GV_BVX;
        }
    }
    return ((eq || lt) ? GV_BV1 : GV_BVX);
}

/* -------------------------------------------------- *\
 * Class GVBitVecS Implementations
\* -------------------------------------------------- */
// Constructors and Destructors
GVBitVecS::GVBitVecS(const uint64_t& zeros, const uint64_t& ones) {
    _data0 = zeros;
    _data1 = ones;
}

GVBitVecS::GVBitVecS(const GVBitVecS& v) {
    _data0 = v._data0;
    _data1 = v._data1;
}

GVBitVecS::GVBitVecS(const GVBitVecX& v) {
    assert(v.size() <= 64);
    _data0 = _data1 = 0;
    memcpy(&_data0, v._data0, words(v.size()));
    memcpy(&_data1, v._data1, words(v.size()));
}

GVBitVecS::GVBitVecS() {
    clear();
}

GVBitVecS::~GVBitVecS() {}

// Operators Overloads
const char
GVBitVecS::operator[](const unsigned& i) const {
    assert(i < 64);
    return (_data0 & (1ul << i)) ? GV_BV0 : (_data1 & (1ul << i)) ? GV_BV1 : GV_BVX;
}
ostream&
operator<<(ostream& os, const GVBitVecS& v) {
    unsigned i = 64;
    while (i--) {
        os << v[i];
        if (i && (i % 8 == 0)) os << "_";
    }
    return os;
}

// Operators Overloads
GVBitVecS&
GVBitVecS::operator^=(const GVBitVecS& v) {
    const uint64_t zeroValue = (_data0 & v._data0) | (_data1 & v._data1);
    const uint64_t oneValue  = (_data0 & v._data1) | (_data1 & v._data0);
    _data0                   = zeroValue;
    _data1                   = oneValue;
    return *this;
}

// Special Operators
void
GVBitVecS::bv_and(const GVBitVecS& v1, const bool& inv1, const GVBitVecS& v2, const bool& inv2) {
    if (inv1) {
        if (inv2) {
            _data0 = v1._data1 | v2._data1;
            _data1 = v1._data0 & v2._data0;
        } else {
            _data0 = v1._data1 | v2._data0;
            _data1 = v1._data0 & v2._data1;
        }
    } else {
        if (inv2) {
            _data0 = v1._data0 | v2._data1;
            _data1 = v1._data1 & v2._data0;
        } else {
            _data0 = v1._data0 | v2._data0;
            _data1 = v1._data1 & v2._data1;
        }
    }
}

#endif
