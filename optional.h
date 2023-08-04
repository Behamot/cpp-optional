#include <stdexcept>
#include <utility>

// Исключение этого типа должно генерироватся при обращении к пустому optional
class BadOptionalAccess : public std::exception {
public:
    using exception::exception;

    virtual const char* what() const noexcept override {
        return "Bad optional access";
    }
};

template <typename T>
class Optional {
public:
    Optional() = default;

    Optional(const T& value) : value_(new (&data_[0]) T(value)) {
        is_initialized_ = true;
    }
    Optional(T&& value) : value_(new (&data_[0]) T(std::forward<T>(value))) {
        is_initialized_ = true;
    }
    Optional(const Optional& other) {
        if (other.HasValue()) {
            value_ = new (&data_[0]) T(other.Value());
            is_initialized_ = true;
        }
    }
    Optional(Optional&& other) {
        if (other.HasValue()) {
            value_ = new (&data_[0]) T(std::move(other.Value()));
            is_initialized_ = true;
        }
    }

    Optional& operator=(const T& value) {
        if (HasValue()) {
            *value_ = (value);
        }
        else if (!HasValue()) {
            value_ = new (&data_[0]) T(value);
            is_initialized_ = true;
        }
        return *this;
    }
    Optional& operator=(T&& rhs) {
        if (HasValue()) {
            *value_ = std::move(rhs);
        }
        else if (!HasValue()) {
            value_ = new (&data_[0]) T(std::move(rhs));
            is_initialized_ = true;
        }
        return *this;
    }
    Optional& operator=(const Optional& rhs) {
        if (HasValue() && rhs.HasValue()) {
            *value_ = rhs.Value();
        }
        else if (HasValue() && !(rhs.HasValue())) {
            Reset();
        }
        else if (!HasValue() && rhs.HasValue()) {
            value_ = new (&data_[0]) T(rhs.Value());
            is_initialized_ = true;
        }
        return *this;
    }
    Optional& operator=(Optional&& rhs) {
        if (HasValue() && rhs.HasValue()) {
            *value_ = std::move(rhs.Value());
        }
        else if (HasValue() && !(rhs.HasValue())) {
            Reset();
        }
        else if (!HasValue() && rhs.HasValue()) {
            value_ = new (&data_[0]) T(std::move(rhs.Value()));
            is_initialized_ = true;
        }
        return *this;
    }

    ~Optional() {
        Reset();
    }

    //Метод Emplace должен принимать свои параметры,
    //используя forwarding-ссылки, и передавать их конструктору типа T.
    template <typename ... Args>
    void Emplace(Args&& ...args)
    {
        if (is_initialized_) {
            value_->~T();
        }
        value_ = new(&data_[0]) T(std::forward<Args>(args)...);
        is_initialized_ = true;
    }

    bool HasValue() const {
        return is_initialized_;
    }

    // Операторы * и -> не должны делать никаких проверок на пустоту Optional.
    // Эти проверки остаются на совести программиста
    T& operator*() &{
        return *value_;
    }

    const T& operator*() const &{
        return *value_;
    }

    T&& operator*() &&{
        return std::move(*value_);
    }

    T* operator->() {
        return value_;
    }

    const T* operator->() const {
        return value_;
    }


    // Метод Value() генерирует исключение BadOptionalAccess, если Optional пуст
    T& Value()& {
        if (!HasValue()) {
            throw BadOptionalAccess();
        }
        return *value_;
    }
    const T& Value() const& {
        if (!HasValue()) {
            throw BadOptionalAccess();
        }
        return *value_;
    }

    T&& Value() &&{
        if (!HasValue()) {
            throw BadOptionalAccess();
        }
        return std::move(*value_);
    }

    void Reset() {
        if (HasValue()) {
            value_->~T();
            is_initialized_ = false;
        }
    }

private:
    // alignas нужен для правильного выравнивания блока памяти
    alignas(T) char data_[sizeof(T)];
    T* value_;
    bool is_initialized_ = false;
};