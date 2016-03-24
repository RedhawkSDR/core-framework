#include <iostream>
#include <cstring>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

int main (int argc, char **argv )
{
   using namespace boost::interprocess;
   try{
      //Erase previous shared memory
      shared_memory_object::remove(argv[1]);

   }
   catch(interprocess_exception &ex){
      shared_memory_object::remove("shared_memory-a");
      std::cout << ex.what() << std::endl;
      return 1;
   }
   return 0;
}
