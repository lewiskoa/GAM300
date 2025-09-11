#include "Vector.hpp"
#include <numeric>
#include <cmath>
#include <stdexcept>
#include <glm/glm.hpp> // For GLM compatibility

// --- General Vector Template Implementations ---

template<size_t N, typename T>
Vector<N, T>::Vector() {
    m.fill(T());
}

template<size_t N, typename T>
Vector<N, T>::Vector(T fill_value) {
    m.fill(fill_value);
}

template<size_t N, typename T>
Vector<N, T>::Vector(const std::initializer_list<T>& list) {
    if (list.size() != N) {
        throw std::invalid_argument("Initializer list size does not match vector dimension.");
    }
    std::copy(list.begin(), list.end(), m.begin());
}

template<size_t N, typename T>
Vector<N, T>::Vector(const Vector& other) {
    m = other.m;
}

template<size_t N, typename T>
Vector<N, T>& Vector<N, T>::operator=(const Vector& other) {
    if (this != &other) {
        m = other.m;
    }
    return *this;
}

template<size_t N, typename T>
Vector<N, T> Vector<N, T>::operator+(const Vector& other) const {
    Vector result;
    for (size_t i = 0; i < N; ++i) {
        result.m[i] = m[i] + other.m[i];
    }
    return result;
}

template<size_t N, typename T>
Vector<N, T>& Vector<N, T>::operator+=(const Vector& other) {
    for (size_t i = 0; i < N; ++i) {
        m[i] += other.m[i];
    }
    return *this;
}

template<size_t N, typename T>
Vector<N, T> Vector<N, T>::operator-(const Vector& other) const {
    Vector result;
    for (size_t i = 0; i < N; ++i) {
        result.m[i] = m[i] - other.m[i];
    }
    return result;
}

template<size_t N, typename T>
Vector<N, T>& Vector<N, T>::operator-=(const Vector& other) {
    for (size_t i = 0; i < N; ++i) {
        m[i] -= other.m[i];
    }
    return *this;
}

template<size_t N, typename T>
Vector<N, T> Vector<N, T>::operator-() const {
    Vector result;
    for (size_t i = 0; i < N; ++i) {
        result.m[i] = -m[i];
    }
    return result;
}

template<size_t N, typename T>
Vector<N, T> Vector<N, T>::operator*(T scalar) const {
    Vector result;
    for (size_t i = 0; i < N; ++i) {
        result.m[i] = m[i] * scalar;
    }
    return result;
}

template<size_t N, typename T>
Vector<N, T>& Vector<N, T>::operator*=(T scalar) {
    for (size_t i = 0; i < N; ++i) {
        m[i] *= scalar;
    }
    return *this;
}

template<size_t N, typename T>
Vector<N, T> Vector<N, T>::operator/(T scalar) const {
    Vector result;
    if (scalar == 0) {
        throw std::invalid_argument("Cannot divide by zero.");
    }
    for (size_t i = 0; i < N; ++i) {
        result.m[i] = m[i] / scalar;
    }
    return result;
}

template<size_t N, typename T>
Vector<N, T>& Vector<N, T>::operator/=(T scalar) {
    if (scalar == 0) {
        throw std::invalid_argument("Cannot divide by zero.");
    }
    for (size_t i = 0; i < N; ++i) {
        m[i] /= scalar;
    }
    return *this;
}

template<size_t N, typename T>
const T& Vector<N, T>::operator[](size_t index) const {
    if (index >= N) {
        throw std::out_of_range("Vector index out of bounds.");
    }
    return m[index];
}

template<size_t N, typename T>
T& Vector<N, T>::operator[](size_t index) {
    if (index >= N) {
        throw std::out_of_range("Vector index out of bounds.");
    }
    return m[index];
}

template<size_t N, typename T>
T Vector<N, T>::lengthSquared() const {
    T sum = T();
    for (const auto& val : m) {
        sum += val * val;
    }
    return sum;
}

template<size_t N, typename T>
T Vector<N, T>::length() const {
    return std::sqrt(lengthSquared());
}

template<size_t N, typename T>
Vector<N, T>& Vector<N, T>::normalize() {
    T len = length();
    if (len > 0) {
        *this /= len;
    }
    return *this;
}

template<size_t N, typename T>
T Vector<N, T>::dot(const Vector& other) const {
    T sum = T();
    for (size_t i = 0; i < N; ++i) {
        sum += m[i] * other.m[i];
    }
    return sum;
}

template<size_t N, typename T>
T Vector<N, T>::distance(const Vector& a, const Vector& b) {
    return (a - b).length();
}

template<size_t N, typename T>
Vector<N, T> operator*(T lhs, const Vector<N, T>& rhs) {
    return rhs * lhs;
}

