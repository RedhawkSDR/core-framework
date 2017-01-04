#include <iostream>
#include <cstring>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/algorithm/string.hpp>


std::string clean_fname( const std::string &fname ) {
  return boost::replace_all_copy( fname, "/", "-" );
}

int main (int argc, char **argv )
{

  if ( argc < 2 ) {
    printf("usage cleanmem <named memory segment>\n");
    return -1;
  }

   using namespace boost::interprocess;
   try{
      //Erase previous shared memory
     std::string fname = clean_fname(argv[1]);
     shared_memory_object::remove(fname.c_str());

   }
   catch(interprocess_exception &ex){
      std::cout << ex.what() << std::endl;
      return -1;
   }

   return 0;
}
