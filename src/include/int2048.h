#ifndef SJTU_BIGINTEGER
#define SJTU_BIGINTEGER

// Integer 1:
// Implement a signed big integer class that only needs to support simple addition and subtraction

// Integer 2:
// Implement a signed big integer class that supports addition, subtraction, multiplication, and division, and overload related operators

// Do not use any header files other than the following
#include <complex>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

// Do not use "using namespace std;"

namespace sjtu {

class int2048 {
private:
  // Absolute value of the number, stored little-endian in base 1000.
  // An empty vector represents zero (then `sign` is always 1).
  std::vector<int> num;
  // 1 for non-negative numbers, -1 for negative numbers.
  int sign;

  // Remove leading zero limbs and normalize the sign of zero.
  void trim();
  // Decimal representation of the number.
  std::string toString() const;
  // Three-way comparison: -1 / 0 / 1.
  static int compare(const int2048 &lhs, const int2048 &rhs);
  // quot = floor(lhs / rhs) (rounded toward negative infinity),
  // rem  = lhs - quot * rhs.
  static void divmod(const int2048 &lhs, const int2048 &rhs, int2048 &quot,
                     int2048 &rem);

public:
  // Constructors
  int2048();
  int2048(long long value);
  int2048(const std::string &str);
  int2048(const int2048 &other);
  int2048(int2048 &&other) noexcept;
  ~int2048() = default;

  // ===================================
  // Integer1
  // ===================================

  // Read a big integer
  void read(const std::string &str);
  // Output the stored big integer, no need for newline
  void print();

  // Add a big integer
  int2048 &add(const int2048 &rhs);
  // Return the sum of two big integers
  friend int2048 add(int2048 lhs, const int2048 &rhs);

  // Subtract a big integer
  int2048 &minus(const int2048 &rhs);
  // Return the difference of two big integers
  friend int2048 minus(int2048 lhs, const int2048 &rhs);

  // ===================================
  // Integer2
  // ===================================

  int2048 operator+() const;
  int2048 operator-() const;

  int2048 &operator=(const int2048 &other);
  int2048 &operator=(int2048 &&other) noexcept;

  int2048 &operator+=(const int2048 &rhs);
  friend int2048 operator+(int2048 lhs, const int2048 &rhs);

  int2048 &operator-=(const int2048 &rhs);
  friend int2048 operator-(int2048 lhs, const int2048 &rhs);

  int2048 &operator*=(const int2048 &rhs);
  friend int2048 operator*(int2048 lhs, const int2048 &rhs);

  int2048 &operator/=(const int2048 &rhs);
  friend int2048 operator/(int2048 lhs, const int2048 &rhs);

  int2048 &operator%=(const int2048 &rhs);
  friend int2048 operator%(int2048 lhs, const int2048 &rhs);

  friend std::istream &operator>>(std::istream &is, int2048 &value);
  friend std::ostream &operator<<(std::ostream &os, const int2048 &value);

  friend bool operator==(const int2048 &lhs, const int2048 &rhs);
  friend bool operator!=(const int2048 &lhs, const int2048 &rhs);
  friend bool operator<(const int2048 &lhs, const int2048 &rhs);
  friend bool operator>(const int2048 &lhs, const int2048 &rhs);
  friend bool operator<=(const int2048 &lhs, const int2048 &rhs);
  friend bool operator>=(const int2048 &lhs, const int2048 &rhs);
};

} // namespace sjtu

#endif // SJTU_BIGINTEGER
