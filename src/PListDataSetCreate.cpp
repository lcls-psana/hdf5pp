//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class PListDataSetCreate...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------
#include "Lusi/Lusi.h"

//-----------------------
// This Class's Header --
//-----------------------
#include "hdf5pp/PListDataSetCreate.h"

//-----------------
// C/C++ Headers --
//-----------------

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "hdf5pp/Exceptions.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace hdf5pp {

//----------------
// Constructors --
//----------------
PListDataSetCreate::PListDataSetCreate ()
  : m_impl(H5P_DATASET_CREATE)
{
}

//--------------
// Destructor --
//--------------
PListDataSetCreate::~PListDataSetCreate ()
{
}

// set chunk size
void
PListDataSetCreate::set_chunk ( int rank, const hsize_t chunk_size[] )
{
  herr_t stat = H5Pset_chunk ( m_impl.id(), rank, chunk_size ) ;
  if ( stat < 0 ) {
    throw Hdf5CallException ( "PListDataSetCreate::set_chunk", "H5Pset_chunk" ) ;
  }
}

// set chunk size for rank-1
void
PListDataSetCreate::set_chunk ( const hsize_t chunk_size )
{
  herr_t stat = H5Pset_chunk ( m_impl.id(), 1, &chunk_size ) ;
  if ( stat < 0 ) {
    throw Hdf5CallException ( "PListDataSetCreate::set_chunk", "H5Pset_chunk" ) ;
  }
}

// set deflate compression method
void
PListDataSetCreate::set_deflate ( unsigned level )
{
  herr_t stat = H5Pset_deflate ( m_impl.id(), level ) ;
  if ( stat < 0 ) {
    throw Hdf5CallException ( "PListDataSetCreate::set_deflate", "H5Pset_deflate" ) ;
  }
}



} // namespace hdf5pp