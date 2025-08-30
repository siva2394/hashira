// shamir_recover_debug2.cpp
#include <bits/stdc++.h>
using namespace std;
using u128 = unsigned __int128;
const u128 PRIME = (((u128)1) << 127) - 1;

string toString(u128 x) {
    if (x == 0) return "0";
    string s;
    while (x > 0) { s.push_back('0' + (x % 10)); x /= 10; }
    reverse(s.begin(), s.end());
    return s;
}
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
        num = modMul(num, (u128)base);
        num = modAdd(num, (u128)d);
    }
    return num;
}
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
    cerr << "DEBUG: parsed keys n=" << n << " k=" << k << "\n";

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
                if (input[i] == '{') d++; else if (input[i] == '}') { d--; if (d==0) { ++i; break; } }
            }
            if (i > input.size()) break;
            string inner = input.substr(innerStart, i - innerStart);
            string baseStr = extract_value_after_token(inner, "\"base\"");
            string valueStr = extract_value_after_token(inner, "\"value\"");
            if (key != "keys") {
                string digits;
                for (char c: key) if (isdigit((unsigned char)c)) digits.push_back(c);
                int x = digits.empty() ? 0 : stoi(digits);
                int base = safe_stoi_from_digits(baseStr); if (base==0) base = 10;
                u128 y = fromBase(valueStr, base);
                cerr << "DEBUG key=" << key << " baseStr=\"" << baseStr << "\" valueStr=\"" << valueStr << "\" parsed=(" << x << "," << toString(y) << ")\n";
                shares.emplace_back((u128)x, y);
            }
            pos = i;
            continue;
        }
        ++pos;
    }

    cerr << "Parsed shares:\n";
    for (size_t i = 0; i < shares.size(); ++i) {
        cerr << i << ": (" << toString(shares[i].first) << "," << toString(shares[i].second) << ")\n";
    }

    if (k <= 0) { cout << "Impossible: invalid or missing k.\n"; return 0; }
    if ((int)shares.size() < k) { cout << "Impossible: fewer than k shares.\n"; return 0; }

    int N = shares.size();
    vector<u128> secrets;
    vector<vector<int>> combos;
    vector<int> idx(k);
    for (int i = 0; i < k; ++i) idx[i] = i;
    auto push_current = [&]() {
        vector<u128> xs, ys;
        for (int id : idx) { xs.push_back(shares[id].first); ys.push_back(shares[id].second); }
        u128 s = lagrangeAt0(xs, ys);
        secrets.push_back(s);
        combos.push_back(idx);
        // debug print the combo and its secret
        cerr << "COMBO indices: ";
        for (int id : idx) cerr << id << " ";
        cerr << " -> xs: ";
        for (int id : idx) cerr << toString(shares[id].first) << " ";
        cerr << " ys: ";
        for (int id : idx) cerr << toString(shares[id].second) << " ";
        cerr << " secret=" << toString(s) << "\n";
    };
    bool done = false;
    while (!done) {
        push_current();
        int j = k - 1;
        while (j >= 0 && idx[j] == N - k + j) --j;
        if (j < 0) done = true;
        else {
            ++idx[j];
            for (int t = j + 1; t < k; ++t) idx[t] = idx[t - 1] + 1;
        }
    }

    unordered_map<string, int> freq;
    unordered_map<string, u128> strToVal;
    for (auto s : secrets) {
        string ss = toString(s);
        freq[ss]++;
        strToVal[ss] = s;
    }
    cerr << "FREQUENCY MAP:\n";
    for (auto &p: freq) cerr << p.first << " -> " << p.second << "\n";

    int best = 0; string bestKey;
    for (auto &p: freq) if (p.second > best) { best = p.second; bestKey = p.first; }
    if (best == 0) { cout << "Impossible: no consistent secret.\n"; return 0; }
    u128 finalSecret = strToVal[bestKey];
    cout << "Recovered secret: " << toString(finalSecret) << "\n";

    set<int> bad;
    for (size_t i = 0; i < secrets.size(); ++i) {
        string ss = toString(secrets[i]);
        if (ss != bestKey) for (int id : combos[i]) bad.insert(id);
    }
    if (bad.empty()) cout << "No culprit detected (all consistent)\n";
    else if (bad.size() == 1) { int b = *bad.begin(); cout << "Culprit share: (" << toString(shares[b].first) << "," << toString(shares[b].second) << ")\n"; }
    else cout << "Impossible: more than one culprit.\n";
    return 0;
}
