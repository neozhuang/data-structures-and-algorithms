/**
 * The equation of minimum edit distance:
 *
 * D[i,j] = min(
 *             D[i-1,j] + del-cost(source[i]), 
 *             D[i,j-1] + ins-cost(target[j]), 
 *             D[i-1,j-1] + sub-cost(source[i],target[j])
 *             )
 *
 * These costs have different weighted factors scheme,
 * here we use that version of Levenshtein distance in which 
 * the insertions and deletions each have a cost of 1 
 * (ins-cost(·) = del-cost(·) = 1), and substitutions have a cost of 2 
 * (except substitution of identical letters has zero cost).
 */

/**
 * References:
 *
 * Daniel Jurafsky and James H. Martin. 2026.
 * Speech and Language Processing: An Introduction to Natural Language Processing,
 * Computational Linguistics, and Speech Recognition with Language Models, 3rd edition. 
 * Online manuscript released August 19, 2026. 
 * https://web.stanford.edu/~jurafsky/slp3, 
 * Section 2.9, Minimum Edit Diastance.
 */

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

// Compute the minimum edit distance between source string s and target string t.
int minEditDistance(const std::string &s, const std::string &t);
int minEditDistanceUtf8(const std::string &s, const std::string &t);

// Compute the count of bytes that begin at ch with utf-8.
size_t nBytesCode(const char ch);
// Compute the length of str, i.e., the count of unicode points.
std::size_t length(const std::string& str);

void test_ascii()
{
    std::string source = "intention";
    std::string target = "execution";
    int dist = minEditDistance(source, target);
    std::cout << "The minimum distance between \"" << source << "\" and \""
        << target << "\" is: " << dist << "." << std::endl;
}

void test_utf8()
{
    // The distance is 3, 
    // insert - that costs 1 and substitute 我 to 你 that costs 2.
    std::string source = "neo我";
    std::string target = "neo-你";

    // std::string source = "喜欢你";
    // std::string target = "欢喜你";

    int dist = minEditDistanceUtf8(source, target);
    std::cout << "The minimum distance between \"" << source << "\" and \""
        << target << "\" is: " << dist << "." << std::endl;
}


int main (int argc, char *argv[]) {
    test_ascii();
    test_utf8();

    return 0;
}

int minEditDistance(const std::string& s, const std::string& t)
{
    const std::size_t m = s.size();
    const std::size_t n = t.size();

    std::vector<std::vector<int>> D(m + 1, std::vector<int>(n + 1));

    // Initialize the boundries.
    for (std::size_t i = 1; i <= m; i++)  D[i][0] = static_cast<int>(i);
    for (std::size_t j = 1; j <= n; j++)  D[0][j] = static_cast<int>(j);

    // Dynamic programming.
    for (std::size_t i = 1; i <= m; i++) {
        for (std::size_t j = 1; j <= n; j++) {
            const int substitution_cost = s[i - 1] == t[j - 1] ? 0 : 2;
            D[i][j] = std::min({
                D[i - 1][j] + 1, 
                D[i][j - 1] + 1,
                D[i - 1][j - 1] + substitution_cost
            });
        }
    }

    return D[m][n];
}

int minEditDistanceUtf8(const std::string& s, const std::string& t)
{
    const std::size_t m = length(s);
    const std::size_t n = length(t);

    std::vector<std::vector<int>> D(m + 1, std::vector<int>(n + 1));

    // Initialize the boundries.
    for (std::size_t i = 1; i <= m; i++) {
        D[i][0] = static_cast<int>(i);
    }
    for (std::size_t j = 1; j <= n; j++) {
        D[0][j] = static_cast<int>(j);
    }

    // Dynamic programming.
    std::string substring_s, substring_t;
    for (std::size_t i = 1, s_idx = 0; i <= m; i++) {
        size_t nBytes = nBytesCode(s[s_idx]);
        substring_s = s.substr(s_idx, nBytes);
        s_idx += nBytes;

        for (std::size_t j = 1, t_idx = 0; j <= n; j++) {
            nBytes = nBytesCode(t[t_idx]);
            substring_t = t.substr(t_idx, nBytes);
            t_idx += nBytes;

            const int substitution_cost = (substring_s == substring_t) ? 0 : 2;
            D[i][j] = std::min({
                D[i - 1][j] + 1,
                D[i][j - 1] + 1,
                D[i - 1][j - 1] + substitution_cost
            });
        }
    }

    return D[m][n];
}

size_t nBytesCode(const char ch)
{
    if(ch & (1 << 7))
    {
        int nBytes = 1;
        for(int idx = 0; idx != 6; ++idx)
        {
            if(ch & (1 << (6 - idx)))
                ++nBytes;
            else
                break;
        }
        return nBytes;
    }
    return 1;
}

std::size_t length(const std::string& str)
{
    std::size_t len = 0;
    for(std::size_t idx = 0; idx != str.size();)
    {
        int nBytes = nBytesCode(str[idx]);
        idx += nBytes;
        ++len;
    }
    return len;
}
