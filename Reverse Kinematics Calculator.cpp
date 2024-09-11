#include <iostream>
#include <complex>
#include <cmath>
#include <random>
#include <chrono>

uint64_t getMills() {
    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

struct Vec2 {
    float x;
    float y;

    Vec2()
        : x(0.f), y(0.f) { }
    Vec2(float X, float Y)
        : x(X), y(Y) { }

    float MagSq() {
        return x * x + y * y;
    }

    float Mag() {
        return std::sqrt(MagSq());
    }

    Vec2 Norm() {
        float magInv = 1.f / Mag();
        return Vec2(x * magInv, y * magInv);
    }

    static Vec2 FromAng(float Ang, float Mag) {
        float x = std::cos(Ang);
        float y = std::sin(Ang);

        return Vec2(x * Mag, y * Mag);
    }

    Vec2 operator+(const Vec2& Other) const {
        return Vec2(x + Other.x, y + Other.y);
    }
    Vec2& operator+=(const Vec2& Other) {
        x += Other.x;
        y += Other.y;
        return *this;
    }
    Vec2 operator-(const Vec2& Other) const {
        return Vec2(x - Other.x, y - Other.y);
    }
    Vec2& operator-=(const Vec2& Other) {
        x -= Other.x;
        y -= Other.y;
        return *this;
    }

    Vec2 operator*(float Other) const {
        return Vec2(x * Other, y * Other);
    }
    Vec2& operator*=(float Other) {
        x *= Other;
        y *= Other;
        return *this;
    }
    Vec2 operator/(float Other) const {
        return Vec2(x / Other, y / Other);
    }
    Vec2& operator/=(float Other) {
        x /= Other;
        y /= Other;
        return *this;
    }
};

enum EndType {
    endTypeIteration,
    endTypePrecision
};

void getAngles(Vec2* Positions, float* Angles, uint64_t LenCount) {
    Vec2 currentPos;
    float currentAngle = 0.f;
    for (uint64_t i = 0; i < LenCount; ++i) {
        Vec2 newPos = Positions[i];
        Vec2 diff = newPos - currentPos;
        float newAngle = std::atan2(diff.y, diff.x);

        Angles[i] = newAngle - currentAngle;

        currentAngle = newAngle;
        currentPos = newPos;
    }
}

int main() {
    uint64_t lenCount;
    std::cout << "How many lengths does your arm have? ";
    std::cin >> lenCount;

    if (!lenCount) {
        std::cout << "Well, I need at least 1!\n";
        return 1;
    }
    uint64_t pointCount = lenCount - 1;

    float* lens = new float[lenCount];
    std::cout << '\n'
        << "Please enter the lengths below:\n";
    for (uint64_t i = 0; i < lenCount; ++i) {
        std::cout << i << ": ";
        std::cin >> lens[i];
    }

    char existPosResponse;
    bool existingPos;
    std::cout << '\n'
        << "Would you like to start from a position (y/n)? ";
    std::cin >> existPosResponse;
    if (existPosResponse == 'y') existingPos = true;
    else if (existPosResponse == 'n') existingPos = false;
    else {
        std::cout << "Invalid response!\n";
        return 1;
    }

    float* angles = new float[lenCount];

    if (existingPos) {
        angles = new float[lenCount];
        std::cout << "Please enter angles in radians:\n";
        for (int i = 0; i < lenCount; ++i) {
            std::cout << i << ": ";
            std::cin >> angles[i];
        }
    }

    float posX;
    float posY;
    std::cout << '\n'
        << "Please enter the position you want achieved:\n"
        << "  X: ";
    std::cin >> posX;
    std::cout << "  Y: ";
    std::cin >> posY;
    Vec2 pos(posX, posY);

    float lenSum = 0.f;
    for (uint64_t i = 0; i < lenCount; ++i)
        lenSum += lens[i];
    if (lenSum * lenSum < pos.MagSq()) {
        std::cout << '\n'
            << "That is impossible.\n";
        return 1;
    }

    auto* positions = new Vec2[lenCount];
    std::ranlux48 rl48(getMills());
    std::uniform_real_distribution<float> disN(-0.0001f, 0.0001f);
    if (existingPos) {
        Vec2 currentPos;
        float currentAngle = 0.f;
        for (uint64_t i = 0; i < lenCount; ++i) {
            currentAngle += angles[i];
            currentPos += Vec2::FromAng(currentAngle, lens[i]);
            Vec2 newPos = currentPos;

            newPos.x += disN(rl48);
            newPos.y += disN(rl48);

            positions[i] = newPos;
        }
    }
    else {
        float invLenCount = 1.f / lenCount;
        for (uint64_t i = 0; i < pointCount; ++i) {
            float s = (i + 1) * invLenCount;
            Vec2 newPos = pos * s;

            newPos.x += disN(rl48);
            newPos.y += disN(rl48);

            positions[i] = newPos;
        }
        positions[pointCount] = pos;
    }

    uint64_t iterationCount;
    std::cout << '\n'
        << "For how many iterations would you like to iterate the process for? ";
    std::cin >> iterationCount;

    float precision;
    std::cout << '\n'
        << "Below what rate of change would you like to terminate prematurely (put 0 if inapplicable)? ";
    std::cin >> precision;
    if (precision < 0.f) precision = 0.f;

    std::cout << '\n'
        << "The starting positions are:\n";
    for (uint64_t i = 0; i < lenCount; ++i) {
        const Vec2& currentPos = positions[i];
        std::cout << currentPos.x << ' ' << currentPos.y << '\n';
    }

    if (!existingPos) getAngles(positions, angles, lenCount);
    std::cout << '\n'
        << "The starting angles are:\n";
    for (uint64_t i = 0; i < lenCount; ++i) {
        float angle = angles[i];
        std::cout << angle << " rad\n";
    }

    for (uint64_t i = 0; i < iterationCount; ++i) {
        float maxChange = 0.f;
        if (i & 1) {
            Vec2 currentPos = pos;
            for (uint64_t j = pointCount - 1; j != ~0; --j) {
                Vec2 nextPos = positions[j];
                Vec2 diff = nextPos - currentPos;
                float diffMag = diff.Mag();

                float len = lens[j];
                float absDiffLenMag = std::abs(diffMag - len);
                if (maxChange < absDiffLenMag) maxChange = absDiffLenMag;

                float scalar = len / diffMag;
                diff *= scalar;

                currentPos += diff;
                positions[j] = currentPos;
            }
        }
        else {
            Vec2 currentPos;
            for (uint64_t j = 0; j < pointCount; ++j) {
                Vec2 nextPos = positions[j];
                Vec2 diff = nextPos - currentPos;
                float diffMag = diff.Mag();

                float len = lens[j];
                float absDiffLenMag = std::abs(diffMag - len);
                if (maxChange < absDiffLenMag) maxChange = absDiffLenMag;

                float scalar = len / diffMag;
                diff *= scalar;

                currentPos += diff;
                positions[j] = currentPos;
            }
        }
        if (maxChange < precision) break;
    }

    std::cout << '\n'
        << "The ending positions are:\n";
    for (uint64_t i = 0; i < lenCount; ++i) {
        const Vec2& currentPos = positions[i];
        std::cout << currentPos.x << ' ' << currentPos.y << '\n';
    }

    float* endingAngles = new float[lenCount];
    getAngles(positions, endingAngles, lenCount);
    std::cout << '\n'
        << "The ending angles are:\n";
    for (uint64_t i = 0; i < lenCount; ++i) {
        float angle = endingAngles[i];
        std::cout << angle << " rad\n";
    }

    std::cout << '\n'
        << "The angle deltas are:\n";
    for (uint64_t i = 0; i < lenCount; ++i) {
        float angleDiff = endingAngles[i] - angles[i];
        std::cout << angleDiff << " rad\n";
    }

    delete[] lens;
    delete[] angles;
    delete[] positions;
    delete[] endingAngles;

    return 0;
}