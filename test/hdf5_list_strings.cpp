#include "hdf5pp/File.h"
#include "hdf5pp/Utils.h"
#include <cstdio>
#include <stdexcept>
#include <vector>
#include <string>
#include <iostream>

// helper class to create a test file name 
// for a test, and remove it in the desctructor
struct TestFile {
  std::string fname;
  TestFile(std::string ext="") {
    fname = std::tmpnam(NULL);
    if (fname.size()==0) throw std::runtime_error("std::tmpname returned null string");
    fname += ext;
  }
  
  ~TestFile() {
    if (FILE * f = fopen(fname.c_str(), "r")) {
      fclose(f);
      if( 0 != std::remove(fname.c_str())) {
        perror( "Error deleting file" );
      }
    }
  };
};

    
void test_store() {
  std::vector<std::string> writeListStrings;
  writeListStrings.push_back("hi there");
  writeListStrings.push_back("one");
  writeListStrings.push_back("");
  writeListStrings.push_back("another string");

  TestFile fname(".h5");
  hdf5pp::File h5out = hdf5pp::File::create(fname.fname,hdf5pp::File::Truncate);
  

  hdf5pp::Group group = h5out.createGroup("group");
  hdf5pp::Utils::storeListStrings(group, "data", writeListStrings);
  group.close();
  h5out.close();

  hdf5pp::File h5in = hdf5pp::File::open(fname.fname, hdf5pp::File::Read);
  if (!h5in.valid()) throw std::runtime_error("h5in file is not valid");

  group = h5in.openGroup("group");
  std::vector<std::string> readListStrings = hdf5pp::Utils::readListStrings(group, "data");
  group.close();
  h5in.close();

  if (!(readListStrings == writeListStrings)) {
    throw std::runtime_error("readList not the same as write list");
  }
}

void test_storeat() {
  throw std::runtime_error("test_storeat - not implemented");
}

int main() {
  test_store();

  // storeat is not implemented yet
  //  test_storeat();

  std::cout << "tests passed" << std::endl;
  return 0;
}
