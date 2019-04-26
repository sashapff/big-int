#include "big_integer.hpp"

#include <cstring>
#include <stdexcept>

big_integer::big_integer() {
    data.clear();
    sign = false;
}

big_integer::big_integer(big_integer const &other) {
    data = other.data;
    sign = other.sign;
}

big_integer::big_integer(int a) {
    *this = big_integer();
    if (a != 0) {
        data.resize(1);
        data[0] = static_cast<uint32_t>(a >= 0 ? a : -a);
        sign = a < 0;
    }
}

big_integer::big_integer(std::string const &str) {
    size_t ind = 0;
    if (str[0] == '-' || str[0] == '+') {
        ind++;
    }
    for (; ind < str.length(); ind++) {
        this->mul_short(10);
        *this += (str[ind] - '0');
    }
    sign = str[0] == '-';
}

big_integer::~big_integer() {
    data.clear();
    sign = false;
}

big_integer &big_integer::operator=(big_integer const &other) {
    if (*this != other) {
        data.resize(other.size());
        for (size_t i = 0; i < other.size(); i++) {
            data[i] = other.data[i];
        }
        sign = other.sign;
    }
    return *this;
}

uint32_t &big_integer::operator[](size_t const i) {
    return data[i];
}

big_integer &big_integer::operator+=(big_integer const &rhs) {
    if (!sign) {
        if (!rhs.sign) {
            add(rhs, 0);
        } else {
            big_integer r(rhs);
            r.sign = !r.sign;
            if (r <= *this) {
                sub(r);
            } else {
                r.sub(*this);
                *this = r;
                sign = true;
            }
        }
    } else {
        if (!rhs.sign) {
            big_integer r(rhs);
            sign = !sign;
            if (r >= *this) {
                r.sub(*this);
                *this = r;
                sign = false;
            } else {
                sub(rhs);
                sign = true;
            }
        } else {
            big_integer r(rhs);
            sign = !sign;
            r.sign = !r.sign;
            add(r, 0);
            sign = true;
        }
    }
    return *this;
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
    big_integer r(rhs);
    r.sign = !r.sign;
    *this += r;
    return *this;
}

big_integer &big_integer::operator*=(big_integer const &rhs) {
    bool sgn = this->sign;
    this->mul(rhs);
    this->sign = sgn ^ rhs.sign;
    return *this;
}

big_integer &big_integer::operator/=(big_integer const &rhs) {
    bool sgn = sign;
    *this = div_mod(rhs).first;
    sign = sgn ^ rhs.sign;
    return *this;
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
    bool sgn = sign;
    *this = div_mod(rhs).second;
    sign = sgn;
    return *this;
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
    big_integer r(rhs);
    r.to_normal_form();
    to_normal_form();
    size_t length = std::max(this->size(), r.size());
    data.resize(length);
    for (size_t i = 0; i < length; i++) {
        data[i] &= r[i];
    }
    sign &= r.sign;
    to_my_form();
    return *this;
}

big_integer &big_integer::operator|=(big_integer const &rhs) {
    big_integer r(rhs);
    r.to_normal_form();
    to_normal_form();
    size_t length = std::max(size(), r.size());
    data.resize(length);
    for (size_t i = 0; i < length; i++) {
        data[i] |= r[i];
    }
    sign |= r.sign;
    to_my_form();
    return *this;
}

big_integer &big_integer::operator^=(big_integer const &rhs) {
    big_integer r(rhs);
    r.to_normal_form();
    to_normal_form();
    size_t length = std::max(size(), r.size());
    data.resize(length);
    for (size_t i = 0; i < length; i++) {
        data[i] ^= r[i];
    }
    sign ^= r.sign;
    to_my_form();
    return *this;
}

big_integer &big_integer::operator<<=(int rhs) {
    to_normal_form();
    shift_left(rhs);
    to_my_form();
    return *this;
}

big_integer &big_integer::operator>>=(int rhs) {
    shift_right(rhs);
    if (sign) {
        add(1, 0);
    }
    return *this;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    big_integer r(*this);
    if (r.size() != 0) {
        r.sign = !r.sign;
    }
    return r;
}

big_integer big_integer::operator~() const {
    big_integer r(*this);
    r.to_normal_form();
    for (size_t i = 0; i < r.size(); i++) {
        r[i] = ~r[i];
    }
    r.sign = !r.sign;
    r.to_my_form();
    return r;
}

big_integer &big_integer::operator++() {
    return *this += 1;
}

big_integer big_integer::operator++(int) {
    big_integer r(*this);
    *this += 1;
    return r;
}

big_integer &big_integer::operator--() {
    return *this -= 1;
}

big_integer big_integer::operator--(int) {
    big_integer r = *this;
    *this -= 1;
    return r;
}

big_integer operator+(big_integer a, big_integer const &b) {
    return a += b;
}

