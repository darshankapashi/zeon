#pragma once

#include <string>
#include <sys/types.h>
#include <vector>

struct Blob {
  uint8_t* data;
  uint32_t len;

  Blob() : data(nullptr), len(0) {}

  ~Blob() {
    if (data) {
      delete [] data;
    }
  }
};

/*
 * Having a separate class managing each file can help to keep some files open
 * for a longer time. Hot keys can then be updated more efficiently.
 */
class FileOps {
 public:
   // TODO: In truncate mode, we actually want an atomic rename
   //         which will help in the case of open and fail to write
  FileOps(std::string name, bool truncate = false);
  ~FileOps();

  // Replace is always synced
  bool syncWriteToFile(Blob const&);

  // Append is not synced
  bool writeToFile(Blob const&);
 
   // Read from file. Make sure buffer is atleast as long as len
  bool readFromFile(Blob&);

  int getId() {
    return fd_;
  }

  static void getFilesInDir(std::string dir, std::vector<std::string> &files);
  static void createDir(std::string name);

 private:
   int fd_;
};