// --- Specializations for 2D, 3D, and 4D Vectors ---
// Implementations for the specific constructors and methods

// 2D Vector
Vector2D::Vector2D() : Vector<2>() {}
Vector2D::Vector2D(float a, float b) {
    m[0] = a;
    m[1] = b;
}
Vector2D::Vector2D(Vector<2> const& rhs) : Vector<2>{ rhs } {}
Vector2D::operator glm::vec2() const { return { m[0], m[1] }; }
Vector2D& Vector2D::operator=(const glm::vec2& rhs) {
    this->m[0] = rhs.x;
    this->m[1] = rhs.y;
    return *this;
}
float Vector2D::DotProduct(const Vector2D& vec1) const noexcept {
    return m[0] * vec1.m[0] + m[1] * vec1.m[1];
}
float Vector2D::CrossProductMag(const Vector2D& vec1) const {
    return m[0] * vec1.m[1] - m[1] * vec1.m[0];
}
float Vector2D::x() const { return m[0]; }
float Vector2D::y() const { return m[1]; }
float& Vector2D::x() { return m[0]; }
void Vector2D::x(float arg) { m[0] = arg; }
float& Vector2D::y() { return m[1]; }
void Vector2D::y(float arg) { m[1] = arg; }

// 3D Vector
Vector3D::Vector3D() : Vector<3>() {}
Vector3D::Vector3D(float a, float b, float c) {
    m[0] = a;
    m[1] = b;
    m[2] = c;
}
Vector3D::Vector3D(Vector<3> const& rhs) : Vector<3>{ rhs } {}
Vector3D::operator glm::vec3() const { return { m[0], m[1], m[2] }; }
Vector3D& Vector3D::operator=(const glm::vec3& rhs) {
    this->m[0] = rhs.x;
    this->m[1] = rhs.y;
    this->m[2] = rhs.z;
    return *this;
}
float Vector3D::CrossProductMag(const Vector3D& rhs) const {
    Vector3D temp = {
        m[1] * rhs.m[2] - m[2] * rhs.m[1],
        m[2] * rhs.m[0] - m[0] * rhs.m[2],
        m[0] * rhs.m[1] - m[1] * rhs.m[0]
    };
    return (float)sqrt(pow(temp.m[0], 2) + pow(temp.m[1], 2) + pow(temp.m[2], 2));
}
float Vector3D::x() const { return m[0]; }
float Vector3D::y() const { return m[1]; }
float Vector3D::z() const { return m[2]; }
float& Vector3D::x() { return m[0]; }
void Vector3D::x(float arg) { m[0] = arg; }
float& Vector3D::y() { return m[1]; }
void Vector3D::y(float arg) { m[1] = arg; }
float& Vector3D::z() { return m[2]; }
void Vector3D::z(float arg) { m[2] = arg; }

// 4D Vector
Vector4D::Vector4D() : Vector<4>() {}
Vector4D::Vector4D(float x, float y, float z, float w) {
    m[0] = x;
    m[1] = y;
    m[2] = z;
    m[3] = w;
}
Vector4D::Vector4D(Vector<4> const& rhs) : Vector<4>{ rhs } {}
Vector4D::operator glm::vec4() const { return { m[0], m[1], m[2], m[3] }; }
Vector4D& Vector4D::operator=(const glm::vec4& rhs) {
    this->m[0] = rhs.x;
    this->m[1] = rhs.y;
    this->m[2] = rhs.z;
    this->m[3] = rhs.w;
    return *this;
}
float Vector4D::x() const { return m[0]; }
float Vector4D::y() const { return m[1]; }
float Vector4D::z() const { return m[2]; }
float Vector4D::w() const { return m[3]; }
float& Vector4D::x() { return m[0]; }
void Vector4D::x(float arg) { m[0] = arg; }
float& Vector4D::y() { return m[1]; }
void Vector4D::y(float arg) { m[1] = arg; }
float& Vector4D::z() { return m[2]; }
void Vector4D::z(float arg) { m[2] = arg; }
float& Vector4D::w() { return m[3]; }
void Vector4D::w(float arg) { m[3] = arg; }

// --- Explicit Template Instantiations ---
template class Vector<2, float>;
template class Vector<3, float>;
template class Vector<4, float>;
template Vector<2, float> operator*<2, float>(float lhs, const Vector<2, float>& rhs);
template Vector<3, float> operator*<3, float>(float lhs, const Vector<3, float>& rhs);
template Vector<4, float> operator*<4, float>(float lhs, const Vector<4, float>& rhs);
template float Vector<2, float>::distance(const Vector<2, float>& a, const Vector<2, float>& b);
template float Vector<3, float>::distance(const Vector<3, float>& a, const Vector<3, float>& b);
template float Vector<4, float>::distance(const Vector<4, float>& a, const Vector<4, float>& b);