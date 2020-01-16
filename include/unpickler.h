#ifndef _UNPICKLER_H_
#define _UNPICKLER_H_

#include <cstring>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace {
inline bool checkIsBigEndian() {
  int num = 1;
  return *(char*)&num != 1;
}

inline size_t read4BytesFromCharArray(const char* buf, bool isBigEndian) {
  if (isBigEndian)
    return buf[3] + (buf[2] << 8) + (buf[1] << 16) + (buf[0] << 24);
  return buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);
}
}  // namespace

namespace unpickler {

// from https://github.com/python/cpython/blob/master/Modules/_pickle.c
enum opcode {
  MARK = '(',
  STOP = '.',
  POP = '0',
  POP_MARK = '1',
  DUP = '2',
  FLOAT = 'F',
  INT = 'I',
  BININT = 'J',
  BININT1 = 'K',
  LONG = 'L',
  BININT2 = 'M',
  NONE = 'N',
  PERSID = 'P',
  BINPERSID = 'Q',
  REDUCE = 'R',
  STRING = 'S',
  BINSTRING = 'T',
  SHORT_BINSTRING = 'U',
  UNICODE = 'V',
  BINUNICODE = 'X',
  APPEND = 'a',
  BUILD = 'b',
  GLOBAL = 'c',
  DICT = 'd',
  EMPTY_DICT = '}',
  APPENDS = 'e',
  GET = 'g',
  BINGET = 'h',
  INST = 'i',
  LONG_BINGET = 'j',
  LIST = 'l',
  EMPTY_LIST = ']',
  OBJ = 'o',
  PUT = 'p',
  BINPUT = 'q',
  LONG_BINPUT = 'r',
  SETITEM = 's',
  TUPLE = 't',
  EMPTY_TUPLE = ')',
  SETITEMS = 'u',
  BINFLOAT = 'G',

  /* Protocol 2. */
  PROTO = '\x80',
  NEWOBJ = '\x81',
  EXT1 = '\x82',
  EXT2 = '\x83',
  EXT4 = '\x84',
  TUPLE1 = '\x85',
  TUPLE2 = '\x86',
  TUPLE3 = '\x87',
  NEWTRUE = '\x88',
  NEWFALSE = '\x89',
  LONG1 = '\x8a',
  LONG4 = '\x8b',

  /* Protocol 3 (Python 3.x) */
  BINBYTES = 'B',
  SHORT_BINBYTES = 'C',

  /* Protocol 4 */
  SHORT_BINUNICODE = '\x8c',
  BINUNICODE8 = '\x8d',
  BINBYTES8 = '\x8e',
  EMPTY_SET = '\x8f',
  ADDITEMS = '\x90',
  FROZENSET = '\x91',
  NEWOBJ_EX = '\x92',
  STACK_GLOBAL = '\x93',
  MEMOIZE = '\x94',
  FRAME = '\x95',

  /* Protocol 5 */
  BYTEARRAY8 = '\x96',
  NEXT_BUFFER = '\x97',
  READONLY_BUFFER = '\x98'
};

struct Frame {
  char opcode;
  size_t frameSize;
  char* content;

  ~Frame() {
    if (content != NULL)
      delete content;
  }
};

struct PickleObject {
  char protocol;
  std::vector<Frame*> frames;
  size_t totalLength;

  ~PickleObject() {
    for (const auto frame : frames) {
      delete frame;
    }
  }
};

class Unpickler {
 public:
  Unpickler() { isBigEndian = checkIsBigEndian(); }
  ~Unpickler() {}

  PickleObject* load(std::string filePath) {
    std::ifstream stream(filePath, std::ios::binary);
    char* buffer;

    // get length of file
    stream.seekg(0, std::ios::end);
    size_t length = stream.tellg();
    stream.seekg(0, std::ios::beg);

    buffer = new char[length];
    stream.read((char*)buffer, length);

    PickleObject* result = loads(buffer, length);
    delete[] buffer;

    return result;
  }

  PickleObject* loads(const char* buffer, size_t bufferLength) {
    if (bufferLength < 2)
      throw std::runtime_error("Cannot handle buffer. This buffer is too short.");

    if ((char)buffer[0] != opcode::PROTO)
      throw std::runtime_error("Because this buffer does not have PROTO(\\x80) in a first byte, we cannot parse.");

    PickleObject* object = new PickleObject;
    object->protocol = buffer[1];

    size_t currentPosition = 2;
    while (currentPosition < bufferLength) {
      char op = buffer[currentPosition];
      if (op == opcode::STOP) {
        object->totalLength = currentPosition + 1;
        break;
      }

      size_t frameSize;
      size_t offset;

      auto parsed = parseOpcode(op, &buffer[currentPosition + 1]);
      frameSize = parsed.first;
      offset = parsed.second;

      char* content;
      if (frameSize > 0) {
        content = new char[frameSize];
        std::memcpy(content, &buffer[currentPosition + 1 + offset], frameSize);
      } else
        content = NULL;

      Frame* frame = new Frame{op, frameSize, content};

      object->frames.push_back(frame);
      currentPosition += 1 + offset + frameSize;
    }

    return object;
  }

 private:
  bool isBigEndian;

  std::pair<size_t, size_t> parseOpcode(char op, const char* buffer) {
    // returns frameSize to read
    switch (op) {
      case opcode::BINPUT:
      case opcode::LONG1:
        return std::make_pair(buffer[0], 1);

      default:
        return std::make_pair(read4BytesFromCharArray(buffer, isBigEndian), 4);
    }
  }
};

}  // namespace unpickler

#endif  // _UNPICKLER_H_
