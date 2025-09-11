#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <initializer_list>
#include <stdexcept>
#include <array>
#include "Vector.hpp"

/**
 * @class Matrix
 * @brief A templated, multi-dimensional square matrix class.
 *
 * This class provides a generic way to represent square matrices of any dimension (N)
 * and data type (T). It implements common matrix operations such as addition,
 * subtraction, multiplication, and transposition. It also includes functions for
 * calculating the determinant and inverse for 2x2 and 3x3 matrices.
 *
 * @tparam N The dimension of the square matrix (N x N).
 * @tparam T The data type of the matrix's components.
 */
template<size_t N, typename T = float>
class Matrix {
public:
    // The underlying data storage using a 2D array.
    std::array<std::array<T, N>, N> data;

    // --- Constructors ---

    /**
     * @brief Default constructor. Initializes all components to zero.
     */
    Matrix();

    /**
     * @brief Constructor that takes a list of values for initialization.
     * @param list An initializer_list of values for the matrix, row by row.
     */
    Matrix(const std::initializer_list<T>& list);

    // --- Operator Overloads ---

    /**
     * @brief Assignment operator.
     * @param other The matrix to assign from.
     * @return A reference to this matrix.
     */
    Matrix& operator=(const Matrix& other);

    /**
     * @brief Matrix addition operator.
     * @param other The matrix to add.
     * @return A new matrix with the result.
     */
    Matrix operator+(const Matrix& other) const;

    /**
     * @brief Matrix subtraction operator.
     * @param other The matrix to subtract.
     * @return A new matrix with the result.
     */
    Matrix operator-(const Matrix& other) const;

    /**
     * @brief Matrix multiplication operator (matrix * matrix).
     * @param other The matrix to multiply by.
     * @return A new matrix with the result.
     */
    Matrix operator*(const Matrix& other) const;

    /**
     * @brief Scalar multiplication operator (matrix * scalar).
     * @param scalar The scalar to multiply by.
     * @return A new matrix with the result.
     */
    Matrix operator*(T scalar) const;
    
    /**
     * @brief Matrix-vector multiplication operator (matrix * vector).
     * @param vec The vector to multiply.
     * @return A new vector with the transformed result.
     */
    Vector<N, T> operator*(const Vector<N, T>& vec) const;

    /**
     * @brief Element access operator for rows (const version).
     * @param row The row index.
     * @return A const reference to the row.
     */
    const std::array<T, N>& operator[](size_t row) const;

    /**
     * @brief Element access operator for rows.
     * @param row The row index.
     * @return A reference to the row.
     */
    std::array<T, N>& operator[](size_t row);

    /**
     * @brief Overloads the stream insertion operator for easy printing.
     * @param os The output stream.
     * @param m The matrix to print.
     * @return The output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const Matrix& m) {
        os << "Matrix" << N << "x" << N << "(\n";
        for (size_t i = 0; i < N; ++i) {
            os << "  ";
            for (size_t j = 0; j < N; ++j) {
                os << m.data[i][j] << (j < N - 1 ? ", " : "");
            }
            os << (i < N - 1 ? "\n" : "");
        }
        os << "\n)";
        return os;
    }

    // --- Matrix Operations ---

    /**
     * @brief Calculates the determinant of the matrix.
     *
     * This function is explicitly implemented for 2x2 and 3x3 matrices.
     * For other dimensions, it throws an exception.
     *
     * @return The determinant value.
     */
    T determinant() const;

    /**
     * @brief Returns a transposed copy of the matrix.
     * @return A new transposed matrix.
     */
    Matrix transposed() const;

    /**
     * @brief Returns an inverted copy of the matrix.
     *
     * This function is explicitly implemented for 2x2 and 3x3 matrices.
     * For other dimensions, it throws an exception.
     *
     * @return A new inverted matrix.
     */
    Matrix inverse() const;

    // --- Static Functions ---
    static Matrix Identity();
};

/**
 * @brief Global scalar multiplication operator (scalar * matrix).
 * @tparam N The matrix dimension.
 * @tparam T The data type.
 * @param lhs The scalar on the left-hand side.
 * @param rhs The matrix on the right-hand side.
 * @return A new matrix with the result.
 */
template<size_t N, typename T>
Matrix<N, T> operator*(T lhs, const Matrix<N, T>& rhs);

// Explicit template instantiations to be defined in Matrix.cpp
extern template class Matrix<2, float>;
extern template class Matrix<3, float>;
extern template class Matrix<4, float>;
extern template Matrix<2, float> operator*<2, float>(float lhs, const Matrix<2, float>& rhs);
extern template Matrix<3, float> operator*<3, float>(float lhs, const Matrix<3, float>& rhs);
extern template Matrix<4, float> operator*<4, float>(float lhs, const Matrix<4, float>& rhs);
