#include "Vector.hpp"
#include <iostream>

#define CHECK_EQ(a, b) if ((a) != (b)) { std::cout << "FAIL: " << #a << " != " << #b << std::endl; }
#define CHECK_FLOAT_EQ(a, b) if (std::abs((a) - (b)) > 0.0001f) { std::cout << "FAIL (float): " << #a << " != " << #b << std::endl; }

void TestVector2DConstruction() {
    std::cout << "Running Vector2D Construction Tests..." << std::endl;
    Vector2D vec(3.0f, 4.0f);
    CHECK_FLOAT_EQ(vec.x(), 3.0f);
    CHECK_FLOAT_EQ(vec.y(), 4.0f);
}

void TestVector2DDotProduct() {
    std::cout << "Running Vector2D Dot Product Tests..." << std::endl;
    Vector2D vec1(1.0f, 0.0f);
    Vector2D vec2(0.0f, 1.0f);
    CHECK_FLOAT_EQ(vec1.DotProduct(vec2), 0.0f);
}

// Add more functions for testing all other features.

int main() {
    TestVector2DConstruction();
    TestVector2DDotProduct();
    return 0;
}