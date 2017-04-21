//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class Utils...
//
// Author List:
//      Andy Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "hdf5pp/Utils.h"
#include "hdf5pp/Exceptions.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <iostream>
#include <algorithm>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace hdf5pp {

// Create a chunked rank=1 dataset.
DataSet
Utils::createDataset(hdf5pp::Group group, const std::string& dataset, const Type& stored_type,
    hsize_t chunk_size, hsize_t chunk_cache_size, int deflate, bool shuffle)
{
  // make extensible data space
  hdf5pp::DataSpace dsp = hdf5pp::DataSpace::makeSimple(0, H5S_UNLIMITED);

  // use chunking
  hdf5pp::PListDataSetCreate plDScreate ;
  plDScreate.set_chunk(chunk_size) ;

  // optionally set filters
  if (shuffle) plDScreate.set_shuffle() ;
  if (deflate >= 0) plDScreate.set_deflate(deflate) ;

  // set chunk cache parameters
  hdf5pp::PListDataSetAccess plDSaccess;
  size_t chunk_cache_bytes = chunk_cache_size * chunk_size * stored_type.size();
  // ideally this number should be prime, can live with non-prime for now
  size_t rdcc_nslots = chunk_cache_size*19;
  plDSaccess.set_chunk_cache(rdcc_nslots, chunk_cache_bytes);

  // make a data set
  return group.createDataSet(dataset, stored_type, dsp, plDScreate, plDSaccess);
}

// Resize rank-1 dataset.
void
Utils::resizeDataset(hdf5pp::Group group, const std::string& dataset, long size)
{
  // open dataset
  DataSet ds = group.openDataSet(dataset);

  if (size < 0) {
    // get current size and increase by one
    DataSpace dsp = ds.dataSpace();
    size = dsp.size() + 1;
  }

  // change extent to new value
  ds.set_extent(size);
}

// template-free implementation of append()
void
Utils::_storeAt(hdf5pp::Group group, const std::string& dataset, const void* data,
    long index, const Type& native_type)
{
  // open dataset
  DataSet ds = group.openDataSet(dataset);

  // get current size
  DataSpace dsp = ds.dataSpace();
  hsize_t size = dsp.size();

  // extend dataset if needed
  if (index < 0) index = size;
  if (index >= long(size)) {
    ds.set_extent(index+1);
  }

  // get updated dataspace
  dsp = ds.dataSpace();

  // store the data in dataset
  ds.store(DataSpace::makeScalar(), dsp.select_single(index), data, native_type);
}

// template-free implementation of storeScalar()
void
Utils::_storeScalar(hdf5pp::Group group, const std::string& dataset, const void* data,
    const Type& native_type, const Type& stored_type)
{
  // create new scalar dataset
  DataSet ds = group.createDataSet(dataset, stored_type, DataSpace::makeScalar());

  // store the data in dataset
  ds.store(DataSpace::makeScalar(), DataSpace::makeScalar(), data, native_type);
}

// template-free implementation of storeArray()
void
Utils::_storeArray(hdf5pp::Group group, const std::string& dataset, const void* data,
    unsigned rank, const unsigned* shape, const Type& native_type, const Type& stored_type)
{
  hsize_t size = std::accumulate(shape, shape+rank, hsize_t(1), std::multiplies<hsize_t>());
  if (size > 0) {
    // store it in simple dataset

    DataSpace dsp;
    if (rank <= 8) {
      hsize_t dims[8];
      std::copy(shape, shape+rank, dims);
      dsp = DataSpace::makeSimple(rank, dims, dims);
    } else {
      std::vector<hsize_t> dims(shape, shape+rank);
      dsp = DataSpace::makeSimple(rank, &dims.front(), &dims.front());
    }

    // create new dataspace
    DataSet ds = group.createDataSet(dataset, stored_type, dsp);

    // store the data in dataset
    ds.store(dsp, dsp, data, native_type);

  } else {

    // for empty data set make null dataspace
    DataSpace dsp = DataSpace::makeNull() ;
    DataSet ds = group.createDataSet(dataset, stored_type, dsp);

  }
}

std::vector<std::string> Utils::readListStrings(hdf5pp::Group group, 
                                                const std::string &dataset, 
                                                hsize_t index)
{
  hdf5pp::DataSet ds = group.openDataSet(dataset);
  hdf5pp::DataSpace sp = ds.dataSpace();
  if (1 != sp.rank()) throw hdf5pp::Hdf5RankMismatch(ERR_LOC, 1, sp.rank()); 
  hdf5pp::Type tp = ds.type();  
  if (tp.tclass() == H5T_STRING) {
    if (index != -1) throw hdf5pp::Exception(ERR_LOC, 
                                             "readListStrings",
                                             "cannot specify index for dataset that is array of string");
  } else if (tp.tclass() == H5T_ARRAY) {
    throw hdf5pp::Exception(ERR_LOC, "readListStrings",
                            "unexpected: read requested fro a list of strings from a stacked array, not implemented");
  }
  
  if (tp.tclass() != H5T_STRING) {
    std::cerr << "tp.tclass()=" << tp.tclass() << std::endl;
    throw hdf5pp::Exception(ERR_LOC, "readListStrings", "tclass() is neither H5T_STRING nor H5T_ARRAY");
  }
  
  unsigned maxStringLen = tp.size();  
  std::vector<char> buffer(maxStringLen+1);
  for (unsigned idx = 0; idx < buffer.size(); ++idx) buffer.at(idx)=0;
  hdf5pp::DataSpace memDspc = hdf5pp::DataSpace::makeSimple(1,1);//maxStringLen, maxStringLen);

  std::vector<std::string> listStrings;
  for (unsigned idx = 0; idx < sp.size(); ++idx) {
    hdf5pp::DataSpace fileDspc = sp.select_single(hsize_t(idx));
    ds.read<char>(memDspc, fileDspc, &buffer[0], tp);
    //    herr_t res = H5Dread(ds.id(), tp.id(), memDspc.id(), fileDspc.id(), H5P_DEFAULT, 
    //            static_cast<void *>(&buffer[0]));
    //    if (res < 0) throw hdf5pp::Hdf5CallException(ERR_LOC, "H5Dread failed");
    listStrings.push_back(std::string(&buffer[0]));
  }
  return listStrings;
}

void Utils::storeListStrings(hdf5pp::Group group, const std::string &dataset, 
                             const std::vector<std::string> & listStrings, long maxStrLen)
{
  if (maxStrLen < 0) {
    maxStrLen = 1;
    for (unsigned idx = 0; idx < listStrings.size(); ++idx) {
      maxStrLen = std::max(maxStrLen, 1+long(listStrings.at(idx).size()));
    }
  }

  char storeData[maxStrLen * listStrings.size()];
  char *dest = storeData;
  for (unsigned idx = 0; idx < listStrings.size(); ++idx) {
    const char *src = listStrings.at(idx).c_str();
    strncpy(dest,src,maxStrLen);
    dest += maxStrLen;
  }
  unsigned shape[1] = { listStrings.size() };
  hdf5pp::Type sizedString = hdf5pp::TypeTraitsHelper::string_h5type(maxStrLen);
  _storeArray(group, dataset, static_cast<const void*>(storeData),
              1, shape, sizedString, sizedString);  
}

void Utils::storeListStringsAt(hdf5pp::Group group, const std::string &dataset, 
                               const std::vector<std::string> & listStrings, long maxStrLen, long index)
{
    throw hdf5pp::Exception(ERR_LOC, "storeListStringsAt", "unexpected, not implemented");
}


} // namespace hdf5pp
