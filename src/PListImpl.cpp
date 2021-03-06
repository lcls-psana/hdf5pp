//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class PListImpl...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "hdf5pp/PListImpl.h"

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
PListImpl::PListImpl()
  : m_id(H5P_DEFAULT)
{
}

PListImpl::PListImpl ( hid_t cls )
  : m_id()
{
  m_id = H5Pcreate ( cls ) ;
  if ( m_id < 0 ) throw Hdf5CallException ( ERR_LOC, "H5Pcreate" ) ;
}

// copy constructor
PListImpl::PListImpl ( const PListImpl& o )
  : m_id()
{
  if (o.m_id == H5P_DEFAULT) {
    m_id = o.m_id;
  } else {
    m_id = H5Pcopy ( o.m_id ) ;
    if ( m_id < 0 ) throw Hdf5CallException ( ERR_LOC, "H5Pcopy" ) ;
  }
}

//--------------
// Destructor --
//--------------
PListImpl::~PListImpl ()
{
  if (m_id != H5P_DEFAULT) H5Pclose(m_id) ;
}

// assignment
PListImpl&
PListImpl::operator = ( const PListImpl& o )
{
  if ( &o != this ) {
    if (m_id != H5P_DEFAULT) {
      if ( H5Pclose(m_id) < 0 ) throw Hdf5CallException ( ERR_LOC, "H5Pclose" ) ;
    }
    if (o.m_id == H5P_DEFAULT) {
      m_id = o.m_id;
    } else {
      m_id = H5Pcopy ( o.m_id ) ;
      if ( m_id < 0 ) throw Hdf5CallException ( ERR_LOC, "H5Pcopy" ) ;
    }
  }
  return *this ;
}

// changes class, only if id is H5P_DEFAULT
void
PListImpl::setClass(hid_t cls) 
{
  if (m_id == H5P_DEFAULT) {
    m_id = H5Pcreate ( cls ) ;
    if ( m_id < 0 ) throw Hdf5CallException ( ERR_LOC, "H5Pcreate" ) ;
  }
}

} // namespace hdf5pp