big_integer operator-(big_integer a, big_integer const &b) {
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const &b) {
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const &b) {
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const &b) {
    return a %= b;
}

big_integer operator&(big_integer a, big_integer const &b) {
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const &b) {
    return a |= b;
}

big_integer operator^(big_integer a, big_integer const &b) {
    return a ^= b;
}

big_integer operator<<(big_integer a, int b) {
    return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
    return a >>= b;
}

bool operator==(big_integer const &a, big_integer const &b) {
    if (a.is_zero() && b.is_zero()) {
        return true;
    }
    if (a.sign != b.sign) {
        return false;
    }
    big_integer x(a);
    big_integer y(b);
    x.shrink_to_fit();
    y.shrink_to_fit();
    size_t min_len = std::min(x.size(), y.size());
    size_t max_len = std::max(x.size(), y.size());
    for (size_t i = 0; i < min_len; i++) {
        if (x[i] != y[i]) {
            return false;
        }
    }
    for (size_t i = min_len; i < max_len; i++) {
        if (x.size() > min_len && x[i] != 0) {
            return false;
        }
        if (y.size() > min_len && y[i] != 0) {
            return false;
        }
    }
    return true;
}

bool operator!=(big_integer const &a, big_integer const &b) {
    return !(a == b);
}

bool operator<(big_integer const &a, big_integer const &b) {
    big_integer x(a);
    big_integer y(b);
    x.shrink_to_fit();
    y.shrink_to_fit();
    if (x.sign != y.sign) {
        return x.sign;
    }
    if (x.size() != y.size()) {
        return (x.sign ^ (x.size() < y.size()));
    }
    for (int i = (int) x.size() - 1; i >= 0; i--) {
        if (x[i] < y[i]) {
            return !x.sign;
        }
        if (x[i] > y[i]) {
            return x.sign;
        }
    }
    return false;
}

bool operator>(big_integer const &a, big_integer const &b) {
    return (!(a < b)) && a != b;
}

bool operator<=(big_integer const &a, big_integer const &b) {
    return !(a > b);
}

bool operator>=(big_integer const &a, big_integer const &b) {
    return !(a < b);
}

std::string to_string(big_integer const &a) {
    std::string res = "";
    big_integer r(a);
    while (r != 0) {
        uint64_t t = r.div_short(10).second;
        res = (char) ('0' + t) + res;
        r = r.div_short(10).first;
    }
    if (a.sign) {
        res = '-' + res;
    }
    return (res != "" && res != "-") ? res : "0";
}

size_t big_integer::size() const {
    return data.size();
}

uint64_t block = ((uint64_t) 1 << 32);
int len = 32;

void big_integer::add(const big_integer &a, size_t pos) {
    uint32_t carry = 0;
    size_t length = std::max(this->size(), a.size() + pos);
    data.resize(length);
    for (size_t i = 0; i < a.size(); i++) {
        uint64_t sum = (uint64_t) data[i + pos] + a.data[i] + carry;
        data[i + pos] = (uint32_t) sum;
        carry = (uint32_t) (sum >> len);
    }
    for (size_t i = a.size() + pos; i < length; i++) {
        uint64_t sum = (uint64_t) data[i] + carry;
        data[i] = (uint32_t) sum;
        carry = (uint32_t) (sum >> len);
    }
    if (carry != 0) {
        data.resize(size() + 1);
        data.back() = carry;
    }
    shrink_to_fit();
}

void big_integer::sub(const big_integer &a) {
    uint32_t borrow = 0;
    for (size_t i = 0; i < a.size(); i++) {
        uint64_t diff = 0;
        if (data[i] >= a.data[i] + borrow) {
            diff = data[i] - a.data[i] - borrow;
            borrow = 0;
        } else {
            diff = (uint64_t) data[i] + block - a.data[i] - borrow;
            borrow = 1;
        }
        data[i] = (uint32_t) diff;
    }
    for (size_t i = a.size(); i < size(); i++) {
        uint64_t diff = 0;
        if (data[i] >= borrow) {
            diff = data[i] - borrow;
            borrow = 0;
        } else {
            diff = (uint64_t) data[i] + block - borrow;
            borrow = 1;
        }
        data[i] = (uint32_t) diff;
    }
    shrink_to_fit();
}

void big_integer::to_normal_form() {
    if (sign) {
        for (size_t i = 0; i < size(); i++) {
            data[i] = ~data[i];
        }
        add(1, 0);
    }
}

void big_integer::to_my_form() {
    if (sign) {
        sub(1);
        for (size_t i = 0; i < size(); i++) {
            data[i] = ~data[i];
        }
    }
    shrink_to_fit();
}

