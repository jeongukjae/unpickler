# unpickler

![Run Test Status](https://github.com/jeongukjae/unpickler/workflows/Run%20Test/badge.svg)

Parser to parse frames in Python's pickle in C++.

## Usage

### `CMakeLists.txt`

```cmake
include_directories("${PROJECT_SOURCE_DIR}/third_party/unpickler/include")
```

### In C++ Code

- parse from path

  ```c++
  unpickler::Unpickler unpickler;
  unpickler::PickleObject* object = unpickler.load("./tests/test-data/data1.pkl");

  delete object;
  ```

- parse from char array

  ```c++

  const char* pickled = "\x80\x03X\x0c\x00\x00\x00hello world!q\x00.";

  unpickler::Unpickler unpickler;
  unpickler::PickleObject* object = unpickler.loads(pickled, 22);

  delete object;
  ```

- Notes
  - `unpickler` do not save protocol verion in frame. Protocol version is in `unpickler::PikcleObject::protocol`.
  - `unpickler` do not save stop frame.
  - You can check total length of parsed pickled object using `unpickler::PickleOjbect::totalLength`.

## References

- https://docs.python.org/3.8/library/pickle.html
- https://www.python.org/dev/peps/pep-0307/
- https://www.python.org/dev/peps/pep-3154/
- https://www.python.org/dev/peps/pep-0574/
