import pickle

with open("./tests/test-data/data1.pkl", "wb") as f:
  f.write(pickle.dumps("hello world!"))
