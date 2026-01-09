#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "huffman.hpp"

// =============================================================================
// Helper function
// =============================================================================

std::string getLongText() {
  return "In the realm of software engineering, compression algorithms play a "
         "vital role in reducing storage requirements and transmission times. "
         "Huffman coding, invented by David Huffman in 1952 while he was a PhD "
         "student at MIT, remains one of the most elegant and widely used "
         "lossless compression techniques. The algorithm works by assigning "
         "variable-length codes to input characters, with shorter codes "
         "assigned to more frequent characters. This approach is optimal among "
         "all methods that encode symbols separately. The beauty of Huffman "
         "coding lies in its simplicity and efficiency. By building a binary "
         "tree based on character frequencies, we can generate prefix-free "
         "codes that minimize the expected code length. This property ensures "
         "that no code is a prefix of another, allowing for unambiguous "
         "decoding. Modern applications of Huffman coding include file "
         "compression utilities like ZIP and GZIP, image formats like JPEG, "
         "and network protocols. While newer algorithms like arithmetic coding "
         "and ANS can achieve better compression ratios, Huffman coding "
         "remains popular due to its speed, simplicity, and patent-free "
         "status. The algorithm demonstrates how theoretical computer science "
         "directly impacts practical applications. Defense technology systems "
         "often rely on efficient compression for transmitting sensor data, "
         "radar signals, and reconnaissance imagery over bandwidth-limited "
         "channels. In these scenarios, the trade-off between compression "
         "ratio and computational overhead becomes critical. Huffman coding "
         "provides an excellent balance, offering reasonable compression with "
         "minimal latency. As data volumes continue to grow exponentially, the "
         "importance of compression algorithms only increases. Understanding "
         "the fundamentals of Huffman coding provides a solid foundation for "
         "exploring more advanced techniques like LZ77, LZW, and modern neural "
         "compression methods. The principles of entropy encoding that Huffman "
         "pioneered continue to influence algorithm design today.";
}

// =============================================================================
// Frequency Table Tests
// =============================================================================

TEST_CASE("Frequency table is built correctly", "[huffman][frequency]") {
  HuffmanCoder coder;

  SECTION("simple string") {
    auto freq = coder.buildFreqTable("aaabbc");
    REQUIRE(freq['a'] == 3);
    REQUIRE(freq['b'] == 2);
    REQUIRE(freq['c'] == 1);
  }

  SECTION("empty string") {
    auto freq = coder.buildFreqTable("");
    REQUIRE(freq.empty());
  }

  SECTION("single character") {
    auto freq = coder.buildFreqTable("aaaaa");
    REQUIRE(freq.size() == 1);
    REQUIRE(freq['a'] == 5);
  }
}

// =============================================================================
// Tree Building Tests
// =============================================================================

TEST_CASE("Huffman tree is built correctly", "[huffman][tree]") {
  HuffmanCoder coder;

  SECTION("combined frequency equals sum of all characters") {
    std::unordered_map<char, int> freq = {{'a', 3}, {'b', 2}, {'c', 1}};
    auto root = coder.buildTree(freq);
    REQUIRE(root != nullptr);
    REQUIRE(root->freq == 6);
  }

  SECTION("single character creates valid tree") {
    std::unordered_map<char, int> freq = {{'x', 10}};
    auto root = coder.buildTree(freq);
    REQUIRE(root != nullptr);
    REQUIRE(root->freq == 10);
  }

  SECTION("empty frequency table returns nullptr") {
    std::unordered_map<char, int> freq;
    auto root = coder.buildTree(freq);
    REQUIRE(root == nullptr);
  }
}

// =============================================================================
// Code Generation Tests
// =============================================================================

