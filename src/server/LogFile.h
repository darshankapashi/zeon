#pragma once

/*
 * This class encapsulates a log file on disk
 */

#include "Structs.h"

class LogFile {
 public:
  LogFile() = default;
  ~LogFile() = default;

  int write(Record const& record);

 private:
  int fd_;
};
