#ifndef HDF5PP_UTILS_H
#define HDF5PP_UTILS_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class Utils.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

//----------------------
// Base Class Headers --
//----------------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "hdf5pp/ArrayType.h"
#include "hdf5pp/Group.h"
#include "hdf5pp/DataSet.h"
#include "ndarray/ndarray.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

namespace hdf5pp {

/// @addtogroup hdf5pp

/**
 *  @ingroup hdf5pp
 *
 *  @brief Set of utility methods to facilitate work with HDF5 library
 *
 *  This software was developed for the LCLS project.  If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @version $Id$
 *
 *  @author Andy Salnikov
 */

class Utils  {
public:

  /**
   *  @brief Read one object from dataset
   *
   *  Reads one object from scalar or 1-dimensional dataset. If dataset is scalar then
   *  index must be set to -1, any other value means that dataset is 1-dimensional
   *  and an object at given index will be read. Template parameter defines the type
   *  of the returned object, this class must define static method native_type()
   *  which returns hdf5pp::Type object.
   *
   *  @param[in] ds    dataset object
   *  @param[in] index Object index, if negative then dataset must be scalar
   *
   *  @throw hdf5pp::Exception
   */
  template <typename Data>
  static boost::shared_ptr<Data> readDataSet(hdf5pp::DataSet<Data> ds, hsize_t index = -1)
  {
    hdf5pp::DataSpace file_dsp = ds.dataSpace();
    if (index != hsize_t(-1)) file_dsp.select_single(index);
    boost::shared_ptr<Data> ptr = boost::make_shared<Data>();
    ds.read(hdf5pp::DataSpace::makeScalar(), file_dsp, ptr.get(), TypeTraits<Data>::native_type());
    return ptr;
  }


  /**
   *  @brief Read ndarray from dataset.
   *
   *  Reads one object from 1-dimensional dataset or the whole dataset.
   *  if index is not specified then whole dataset is read, ndarray rank
   *  must be the same as dataset rank. If index is specified (and is not -1)
   *  the dataset must have rank 1, single element of dataset will be read
   *  and dataset type must be array with the same rank as ndarray rank.
   *
   *  @param[in] ds    dataset object
   *  @param[in] index Object index, if negative then whole dataset is read.
   *
   *  @throw hdf5pp::Exception
   */
  template <typename Data, unsigned Rank>
  static ndarray<Data, Rank> readNdarray(hdf5pp::DataSet<Data> ds, hsize_t index = -1)
  {
    hdf5pp::DataSpace file_dsp = ds.dataSpace();

    unsigned rank = 0;
    const unsigned MaxRank = 6;
    hsize_t dims[MaxRank];
    if (index == hsize_t(-1)) {

      // read whole dataset, has to know its rank and dimensions
      rank = file_dsp.rank();

      // check rank
      if (rank > MaxRank) throw Hdf5RankTooHigh(ERR_LOC, rank, MaxRank);
      if (rank != Rank) throw Hdf5RankMismatch(ERR_LOC, Rank, rank);

      file_dsp.dimensions(dims);

    } else {

      // read one element from rank-1 dataset, array dimensions should come from
      // the type of data stored which must have array type

      // this will throw if type of data is not an array
      ArrayType type = ArrayType(ds.type());

      // check array type rank, get dimensions
      rank = type.rank();
      if (rank > MaxRank) throw Hdf5RankTooHigh(ERR_LOC, rank, MaxRank);
      if (rank != Rank) throw Hdf5RankMismatch(ERR_LOC, Rank, rank);
      type.dimensions(dims);

      // select single item for dataset
      file_dsp.select_single(index);

    }

    // make ndarray
    unsigned shape[MaxRank];
    std::copy(dims, dims+rank, shape);
    ndarray<Data, Rank> array(shape);

    // read it
    DataSpace mem_dsp = DataSpace::makeSimple(rank, dims, dims);
    ds.read(mem_dsp, file_dsp, array.data());

    return array;

  }


  /**
   *  @brief Read object from a named dataset.
   *
   *  Meaning and operation are the same as for readDataSet() method, the difference
   *  is that dataset with the given name and parent is open first, then readDataSet()
   *  is called on that object.
   *
   *  @param[in] group   Group object, parent of the dataset.
   *  @param[in] dataset Dataset name
   *  @param[in] index   Object index, if negative then dataset must be scalar
   *
   *  @throw hdf5pp::Exception
   */
  template <typename Data>
  static boost::shared_ptr<Data> readGroup(hdf5pp::Group group, const std::string& dataset, hsize_t index = -1)
  {
    return readDataSet(group.openDataSet<Data>(dataset), index);
  }

  /**
   *  @brief Read ndarray from a named dataset.
   *
   *  Meaning and operation are the same as for readNdarray(Dataset) method, the difference
   *  is that dataset with the given name and parent is open first, then readDataSet()
   *  is called on that object.
   *
   *  @param[in] group   Group object, parent of the dataset.
   *  @param[in] dataset Dataset name
   *  @param[in] index   Object index, if negative then dataset must be scalar
   *
   *  @throw hdf5pp::Exception
   */
  template <typename Data, unsigned Rank>
  static ndarray<Data, Rank> readNdarray(hdf5pp::Group group, const std::string& dataset, hsize_t index = -1)
  {
    return readNdarray<Data, Rank>(group.openDataSet<Data>(dataset), index);
  }

};

} // namespace hdf5pp

#endif // HDF5PP_UTILS_H
