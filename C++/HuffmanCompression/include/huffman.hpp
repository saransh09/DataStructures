#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

/*
 * HuffmanCoder
 * The idea is that we generate aabbbcccc --> {'a': 2, 'b': 3, 'c': 4}
 *
 * We build a BinaryTree :
 * 1) start with all the nodes as leaf nodes
 *    in the queue --> ['a': 2, 'b': 3, 'c': 4]
 * 2) take the two elements with least freq --> and then generate a new node
 * with these two nodes as children
 *    [{ch: 'c', f: 4, l: null, r: null}, {ch: '\0', f: 5, l: 'a', r: 'b'}]
 *                          {'\0', f: 9}
 *                              /     \
 *                      {'c', f: 4}  { '\0', f: 5}
 *                                      /      \
 *                                  {'a', 2}   {'b', 3}
 *
 * 3) we need to generate codes for the different leaves
 *      every time you go left --> '0'
 *      every time you go right --> '1'
 *       code for c --> "0"
 *       code for a --> "10"
 *       code for b --> "11"
 * 4) encode
 *    so for aabbbbcccc    --> 40 bytes
 *        "[10101111][00110000]" --> 2 bytes
 *
 */

struct HuffmanNode {
  char ch;
  char minCh;
  int freq;
  std::shared_ptr<HuffmanNode> left;
  std::shared_ptr<HuffmanNode> right;

  HuffmanNode(char c, int f)
      : ch(c), minCh(ch), freq(f), left(nullptr), right(nullptr) {}
  HuffmanNode(int f, const std::shared_ptr<HuffmanNode> &l,
              const std::shared_ptr<HuffmanNode> &r)
      : ch('\0'), minCh(std::min(l->minCh, r->minCh)), freq(f), left(l),
        right(r) {}
  [[nodiscard]] bool isLeaf() const {
    return left == nullptr && right == nullptr;
  }
};

struct CompareHuffman {
  bool operator()(const std::shared_ptr<HuffmanNode> &a,
                  const std::shared_ptr<HuffmanNode> &b) {
    if (a->freq != b->freq) {
      return a->freq > b->freq;
    }
    return a->minCh > b->minCh;
  }
};

struct CompressedData {
  std::uint32_t numBits;
  std::unordered_map<char, int> freqTable;
  std::vector<std::uint8_t> packedBits;
};

struct HuffmanCoder {
  // we need to build the frequencies table for each character
  std::unordered_map<char, int> buildFreqTable(const std::string &text) {
    std::unordered_map<char, int> freqTable;
    for (const auto &ch : text) {
      if (freqTable.contains(ch)) {
        freqTable[ch]++;
      } else {
        freqTable[ch] = 1;
      }
    }
    return freqTable;
  }

  // build the huffman tree
  std::shared_ptr<HuffmanNode>
  buildTree(const std::unordered_map<char, int> &freqTable) {
    std::priority_queue<std::shared_ptr<HuffmanNode>,
                        std::vector<std::shared_ptr<HuffmanNode>>,
                        CompareHuffman>
        minHeap;
    for (const auto &pair : freqTable) {
      minHeap.emplace(std::make_shared<HuffmanNode>(pair.first, pair.second));
    }
    if (minHeap.size() == 1) {
      auto node = minHeap.top();
      minHeap.pop();
      minHeap.emplace(std::make_shared<HuffmanNode>(node->freq, node, node));
    }
    while (minHeap.size() > 1) {
      auto l = minHeap.top();
      minHeap.pop();
      auto r = minHeap.top();
      minHeap.pop();
      minHeap.emplace(std::make_shared<HuffmanNode>(l->freq + r->freq, l, r));
    }
    return minHeap.empty() ? nullptr : minHeap.top();
  }

  // generate codes for all the characters using this tree
  // simple tree parsing
  void generateCodes(const std::shared_ptr<HuffmanNode> &root,
                     const std::string &code,
                     std::unordered_map<char, std::string> &codes) {
    if (root->isLeaf()) {
      codes[root->ch] = code.empty() ? "0" : code;
      return;
    }
    generateCodes(root->left, code + '0', codes);
    generateCodes(root->right, code + '1', codes);
  }

  std::string
  get_encoded_string(const std::string &text,
                     const std::unordered_map<char, std::string> &codes) {
    std::string encoded_string;
    for (const auto &ch : text) {
      encoded_string += codes.at(ch);
    }
    return encoded_string;
  }

  std::vector<uint8_t> packBits(const std::string &encoded_string) {
    std::vector<uint8_t> packed;
    packed.reserve((encoded_string.size() + 7) / 8);

    uint8_t currentByte = 0;
    int bitCount = 0;

    for (const auto &bit : encoded_string) {
      currentByte = static_cast<uint8_t>((currentByte << 1) | (bit == '1' ? 1 : 0));
      bitCount += 1;

      if (bitCount == 8) {
        packed.emplace_back(currentByte);
        bitCount = 0;
        currentByte = 0;
      }
    }

    if (bitCount > 0) {
      currentByte <<= (8 - bitCount);
      packed.push_back(currentByte);
    }
    return packed;
  }

