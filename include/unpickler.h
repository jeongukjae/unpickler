#ifndef _UNPICKLER_H_
#define _UNPICKLER_H_

#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace unpickler {

namespace {
#define SIZE_T(x) (size_t)(0xff & x)

inline bool checkIsBigEndian() {
  int num = 1;
  return *(char*)&num != 1;
}

inline size_t read4BytesFromCharArray(const char* buf, bool isBigEndian) {
  if (isBigEndian)
    return SIZE_T(buf[3]) + (SIZE_T(buf[2]) << 8) + (SIZE_T(buf[1]) << 16) + (SIZE_T(buf[0]) << 24);
  return SIZE_T(buf[0]) + (SIZE_T(buf[1]) << 8) + (SIZE_T(buf[2]) << 16) + (SIZE_T(buf[3]) << 24);
}

inline size_t read2BytesFromCharArray(const char* buf, bool isBigEndian) {
  if (isBigEndian)
    return buf[1] + (buf[0] << 8);

  return SIZE_T(buf[0]) + (SIZE_T(buf[1]) << 8);
}

#undef SIZE_T
}  // namespace

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

  std::string toString() { return std::string(content, content + frameSize); }
  size_t to2BytesInteger(bool isBigEndian) { return read2BytesFromCharArray(content, isBigEndian); }
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
      size_t frameSize = 0;
      size_t offset = 0;
      char* content;

      if (op == opcode::STOP) {
        // stop parsing
        object->totalLength = currentPosition + 1;
        break;
      }

      auto parsed = parseOpcode(op, &buffer[currentPosition + 1]);
      frameSize = parsed.first;
      offset = parsed.second;

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
    // returns <frameSize, offset>
    switch (op) {
      case opcode::EMPTY_DICT:
      case opcode::EMPTY_TUPLE:
      case opcode::TUPLE:
      case opcode::TUPLE1:
      case opcode::TUPLE2:
      case opcode::NONE:
      case opcode::NEWTRUE:
      case opcode::NEWFALSE:
      case opcode::MARK:
      case opcode::SETITEM:
      case opcode::SETITEMS:
      case opcode::BUILD:
      case opcode::REDUCE:
      case opcode::BINPERSID:
        // read zero byte
        return std::make_pair(0, 0);

      case opcode::BINPUT:
      case opcode::BININT1:
      case opcode::BINGET:
        // read 1 byte
        return std::make_pair(1, 0);

      case opcode::LONG1:
        // read buffer[0] bytes
        return std::make_pair(buffer[0], 1);

      case opcode::BININT2:
        // read 2 bytes
        return std::make_pair(2, 0);

      case opcode::BINUNICODE:
        // read buffer[0] ~ buffer[3] bytes
        return std::make_pair(read4BytesFromCharArray(buffer, isBigEndian), 4);

      case opcode::GLOBAL:
        // this opcode stops parsing when met \n (0x0a) twice.
        return std::make_pair(parseGlobalOpcode(buffer), 0);

      default:
        std::stringstream s;
        s << "unknown opcode: " << op << " (0x" << std::hex << (int)(op & 0xff) << ")";
        throw std::runtime_error(s.str());
    }
  }

  size_t parseGlobalOpcode(const char* buffer) {
    size_t offset = 0;
    bool metAlready = false;
    while (true) {
      if (buffer[offset++] == '\x0a') {
        if (metAlready)
          break;
        metAlready = true;
      }
    }

    return offset;
  }
};

}  // namespace unpickler

#endif  // _UNPICKLER_H_
