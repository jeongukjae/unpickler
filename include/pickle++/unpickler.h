#ifndef _PICKLEPP_UNPICKLER_H_
#define _PICKLEPP_UNPICKLER_H_

#include <fstream>
#include <string>
#include <vector>
#include "pickle++/common.h"

namespace picklepp {

namespace {
inline bool checkIsLittleEndian() {
  int num = 1;
  return *(char*)&num == 1;
}

inline int convertLittleEndianToBigEndian(unsigned char* bytes) {
  return bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
}
}  // namespace

struct Frame {
  char opcode;
  unsigned int frameSize;  // 8 bytes
  char* content;

  ~Frame() { delete content; }
};

struct PickleObject {
  char protocol;
  std::vector<Frame> frames;
};

class Unpickler {
 public:
  Unpickler() { isLittleEndian = checkIsLittleEndian(); }
  ~Unpickler() {}

  PickleObject* load(std::string filePath) {
    std::ifstream stream(filePath, std::ios::binary);
    unsigned char* buffer;

    // get length of file
    stream.seekg(0, std::ios::end);
    size_t length = stream.tellg();
    stream.seekg(0, std::ios::beg);

    buffer = new unsigned char[length];
    stream.read((char*)buffer, length);

    PickleObject* result = loads(buffer, length);
    delete buffer;

    return result;
  }

  PickleObject* loads(const unsigned char* buffer, size_t bufferLength) {
    if (bufferLength < 2)
      throw std::runtime_error("Cannot handle buffer. This buffer is too short.");

    if (buffer[0] != opcode::PROTO)
      throw std::runtime_error("Because this buffer does not have PROTO(\\x80) in a first byte, we cannot parse.");

    PickleObject* object = new PickleObject;
    object->protocol = buffer[1];
  }

 private:
  bool isLittleEndian;
};

}  // namespace picklepp

#endif  // _PICKLEPP_UNPICKLER_H_
