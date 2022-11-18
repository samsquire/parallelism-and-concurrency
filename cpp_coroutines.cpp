#include <future>
#include <iostream>
#include <coroutine>
#include <type_traits>

using namespace std;


template <typename... Args>
    struct coroutine_traits<int, Args...>
    {
        struct promise_type
        {
            int holder;

            int get_return_object() const noexcept
            {
                return holder;
            }

            void return_value(int const& v) const
            {
                holder = v;
            }

            void unhandled_exception() const noexcept
            {
                this.set_exception(std::current_exception());
            }

            suspend_never initial_suspend() const noexcept
            {
                return{};
            }

            suspend_never final_suspend() const noexcept
            {
                return{};
            }
        };
    };

template <>
struct std::coroutine_traits<std::future<int>> {
  struct promise_type : std::promise<int> {
    future<int> get_return_object() { return this->get_future(); }
    std::suspend_never initial_suspend() noexcept { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }
    void return_value(int value) { this->set_value(value); }
    void unhandled_exception() {
      this->set_exception(std::current_exception());
    }
  };
};

template <>
struct std::coroutine_traits<std::future<int>, int> {
  struct promise_type : std::promise<int> {
    future<int> get_return_object() { return this->get_future(); }
    std::suspend_never initial_suspend() noexcept { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }
    void return_value(int value) { this->set_value(value); }
    void unhandled_exception() {
      this->set_exception(std::current_exception());
    }
  };
};

auto operator co_await(std::future<int> future) {
  struct awaiter : std::future<int> {
  
    bool await_ready() { return false; } // suspend always
    void await_suspend(std::coroutine_handle<> handle) {
      std::thread([this, handle]() {
        this->wait();
        handle.resume();
      }).detach();
    }
    int await_resume() { return this->get(); }
  };
  return awaiter{std::move(future)};
}

future<int> async_add(int a, int b)
{
    auto fut = std::async([=]() {
        int c = a + b;
        return c;
    });

    return fut;
}

future<int> async_fib(int n)
{
    if (n <= 2)
        co_return 1;

    int a = 1;
    int b = 1;

    // iterate computing fib(n)
    for (int i = 0; i < n - 2; ++i)
    {
        int c = co_await async_add(a, b);
        a = b;
        b = c;
    }

    co_return b;
}

future<int> test_async_fib()
{
    for (int i = 1; i < 10; ++i)
    {
        int ret = co_await async_fib(i);
        cout << "async_fib(" << i << ") returns " << ret << endl;
    }
}

int main()
{
    auto fut = test_async_fib();
    fut.wait();

    return 0;
}
