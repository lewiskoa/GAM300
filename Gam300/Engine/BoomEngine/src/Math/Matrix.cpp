#include "Matrix.hpp"

// --- Constructors ---

template<size_t N, typename T>
Matrix<N, T>::Matrix() {
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            data[i][j] = static_cast<T>(0);
        }
    }
}

template<size_t N, typename T>
Matrix<N, T>::Matrix(const std::initializer_list<T>& list) {
    if (list.size() != N * N) {
        throw std::runtime_error("Initializer list size does not match matrix dimension.");
    }
    size_t row = 0;
    size_t col = 0;
    for (const auto& value : list) {
        data[row][col++] = value;
        if (col == N) {
            col = 0;
            row++;
        }
    }
}

// --- Operator Overloads ---

template<size_t N, typename T>
Matrix<N, T>& Matrix<N, T>::operator=(const Matrix& other) {
    if (this != &other) {
        data = other.data;
    }
    return *this;
}

template<size_t N, typename T>
Matrix<N, T> Matrix<N, T>::operator+(const Matrix& other) const {
    Matrix result;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            result.data[i][j] = data[i][j] + other.data[i][j];
        }
    }
    return result;
}

template<size_t N, typename T>
Matrix<N, T> Matrix<N, T>::operator-(const Matrix& other) const {
    Matrix result;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            result.data[i][j] = data[i][j] - other.data[i][j];
        }
    }
    return result;
}

template<size_t N, typename T>
Matrix<N, T> Matrix<N, T>::operator*(const Matrix& other) const {
    Matrix result;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            for (size_t k = 0; k < N; ++k) {
                result.data[i][j] += data[i][k] * other.data[k][j];
            }
        }
    }
    return result;
}

template<size_t N, typename T>
Matrix<N, T> Matrix<N, T>::operator*(T scalar) const {
    Matrix result;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            result.data[i][j] = data[i][j] * scalar;
        }
    }
    return result;
}

template<size_t N, typename T>
Vector<N, T> Matrix<N, T>::operator*(const Vector<N, T>& vec) const {
    Vector<N, T> result;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            result.data[i] += data[i][j] * vec.data[j];
        }
    }
    return result;
}

template<size_t N, typename T>
const std::array<T, N>& Matrix<N, T>::operator[](size_t row) const {
    if (row >= N) {
        throw std::out_of_range("Matrix row index out of bounds.");
    }
    return data[row];
}

template<size_t N, typename T>
std::array<T, N>& Matrix<N, T>::operator[](size_t row) {
    if (row >= N) {
        throw std::out_of_range("Matrix row index out of bounds.");
    }
    return data[row];
}

// --- Matrix Operations ---

template<size_t N, typename T>
T Matrix<N, T>::determinant() const {
    if constexpr (N == 2) {
        return data[0][0] * data[1][1] - data[0][1] * data[1][0];
    } else if constexpr (N == 3) {
        return data[0][0] * (data[1][1] * data[2][2] - data[1][2] * data[2][1])
             - data[0][1] * (data[1][0] * data[2][2] - data[1][2] * data[2][0])
             + data[0][2] * (data[1][0] * data[2][1] - data[1][1] * data[2][0]);
    } else {
        throw std::runtime_error("Determinant not implemented for this dimension.");
    }
}

template<size_t N, typename T>
Matrix<N, T> Matrix<N, T>::transposed() const {
    Matrix result;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            result.data[i][j] = data[j][i];
        }
    }
    return result;
}

template<size_t N, typename T>
Matrix<N, T> Matrix<N, T>::inverse() const {
    T det = determinant();
    if (det == static_cast<T>(0)) {
        throw std::runtime_error("Matrix is not invertible.");
    }
    T invDet = static_cast<T>(1) / det;
    Matrix result;

    if constexpr (N == 2) {
        result.data[0][0] = data[1][1] * invDet;
        result.data[0][1] = -data[0][1] * invDet;
        result.data[1][0] = -data[1][0] * invDet;
        result.data[1][1] = data[0][0] * invDet;
    } else if constexpr (N == 3) {
        result.data[0][0] = (data[1][1] * data[2][2] - data[1][2] * data[2][1]) * invDet;
        result.data[0][1] = (data[0][2] * data[2][1] - data[0][1] * data[2][2]) * invDet;
        result.data[0][2] = (data[0][1] * data[1][2] - data[0][2] * data[1][1]) * invDet;

        result.data[1][0] = (data[1][2] * data[2][0] - data[1][0] * data[2][2]) * invDet;
        result.data[1][1] = (data[0][0] * data[2][2] - data[0][2] * data[2][0]) * invDet;
        result.data[1][2] = (data[0][2] * data[1][0] - data[0][0] * data[1][2]) * invDet;

        result.data[2][0] = (data[1][0] * data[2][1] - data[1][1] * data[2][0]) * invDet;
        result.data[2][1] = (data[0][1] * data[2][0] - data[0][0] * data[2][1]) * invDet;
        result.data[2][2] = (data[0][0] * data[1][1] - data[0][1] * data[1][0]) * invDet;
    } else {
        throw std::runtime_error("Inverse not implemented for this dimension.");
    }
    
    return result;
}

// --- Static Functions ---

template<size_t N, typename T>
Matrix<N, T> Matrix<N, T>::Identity() {
    Matrix result;
    for (size_t i = 0; i < N; ++i) {
        result.data[i][i] = static_cast<T>(1);
    }
    return result;
}

// --- Global scalar multiplication operator ---

template<size_t N, typename T>
Matrix<N, T> operator*(T lhs, const Matrix<N, T>& rhs) {
    return rhs * lhs;
}

// Explicit template instantiations for common dimensions
template class Matrix<2, float>;
template class Matrix<3, float>;
template class Matrix<4, float>;
template Matrix<2, float> operator*<2, float>(float lhs, const Matrix<2, float>& rhs);
template Matrix<3, float> operator*<3, float>(float lhs, const Matrix<3, float>& rhs);
template Matrix<4, float> operator*<4, float>(float lhs, const Matrix<4, float>& rhs);
