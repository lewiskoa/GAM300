#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <initializer_list>
#include <stdexcept>
#include <array>
#include <numeric>
#include <glm/glm.hpp> // For GLM compatibility

/**
 * @class Vector
 * @brief A templated, multi-dimensional vector class.
 *
 * This class provides a generic way to represent vectors of any dimension (N)
 * and data type (T). It implements common vector operations such as addition,
 * subtraction, scalar multiplication, dot product, and normalization.
 *
 * @tparam N The dimension of the vector.
 * @tparam T The data type of the vector's components.
 */
template<size_t N, typename T = float>
class Vector {
public:
    // The underlying data storage using a C++11 std::array for safety.
    std::array<T, N> m;

    // --- Constructors ---
    Vector();
    Vector(T fill_value);
    Vector(const std::initializer_list<T>& list);
    Vector(const Vector& other);

    // --- Operator Overloads ---
    Vector& operator=(const Vector& other);
    Vector operator+(const Vector& other) const;
    Vector& operator+=(const Vector& other);
    Vector operator-(const Vector& other) const;
    Vector& operator-=(const Vector& other);
    Vector operator-() const;
    Vector operator*(T scalar) const;
    Vector& operator*=(T scalar);
    Vector operator/(T scalar) const;
    Vector& operator/=(T scalar);
    const T& operator[](size_t index) const;
    T& operator[](size_t index);

    // --- Vector Operations ---
    T lengthSquared() const;
    T length() const;
    Vector& normalize();
    T dot(const Vector& other) const;
    
    // --- Static Functions ---
    static T distance(const Vector& a, const Vector& b);

    // Helper function for stream insertion
    friend std::ostream& operator<<(std::ostream& os, const Vector& v) {
        os << "Vector" << N << "(";
        for (size_t i = 0; i < N; ++i) {
            os << v.m[i];
            if (i < N - 1) {
                os << ", ";
            }
        }
        os << ")";
        return os;
    }
};

/**
 * @brief Global scalar multiplication operator (scalar * vector).
 * @tparam N The vector dimension.
 * @tparam T The data type.
 * @param lhs The scalar on the left-hand side.
 * @param rhs The vector on the right-hand side.
 * @return A new vector with the result.
 */
template<size_t N, typename T>
Vector<N, T> operator*(T lhs, const Vector<N, T>& rhs);

// --- Specializations for 2D, 3D, and 4D Vectors ---
struct Vector2D : public Vector<2> {
    Vector2D();
    Vector2D(float a, float b);
    Vector2D(Vector<2> const& rhs);
    
    // GLM interoperability
    operator glm::vec2() const;
    Vector2D& operator=(const glm::vec2& rhs);

    // Specific operations
    float DotProduct(const Vector2D& vec1) const noexcept;
    float CrossProductMag(const Vector2D& vec1) const;
    
    // Named accessors
    float x() const;
    float& x();
    void x(float arg);
    float y() const;
    float& y();
    void y(float arg);
};

struct Vector3D : public Vector<3> {
    Vector3D();
    Vector3D(float a, float b, float c);
    Vector3D(Vector<3> const& rhs);

    // GLM interoperability
    operator glm::vec3() const;
    Vector3D& operator=(const glm::vec3& rhs);

    // Specific operations
    float CrossProductMag(const Vector3D& rhs) const;

    // Named accessors
    float x() const;
    float& x();
    void x(float arg);
    float y() const;
    float& y();
    void y(float arg);
    float z() const;
    float& z();
    void z(float arg);
};

struct Vector4D : public Vector<4> {
    Vector4D();
    Vector4D(float x, float y, float z, float w);
    Vector4D(Vector<4> const& rhs);

    // GLM interoperability
    operator glm::vec4() const;
    Vector4D& operator=(const glm::vec4& rhs);
    
    // Named accessors
    float x() const;
    float& x();
    void x(float arg);
    float y() const;
    float& y();
    void y(float arg);
    float z() const;
    float& z();
    void z(float arg);
    float w() const;
    float& w();
    void w(float arg);
};

// Explicit template instantiations to be defined in Vector.cpp
extern template class Vector<2, float>;
extern template class Vector<3, float>;
extern template class Vector<4, float>;
extern template Vector<2, float> operator*<2, float>(float lhs, const Vector<2, float>& rhs);
extern template Vector<3, float> operator*<3, float>(float lhs, const Vector<3, float>& rhs);
extern template Vector<4, float> operator*<4, float>(float lhs, const Vector<4, float>& rhs);
extern template float Vector<2, float>::distance(const Vector<2, float>& a, const Vector<2, float>& b);
extern template float Vector<3, float>::distance(const Vector<3, float>& a, const Vector<3, float>& b);
extern template float Vector<4, float>::distance(const Vector<4, float>& a, const Vector<4, float>& b);