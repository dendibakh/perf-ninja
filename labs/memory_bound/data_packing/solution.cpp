// NOTE: this lab is currently broken.
// After migrating to a new compiler version the speedup is no longer measurable consistently.
// You can still try to solve it to learn the concept, but the result is not guaranteed.

#include "solution.h"
#include <algorithm>
#include <random>
#include <memory>
template<class It,class F,bool DoUnsafeStackAlloc=false>void radix_sort_impl(It begin,It end,F f){const auto n=std::distance(begin,end);using T=std::remove_const_t<std::remove_reference_t<decltype(*std::declval<It>())>>;using SortType=std::invoke_result_t<F,T>;std::unique_ptr<T[]>up=DoUnsafeStackAlloc?nullptr:std::make_unique<T[]>(n);T stack_arr[DoUnsafeStackAlloc?n:1];T*second=DoUnsafeStackAlloc?&stack_arr[0]:up.get();std::array<std::size_t,256>count{};std::uint8_t or_of_all_bytes[sizeof(SortType)]{};std::uint8_t and_of_all_bytes[sizeof(SortType)]{};for(std::size_t i=0;i<sizeof(SortType);++i){or_of_all_bytes[i]=0;and_of_all_bytes[i]=-1;}for(std::ptrdiff_t i=0;i<n;++i){auto tmp=f(begin[i]);for(std::size_t which_byte=0;which_byte<sizeof(SortType);++which_byte){std::uint8_t byte=((std::uint8_t const*)&tmp)[which_byte];or_of_all_bytes[which_byte]|=byte;and_of_all_bytes[which_byte]&=byte;}}bool needs_swap=false;for(std::size_t which_byte=0;which_byte<sizeof(SortType);++which_byte){auto do_iter=[&](auto a,auto b){count.fill(0);for(std::ptrdiff_t i=0;i<n;++i){auto tmp=f(a[i]);if constexpr(std::is_signed_v<SortType>&&std::is_integral_v<SortType>){tmp^=SortType{1}<<(8*sizeof(SortType)-1);}std::uint8_t sort_byte=((std::uint8_t const*)&tmp)[which_byte];++count[sort_byte];}for(std::ptrdiff_t i=1;i<256;++i){count[i]+=count[i-1];}for(std::ptrdiff_t i=n;i-->0;){auto tmp=f(a[i]);if constexpr(std::is_signed_v<SortType>&&std::is_integral_v<SortType>){tmp^=SortType{1}<<(8*sizeof(SortType)-1);}std::uint8_t sort_byte=((std::uint8_t const*)&tmp)[which_byte];b[--count[sort_byte]]=std::move(a[i]);}};if(or_of_all_bytes[which_byte]!=and_of_all_bytes[which_byte]){if(needs_swap){do_iter(second,begin);}else{do_iter(begin,second);}needs_swap=!needs_swap;}}if(needs_swap)std::copy(second,second+n,begin);}template<class It,class F>auto radix_sort(It begin,It end,F&&sort_by,bool do_unsafe_stack_alloc=false){if(do_unsafe_stack_alloc)radix_sort_impl<It,F,true>(begin,end,sort_by);else radix_sort_impl<It,F,false>(begin,end,sort_by);}template<class It>auto radix_sort(It begin,It end,bool do_unsafe_stack_alloc=false){const auto identity_fn=[](auto&&x){return x;};radix_sort(begin,end,identity_fn,do_unsafe_stack_alloc);}

void solution(std::array<S, N> &arr) {
  // 1. shuffle
  static std::random_device rd;
  static std::mt19937 g(rd());
  std::shuffle(arr.begin(), arr.end(), g);

  // 2. sort
  radix_sort(arr.begin(), arr.end(), [](const S& x) { return x.i; });
}
