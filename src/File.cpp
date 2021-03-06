//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class File...
//
// Author List:
//      Andrei Salnikov
//
//------------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "hdf5pp/File.h"

//-----------------
// C/C++ Headers --
//-----------------
#include <sstream>

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "hdf5pp/Exceptions.h"

//-----------------------------------------------------------------------
// Local Macros, Typedefs, Structures, Unions and Forward Declarations --
//-----------------------------------------------------------------------

namespace {

  // deleter for  boost smart pointer
  struct FilePtrDeleter {
    void operator()( hid_t* id ) {
      if ( id ) H5Fclose ( *id );
      delete id ;
    }
  };

  std::string CreateMode2str(hdf5pp::File::CreateMode mode) {
    if (mode == hdf5pp::File::Truncate) return "Truncate";
    if (mode == hdf5pp::File::Exclusive) return "Exclusive";
    return "*unknown*";
  }

  std::string OpenMode2str(hdf5pp::File::OpenMode mode) {
    if (mode == hdf5pp::File::Read) return "Read";
    if (mode == hdf5pp::File::Update) return "Update";
    return "*unknown*";
  }

}

//		----------------------------------------
// 		-- Public Function Member Definitions --
//		----------------------------------------

namespace hdf5pp {

//----------------
// Constructors --
//----------------
File::File ()
  : m_id ()
{
}

File::File ( hid_t id )
  : m_id ( new hid_t(id), FilePtrDeleter() )
{
}

//--------------
// Destructor --
//--------------
File::~File ()
{
}

/**
 *  Create new HDF5 file.
 */
File
File::create( const std::string& path,
                CreateMode mode,
                const PListFileCreate& plCreate,
                const PListFileAccess& plAccess )
{
  unsigned flags = H5F_ACC_EXCL ;
  if ( mode == Truncate ) flags = H5F_ACC_TRUNC ;
  hid_t f_id = H5Fcreate ( path.c_str(), flags, plCreate.plist(), plAccess.plist() ) ;
  if ( f_id < 0 ) {
    std::stringstream callMsg;
    callMsg << "H5Fcreate path=" << path <<" mode=" << CreateMode2str(mode);
    throw Hdf5CallException( ERR_LOC, callMsg.str()) ;
  }
  return File(f_id) ;
}

/**
 *  open existing HDF5 file.
 */
File
File::open( const std::string& path,
             OpenMode mode,
             const PListFileAccess& plAccess )
{
  unsigned flags = H5F_ACC_RDONLY ;
  if ( mode == Update ) flags = H5F_ACC_RDWR ;
  hid_t f_id = H5Fopen ( path.c_str(), flags, plAccess.plist() ) ;
  if ( f_id < 0 ) {
    std::stringstream callMsg;
    callMsg << "H5Fopen path=" << path <<" mode=" << OpenMode2str(mode);
    throw Hdf5CallException( ERR_LOC, callMsg.str()) ;
  }
  return File(f_id) ;
}

// close the file
void
File::close()
{
  m_id.reset();
}


} // namespace hdf5pp
