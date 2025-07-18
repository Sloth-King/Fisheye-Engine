#pragma once

#include <memory>

// using shared_ptr allows us to split resource ownership. 
// As long as one thing is using the ressource, it stays in-memory.
// The memory overhead is really small and the perf overhead shouldn't be a problem.
// this is a small handle wrapper
template<typename T>
class Handle{

    std::shared_ptr<T> _ptr;
public:
    Handle()
        : _ptr(std::make_shared<T>())
        {}

    template<class ...Args>
    Handle(Args &&...args)
        :_ptr(new T(std::forward<Args>(args)...))
    {}

    inline T* operator -> () const{
        return _ptr.get();
    }
};