TEST_CASE("Codes are generated correctly", "[huffman][codes]") {
  HuffmanCoder coder;

  SECTION("each character has unique non-empty code") {
    std::unordered_map<char, int> freq = {{'a', 5}, {'b', 2}};
    auto root = coder.buildTree(freq);
    std::unordered_map<char, std::string> codes;
    coder.generateCodes(root, "", codes);

    REQUIRE(codes.count('a') == 1);
    REQUIRE(codes.count('b') == 1);
    REQUIRE_FALSE(codes['a'].empty());
    REQUIRE_FALSE(codes['b'].empty());
    REQUIRE(codes['a'] != codes['b']);
  }

  SECTION("codes only contain 0s and 1s") {
    std::unordered_map<char, int> freq = {{'a', 5}, {'b', 3}, {'c', 2}};
    auto root = coder.buildTree(freq);
    std::unordered_map<char, std::string> codes;
    coder.generateCodes(root, "", codes);

    for (const auto &[ch, code] : codes) {
      for (char bit : code) {
        REQUIRE((bit == '0' || bit == '1'));
      }
    }
  }
}

// =============================================================================
// Roundtrip Tests
// =============================================================================

TEST_CASE("Compress and decompress roundtrip", "[huffman][roundtrip]") {
  HuffmanCoder coder;

  SECTION("simple string") {
    std::string original = "aaabbc";
    auto compressed = coder.compress(original);
    std::string decoded = coder.decompress(compressed);
    REQUIRE(decoded == original);
    INFO("Original: " << original.size() * 8 << " bits, Encoded: " << compressed.numBits << " bits");
  }

  SECTION("custom string aabbbccc") {
    std::string original = "aabbbccc";
    auto compressed = coder.compress(original);
    std::string decoded = coder.decompress(compressed);
    REQUIRE(decoded == original);
  }

  SECTION("sentence") {
    std::string original = "the quick brown fox jumps over the lazy dog";
    auto compressed = coder.compress(original);
    std::string decoded = coder.decompress(compressed);
    REQUIRE(decoded == original);
  }

  SECTION("single repeated character") {
    std::string original = "aaaaa";
    auto compressed = coder.compress(original);
    std::string decoded = coder.decompress(compressed);
    REQUIRE(decoded == original);
  }

  SECTION("empty string") {
    std::string original = "";
    auto compressed = coder.compress(original);
    std::string decoded = coder.decompress(compressed);
    REQUIRE(decoded == original);
    REQUIRE(compressed.numBits == 0);
    REQUIRE(compressed.freqTable.empty());
    REQUIRE(compressed.packedBits.empty());
  }

  SECTION("long text") {
    std::string original = getLongText();
    auto compressed = coder.compress(original);
    std::string decoded = coder.decompress(compressed);
    REQUIRE(decoded == original);
  }
}

// =============================================================================
// Serialization Tests
// =============================================================================

TEST_CASE("Serialize and deserialize", "[huffman][serialization]") {
  HuffmanCoder coder;

  SECTION("full roundtrip with serialization") {
    std::string original = "the quick brown fox jumps over the lazy dog";

    auto compressed = coder.compress(original);
    auto bytes = coder.serialize(compressed);
    auto restored = coder.deserialize(bytes);
    std::string decoded = coder.decompress(restored);

    REQUIRE(decoded == original);
    REQUIRE(restored.numBits == compressed.numBits);
    REQUIRE(restored.freqTable.size() == compressed.freqTable.size());
    REQUIRE(restored.packedBits.size() == compressed.packedBits.size());
  }

  SECTION("empty string serialization") {
    std::string original = "";

    auto compressed = coder.compress(original);
    auto bytes = coder.serialize(compressed);
    auto restored = coder.deserialize(bytes);
    std::string decoded = coder.decompress(restored);

    REQUIRE(decoded == original);
    REQUIRE(restored.numBits == 0);
  }

  SECTION("long text full roundtrip") {
    std::string original = getLongText();

    auto compressed = coder.compress(original);
    auto bytes = coder.serialize(compressed);
    auto restored = coder.deserialize(bytes);
    std::string decoded = coder.decompress(restored);

    REQUIRE(decoded == original);

    double ratio = static_cast<double>(original.size()) / static_cast<double>(bytes.size());
    INFO("Original: " << original.size() << " bytes -> Wire: " << bytes.size() << " bytes, ratio: " << ratio << "x");
    REQUIRE(ratio > 1.0);  // Long text should compress
  }
}

