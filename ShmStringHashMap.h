#ifndef __SHM_STRING_HASH_MAP__H_
#define __SHM_STRING_HASH_MAP__H_

#include <string>
#include <iostream>

#include <boost/interprocess/shared_memory_object.hpp>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>
#include <functional>

#include <boost/interprocess/sync/interprocess_upgradable_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <boost/interprocess/sync/upgradable_lock.hpp>


namespace shm_string_hashmap {
  typedef boost::interprocess::allocator<char, boost::interprocess::managed_shared_memory::segment_manager> CharAllocator;
  typedef boost::interprocess::basic_string<char, std::char_traits<char>, CharAllocator> ShmString;
  typedef ShmString KeyType;
  typedef ShmString MappedType;
  typedef std::pair<const KeyType, MappedType> ValueType;
  typedef boost::interprocess::allocator<ValueType, boost::interprocess::managed_shared_memory::segment_manager> ShmAlloc;
  typedef boost::unordered_map<KeyType, MappedType, boost::hash<KeyType>, std::equal_to<KeyType>, ShmAlloc> ShmHashMap;

  template<class T>
  inline std::string to_string(const T &val)
  { std::ostringstream ostr; ostr << val; return ostr.str(); }

  inline std::string to_string(const ShmString &val)
  { return std::string(val.begin(), val.end());}

  template<class T>
  inline ShmString to_shm_string(const T &val, boost::interprocess::managed_shared_memory & segment){
    std::ostringstream ostr;
    ostr << val;
    return ShmString(ostr.str().c_str(),segment.get_allocator<ShmString>());
  }

  using boost::unordered_map;
  class ShmSafeHashMap {
  private:
    typedef boost::interprocess::interprocess_upgradable_mutex upgradable_mutex_type;
    mutable upgradable_mutex_type m_mutex;
    ShmHashMap m_shm_hashmap;

  public:
    explicit ShmSafeHashMap(size_t bucket_count,
                            const boost::hash<KeyType>& hash,
                            const std::equal_to<KeyType>& equal,
                            const ShmAlloc& alloc):
      m_shm_hashmap(bucket_count, hash, equal, alloc){}

    bool find(const ShmString & key, std::string & val) const {
      boost::interprocess::sharable_lock<upgradable_mutex_type> lock(m_mutex);
      ShmHashMap::const_iterator iter = m_shm_hashmap.find(key);
      if (iter == m_shm_hashmap.end()) {
        return false;
      }
      val = to_string(iter->second);
      return true;
    }

    bool insert(const ShmString & key, const ShmString & val){
      boost::interprocess::scoped_lock<upgradable_mutex_type> lock(m_mutex);
      ShmHashMap::iterator it = m_shm_hashmap.find(key);
      if(it!=m_shm_hashmap.end()){
        it->second = val;
        return true;
      }

      return m_shm_hashmap.insert(ValueType(key,val)).second;
    }

    void dump(){
      boost::interprocess::sharable_lock<upgradable_mutex_type> lock(m_mutex);
      ShmHashMap::const_iterator iter = m_shm_hashmap.begin();
      for(;iter!=m_shm_hashmap.end();++iter){
        std::cout<<iter->first<<" "<<iter->second<<std::endl;
      }
    }

    size_t size(){
      return m_shm_hashmap.size();
    }
  };

  class ShmStringHashMap {
  private:
    std::string m_shm_name;
    int m_shm_bytes;

    std::string m_hashmap_name;
    int m_hashmap_size;

    mutable boost::interprocess::managed_shared_memory m_segment;
    ShmSafeHashMap * m_shm_hashmap_ptr;

    bool checkValid() const {
      if(!m_shm_hashmap_ptr){
        std::cout<<"ERROR:m_shm_hashmap_ptr failed to initialize!"<<std::endl;
        return false;
      }

      return true;
    }

  public:
    /*Constructor*/
    //open or create
    ShmStringHashMap(const std::string & shm_name, const std::string & hashmap_name,
                     const int & shm_bytes=655350, const int & hashmap_size=3000):
      m_shm_name(shm_name), m_shm_bytes(shm_bytes),
      m_hashmap_name(hashmap_name),m_hashmap_size(hashmap_size),
      m_segment(boost::interprocess::open_or_create, m_shm_name.c_str(), m_shm_bytes){
      //If anything fails, throws interprocess_exception
      //cannot use open_read_only because mutex inside ShmSafeHashMap will be changed

      //can also use boost::interprocess::unique_instance if you only need one uniq object without naming it
      m_shm_hashmap_ptr = m_segment.find_or_construct<ShmSafeHashMap>(m_hashmap_name.c_str())
        //unordered_map constructor params
        (m_hashmap_size,                       // initial bucket count
         boost::hash<KeyType>(),               // the hash function
         std::equal_to<KeyType>(),             // the equality function
         m_segment.get_allocator<ValueType>());  // the allocator

      checkValid();
    }

    /*Insert*/
    bool insert(const std::string & key, const std::string & val){
      //check
      if(!checkValid()){ return false; }

      //find
      ShmString shm_key = to_shm_string(key,m_segment);
      ShmString shm_val = to_shm_string(val,m_segment);
      return m_shm_hashmap_ptr->insert(shm_key, shm_val);
    }

    bool append(const std::string & key,std::string & val){
      //check
      if(!checkValid()){ return false; }

      //find
      ShmString shm_key = to_shm_string(key,m_segment);
      std::string orig_val;
      m_shm_hashmap_ptr->find(shm_key,orig_val);

      //insert
      ShmString shm_val = to_shm_string(orig_val+val,m_segment);
      return m_shm_hashmap_ptr->insert(shm_key, shm_val);
    }

    /*Find*/
    bool find(const std::string & key,std::string & val) const {
      if(!checkValid()){ return false; }

      return m_shm_hashmap_ptr->find(to_shm_string(key,m_segment), val);
    }

    /*Dump*/
    void dump() const {
      if(!checkValid()){ return; }
      m_shm_hashmap_ptr->dump();
    }

    /*Destroy*/
    bool destroy(){
      return m_segment.destroy<ShmSafeHashMap>(m_hashmap_name.c_str());
    }

    /*Size*/
    size_t size() const {
      if(!checkValid()){ return 0; }
      return m_shm_hashmap_ptr->size();
    }

    /*Free Memory (bytes)*/
    size_t get_free_memory() const {
      return m_segment.get_free_memory();
    }

  };

}//namespace

#endif // __SHM_STRING_HASH_MAP__H_
