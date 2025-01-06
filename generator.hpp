#include <coroutine>
#include <exception>
#include <iostream>
#include <memory>
#include <vector>

template <typename T> struct Generator
{
    struct promise_type
    {
        T current_value;
        std::exception_ptr exception;

        auto get_return_object()
        {
            return Generator{
                std::coroutine_handle<promise_type>::from_promise(
                    *this)};
        }

        auto initial_suspend() { return std::suspend_always{}; }
        auto final_suspend() noexcept
        {
            return std::suspend_always{};
        }

        void unhandled_exception()
        {
            exception = std::current_exception();
        }

        auto yield_value(T value)
        {
            current_value = value;
            return std::suspend_always{};
        }

        void return_void() {}
    };

    using Handle = std::coroutine_handle<promise_type>;
    Handle coroutine;

    Generator(Handle h) : coroutine(h) {}
    ~Generator()
    {
        if (coroutine)
            coroutine.destroy();
    }

    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    Generator(Generator&& other) noexcept : coroutine(other.coroutine)
    {
        other.coroutine = nullptr;
    }

    T operator()()
    {
        coroutine.resume();
        if (coroutine.done())
            throw std::runtime_error("Generator exhausted");
        return coroutine.promise().current_value;
    }

    bool next()
    {
        coroutine.resume();
        return !coroutine.done();
    }

    T value() const { return coroutine.promise().current_value; }
};
