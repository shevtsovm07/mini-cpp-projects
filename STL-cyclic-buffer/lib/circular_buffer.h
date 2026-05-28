#include <memory>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <initializer_list>
#include <concepts>
#include <algorithm>

template<typename T, bool Extendable = false, typename Allocator = std::allocator<T>>
class circular_buffer {
public:
    template <bool IsConst, bool IsReverse>
    class base_iterator {
    public:
      using value_type = T;
      using difference_type = std::ptrdiff_t;
      using pointer = typename std::conditional<IsConst, const T*, T*>::type;
      using reference = typename std::conditional<IsConst, const T&, T&>::type;
      using size_type = std::size_t;

      base_iterator() : data_ptr_(nullptr), cap_(0), head_(0), pos_(0) {};
      base_iterator(T* data, size_type capacity, size_type head, difference_type pos) : data_ptr_(data), cap_(capacity), head_(head), pos_(pos) {}

    template <bool OtherIsConst, bool OtherIsReverse>
    base_iterator(const base_iterator<OtherIsConst, OtherIsReverse>& other) : data_ptr_(other.data_ptr_), cap_(other.cap_), head_(other.head_), pos_(other.pos_) {
        static_assert(IsConst >= OtherIsConst, "Cannot cast const_iterator to iterator");
        static_assert(IsReverse == OtherIsReverse, "Cannot change direction");
    }


      reference operator*() const {
        return data_ptr_[(head_ + pos_) % cap_];
      }

      base_iterator& operator++() {
        IsReverse ? --pos_ : ++pos_;
        return *this;
      }

      base_iterator operator++(int) {
        base_iterator tmp = *this;
        ++(*this);
        return tmp;
      }

      base_iterator& operator--() {
        IsReverse ? ++pos_ : --pos_;
        return *this;
      }

      base_iterator operator--(int) {
        base_iterator tmp = *this;
        --(*this);
        return tmp;
      }

      base_iterator& operator+=(difference_type n) {
        IsReverse ? pos_ -= n : pos_ += n;
        return *this;
      }

      base_iterator operator+(difference_type n) const {
        return base_iterator(data_ptr_, cap_, head_, IsReverse ? pos_ - n : pos_ + n);
      }

      base_iterator& operator-=(difference_type n) {
        IsReverse ? pos_ += n : pos_ -= n;
        return *this;
      }

      base_iterator operator-(difference_type n) const {
        return base_iterator(data_ptr_, cap_, head_, IsReverse ? pos_ + n : pos_ - n);
      }

      difference_type operator-(const base_iterator& other) const {
        return IsReverse ? (other.pos_ - pos_) : (pos_ - other.pos_);
      }

      reference operator[](difference_type n) const {
        return *(*this + n);
      }

      pointer operator->() const {
        return &(operator*());
      }

      bool operator==(const base_iterator& other) const { return pos_ == other.pos_; }
      bool operator!=(const base_iterator& other) const { return pos_ != other.pos_; }
      bool operator<(const base_iterator& other) const {
        return IsReverse ? (pos_ > other.pos_) : (pos_ < other.pos_);
      }
      bool operator>(const base_iterator& other) const { return other < *this; }
      bool operator<=(const base_iterator& other) const { return !(*this > other); }
      bool operator>=(const base_iterator& other) const { return !(*this < other); }

      friend base_iterator operator+(difference_type n, const base_iterator& it) {
        return it + n;
      }
      template <bool, bool> friend class base_iterator;

    private:
      T* data_ptr_;
      size_type cap_;
      size_type head_;
      difference_type pos_;
    };

    using iterator = base_iterator<false, false>;
    using const_iterator = base_iterator<true, false>;
    using reverse_iterator = base_iterator<false, true>;
    using const_reverse_iterator = base_iterator<true, true>;

    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;

private:
    pointer data_;
    size_type head_;
    size_type size_;
    size_type capacity_;
    Allocator alloc_;

public:
    circular_buffer() : data_(nullptr), head_(0), size_(0), capacity_(0), alloc_(Allocator()) {};

    explicit circular_buffer(size_type capacity, const Allocator& alloc = Allocator()) : alloc_(alloc), capacity_(capacity), size_(0), head_(0) {
      data_ = (capacity_ > 0) ? std::allocator_traits<Allocator>::allocate(alloc_, capacity_) : nullptr;
    }

