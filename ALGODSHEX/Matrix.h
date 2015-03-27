#ifndef MATRIX_H
#define MATRIX_H

#include <array>

template <typename Object, int NumRows, int NumCols>
class matrix {
private:
    typedef std::array<Object, NumRows * NumCols> ArrayType;
    ArrayType a;
public:
    typedef typename ArrayType::const_iterator const_iterator;
    typedef typename ArrayType::iterator iterator;
    const Object& operator()(int row, int column) const {
        return a[row * NumCols + column];
    }
    Object& operator()(int row, int column) {
        return a[row * NumCols + column];
    }
    const Object& at(int row, int column) const {
        return a.at(row * NumCols + column);
    }
    Object& at(int row, int column) {
        return a.at(row * NumCols + column);
    }
    int numRows() const {
        return NumRows;
    }
    int numCols() const {
        return NumCols;
    }
    iterator begin() {
        return a.begin();
    }
    const_iterator cbegin() const {
        return a.cbegin();
    }
    iterator end() {
        return a.end();
    }
    const_iterator cend() const {
        return a.cend();
    }
};

#endif