  CompressedData compress(const std::string &text) {
    CompressedData compressed;
    if (text.empty()) {
      compressed.numBits = 0;
      return compressed;
    }

    // build Freq Table
    std::unordered_map<char, int> freqTable = buildFreqTable(text);
    // build Tree
    auto root = buildTree(freqTable);
    // generate codes for all characters (leaf nodes)
    std::unordered_map<char, std::string> codes;
    generateCodes(root, "", codes);
    // using these codes get the encoded string
    std::string encoded_string = get_encoded_string(text, codes);
    compressed.numBits = static_cast<uint32_t>(encoded_string.size());
    // pack the bits into byte vector
    compressed.packedBits = packBits(encoded_string);
    compressed.freqTable = std::move(freqTable);
    return compressed;
  }

  std::string unpackBytes(const std::vector<uint8_t> &packed,
                          const uint32_t &numBits) {
    std::string bitString;
    bitString.reserve(numBits);

    for (const auto &byte : packed) {
      for (int i = 7; i >= 0; i--) {
        bitString += ((byte >> i) & 1) ? '1' : '0';
      }
    }

    return bitString.substr(0, numBits);
  }

  std::string decode(const std::shared_ptr<HuffmanNode> &root,
                     const std::string &bitString) {
    if (!root || bitString.empty()) {
      return "";
    }

    std::string result;
    auto current = root;

    for (const auto &bit : bitString) {
      current = (bit == '0') ? current->left : current->right;
      if (current->isLeaf()) {
        result += current->ch;
        current = root;
      }
    }

    return result;
  }

  std::string decompress(const CompressedData &data) {
    if (data.numBits == 0 || data.freqTable.empty()) {
      return "";
    }

    auto root = buildTree(data.freqTable);
    std::string bitString = unpackBytes(data.packedBits, data.numBits);
    return decode(root, bitString);
  }

  /*
   *            4          2        5*N       M        --> total size(4+2+5N+M)
   * vs original(numChars*4)
   *         ________________________________________
   *        |totalBytes|numChars|freqTable|packedBits|
   *        |__________|________|_________|__________|
   */

  std::vector<uint8_t> serialize(const CompressedData &compressed) {
    std::vector<uint8_t> buffer;
    size_t numChars = compressed.freqTable.size();
    buffer.reserve(4 + 2 + numChars * 5 + compressed.packedBits.size());

    // write totalBytes
    buffer.emplace_back(static_cast<uint8_t>(compressed.numBits & 0xFF));
    buffer.emplace_back(static_cast<uint8_t>((compressed.numBits >> 8) & 0xFF));
    buffer.emplace_back(
        static_cast<uint8_t>((compressed.numBits >> 16) & 0xFF));
    buffer.emplace_back(
        static_cast<uint8_t>((compressed.numBits >> 24) & 0xFF));

    // write numChars
    buffer.emplace_back(static_cast<uint8_t>(numChars & 0xFF));
    buffer.emplace_back(static_cast<uint8_t>((numChars >> 8) & 0xFF));

    // write freqTable
    for (const auto &pair : compressed.freqTable) {
      buffer.emplace_back(static_cast<uint8_t>(pair.first));
      buffer.emplace_back(static_cast<uint8_t>(pair.second & 0xFF));
      buffer.emplace_back(static_cast<uint8_t>((pair.second >> 8) & 0xFF));
      buffer.emplace_back(static_cast<uint8_t>((pair.second >> 16) & 0xFF));
      buffer.emplace_back(static_cast<uint8_t>((pair.second >> 24) & 0xFF));
    }

    // write the packedBits
    buffer.insert(buffer.end(), compressed.packedBits.begin(),
                  compressed.packedBits.end());

    return buffer;
  }

  CompressedData deserialize(const std::vector<uint8_t> &buffer) {
    CompressedData data;

    if (buffer.size() < 6) {
      throw std::runtime_error(
          "buffer doesn't store the number of bits and number of characters! "
          "serialised data corrupted");
    }

    size_t offset = 0;

    data.numBits = static_cast<uint32_t>(buffer[offset]) |
                   (static_cast<uint32_t>(buffer[offset + 1]) << 8) |
                   (static_cast<uint32_t>(buffer[offset + 2]) << 16) |
                   (static_cast<uint32_t>(buffer[offset + 3]) << 24);

    offset += 4;

    uint16_t numChars = static_cast<uint16_t>(
        static_cast<uint16_t>(buffer[offset]) |
        (static_cast<uint16_t>(buffer[offset + 1]) << 8));

    offset += 2;

    size_t expectedMinSize = (numChars * 5) + 6;
    if (buffer.size() < expectedMinSize) {
      throw std::runtime_error("buffer not large enough to hold "
                               "frequencyTable, serialised data corrupted");
    }
    // Read frequency table entries
    for (uint16_t i = 0; i < numChars; i++) {
      char ch = static_cast<char>(buffer[offset]);
      int freq = static_cast<int>(buffer[offset + 1]) |
                 (static_cast<int>(buffer[offset + 2]) << 8) |
                 (static_cast<int>(buffer[offset + 3]) << 16) |
                 (static_cast<int>(buffer[offset + 4]) << 24);
      data.freqTable[ch] = freq;
      offset += 5;
    }

    // Read remaining bytes as packed data
    data.packedBits.assign(buffer.begin() + static_cast<long>(offset),
                           buffer.end());

    return data;
  }
};
