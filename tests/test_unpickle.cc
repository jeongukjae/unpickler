#include "gtest/gtest.h"
#include "unpickler.h"

TEST(unpickle, unpickle_file) {
  unpickler::Unpickler unpickler;
  unpickler::PickleObject* object = unpickler.load("../tests/test-data/data1.pkl");

  ASSERT_EQ(object->protocol, 3);
  ASSERT_EQ(object->frames.size(), 2);
  ASSERT_EQ(object->frames.at(0)->opcode, 'X');
  ASSERT_EQ(object->frames.at(0)->frameSize, 12);
  ASSERT_EQ(object->frames.at(1)->opcode, 'q');
  ASSERT_EQ(object->frames.at(1)->frameSize, 0);

  delete object;
}

TEST(unpickle, unpickle_string) {
  const char* pickled = "\x80\x03X\x0c\x00\x00\x00hello world!q\x00.";

  unpickler::Unpickler unpickler;
  unpickler::PickleObject* object = unpickler.loads(pickled, 22);

  ASSERT_EQ(object->protocol, 3);
  ASSERT_EQ(object->frames.size(), 2);
  ASSERT_EQ(object->frames.at(0)->opcode, 'X');
  ASSERT_EQ(object->frames.at(0)->frameSize, 12);
  ASSERT_EQ(object->frames.at(1)->opcode, 'q');
  ASSERT_EQ(object->frames.at(1)->frameSize, 0);

  delete object;
}

TEST(unpickle, unpickle_long) {
  unpickler::Unpickler unpickler;
  unpickler::PickleObject* object = unpickler.loads("\x80\x02\x8a\x0a\x6c\xfc\x9c\x46\xf9\x20\x6a\xa8\x50\x19\x2e", 15);

  ASSERT_EQ(object->protocol, 2);
  ASSERT_EQ(object->frames.size(), 1);
  ASSERT_EQ(object->frames.at(0)->opcode, '\x8A');
  ASSERT_EQ(object->frames.at(0)->frameSize, 0x0a);
}
