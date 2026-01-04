#include <stdexcept>
#include <utility>
namespace ds
{
    template <typename T>
    struct custom_deleter
    {
        void operator()(T* pointer) const
        {
            delete pointer;
        }
    };

    template <typename T, typename custom_deleter = custom_deleter<T>>
    class unique_ptr
    {
    public:
        unique_ptr() : ptr(nullptr) {}
        unique_ptr(T* pointer) : ptr(pointer) {}

        unique_ptr(const unique_ptr&) {
            throw std::invalid_argument("Copy constructor not available for unique ptr");
        };
        unique_ptr& operator=(const unique_ptr&) {
            throw std::invalid_argument("Copy assignment operator not available for unique ptr");
        };

        unique_ptr(unique_ptr&& other) noexcept : ptr(other.ptr) {
            other.ptr = nullptr;
        }

        unique_ptr& operator=(unique_ptr&& other) noexcept {
            if (this == &other) return *this;
            ds::custom_deleter<T>()(ptr);
            ptr = other.ptr;
            other.ptr = nullptr;
            return *this;
        }

        ~unique_ptr(){
            ds::custom_deleter<T>()(ptr);
        }

        T* release() {
            return std::exchange(ptr, nullptr);
        }

        void reset(T* pointer = nullptr) {
            ds::custom_deleter<T>()(ptr);
            ptr = pointer;
        }

        bool is_owning() const {
            return ptr != nullptr;
        }


        T& operator*() const {
            return *ptr;
        }
        T* operator->() const {
            return ptr;
        }
        operator bool() const {
            return ptr != nullptr;
        }

    private:
        T* ptr {};
    };
}