void big_integer::shift_left(int rhs) {
    uint32_t carry = 0;
    int number = rhs / len;
    int small_shift = rhs - number * rhs;
    size_t length = size() + number;
    data.resize(length);
    for (size_t i = 0; i < number; i++) {
        data[i] = 0;
    }
    for (size_t i = 0; i < length; i++) {
        uint64_t offset_number = (uint64_t) data[i] << small_shift;
        data[i + number] = (uint32_t) offset_number + carry;
        carry = (uint32_t) (offset_number >> len);
    }
}

void big_integer::shift_right(int rhs) {
    int number = rhs / len;
    int small_shift = rhs - number * rhs;
    size_t length = this->size() + number;
    data.resize(length);
    for (size_t i = (size_t) number; i < length; i++) {
        if (i >= number + 1) {
            data[i - number - 1] += ((block * data[i]) >> (small_shift));
        }
        this->data[i - number] = (data[i] >> small_shift);
    }
    for (size_t i = length - number; i < length; i++) {
        data[i] = 0;
    }
    shrink_to_fit();
}

void big_integer::mul(const big_integer &a) {
    big_integer ans(0);
    ans.data.resize(std::max(size(), a.size()));
    for (size_t i = 0; i < a.size(); i++) {
        big_integer mull(*this);
        mull.mul_short(a.data[i]);
        ans.add(mull, i);
    }
    *this = ans;
}

void big_integer::mul_short(uint32_t a) {
    if (a == 0) {
        *this = big_integer(0);
        return;
    }
    uint32_t multiplier = a;
    uint32_t carry = 0;
    for (size_t i = 0; i < this->size(); i++) {
        uint64_t mull = (uint64_t) data[i] * multiplier + carry;
        data[i] = (uint32_t) mull;
        carry = (uint32_t) (mull >> len);
    }
    if (carry != 0) {
        data.resize(size() + 1);
        data.back() = carry;
    }
}

std::pair<big_integer, big_integer> big_integer::div_mod(const big_integer &a) const {
    big_integer v(a);
    big_integer u(*this);
    v.sign = false;
    u.sign = false;
    if (u < v) {
        return std::make_pair(0, *this);
    }
    v.shrink_to_fit();
    size_t n = v.size();
    size_t m = u.size() - v.size();
    int j = (int) m;
    if (n == 1) {
        return u.div_short(v[0]);
    }
    if (u.data.size() != n + m + 1) {
        u.data.push_back(0);
    }
    uint32_t d = (uint32_t) (block / ((uint64_t) v.data.back() + 1));
    u.mul_short(d);
    v.mul_short(d);
    big_integer q_ans(0);
    q_ans.data.resize(m + 1);
    while (j >= 0) {
        uint64_t tmp = (uint64_t) u[j + n] * block + u[j + n - 1];
        uint64_t q = tmp / v.data.back();
        uint64_t r = tmp % v.data.back();
        while (q == block || (r < block && q * v[n - 2] > block * r + u[j + n - 2])) {
            --q;
            r += v.data.back();
        }
        big_integer w(v);
        w.mul_short(q);
        if (u.sub_prefix(w, j)) {
            q--;
            u.add(v, j);
        }
        q_ans[j] = (uint32_t) q;
        j--;
    }
    u = u.div_short(d).first;
    return std::make_pair(q_ans, u);
}

int big_integer::sub_prefix(big_integer const &b, size_t ind) {
    uint32_t carry = 0;
    for (size_t i = 0; i < b.size(); i++) {
        if (data[ind + i] - carry >= b.data[i]) {
            data[ind + i] = (uint32_t)(data[ind + i] - carry - b.data[i]);
            carry = 0;
        } else {
            data[ind + i] += block - carry - b.data[i];
            carry = 1;
        }
    }
    if (carry != 0 && b.size() < data.size()) {
        if (data[b.size() + ind] != 0) {
            data[b.size() + ind]--;
            carry = 0;
        } else {
            data[b.size() + ind] = (uint32_t) block - 1;
        }
    }
    return carry;
}

void big_integer::shrink_to_fit() {
    while (!data.empty() && data.back() == 0) {
        data.pop_back();
    }
}

std::pair<big_integer, uint64_t> big_integer::div_short(uint32_t a) const {
    if (a == 0) {
        return std::make_pair(0, 0);
    }
    uint32_t dividend = a;
    uint64_t carry = 0;
    big_integer r(*this);
    for (int i = (int) size() - 1; i >= 0; i--) {
        uint64_t res = ((uint64_t) data[i] + carry * block) / dividend;
        carry = (uint64_t) data[i] - res * dividend;
        r[i] = (uint32_t) res;
    }
    return std::make_pair(r, carry);
}

bool big_integer::is_zero() const {
    for (size_t i = 0; i < size(); i++) {
        if (data[i] != 0) {
            return false;
        }
    }
    return true;
}

std::ostream &operator<<(std::ostream &s, big_integer const &a) {
    return s << to_string(a);
}

std::istream& operator>>(std::istream& s, big_integer& a) {
    std::string ss;
    s >> ss;
    a = big_integer(ss);
    return s;
}