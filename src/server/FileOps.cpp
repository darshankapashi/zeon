#include "FileOps.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

FileOps::FileOps(std::string name, bool truncate) {
  auto flags = O_APPEND | O_CREAT | O_RDWR;
  if (truncate) {
    flags |= O_TRUNC;
  }
  fd_ = open(name.c_str(), flags);
  if (fd_ == -1) {
    throw std::runtime_error("Could not open file: " + name);
  }
}

bool FileOps::syncWriteToFile(Blob const& b) {
  int ret = write(fd_, &b.len, sizeof(b.len));
  ret +=  write(fd_, b.data, b.len);
  int sync = fsync(fd_);
  return (ret == sizeof(b.len) + b.len) && (sync == 0);
}

bool FileOps::writeToFile(Blob const& b) {
  int ret = write(fd_, &b.len, sizeof(b.len));
  ret +=  write(fd_, b.data, b.len);
  return (ret == sizeof(b.len) + b.len);
}

bool FileOps::readFromFile(Blob& b) {
  int ret = read(fd_, &b.len, sizeof(b.len));
  if (ret != sizeof(b.len)) {
    return false;
  }
  b.data = new uint8_t[b.len];
  ret = read(fd_, b.data, b.len);
  return (ret == b.len);
}

FileOps::~FileOps() {
  close(fd_);
}

void FileOps::getFilesInDir(std::string dir, std::vector<std::string>& files) {
  files.clear();
  DIR *dp;
  struct dirent *dirp;
  if ((dp  = opendir(dir.c_str())) == NULL) {
      throw std::runtime_error("Error opening " + dir + ": errno=" + std::to_string(errno));
  }
  while ((dirp = readdir(dp)) != NULL) {
    if (dirp->d_type == DT_REG) {
      files.push_back(std::string(dirp->d_name));
    }
  }
  closedir(dp);
}

void FileOps::createDir(std::string name) {
  struct stat st = {0};
  if (stat(name.c_str(), &st) == -1) {
    mkdir(name.c_str(), 0700);
  }  
}