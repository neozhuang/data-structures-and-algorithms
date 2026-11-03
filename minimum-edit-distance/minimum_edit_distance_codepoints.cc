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

struct Utf8Character {
    char32_t codePoint; 
    std::size_t byteCount;
};

// Compute the minimum edit distance between source string s and target string t.
int minEditDistance(const std::string &s, const std::string &t);
int minEditDistanceUtf8(const std::string &s, const std::string &t);

// Split each character to vector element. 喜欢你 --> [喜, 欢, 你]
std::vector<std::string> splitCharacters(const std::string& keyword);

// Decode one UTF-8 character beginning at offset. The caller must ensure that
// offset is within text; malformed or incomplete UTF-8 throws invalid_argument.
Utf8Character readUtf8(const std::string& text, std::size_t offset);


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
    for (std::size_t i = 1; i <= m; i++) D[i][0] = static_cast<int>(i);
    for (std::size_t j = 1; j <= n; j++) D[0][j] = static_cast<int>(j);

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
    const auto source = splitCharacters(s);
    const auto target = splitCharacters(t);
    const std::size_t m = source.size();
    const std::size_t n = target.size();

    std::vector<std::vector<int>> D(m + 1, std::vector<int>(n + 1));

    // Initialize the boundries.
    for (std::size_t i = 1; i <= m; i++)  D[i][0] = static_cast<int>(i);
    for (std::size_t j = 1; j <= n; j++)  D[0][j] = static_cast<int>(j);

    // Dynamic programming.
    for (std::size_t i = 1; i <= m; i++) {
        for (std::size_t j = 1; j <= n; j++) {
            const int substitution_cost = source[i - 1] == target[j - 1] ? 0 : 2;
            D[i][j] = std::min({
                D[i - 1][j] + 1, 
                D[i][j - 1] + 1,
                D[i - 1][j - 1] + substitution_cost
            });
        }
    }

    return D[m][n];
}

std::vector<std::string> splitCharacters(const std::string& keyword)
{
    std::vector<std::string> characters;

    for (std::size_t offset = 0; offset < keyword.size(); ) {
        const auto character = readUtf8(keyword, offset);
        characters.push_back(keyword.substr(offset, character.byteCount));
        offset += character.byteCount;
    }

    return characters;
}

Utf8Character readUtf8(const std::string& text, std::size_t offset)
{
    // ASCII characters are represented by a single byte.
    const auto first = static_cast<unsigned char>(text[offset]);
    if (first < 0x80) {
        return {first, 1};
    }

    std::size_t count = 0;
    char32_t codePoint = 0;

    // The leading byte determines the character's byte count and contributes
    // the initial bits of its Unicode code point.
    if (first >= 0xC2 && first <= 0xDF) {
        count = 2;
        codePoint = first & 0x1F;
    } else if (first >= 0xE0 && first <= 0xEF) {
        count = 3;
        codePoint = first & 0x0F;
    } else if (first >= 0xF0 && first <= 0xF4) {
        count = 4;
        codePoint = first & 0x07;
    } else {
        throw std::invalid_argument("Invalid UTF-8 query");
    }

    if (count > text.size() - offset) {
        throw std::invalid_argument("Incomplete UTF-8 query");
    }

    // Validate and combine the continuation bytes (10xxxxxx).
    for (std::size_t i = 1; i < count; ++i) {
        const auto byte =
            static_cast<unsigned char>(text[offset + i]);

        if ((byte & 0xC0) != 0x80) {
            throw std::invalid_argument("Invalid UTF-8 query");
        }

        codePoint = (codePoint << 6) | (byte & 0x3F);
    }

    // Reject overlong encodings, surrogate code points, and values outside
    // the Unicode range.
    if ((count == 3 && codePoint < 0x800) ||
        (count == 4 && codePoint < 0x10000) ||
        (codePoint >= 0xD800 && codePoint <= 0xDFFF) ||
        codePoint > 0x10FFFF) {
        throw std::invalid_argument("Invalid UTF-8 query");
    }

    return {codePoint, count};
}
