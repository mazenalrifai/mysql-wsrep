/*
   Copyright (c) 2006, 2014, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef DYNARR256_HPP
#define DYNARR256_HPP

#include "Pool.hpp"
#include <NdbMutex.h>

#define JAM_FILE_ID 299


class DynArr256;
struct DA256Page;

class DynArr256Pool
{
  friend class DynArr256;
public:
  DynArr256Pool();
  
  void init(Uint32 type_id, const Pool_context& pc);
  void init(NdbMutex*, Uint32 type_id, const Pool_context& pc);

  /**
    Memory usage data, used for populating Ndbinfo::pool_entry structures.
   */
  struct Info
  {
    // Number of pages (DA256Page) allocated.
    Uint32 pg_count;
    // Size of each page in bytes.
    Uint32 pg_byte_sz;
    // Number of nodes (DA256Node) in use.
    Uint64 inuse_nodes;
    // Size of each DA256Node in bytes.
    Uint32 node_byte_sz;
    // Number of nodes that fit in a page.
    Uint32 nodes_per_page;
  };
    
  const Info getInfo() const;
               
protected:
  Uint32 m_type_id;
  Uint32 m_first_free;
  Uint32 m_last_free;
  Pool_context m_ctx;
  struct DA256Page* m_memroot;
  NdbMutex * m_mutex;
  // Number of nodes (DA256Node) in use.
  Uint64 m_inuse_nodes;
  // Number of pages (DA256Page) allocated.
  Uint32 m_pg_count;  

private:
  Uint32 seize();
  void release(Uint32);
};

class DynArr256
{
public:
  class Head
  {
    friend class DynArr256;
  public:
#ifdef VM_TRACE
    Head() { m_ptr_i = RNIL; m_sz = 0; m_no_of_nodes = 0; m_high_pos = 0; }
#else
    Head() { m_ptr_i = RNIL; m_sz = 0; m_no_of_nodes = 0; }
#endif
    ~Head()
    {
      assert(m_sz == 0);
      assert(m_no_of_nodes == 0);
    }

    bool isEmpty() const { return m_sz == 0;}
    
    // Get allocated array size in bytes.
    Uint32 getByteSize() const;

  private:
    Uint32 m_ptr_i;
    Uint32 m_sz;
    // Number of DA256Nodes allocated.
    Int32 m_no_of_nodes;
#ifdef VM_TRACE
    Uint32 m_high_pos;
#endif
  };
  
  DynArr256(DynArr256Pool & pool, Head& head) : 
    m_head(head), m_pool(pool){}
  
  Uint32* set(Uint32 pos);
  Uint32* get(Uint32 pos) const ;
  
  struct ReleaseIterator
  {
    Uint32 m_sz;
    Uint32 m_pos;
    Uint32 m_ptr_i[5];
  };
  
  void init(ReleaseIterator&);
  /**
   * return 0 - done
   *        1 - data (in retptr)
   *        2 - nodata
   */
  Uint32 release(ReleaseIterator&, Uint32* retptr);
  Uint32 trim(Uint32 trim_pos, ReleaseIterator&);
  Uint32 truncate(Uint32 trunc_pos, ReleaseIterator&, Uint32* retptr);
protected:
  Head & m_head;
  DynArr256Pool & m_pool;
  
  bool expand(Uint32 pos);
  void handle_invalid_ptr(Uint32 pos, Uint32 ptrI, Uint32 p0);
};

inline
Uint32 DynArr256::release(ReleaseIterator& iter, Uint32* retptr)
{
  return truncate(0, iter, retptr);
}

inline
Uint32 DynArr256::trim(Uint32 pos, ReleaseIterator& iter)
{
  return truncate(pos, iter, NULL);
}


#undef JAM_FILE_ID

#endif
