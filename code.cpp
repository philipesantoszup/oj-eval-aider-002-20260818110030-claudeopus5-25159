// int2048 - big integer with addition, subtraction, multiplication, division.
//
// This file is the single-file version used for Online Judge submission.
// It is the concatenation of `src/include/int2048.h` and `src/int2048.cpp`.
//
// NOTE: this file must never define SJTU_BIGINTEGER: that macro belongs to
// `src/include/int2048.h` alone. Defining it here would suppress the header's
// class declaration whenever both files are processed together.

#ifndef SJTU_BIGINTEGER_SINGLE_FILE
#define SJTU_BIGINTEGER_SINGLE_FILE

#include <complex>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

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

// =====================================================================
// Internal helpers operating on raw magnitude vectors (base 1000).
// =====================================================================

namespace {

const int BASE = 1000;
const int WIDTH = 3;

typedef std::vector<int> Vec;

// Thresholds tuned for the judge limits.
const size_t kNaiveMulLimit = 64;  // schoolbook multiplication below this
const int kNaiveDivLimit = 64;     // schoolbook division below this
const int kRecipBase = 40;         // base case of the Newton recursion
const long long kNaiveDivWork = 5000000LL;

void trimVec(Vec &a) {
  while (!a.empty() && a.back() == 0) a.pop_back();
}

int cmpVec(const Vec &a, const Vec &b) {
  if (a.size() != b.size()) return a.size() < b.size() ? -1 : 1;
  for (int i = static_cast<int>(a.size()) - 1; i >= 0; --i)
    if (a[i] != b[i]) return a[i] < b[i] ? -1 : 1;
  return 0;
}

// a += b
void addTo(Vec &a, const Vec &b) {
  if (b.size() > a.size()) a.resize(b.size(), 0);
  const size_t bs = b.size();
  int carry = 0;
  for (size_t i = 0; i < a.size(); ++i) {
    int cur = a[i] + carry;
    if (i < bs) cur += b[i];
    if (cur >= BASE) {
      cur -= BASE;
      carry = 1;
    } else {
      carry = 0;
    }
    a[i] = cur;
    if (carry == 0 && i >= bs) break;
  }
  if (carry) a.push_back(carry);
}

// a -= b, requires a >= b
void subFrom(Vec &a, const Vec &b) {
  const size_t bs = b.size();
  int borrow = 0;
  for (size_t i = 0; i < a.size(); ++i) {
    int cur = a[i] - borrow;
    if (i < bs) cur -= b[i];
    if (cur < 0) {
      cur += BASE;
      borrow = 1;
    } else {
      borrow = 0;
    }
    a[i] = cur;
    if (borrow == 0 && i >= bs) break;
  }
  trimVec(a);
}

Vec mulSmall(const Vec &a, int d) {
  if (d == 1) return a;
  Vec r;
  r.reserve(a.size() + 1);
  long long carry = 0;
  for (size_t i = 0; i < a.size(); ++i) {
    long long cur = static_cast<long long>(a[i]) * d + carry;
    r.push_back(static_cast<int>(cur % BASE));
    carry = cur / BASE;
  }
  while (carry) {
    r.push_back(static_cast<int>(carry % BASE));
    carry /= BASE;
  }
  trimVec(r);
  return r;
}

Vec divSmall(const Vec &a, int d) {
  if (d == 1) return a;
  Vec r(a.size(), 0);
  long long rem = 0;
  for (int i = static_cast<int>(a.size()) - 1; i >= 0; --i) {
    long long cur = rem * BASE + a[i];
    r[i] = static_cast<int>(cur / d);
    rem = cur % d;
  }
  trimVec(r);
  return r;
}

// ------------------------- FFT -------------------------

struct Cpx {
  double x, y;
  Cpx() : x(0), y(0) {}
  Cpx(double a, double b) : x(a), y(b) {}
};

inline Cpx cadd(const Cpx &a, const Cpx &b) {
  return Cpx(a.x + b.x, a.y + b.y);
}
inline Cpx csub(const Cpx &a, const Cpx &b) {
  return Cpx(a.x - b.x, a.y - b.y);
}
inline Cpx cmul(const Cpx &a, const Cpx &b) {
  return Cpx(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}
inline Cpx cconj(const Cpx &a) { return Cpx(a.x, -a.y); }

void fft(std::vector<Cpx> &a) {
  const int n = static_cast<int>(a.size());
  if (n <= 1) return;
  int lg = 0;
  while ((1 << (lg + 1)) <= n) ++lg;

  static std::vector<Cpx> rt(2, Cpx(1.0, 0.0));
  static int built = 2;
  if (n > static_cast<int>(rt.size())) {
    rt.resize(n);
    for (; built < n; built *= 2) {
      const double kPi = 3.14159265358979323846;
      std::complex<double> w = std::polar(1.0, kPi / built);
      Cpx z(w.real(), w.imag());
      for (int i = built; i < 2 * built; ++i)
        rt[i] = (i & 1) ? cmul(rt[i >> 1], z) : rt[i >> 1];
    }
  }

  std::vector<int> rev(n);
  for (int i = 0; i < n; ++i) rev[i] = (rev[i >> 1] | ((i & 1) << lg)) >> 1;
  for (int i = 0; i < n; ++i)
    if (i < rev[i]) {
      Cpx t = a[i];
      a[i] = a[rev[i]];
      a[rev[i]] = t;
    }

  for (int k = 1; k < n; k *= 2)
    for (int i = 0; i < n; i += 2 * k)
      for (int j = 0; j < k; ++j) {
        Cpx z = cmul(rt[j + k], a[i + j + k]);
        a[i + j + k] = csub(a[i + j], z);
        a[i + j] = cadd(a[i + j], z);
      }
}

Vec mulNaive(const Vec &a, const Vec &b) {
  std::vector<long long> c(a.size() + b.size(), 0);
  for (size_t i = 0; i < a.size(); ++i) {
    const long long ai = a[i];
    if (ai == 0) continue;
    for (size_t j = 0; j < b.size(); ++j) c[i + j] += ai * b[j];
  }
  Vec r(c.size(), 0);
  long long carry = 0;
  for (size_t i = 0; i < c.size(); ++i) {
    long long cur = c[i] + carry;
    r[i] = static_cast<int>(cur % BASE);
    carry = cur / BASE;
  }
  while (carry) {
    r.push_back(static_cast<int>(carry % BASE));
    carry /= BASE;
  }
  trimVec(r);
  return r;
}

Vec mulFFT(const Vec &a, const Vec &b) {
  const int rs = static_cast<int>(a.size() + b.size()) - 1;
  int n = 1;
  while (n < rs) n <<= 1;

  std::vector<Cpx> in(n), out(n);
  for (size_t i = 0; i < a.size(); ++i) in[i].x = a[i];
  for (size_t i = 0; i < b.size(); ++i) in[i].y = b[i];
  fft(in);
  for (int i = 0; i < n; ++i) in[i] = cmul(in[i], in[i]);
  for (int i = 0; i < n; ++i)
    out[i] = csub(in[(n - i) & (n - 1)], cconj(in[i]));
  in.clear();
  in.shrink_to_fit();
  fft(out);

  const double scale = 1.0 / (4.0 * n);
  Vec res;
  res.reserve(rs + 2);
  long long carry = 0;
  for (int i = 0; i < rs; ++i) {
    long long v = static_cast<long long>(out[i].y * scale + 0.5) + carry;
    res.push_back(static_cast<int>(v % BASE));
    carry = v / BASE;
  }
  while (carry) {
    res.push_back(static_cast<int>(carry % BASE));
    carry /= BASE;
  }
  trimVec(res);
  return res;
}

Vec mulVec(const Vec &a, const Vec &b) {
  if (a.empty() || b.empty()) return Vec();
  const size_t mn = a.size() < b.size() ? a.size() : b.size();
  if (mn <= kNaiveMulLimit) return mulNaive(a, b);
  return mulFFT(a, b);
}

// ------------------------- division -------------------------

// Schoolbook division (Knuth, algorithm D). Both operands positive.
void divmodNaive(const Vec &a, const Vec &b, Vec &q, Vec &r) {
  q.clear();
  r.clear();
  if (b.empty()) return;
  if (cmpVec(a, b) < 0) {
    r = a;
    return;
  }
  const int an = static_cast<int>(a.size());
  const int n = static_cast<int>(b.size());

  if (n == 1) {
    const int d = b[0];
    q.assign(an, 0);
    long long rem = 0;
    for (int i = an - 1; i >= 0; --i) {
      long long cur = rem * BASE + a[i];
      q[i] = static_cast<int>(cur / d);
      rem = cur % d;
    }
    trimVec(q);
    if (rem) r.push_back(static_cast<int>(rem));
    return;
  }

  const int m = an - n;
  const int d = BASE / (b[n - 1] + 1);
  Vec u = mulSmall(a, d);
  Vec v = mulSmall(b, d);
  u.resize(an + 1, 0);
  if (static_cast<int>(v.size()) < n) v.resize(n, 0);

  q.assign(m + 1, 0);
  for (int j = m; j >= 0; --j) {
    long long top = static_cast<long long>(u[j + n]) * BASE + u[j + n - 1];
    long long qhat = top / v[n - 1];
    long long rhat = top - qhat * v[n - 1];
    while (true) {
      if (qhat >= BASE || qhat * v[n - 2] > rhat * BASE + u[j + n - 2]) {
        --qhat;
        rhat += v[n - 1];
        if (rhat < BASE) continue;
      }
      break;
    }

    long long carry = 0, borrow = 0;
    for (int i = 0; i < n; ++i) {
      long long p = qhat * v[i] + carry;
      carry = p / BASE;
      long long low = p - carry * BASE;
      long long t = static_cast<long long>(u[i + j]) - low - borrow;
      if (t < 0) {
        t += BASE;
        borrow = 1;
      } else {
        borrow = 0;
      }
      u[i + j] = static_cast<int>(t);
    }
    long long t = static_cast<long long>(u[j + n]) - carry - borrow;
    if (t < 0) {
      u[j + n] = static_cast<int>(t + BASE);
      --qhat;
      long long c = 0;
      for (int i = 0; i < n; ++i) {
        long long s = static_cast<long long>(u[i + j]) + v[i] + c;
        if (s >= BASE) {
          s -= BASE;
          c = 1;
        } else {
          c = 0;
        }
        u[i + j] = static_cast<int>(s);
      }
      u[j + n] = static_cast<int>((static_cast<long long>(u[j + n]) + c) % BASE);
    } else {
      u[j + n] = static_cast<int>(t);
    }
    q[j] = static_cast<int>(qhat);
  }
  trimVec(q);

  Vec rr(u.begin(), u.begin() + n);
  trimVec(rr);
  r = divSmall(rr, d);
}

// Returns floor(BASE^(2n) / b), where n = b.size().
Vec recipVec(const Vec &b) {
  const int n = static_cast<int>(b.size());
  Vec pow2n(2 * n + 1, 0);
  pow2n[2 * n] = 1;

  if (n <= kRecipBase) {
    Vec q, r;
    divmodNaive(pow2n, b, q, r);
    return q;
  }

  const int m = n / 2 + 3;  // kept high limbs, guarantees 2m >= n + 5
  const int s = n - m;
  Vec bh(b.begin() + s, b.end());
  Vec rh = recipVec(bh);

  Vec r0(s, 0);
  r0.insert(r0.end(), rh.begin(), rh.end());
  trimVec(r0);

  // One Newton step: x <- x + x * (BASE^(2n) - x * b) / BASE^(2n)
  Vec res = r0;
  Vec p = mulVec(r0, b);
  const int c = cmpVec(p, pow2n);
  if (c < 0) {
    Vec e = pow2n;
    subFrom(e, p);
    Vec t = mulVec(r0, e);
    Vec delta;
    if (static_cast<int>(t.size()) > 2 * n)
      delta.assign(t.begin() + 2 * n, t.end());
    trimVec(delta);
    addTo(res, delta);
  } else if (c > 0) {
    Vec e = p;
    subFrom(e, pow2n);
    Vec t = mulVec(r0, e);
    Vec delta;
    if (static_cast<int>(t.size()) > 2 * n)
      delta.assign(t.begin() + 2 * n, t.end());
    trimVec(delta);
    if (cmpVec(res, delta) <= 0)
      res.clear();
    else
      subFrom(res, delta);
  }

  // Fix the remaining O(1) error.
  const Vec one(1, 1);
  p = mulVec(res, b);
  while (cmpVec(p, pow2n) > 0) {
    subFrom(p, b);
    subFrom(res, one);
  }
  while (true) {
    Vec next = p;
    addTo(next, b);
    if (cmpVec(next, pow2n) <= 0) {
      p.swap(next);
      addTo(res, one);
    } else {
      break;
    }
  }
  return res;
}

// Divides u (at most 2n limbs, u < b * BASE^n) by b using a precomputed
// reciprocal rc = floor(BASE^(2n) / b).
void divBlock(const Vec &u, const Vec &b, const Vec &rc, Vec &q, Vec &r) {
  const int n = static_cast<int>(b.size());
  q.clear();
  if (cmpVec(u, b) < 0) {
    r = u;
    return;
  }
  Vec t = mulVec(u, rc);
  if (static_cast<int>(t.size()) > 2 * n)
    q.assign(t.begin() + 2 * n, t.end());
  trimVec(q);

  const Vec one(1, 1);
  Vec p = mulVec(q, b);
  while (cmpVec(p, u) > 0) {
    subFrom(p, b);
    subFrom(q, one);
  }
  r = u;
  subFrom(r, p);
  while (cmpVec(r, b) >= 0) {
    subFrom(r, b);
    addTo(q, one);
  }
}

void divmodVec(const Vec &a, const Vec &b, Vec &q, Vec &r) {
  q.clear();
  r.clear();
  if (b.empty() || a.empty()) return;
  if (cmpVec(a, b) < 0) {
    r = a;
    return;
  }
  const int n = static_cast<int>(b.size());
  const int an = static_cast<int>(a.size());
  if (n <= kNaiveDivLimit ||
      static_cast<long long>(an - n + 1) * n <= kNaiveDivWork) {
    divmodNaive(a, b, q, r);
    return;
  }

  const Vec rc = recipVec(b);
  const int chunk = n;
  const int blocks = (an + chunk - 1) / chunk;
  q.assign(an, 0);
  Vec rem;
  for (int idx = blocks - 1; idx >= 0; --idx) {
    const int lo = idx * chunk;
    int hi = lo + chunk;
    if (hi > an) hi = an;
    Vec u(a.begin() + lo, a.begin() + hi);
    for (size_t i = 0; i < rem.size(); ++i) u.push_back(rem[i]);
    trimVec(u);
    Vec bq, br;
    divBlock(u, b, rc, bq, br);
    for (size_t i = 0; i < bq.size(); ++i) q[lo + i] = bq[i];
    rem.swap(br);
  }
  trimVec(q);
  r.swap(rem);
}

}  // namespace

// =====================================================================
// int2048 implementation
// =====================================================================

void int2048::trim() {
  trimVec(num);
  if (num.empty()) sign = 1;
}

std::string int2048::toString() const {
  if (num.empty()) return std::string("0");
  std::string s;
  s.reserve(num.size() * WIDTH + 2);
  if (sign < 0) s += '-';
  int v = num.back();
  if (v >= 100) {
    s += static_cast<char>('0' + v / 100);
    s += static_cast<char>('0' + v / 10 % 10);
    s += static_cast<char>('0' + v % 10);
  } else if (v >= 10) {
    s += static_cast<char>('0' + v / 10);
    s += static_cast<char>('0' + v % 10);
  } else {
    s += static_cast<char>('0' + v);
  }
  for (int i = static_cast<int>(num.size()) - 2; i >= 0; --i) {
    v = num[i];
    s += static_cast<char>('0' + v / 100);
    s += static_cast<char>('0' + v / 10 % 10);
    s += static_cast<char>('0' + v % 10);
  }
  return s;
}

int int2048::compare(const int2048 &lhs, const int2048 &rhs) {
  if (lhs.num.empty() && rhs.num.empty()) return 0;
  if (lhs.sign != rhs.sign) return lhs.sign < rhs.sign ? -1 : 1;
  const int c = cmpVec(lhs.num, rhs.num);
  return lhs.sign > 0 ? c : -c;
}

void int2048::divmod(const int2048 &lhs, const int2048 &rhs, int2048 &quot,
                     int2048 &rem) {
  Vec q, r;
  divmodVec(lhs.num, rhs.num, q, r);

  quot.num.clear();
  quot.sign = 1;
  rem.num.clear();
  rem.sign = 1;

  if (lhs.sign == rhs.sign) {
    quot.num.swap(q);
    quot.sign = 1;
    rem.num.swap(r);
    rem.sign = rem.num.empty() ? 1 : lhs.sign;
  } else if (r.empty()) {
    quot.num.swap(q);
    quot.sign = quot.num.empty() ? 1 : -1;
  } else {
    const Vec one(1, 1);
    addTo(q, one);
    quot.num.swap(q);
    quot.sign = -1;
    Vec rr = rhs.num;
    subFrom(rr, r);
    rem.num.swap(rr);
    rem.sign = rem.num.empty() ? 1 : rhs.sign;
  }
}

int2048::int2048() : sign(1) {}

int2048::int2048(long long value) : sign(1) {
  unsigned long long v;
  if (value < 0) {
    sign = -1;
    v = static_cast<unsigned long long>(-(value + 1)) + 1ULL;
  } else {
    v = static_cast<unsigned long long>(value);
  }
  while (v) {
    num.push_back(static_cast<int>(v % BASE));
    v /= BASE;
  }
  if (num.empty()) sign = 1;
}

int2048::int2048(const std::string &str) : sign(1) { read(str); }

int2048::int2048(const int2048 &other) : num(other.num), sign(other.sign) {}

int2048::int2048(int2048 &&other) noexcept : sign(other.sign) {
  num.swap(other.num);
  other.num.clear();
  other.sign = 1;
}

void int2048::read(const std::string &str) {
  num.clear();
  sign = 1;
  const int len = static_cast<int>(str.size());
  int i = 0;
  while (i < len && (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' ||
                     str[i] == '\r'))
    ++i;
  if (i < len && (str[i] == '-' || str[i] == '+')) {
    if (str[i] == '-') sign = -1;
    ++i;
  }
  int start = i;
  while (i < len && str[i] >= '0' && str[i] <= '9') ++i;
  const int end = i;
  while (start < end && str[start] == '0') ++start;
  if (start >= end) {
    sign = 1;
    return;
  }
  num.reserve((end - start) / WIDTH + 1);
  for (int p = end; p > start; p -= WIDTH) {
    int lo = p - WIDTH;
    if (lo < start) lo = start;
    int v = 0;
    for (int j = lo; j < p; ++j) v = v * 10 + (str[j] - '0');
    num.push_back(v);
  }
  trim();
}

void int2048::print() { std::cout << toString(); }

int2048 &int2048::add(const int2048 &rhs) { return *this += rhs; }

int2048 add(int2048 lhs, const int2048 &rhs) {
  lhs += rhs;
  return lhs;
}

int2048 &int2048::minus(const int2048 &rhs) { return *this -= rhs; }

int2048 minus(int2048 lhs, const int2048 &rhs) {
  lhs -= rhs;
  return lhs;
}

int2048 int2048::operator+() const { return *this; }

int2048 int2048::operator-() const {
  int2048 res(*this);
  if (!res.num.empty()) res.sign = -res.sign;
  return res;
}

int2048 &int2048::operator=(const int2048 &other) {
  if (this != &other) {
    num = other.num;
    sign = other.sign;
  }
  return *this;
}

int2048 &int2048::operator=(int2048 &&other) noexcept {
  if (this != &other) {
    num.swap(other.num);
    sign = other.sign;
    other.num.clear();
    other.sign = 1;
  }
  return *this;
}

int2048 &int2048::operator+=(const int2048 &rhs) {
  if (this == &rhs) {
    int2048 tmp(rhs);
    return *this += tmp;
  }
  if (rhs.num.empty()) return *this;
  if (num.empty()) {
    num = rhs.num;
    sign = rhs.sign;
    return *this;
  }
  if (sign == rhs.sign) {
    addTo(num, rhs.num);
    return *this;
  }
  const int c = cmpVec(num, rhs.num);
  if (c == 0) {
    num.clear();
    sign = 1;
  } else if (c > 0) {
    subFrom(num, rhs.num);
  } else {
    Vec t = rhs.num;
    subFrom(t, num);
    num.swap(t);
    sign = rhs.sign;
  }
  trim();
  return *this;
}

int2048 operator+(int2048 lhs, const int2048 &rhs) {
  lhs += rhs;
  return lhs;
}

int2048 &int2048::operator-=(const int2048 &rhs) {
  if (this == &rhs) {
    num.clear();
    sign = 1;
    return *this;
  }
  if (rhs.num.empty()) return *this;
  if (num.empty()) {
    num = rhs.num;
    sign = -rhs.sign;
    return *this;
  }
  if (sign != rhs.sign) {
    addTo(num, rhs.num);
    return *this;
  }
  const int c = cmpVec(num, rhs.num);
  if (c == 0) {
    num.clear();
    sign = 1;
  } else if (c > 0) {
    subFrom(num, rhs.num);
  } else {
    Vec t = rhs.num;
    subFrom(t, num);
    num.swap(t);
    sign = -sign;
  }
  trim();
  return *this;
}

int2048 operator-(int2048 lhs, const int2048 &rhs) {
  lhs -= rhs;
  return lhs;
}

int2048 &int2048::operator*=(const int2048 &rhs) {
  if (num.empty() || rhs.num.empty()) {
    num.clear();
    sign = 1;
    return *this;
  }
  Vec res = mulVec(num, rhs.num);
  num.swap(res);
  sign = sign * rhs.sign;
  trim();
  return *this;
}

int2048 operator*(int2048 lhs, const int2048 &rhs) {
  lhs *= rhs;
  return lhs;
}

int2048 &int2048::operator/=(const int2048 &rhs) {
  int2048 q, r;
  divmod(*this, rhs, q, r);
  num.swap(q.num);
  sign = q.sign;
  trim();
  return *this;
}

int2048 operator/(int2048 lhs, const int2048 &rhs) {
  lhs /= rhs;
  return lhs;
}

int2048 &int2048::operator%=(const int2048 &rhs) {
  int2048 q, r;
  divmod(*this, rhs, q, r);
  num.swap(r.num);
  sign = r.sign;
  trim();
  return *this;
}

int2048 operator%(int2048 lhs, const int2048 &rhs) {
  lhs %= rhs;
  return lhs;
}

std::istream &operator>>(std::istream &is, int2048 &value) {
  std::string s;
  if (is >> s) value.read(s);
  return is;
}

std::ostream &operator<<(std::ostream &os, const int2048 &value) {
  os << value.toString();
  return os;
}

bool operator==(const int2048 &lhs, const int2048 &rhs) {
  return int2048::compare(lhs, rhs) == 0;
}

bool operator!=(const int2048 &lhs, const int2048 &rhs) {
  return int2048::compare(lhs, rhs) != 0;
}

bool operator<(const int2048 &lhs, const int2048 &rhs) {
  return int2048::compare(lhs, rhs) < 0;
}

bool operator>(const int2048 &lhs, const int2048 &rhs) {
  return int2048::compare(lhs, rhs) > 0;
}

bool operator<=(const int2048 &lhs, const int2048 &rhs) {
  return int2048::compare(lhs, rhs) <= 0;
}

bool operator>=(const int2048 &lhs, const int2048 &rhs) {
  return int2048::compare(lhs, rhs) >= 0;
}

}  // namespace sjtu

#endif  // SJTU_BIGINTEGER_SINGLE_FILE