// =============================================================================
// Compression Size Analysis Tests
// =============================================================================

TEST_CASE("Compression size analysis", "[huffman][analysis]") {
  HuffmanCoder coder;

  SECTION("small string has overhead") {
    std::string text = "aaabbc";
    auto compressed = coder.compress(text);
    auto bytes = coder.serialize(compressed);

    size_t headerBytes = 6;
    size_t freqTableBytes = compressed.freqTable.size() * 5;
    size_t packedDataBytes = compressed.packedBits.size();
    size_t totalCompressedBytes = headerBytes + freqTableBytes + packedDataBytes;

    REQUIRE(bytes.size() == totalCompressedBytes);
    // Small strings typically expand due to overhead
    INFO("Original: " << text.size() << " bytes, Compressed: " << totalCompressedBytes << " bytes");
  }

  SECTION("large repetitive text compresses well") {
    std::string text;
    for (int i = 0; i < 100; i++) {
      text += "aaabbc";
    }

    auto compressed = coder.compress(text);
    auto bytes = coder.serialize(compressed);

    double ratio = static_cast<double>(text.size()) / static_cast<double>(bytes.size());
    REQUIRE(ratio > 1.0);
    INFO("Original: " << text.size() << " bytes, Compressed: " << bytes.size() << " bytes, ratio: " << ratio << "x");
  }

  SECTION("single character repeated compresses extremely well") {
    std::string text(100, 'x');

    auto compressed = coder.compress(text);
    auto bytes = coder.serialize(compressed);

    // Single character = 1 bit per char = 100 bits = ~13 bytes + overhead
    REQUIRE(compressed.numBits == 100);
    INFO("Original: " << text.size() << " bytes, Encoded bits: " << compressed.numBits);
  }

  SECTION("long natural text achieves good compression") {
    std::string text = getLongText();

    auto compressed = coder.compress(text);
    auto bytes = coder.serialize(compressed);

    double ratio = static_cast<double>(text.size()) / static_cast<double>(bytes.size());
    REQUIRE(ratio > 1.4);  // Natural text should compress reasonably
    INFO("Original: " << text.size() << " bytes, Wire: " << bytes.size() << " bytes, ratio: " << ratio << "x");
  }
}

// =============================================================================
// Edge Cases and Error Handling
// =============================================================================

TEST_CASE("Edge cases", "[huffman][edge]") {
  HuffmanCoder coder;

  SECTION("binary characters") {
    std::string original = "\x00\x01\x02\x03\x00\x00\x01";
    original[0] = '\x00';  // Ensure null bytes are included
    auto compressed = coder.compress(original);
    std::string decoded = coder.decompress(compressed);
    REQUIRE(decoded == original);
  }

  SECTION("all unique characters") {
    std::string original = "abcdefghijklmnopqrstuvwxyz";
    auto compressed = coder.compress(original);
    std::string decoded = coder.decompress(compressed);
    REQUIRE(decoded == original);
  }

  SECTION("whitespace and special characters") {
    std::string original = "  \t\n\r  hello\tworld\n";
    auto compressed = coder.compress(original);
    std::string decoded = coder.decompress(compressed);
    REQUIRE(decoded == original);
  }
}

TEST_CASE("Deserialize error handling", "[huffman][error]") {
  HuffmanCoder coder;

  SECTION("buffer too small throws") {
    std::vector<uint8_t> tinyBuffer = {0x01, 0x02};
    REQUIRE_THROWS_AS(coder.deserialize(tinyBuffer), std::runtime_error);
  }

  SECTION("corrupted frequency table throws") {
    // Create a buffer that claims to have more characters than data present
    std::vector<uint8_t> corruptBuffer = {
        0x10, 0x00, 0x00, 0x00,  // numBits = 16
        0xFF, 0x00               // numChars = 255 (way more than buffer can hold)
    };
    REQUIRE_THROWS_AS(coder.deserialize(corruptBuffer), std::runtime_error);
  }
}
