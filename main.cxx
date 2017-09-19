#include <string>
#include <iostream>
#include "ShmStringHashMap.h"


using namespace shm_string_hashmap;

int main (int argc, char *argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " [WHICH] create(c),update(u),append(a),read(r),destroy(d) [ID]" << std::endl;
    return 1;
  }
  const std::string which = argv[1];
  const std::string id = argv[2];

  std::string shm_name = "MizhangSharedMemory";
  std::string hashtable_name = "ShmHashMap1";
  int shm_size = 655350;
  int hash_size = 3000;
  int record_num = 10;

  //Create
  if(which=="c"){
    std::cout<<id<<" "+which+" "<<":"<<"Creating"<<std::endl;

    boost::interprocess::shared_memory_object::remove(shm_name.c_str());

    ShmStringHashMap shm_hash(shm_name,hashtable_name,shm_size,hash_size);
    std::cout<<id<<" "+which+" "<<":"<<"Free Memory: ["<<shm_hash.get_free_memory()<<"] bytes"<<std::endl;

    for(int i = 0; i < record_num; ++i) {
      std::cout<<id<<" "+which+" "<<":"<<"Creating: Shm Hash: "<<i<<" -> create"<<std::endl;
      shm_hash.insert(to_string(i),"create");
    }

    std::cout<<id<<" "+which+" "<<":"<<"Free Memory: ["<<shm_hash.get_free_memory()<<"] bytes"<<std::endl;
    std::cout<<id<<" "+which+" "<<":"<<"Creating done"<<std::endl;
  }

  //Update
  if(which=="u"){
    std::cout<<id<<" "+which+" "<<":"<<"Updating"<<std::endl;

    ShmStringHashMap shm_hash(shm_name,hashtable_name,shm_size,hash_size);
    std::cout<<id<<" "+which+" "<<":"<<"Free Memory: ["<<shm_hash.get_free_memory()<<"] bytes"<<std::endl;
    for(int i = 0; i < record_num; ++i) {
      std::string val = to_string(id);
      std::cout<<id<<" "+which+" "<<":"<<"Updating: Shm Hash: "<<i<<" -> "<<val<<std::endl;
      shm_hash.insert(to_string(i),val);
    }

    std::cout<<id<<" "+which+" "<<":"<<"Free Memory: ["<<shm_hash.get_free_memory()<<"] bytes"<<std::endl;
    std::cout<<id<<" "+which+" "<<":"<<"Updating done"<<std::endl;
  }

  //Append
  if(which=="a"){
    std::cout<<id<<" "+which+" "<<":"<<"Appending"<<std::endl;

    ShmStringHashMap shm_hash(shm_name,hashtable_name,shm_size,hash_size);
    std::cout<<id<<" "+which+" "<<":"<<"Free Memory: ["<<shm_hash.get_free_memory()<<"] bytes"<<std::endl;
    for(int i = 0; i < record_num; ++i) {
      std::string val = "+" + to_string(id);
      std::cout<<id<<" "+which+" "<<":"<<"Appending: Shm Hash: "<<i<<" -> "<<val<<std::endl;
      shm_hash.append(to_string(i),val);
    }

    std::cout<<id<<" "+which+" "<<":"<<"Appending done"<<std::endl;
    std::cout<<id<<" "+which+" "<<":"<<"Free Memory: ["<<shm_hash.get_free_memory()<<"] bytes"<<std::endl;
  }

  //Read
  if(which=="r"){
    std::cout<<id<<" "+which+" "<<":"<<"Reading"<<std::endl;

    ShmStringHashMap shm_hash(shm_name,hashtable_name,shm_size,hash_size);
    for(int i = 0; i < record_num; ++i) {
      //boost::this_thread::sleep( boost::posix_time::milliseconds(100) );
      std::string val;
      if(shm_hash.find(to_string(i),val)){
        std::cout<<id<<" "+which+" "<<":"<<"Reading Shm Hash: "<<i<<" -> "<<val<<std::endl;
      }
    }

    std::cout<<id<<" "+which+" "<<":"<<"Reading done"<<std::endl;
  }

  //Read
  if(which=="v"){
    std::cout<<id<<" "+which+" "<<":"<<"Dumping"<<std::endl;
    ShmStringHashMap shm_hash(shm_name,hashtable_name,shm_size,hash_size);
    shm_hash.dump();
    std::cout<<id<<" "+which+" "<<":"<<"Dumping done"<<std::endl;
  }


  //Destroy
  if(which=="d"){
    std::cout<<id<<" "+which+" "<<":"<<"Destroying"<<std::endl;

    ShmStringHashMap shm_hash(shm_name,hashtable_name);
    shm_hash.destroy();
    for(int i = 0; i < record_num; ++i) {
      std::string val;
      if(shm_hash.find(to_string(i),val)){
        std::cout<<id<<" "+which+" "<<":"<<"Shm Hash after destroy: "<<i<<" -> "<<val<<std::endl;
      } else {
        std::cout<<id<<" "+which+" "<<":"<<"Can't find anything for Shm Hash after destroy: "<<i<<" -> "<<val<<std::endl;
      }
    }

    std::cout<<id<<" "+which+" "<<":"<<"Destroying done"<<std::endl;
  }
  return 0;
}
