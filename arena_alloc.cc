#include "arena_alloc.hh"
#include <memory>
#include <vector>
#include <mutex>
#include <cstring>

namespace arena
{
namespace detail
{

static std::allocator<char> S_alloc {};

struct Region
{
  enum : std::size_t { S_capacity = 4096 };

  Region (std::size_t min_cap)
    : M_capacity (std::max (static_cast<std::size_t> (S_capacity), min_cap))
    , M_data (S_alloc.allocate (M_capacity))
    , M_size (0)
    , M_ref_count (0)
  {}

  void destruct () { S_alloc.deallocate (M_data, M_capacity); }

  char * data () { return M_data; }
  char * top () { return M_data + M_size; }
  char * end () { return M_data + M_capacity; }
  void resize (std::ptrdiff_t diff) { M_size += diff; }
  void clear () { M_size = 0; }
  void ref () { ++M_ref_count; }
  void unref () { --M_ref_count; }
  bool unused () const { return M_ref_count == 0; }

private:
  const std::size_t M_capacity;
  char *M_data;
  std::size_t M_size;
  unsigned M_ref_count;
};

static std::vector<Region> S_regions {};

using region_iterator = decltype (S_regions)::iterator;

static struct RegionDeleter
{
  RegionDeleter ()
  {
    S_regions.reserve (4);
  }

  ~RegionDeleter ()
  {
    for (auto &r : S_regions)
      r.destruct ();
  }
} const S_region_deleter {};

static std::mutex S_mutex {};

Lock::Lock ()
{
  S_mutex.lock ();
}

Lock::~Lock ()
{
  S_mutex.unlock ();
}

static region_iterator
find_region_containing (const char *p)
{
  const auto end = S_regions.end ();
  for (auto it = S_regions.begin (); it != end; ++it)
    {
      if (p >= it->data () && p < it->top ())
        return it;
    }
  return end;
}

static region_iterator
find_region_fitting (std::size_t n, const char *hint)
{
  const auto end = S_regions.end ();
  region_iterator it;

  if (hint)
    {
      it = find_region_containing (hint);
      if ((it != end) && (it->top () + n < it->end ()))
        return it;
    }

  for (it = S_regions.begin (); it != end; ++it)
    {
      if (it->top () + n < it->end ())
        return it;
    }
  return end;
}

char *
allocate (std::size_t n, const char *hint)
{
  auto it = find_region_fitting (n, hint);
  if (it == S_regions.end ())
    {
      S_regions.emplace_back (n);
      it = std::prev (S_regions.end ());
    }
  const auto r = it->top ();
  it->resize (n);
  it->ref ();
  return r;
}

void
deallocate (char *p, std::size_t n)
{
  const auto it = find_region_containing (p);
  if (it == S_regions.end ())
    return;
  it->unref ();
  if (it->unused ())
    it->clear ();
  else if (it->top () - n == p)
    it->resize (0ll - n);
}

char *
reallocate (char *p, std::size_t from_n, std::size_t to_n, const char *hint)
{
  if (p == nullptr)
    return allocate (to_n, hint);
  const auto it = find_region_containing (p);
  if (it == S_regions.end ())
    return nullptr;
  if (to_n == 0)
    {
      deallocate (p, from_n);
      return nullptr;
    }
  const std::ptrdiff_t diff = to_n - from_n;
  if (it->top () - from_n == p && it->top () + diff < it->end ())
    {
      it->resize (diff);
      return p;
    }
  if (to_n <= from_n)
    return p;
  char *const new_p = allocate (to_n, hint);
  std::memcpy (new_p, p, from_n);
  deallocate (p, from_n);
  return new_p;
}

std::size_t
default_region_size ()
{
  return Region::S_capacity;
}

} // namespace detail

} // namespace arena

