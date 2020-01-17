#include "gtest/gtest.h"
#include "unpickler.h"

TEST(unpickle, unpickle_file) {
  /*
    0: \x80 PROTO      3
    2: X    BINUNICODE 'hello world!'
   19: q    BINPUT     0
   21: .    STOP
  */
  unpickler::Unpickler unpickler;
  unpickler::PickleObject* object = unpickler.load("../tests/test-data/data1.pkl");

  ASSERT_EQ(object->protocol, 3);
  ASSERT_EQ(object->totalLength, 22);
  ASSERT_EQ(object->frames.size(), 2);
  ASSERT_EQ(object->frames.at(0)->opcode, 'X');
  ASSERT_EQ(object->frames.at(0)->frameSize, 12);
  ASSERT_EQ(object->frames.at(1)->opcode, 'q');
  ASSERT_EQ(object->frames.at(1)->frameSize, 1);
  ASSERT_STREQ(object->frames.at(1)->content, "\x00");

  delete object;
}

TEST(unpickle, unpickle_string) {
  /*
    0: \x80 PROTO      3
    2: X    BINUNICODE 'hello world!'
   19: q    BINPUT     0
   21: .    STOP
  */
  const char* pickled = "\x80\x03X\x0c\x00\x00\x00hello world!q\x00.";

  unpickler::Unpickler unpickler;
  unpickler::PickleObject* object = unpickler.loads(pickled, 22);

  ASSERT_EQ(object->protocol, 3);
  ASSERT_EQ(object->totalLength, 22);
  ASSERT_EQ(object->frames.size(), 2);
  ASSERT_EQ(object->frames.at(0)->opcode, 'X');
  ASSERT_EQ(object->frames.at(0)->frameSize, 12);
  ASSERT_EQ(object->frames.at(1)->opcode, 'q');
  ASSERT_EQ(object->frames.at(1)->frameSize, 1);
  ASSERT_STREQ(object->frames.at(1)->content, "\x00");

  delete object;
}

TEST(unpickle, unpickle_long) {
  /*
    0: \x80 PROTO      2
    2: \x8a LONG1      119547037146038801333356
   14: .    STOP
  */
  unpickler::Unpickler unpickler;
  unpickler::PickleObject* object = unpickler.loads("\x80\x02\x8a\x0a\x6c\xfc\x9c\x46\xf9\x20\x6a\xa8\x50\x19\x2e", 15);

  ASSERT_EQ(object->protocol, 2);
  ASSERT_EQ(object->totalLength, 15);
  ASSERT_EQ(object->frames.size(), 1);
  ASSERT_EQ(object->frames.at(0)->opcode, '\x8A');
  ASSERT_EQ(object->frames.at(0)->frameSize, 0x0a);
  ASSERT_STREQ(object->frames.at(0)->content, "\x6c\xfc\x9c\x46\xf9\x20\x6a\xa8\x50\x19");

  delete object;
}

TEST(unpickle, unpickle_binint2) {
  /*
    0: \x80 PROTO      2
    2: M    BININT2    1001
    5: .    STOP
  */
  unpickler::Unpickler unpickler;
  unpickler::PickleObject* object = unpickler.loads("\x80\x02M\xe9\x03.", 6);

  ASSERT_EQ(object->protocol, 2);
  ASSERT_EQ(object->totalLength, 6);
  ASSERT_EQ(object->frames.size(), 1);
  ASSERT_EQ(object->frames.at(0)->opcode, 'M');
  ASSERT_EQ(object->frames.at(0)->frameSize, 2);
  ASSERT_STREQ(object->frames.at(0)->content, "\xe9\x03");

  delete object;
}

TEST(unpickle, unpickle_object_that_has_multiple_value) {
  /*
    0: \x80 PROTO      2
    2: }    EMPTY_DICT
    3: q    BINPUT     0
    5: (    MARK
    6: X        BINUNICODE 'protocol_version'
   27: q        BINPUT     1
   29: M        BININT2    1001
   32: X        BINUNICODE 'little_endian'
   50: q        BINPUT     2
   52: \x88     NEWTRUE
   53: X        BINUNICODE 'type_sizes'
   68: q        BINPUT     3
   70: }        EMPTY_DICT
   71: q        BINPUT     4
   73: (        MARK
   74: X            BINUNICODE 'short'
   84: q            BINPUT     5
   86: K            BININT1    2
   88: X            BINUNICODE 'int'
   96: q            BINPUT     6
   98: K            BININT1    4
  100: X            BINUNICODE 'long'
  109: q            BINPUT     7
  111: K            BININT1    4
  113: u            SETITEMS   (MARK at 73)
  114: u        SETITEMS   (MARK at 5)
  115: .    STOP
   */
  unpickler::Unpickler unpickler;
  unpickler::PickleObject* object = unpickler.load("../tests/test-data/data2.pkl");

  ASSERT_EQ(object->protocol, 2);
  ASSERT_EQ(object->totalLength, 116);
  ASSERT_EQ(object->frames.size(), 25);
  ASSERT_EQ(object->frames.at(0)->opcode, unpickler::opcode::EMPTY_DICT);
  ASSERT_EQ(object->frames.at(0)->frameSize, 0);
  ASSERT_EQ(object->frames.at(1)->opcode, unpickler::opcode::BINPUT);
  ASSERT_STREQ(object->frames.at(1)->content, "\x00");
  ASSERT_EQ(object->frames.at(2)->opcode, unpickler::opcode::MARK);
  ASSERT_EQ(object->frames.at(9)->opcode, unpickler::opcode::BINUNICODE);
  ASSERT_STREQ(object->frames.at(9)->content, "type_sizes");
  ASSERT_EQ(object->frames.at(21)->opcode, unpickler::opcode::BINPUT);
  ASSERT_EQ(object->frames.at(21)->frameSize, 1);
  ASSERT_STREQ(object->frames.at(21)->content, "\x07");

  delete object;
}

TEST(unpickle, unpickle_very_long_object) {
  /*
    0: \x80 PROTO      2
    2: c    GLOBAL     'collections OrderedDict'
   27: q    BINPUT     0
   29: )    EMPTY_TUPLE
   30: R    REDUCE
   31: q    BINPUT     1
   33: (    MARK
   34: X        BINUNICODE 'conv1.weight'
   51: q        BINPUT     2
   53: c        GLOBAL     'torch._utils _rebuild_tensor_v2'
   86: q        BINPUT     3
   88: (        MARK
   89: (            MARK
   90: X                BINUNICODE 'storage'
  102: q                BINPUT     4
  104: c                GLOBAL     'torch FloatStorage'
  124: q                BINPUT     5
  126: X                BINUNICODE '140731321996352'
  146: q                BINPUT     6
  148: X                BINUNICODE 'cpu'
  156: q                BINPUT     7
  158: M                BININT2    450
  161: N                NONE
  162: t                TUPLE      (MARK at 89)
  163: q            BINPUT     8
  165: Q            BINPERSID
  166: K            BININT1    0
  ...
  */
  unpickler::Unpickler unpickler;
  unpickler::PickleObject* object = unpickler.load("../tests/test-data/data2.pkl");

  ASSERT_EQ(object->protocol, 2);
  ASSERT_EQ(object->totalLength, 1132);

  delete object;
}
