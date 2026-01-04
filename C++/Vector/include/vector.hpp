#include <cstddef>
#include <memory>
#include <stdexcept>

namespace ds {
template <typename T> class vector {
public:
  vector() : data_(std::allocator<T>{}.allocate(1)), capacity_(1), size_(0) {}
  ~vector() {
    std::allocator<T> alloc;
    for (std::size_t i = 0; i < size_; i++) {
      std::allocator_traits<std::allocator<T>>::destroy(alloc, data_ + i);
    }
    alloc.deallocate(data_, capacity_);
  }
  void push_back(T element) {
    if (size_ + 1 == capacity_) {
      reserve(get_new_capacity());
    }
    std::allocator<T> alloc;
    std::allocator_traits<std::allocator<T>>::construct(alloc, data_ + size_,
                                                        element);
    size_++;
  }
  const T &at(std::size_t index) const {
    if (index >= size_) {
      throw std::out_of_range("Index out of bounds");
    }
    return data_[index];
  }
  std::size_t get_size() const { return size_; }
  std::size_t get_capacity() const { return capacity_; }
  void shrink_to_fit() {
    if (size_ == capacity_)
      return;

    std::allocator<T> alloc;

    if (size_ == 0) {
      alloc.deallocate(data_, capacity_);
      capacity_ = 1;
      data_ = alloc.allocate(capacity_);
      return;
    }

    T *new_data = alloc.allocate(size_);
    for (std::size_t i = 0; i < size_; i++) {
      std::allocator_traits<std::allocator<T>>::construct(alloc, new_data + i,
                                                          std::move(data_[i]));
      std::allocator_traits<std::allocator<T>>::destroy(alloc, data_ + i);
    }
    alloc.deallocate(data_, capacity_);
    data_ = new_data;
    capacity_ = size_;
  }
  void pop_back() {
    if (size_ == 0)
      return;
    size_--;
    std::allocator<T> alloc;
    std::allocator_traits<std::allocator<T>>::destroy(alloc, data_ + size_);
  }

private:
  void reserve(std::size_t new_capacity) {
    if (capacity_ >= new_capacity)
      return;
    std::allocator<T> alloc;
    T *new_data = alloc.allocate(new_capacity);
    for (std::size_t i = 0; i < size_; i++) {
      std::allocator_traits<std::allocator<T>>::construct(alloc, new_data + i,
                                                          data_[i]);
      std::allocator_traits<std::allocator<T>>::destroy(alloc, data_ + i);
    }
    alloc.deallocate(data_, capacity_);
    data_ = new_data;
    capacity_ = new_capacity;
  }
  std::size_t get_new_capacity() const {
    if (capacity_ == 0) {
      return 1;
    }
    return capacity_ * 3;
  }
  T *data_{};
  std::size_t capacity_{};
  std::size_t size_{};
};
} // namespace ds