    explicit circular_buffer(size_type sz, T value, const Allocator& alloc = Allocator()) : capacity_(sz), size_(sz), data_(nullptr), head_(0), alloc_(alloc) {
        data_ = (capacity_ > 0) ? std::allocator_traits<Allocator>::allocate(alloc_, capacity_) : nullptr;
        for (size_type index = 0; index < size_; index++) {
            std::allocator_traits<Allocator>::construct(alloc_, data_ + index, value);
        }
    }

    explicit circular_buffer(const Allocator& alloc) : data_(nullptr), head_(0), size_(0), capacity_(0), alloc_(alloc) {};

    circular_buffer(circular_buffer&& other) noexcept : data_(other.data_), head_(other.head_), size_(other.size_), capacity_(other.capacity_), alloc_(std::move(other.alloc_)) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        other.head_ = 0;
    }

    circular_buffer(circular_buffer&& other, const Allocator& alloc) : alloc_(alloc), head_(other.head_), size_(other.size_), capacity_(other.capacity_) {
        if (alloc_ == other.alloc_) {
            data_ = other.data_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
            other.head_ = 0;
        }
        else {
            data_ = std::allocator_traits<Allocator>::allocate(alloc_, capacity_);
            for (size_type index = 0; index < size_; index++) {
                std::allocator_traits<Allocator>::construct(alloc_, data_ + index, std::move(other[index]));
            }
        }
    }

    circular_buffer& operator=(circular_buffer&& other) noexcept {
        if (this != &other) {
            clear();
            if (data_) {
                std::allocator_traits<Allocator>::deallocate(alloc_, data_, capacity_);
            }
            data_ = other.data_;
            head_ = other.head_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            alloc_ = std::move(other.alloc_);
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
            other.head_ = 0;
        }
        return *this;
    }



    template <typename Iterator>
    explicit circular_buffer(Iterator first, Iterator last, const Allocator* alloc = Allocator()) : alloc_(alloc), head_(0) {
        size_type dist = 0;
        for (Iterator it = first; it != last; it++) {
            dist++;
        }

        capacity_ = dist;
        size_ = dist;

        data_ = (capacity_ > 0) ? std::allocator_traits<Allocator>::allocate(alloc_, capacity_) : nullptr;

        size_type index = 0;
        for (Iterator it = first; it != last; it++, index++) {
            std::allocator_traits<Allocator>::construct(alloc_, data_ + index, *it);
        }
    }

    circular_buffer(std::initializer_list<T> il, const Allocator& alloc = Allocator()) : alloc_(alloc), size_(il.size()), capacity_(il.size()), head_(0) {
        data_ = (capacity_ > 0) ? std::allocator_traits<Allocator>::allocate(alloc_, capacity_) : nullptr;
        size_type index = 0;
        for (const T& value : il) {
            std::allocator_traits<Allocator>::construct(alloc_, data_ + index, value);
            index++;
        }
    }

    circular_buffer& operator=(std::initializer_list<T> il) {
        clear();
        size_type count = (il.size() > capacity_) ? capacity_ : il.size();
        auto it = il.begin();
        for (size_type index = 0; index < count; index++, it++) {
            std::allocator_traits<Allocator>::construct(alloc_, data_ + index, *it);
        }
        size_ = count;
        head_ = 0;
        return *this;
    }

    circular_buffer(const circular_buffer& other) : alloc_(other.alloc_), capacity_(other.capacity_), size_(other.size_), head_(0) {
      data_ = (capacity_ > 0) ? std::allocator_traits<Allocator>::allocate(alloc_, capacity_) : nullptr;
      for (size_type index = 0; index < size_; index++) {
        std::allocator_traits<Allocator>::construct(alloc_, data_ + index, other[index]);
      }
    }

    ~circular_buffer() {
      clear();
      if (data_) {
        std::allocator_traits<Allocator>::deallocate(alloc_, data_, capacity_);
      }
    }

    circular_buffer& operator=(const circular_buffer& other) {
      if (this != &other) {
        circular_buffer temp(other);
        this->swap(temp);
      }
      return *this;
    }

    reference operator[](size_type index) {
        return data_[(head_ + index) % capacity_];
    }

    const_reference operator[](size_type index) const {
        return data_[(head_ + index) % capacity_];
    }

    size_type size() const noexcept { return size_; }
    size_type max_size() const noexcept { return std::allocator_traits<Allocator>::max_size(alloc_); }
    bool empty() const noexcept { return size_ == 0; }

    void swap(circular_buffer& rhs) noexcept {
      std::swap(data_, rhs.data_);
      std::swap(size_, rhs.size_);
      std::swap(capacity_, rhs.capacity_);
      std::swap(head_, rhs.head_);
      std::swap(alloc_, rhs.alloc_);
    }

    void clear() {
      for (size_type index = 0; index < size_; index++) {
        std::allocator_traits<Allocator>::destroy(alloc_, data_ + (head_ + index) % capacity_);
      }
      size_ = 0;
      head_ = 0;
    }

    bool operator==(const circular_buffer& other) const {
      if (size_ != other.size_) return false;
      for (size_type index = 0; index < size_; index++) {
        if ((*this)[index] != other[index]) return false;
      }
      return true;
    }

    bool operator!=(const circular_buffer& other) const { return !(*this == other); }

    bool operator<(const circular_buffer& other) const {
        return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
    }

    bool operator>(const circular_buffer& other) const {
        return other < *this;
    }

    bool operator<=(const circular_buffer& other) const {
        return !(*this > other);
    }

    bool operator>=(const circular_buffer& other) const {
        return !(*this < other);
    }

    iterator begin() noexcept { return iterator(data_, capacity_, head_, 0); }
    iterator end() noexcept { return iterator(data_, capacity_, head_, size_); }
    const_iterator begin() const noexcept { return const_iterator(data_, capacity_, head_, 0); }
    const_iterator end() const noexcept { return const_iterator(data_, capacity_, head_, size_); }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }

    reverse_iterator rbegin() noexcept { return reverse_iterator(data_, capacity_, head_, size_ - 1); }
    reverse_iterator rend() noexcept { return reverse_iterator(data_, capacity_, head_, -1); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(data_, capacity_, head_, size_ - 1);}
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(data_, capacity_, head_, - 1);}
    const_reverse_iterator crbegin() const noexcept { return rbegin();}
    const_reverse_iterator crend() const noexcept { return rend();}


    iterator insert(const_iterator pos, const T& value) {
        size_type idx = pos - cbegin();
        if (size_ < capacity_) {
            if (size_ == 0) {
                push_back(value);
                return begin();
            }
            size_type new_tail = (head_ + size_) % capacity_;
            std::allocator_traits<Allocator>::construct(alloc_, data_ + new_tail, (*this)[size_-1]);
            for (size_type index = size_ - 1; index > idx; index--) {
                (*this)[index] = (*this)[index-1];
            }
            (*this)[idx] = value;
            size_++;
            return begin() + idx;
        }
        throw std::length_error("The container is full");
    }

    iterator insert(const_iterator pos, size_type amount, const T& value) {
        if (amount == 0) return begin() + (pos - cbegin());
        if (size_ + amount > capacity_) {
            throw std::length_error("You are trying to insert too many elements");
        }
        size_type idx_begin = pos - cbegin();

        for (size_type index = size_ + amount - 1; index >= idx_begin + amount; index--) {
            size_type target = (head_ + index) % capacity_;
            size_type source = (head_ + index -amount) % capacity_;
            if (index >= size_) {
                std::allocator_traits<Allocator>::construct(alloc_, data_ + target, data_[source]);
            }
            else {
                data_[target] = data_[source];
            }
        }
        for (size_type index = 0; index < amount; index++) {
            size_type target = (head_ + idx_begin + index) % capacity_;
            if (idx_begin + index >= size_) {
                std::allocator_traits<Allocator>::construct(alloc_, data_ + target, value);
            }
            else {
                data_[target] = value;
            }
        }
        size_ += amount;
        return begin() + idx_begin;
    }

    template <std::input_iterator InputIterator>
    iterator insert(const_iterator pos, InputIterator first, InputIterator last) {
        size_type amount = std::distance(first, last);

        if (amount == 0) return begin() + (pos - cbegin());
        if (size_ + amount > capacity_) {
            throw std::length_error("Insert range is more than capacity");
        }

        size_type idx_begin = pos - cbegin();

        for (size_type index = size_ + amount - 1; index >= idx_begin + amount; index--) {
            size_type target = (head_ + index) % capacity_;
            size_type source = (head_ + index - amount) % capacity_;
            if (index >= size_) {
                std::allocator_traits<Allocator>::construct(alloc_, data_ + target, data_[source]);
            } 
            else {
                data_[target] = data_[source];
            }
        }

        for (size_type idx = 0; idx < amount; idx++, first++) {
            size_type target = (head_ + idx_begin + idx) % capacity_;
            if (idx_begin + idx >= size_) {
                std::allocator_traits<Allocator>::construct(alloc_, data_ + target, *first);
            } 
            else {
                data_[target] = *first;
            }
        }

        size_ += amount;
        return begin() + idx_begin;
    }

    iterator insert(const_iterator pos, std::initializer_list<T> il) {
        return insert(pos, il.begin(), il.end());
    }

    iterator erase(const_iterator pos) {
        if (size_ == 0) {
            throw std::length_error("The container is empty");
        }
        size_type idx = pos - cbegin();
        for (size_type index = idx; index < size_ - 1; index++) {
            (*this)[index] = (*this)[index + 1];
        }
        size_type last_idx = (head_ + size_ - 1) % capacity_;
        std::allocator_traits<Allocator>::destroy(alloc_, data_ + last_idx);
        size_--;
        return begin() + idx;
    }

    iterator erase(const_iterator pos1, const_iterator pos2) {
        if (size_ == 0) {
            throw std::length_error("The container is empty");
        }
        size_type idx_begin = pos1 - cbegin();
        size_type idx_end = pos2 - cbegin();
        size_type amount = idx_end - idx_begin;
        for (size_type index = idx_begin; index + amount < size_; index++) {
            (*this)[index] = (*this)[index + amount];
        }
        for (size_type count = 0; count < amount; count++) {
            size_type tail = (head_ + size_ - 1 - count) % capacity_;
            std::allocator_traits<Allocator>::destroy(alloc_, data_ + tail);
        }
        size_ -= amount;
        return begin() + idx_begin;
    }

    void push_back(const T& value) {
        if (capacity_ == 0) throw std::length_error("The capacity equals 0");
        if (size_ < capacity_) {
            size_type tail_idx = (head_ + size_) % capacity_;
            std::allocator_traits<Allocator>::construct(alloc_, data_ + tail_idx, value);
            size_++;
        }
        else {
            std::allocator_traits<Allocator>::destroy(alloc_, data_ + head_);
            std::allocator_traits<Allocator>::construct(alloc_, data_ + head_, value);
            head_ = (head_ + 1) % capacity_;
        }
    }

    void push_front(const T& value) {
        if (capacity_ == 0) throw std::length_error("The capacity equals 0");
        head_ = (head_ + capacity_ - 1) % capacity_;
        if (size_ < capacity_) {
            std::allocator_traits<Allocator>::construct(alloc_, data_ + head_, value);
            size_++;
        }
        else {
            std::allocator_traits<Allocator>::destroy(alloc_, data_ + head_);
            std::allocator_traits<Allocator>::construct(alloc_, data_ + head_, value);
        }
    }

    void pop_front() {
        if (empty()) throw std::out_of_range("buffer is empty");
        std::allocator_traits<Allocator>::destroy(alloc_, data_ + head_);
        head_ = (head_ + 1) % capacity_;
        size_--;
    }

    void pop_back() {
        if (empty()) throw std::out_of_range("buffer is empty");
        size_type tail_idx = (head_ + size_ - 1) % capacity_;
        std::allocator_traits<Allocator>::destroy(alloc_, data_ + tail_idx);
        size_--;
    }

    allocator_type get_allocator() const noexcept {
        return alloc_;
    }

    reference front() {
        return (*this)[0];
    }

    const_reference front() const {
        return (*this)[0];
    }

    reference back() {
        return (*this)[size_-1];
    }

    const_reference back() const {
        return (*this)[size_-1];
    }

    reference at(size_type index) {
        if (index >= size_) {
            throw std::out_of_range("index is out of range");
        }
        return (*this)[index];
    }

    const_reference at(size_type index) const {
        if (index >= size_) {
            throw std::out_of_range("index is out of range");
        }
        return (*this)[index];
    }

    void resize(size_type n, const T& value = T()) {
        if (n > capacity_) {
            throw std::length_error("You are trying to resize more values than a capacity");
        }
        
        while (size_ > n) {
            pop_back();
        }
        
        while (size_ < n) {
            push_back(value);
        }
    }

    void assign(size_type amount, const T& value) {
        if (amount > capacity_) {
            throw std::length_error("Assign requires more place then a container have");
        }
        clear();
        for (size_type index = 0; index < amount; index++) {
            push_back(value);
        }
    }

    template <std::input_iterator InputIterator>
    void assign(InputIterator it1, InputIterator it2) {
        clear();
        while (it1 != it2) {
            push_back(*it1);
            it1++;
        }
    }

    void assign(std::initializer_list<T> il) {
        assign(il.begin(), il.end());
    }
};

template<typename T, bool E, typename A>
void swap(circular_buffer<T, E, A>& a, circular_buffer<T, E, A>& b) noexcept {
    a.swap(b);

}