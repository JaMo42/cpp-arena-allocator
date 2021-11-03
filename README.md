# cpp-arena-allocator

Region based [STL allocator](https://en.cppreference.com/w/cpp/named_req/Allocator).

## The allocator

```cpp
namespace arena
{
template <class T>
struct Allocator;
}
```

The allocator is stateless, that is, all instances of the given allocator are interchangeable, compare equal and can deallocate memory allocated by any other instance of the same allocator type.

The allocator satisfies [allocator completeness requirements](https://en.cppreference.com/w/cpp/named_req/Allocator#Allocator_completeness_requirements).

`std::allocator` is used as underlying allocator to allocate regions.

### Member types

- `value_type` -> `T`
- `size_type` -> `std::size_t`
- `difference_type` -> `std::ptrdiff_t`

- `propagate_on_container_move_assignment` -> `std::true_type`
- `is_always_equal` -> `std::true_type`

### Member functions

- `[[nodiscard]] T * allocate (std::size_t n, const T *hint = nullptr)`

  Allocates `n * sizeof (T)` bytes of uninitialized storage.
  If `hint` is provided, the allocator will try to place the new allocation in the same region as `hint`.

  If `n` is zero, a null pointer is returned.

- `void deallocate (T *p, std::size_t n)`

  Deallocates the storage referenced by the pointer `p`, which must be a pointer obtained by an earlier call to `allocate ()`.
  The argument `n` must be equal to the first argument of the call to `allocate ()` that originally produced `p`.

  If `p` is a null pointer, the function does nothing.

- `[[nodiscard]] T * reallocate (T *p, std::size_t from_n, std::size_t to_n, const T *hint = nullptr)`

  Reallocates the storage referenced by the pointer `p`, which must be a pointer obtained by an earlier call to `allocate ()` and not yet freed with `deallocate ()`.

  The reallocation is done by either:

    a) expanding or contracting the existing allocation referenced by `p`, if possible.
    The contents of the array remain unchanged up to the lesser of the new and old sizes.
    If the allocation is expanded the contents of the new part of the array are undefined.

    b) doing nothing, if the new size is less than the old size.

    c) allocating a new array that can hold `to_n` objects, copying `from_n` objects, and deallocating the old allocation.

  If `p` is a null pointer, the behavior is the same as calling `allocate (to_n, hint)`.

  If `to_n` is zero, the behavior is the same as calling `deallocate (p, from_n)`.

### Non-member functions

- `operator==`, always returns `true`
- `operator!=`, always returns `false`

## STL typedefs

The following types are automatically defined, if their STL headers are included before including `arena_alloc.hh`.

- `arena::basic_string`
  - `arena::string`
  - `arena::wstring`
  - `arena::u8string`
  - `arena::u16string`
  - `arena::u32string`
- `arena::deque`
- `arena::vector`
- `arena::forward_list`
- `arena::list`
- `arena::set`
- `arena::multiset`
- `arena::map`
- `arena::multimap`
- `arena::unordered_set`
- `arena::unordered_multiset`
- `arena::unordered_map`
- `arena::unordered_multimap`

These have the same template arguments except for the allocator type.

