#include <bits/stdc++.h>
using namespace std;
using u128 = unsigned __int128;
const u128 PRIME = (((u128)1) << 127) - 1;

// ---------- Helpers ----------
string toString(u128 x) {
    if (x == 0) return "0";
    string s;
    while (x > 0) { s.push_back('0' + (x % 10)); x /= 10; }
    reverse(s.begin(), s.end());
    return s;
}
int safe_stoi_from_digits(const string &s) {
    string num;
    for (char c : s) if (isdigit((unsigned char)c)) num.push_back(c);
    if (num.empty()) return 0;
    return stoi(num);
}
int char_to_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'Z') return 10 + (c - 'A');
    if (c >= 'a' && c <= 'z') return 10 + (c - 'a');
    return -1;
}
u128 fromBase(const string &val, int base) {
    u128 num = 0;
    for (char c : val) {
        if (isspace((unsigned char)c)) continue;
        int d = char_to_digit(c);
        if (d < 0 || d >= base) return 0;
        num = ( (__uint128_t)num * base + d ) % PRIME;
    }
    return num;
}

// ---------- Modular arithmetic ----------
u128 modAdd(u128 a, u128 b) { return (a + b) % PRIME; }
u128 modSub(u128 a, u128 b) { return (a >= b) ? (a - b) : (PRIME - (b - a)); }
u128 modMul(u128 a, u128 b) { __uint128_t res = (__uint128_t)a * b; return (u128)(res % PRIME); }
u128 modPow(u128 base, u128 exp) {
    u128 res = 1;
    while (exp > 0) {
        if (exp & 1) res = modMul(res, base);
        base = modMul(base, base);
        exp >>= 1;
    }
    return res;
}
u128 modInv(u128 a) { return modPow(a, PRIME - 2); }
u128 modDiv(u128 a, u128 b) { return modMul(a, modInv(b)); }

// ---------- Lagrange interpolation ----------
u128 lagrangeAt0(const vector<u128> &xs, const vector<u128> &ys) {
    int k = xs.size();
    u128 secret = 0;
    for (int i = 0; i < k; ++i) {
        u128 num = 1, den = 1;
        for (int j = 0; j < k; ++j) if (i != j) {
            num = modMul(num, modSub((u128)0, xs[j]));
            den = modMul(den, modSub(xs[i], xs[j]));
        }
        secret = modAdd(secret, modMul(ys[i], modDiv(num, den)));
    }
    return secret;
}

// ---------- JSON helpers ----------
string extract_value_after_token(const string &block, const string &token) {
    size_t pos = block.find(token);
    if (pos == string::npos) return "";
    size_t colon = block.find(':', pos + token.size());
    if (colon == string::npos) return "";
    size_t p = colon + 1;
    while (p < block.size() && isspace((unsigned char)block[p])) p++;
    if (p >= block.size()) return "";
    if (block[p] == '"') {
        size_t q = block.find('"', p+1);
        if (q == string::npos) return "";
        return block.substr(p+1, q-p-1);
    } else {
        size_t q = p;
        while (q < block.size() && block[q] != ',' && block[q] != '}') q++;
        string sub = block.substr(p, q-p);
        size_t a = 0; while (a < sub.size() && isspace((unsigned char)sub[a])) a++;
        size_t b = sub.size(); while (b>0 && isspace((unsigned char)sub[b-1])) b--;
        if (b <= a) return "";
        return sub.substr(a, b-a);
    }
}
size_t skip_spaces(const string &s, size_t pos) {
    while (pos < s.size() && isspace((unsigned char)s[pos])) pos++;
    return pos;
}

// ---------- Main ----------
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string input, line;
    while (getline(cin, line)) { input += line; input.push_back('\n'); }

    int n = 0, k = 0;
    size_t p_keys = input.find("\"keys\"");
    if (p_keys != string::npos) {
        size_t start = input.find('{', p_keys);
        if (start != string::npos) {
            int depth = 0; size_t i = start;
            for (; i < input.size(); ++i) {
                if (input[i] == '{') depth++;
                else if (input[i] == '}') { depth--; if (depth == 0) break; }
            }
            if (i < input.size()) {
                string keysBlock = input.substr(start, i - start + 1);
                n = safe_stoi_from_digits(extract_value_after_token(keysBlock, "\"n\""));
                k = safe_stoi_from_digits(extract_value_after_token(keysBlock, "\"k\""));
            }
        }
    }

    vector<pair<u128,u128>> shares;
    size_t pos = 0; int depth = 0;
    while (pos < input.size()) {
        char ch = input[pos];
        if (ch == '{') { depth++; ++pos; continue; }
        if (ch == '}') { depth--; ++pos; continue; }
        if (depth == 1 && ch == '"') {
            size_t q = input.find('"', pos+1); if (q == string::npos) break;
            string key = input.substr(pos+1, q-pos-1); pos = q+1;
            pos = skip_spaces(input, pos);
            if (pos < input.size() && input[pos] == ':') pos++;
            pos = skip_spaces(input, pos);
            if (pos >= input.size() || input[pos] != '{') { ++pos; continue; }
            size_t innerStart = pos; int d=0; size_t i = pos;
            for (; i < input.size(); ++i) {
                if (input[i] == '{') d++; else if (input[i] == '}') { d
