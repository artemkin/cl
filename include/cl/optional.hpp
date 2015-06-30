
#pragma once

#include <functional>
#include <string>
#include <type_traits>

namespace cl
{

class bad_optional_access : public std::logic_error
{
public:
   explicit bad_optional_access(const std::string& what_arg) : std::logic_error(what_arg) {}
   explicit bad_optional_access(const char* what_arg)        : std::logic_error(what_arg) {}
};

/// !!! TODO:
// 1. Check casts (T*) and reinterpret_cast. Is this correct to use them there?
template<typename T>
class optional
{
public:
   typedef T value_type;

   optional() noexcept : init_(false) {}

   optional(const optional& rhs)
      : init_(rhs.init_)
   {
      if (init_)
         new (&data_) T(*rhs);
   }

   optional(optional&& rhs)
      : init_(rhs.init_)
   {
      if (init_)
         (*(*this)) = std::move(*rhs);
   }

   optional(const T& v)
   {
      new (&data_) T(v);
      init_ = true;
   }

   optional(T&& v)
   {
      (*(*this)) = std::move(v);
      init_ = true;
   }

   optional& operator=(optional rhs)
   {
      swap(rhs);
      return *this;
   }

   ~optional()
   {
      if (init_)
         (*this)->T::~T();
   }

   void swap(optional& rhs)
   {
      if (!init_ && !rhs.init_)
         return;

      if (init_ && !rhs.init_)
      {
         *rhs = std::move(**this);
         (*this)->T::~T();
         std::swap(init_, rhs.init_);
         return;
      }

      if (!init_ && rhs.init_)
      {
         **this = std::move(*rhs);
         rhs->T::~T();
         std::swap(init_, rhs.init_);
         return;
      }

      if (init_ && rhs.init_)
         std::swap(**this, *rhs);
   }

   const T* operator->() const
   {
      return reinterpret_cast<const T*>(&data_);
   }

   T* operator->()
   {
      return reinterpret_cast<T*>(&data_);
   }

   const T& operator*() const
   {
      // !!! TODO add assert on init_
      return reinterpret_cast<const T&>(data_);
   }

   T& operator*()
   {
      // !!! TODO add assert on init_
      return reinterpret_cast<T&>(data_);
   }

   explicit operator bool() const noexcept
   {
      return init_;
   }

   const T& value() const
   {
      if (!init_)
         throw bad_optional_access("");

      return **this;
   }

   T& value()
   {
      if (!init_)
         throw bad_optional_access("");

      return **this;
   }

   template <typename U> T value_or(U&& v) const&
   {
      if (init_)
         return **this;

      return static_cast<T>(std::forward<U>(v));
   }

   template <typename U> T value_or(U&& v) &&
   {
      if (init_)
         return std::move(**this);

      return static_cast<T>(std::forward<U>(v));
   }

private:
   typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type data_;
   bool init_;
};

template <typename T> bool operator==(const optional<T>& x, const optional<T>& y)
{
   if (bool(x) != bool(y))
      return false;

   if (bool(x) == false)
      return true;

   return *x == *y;
}

template <typename T> bool operator<(const optional<T>& x, const optional<T>& y)
{
   if (!y)
      return false;

   if (!x)
      return true;

   return std::less<T>{}(*x, *y);
}

template <typename T> optional<typename std::decay<T>::type> make_optional(T&& v)
{
   return optional<typename std::decay<T>::type>(std::forward<T>(v));
}

} // namespace cl

namespace std
{

template <typename T> void swap(core::optional<T>& x, core::optional<T>& y)
{
   x.swap(y);
}

} // namespace std

