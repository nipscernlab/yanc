// A tiny standard-library subset written in the CPPComp C++ subset itself.
// Templates are type-erased to one machine word on this target, so the
// containers hold any 1-word element (int / pointer) and are used WITHOUT
// explicit <T> at the use site (e.g. `Vector v;`).
#pragma once

// ----- Vector<T>: a growable dynamic array over the 4K heap, RAII-managed -----
template<class T>
class Vector {
    T*  data;
    int sz;
    int cap;
public:
    Vector()  { sz = 0; cap = 4; data = new T[4]; }
    ~Vector() { delete[] data; }                 // RAII: freed at scope exit
    int size() { return sz; }
    void push(T x) {
        if (sz == cap) {                         // grow: double the capacity
            cap = cap * 2;
            T* nd = new T[cap];
            for (int i = 0; i < sz; i = i + 1) nd[i] = data[i];
            delete[] data;
            data = nd;
        }
        data[sz] = x;
        sz = sz + 1;
    }
    T get(int i)        { return data[i]; }
    T operator[](int i) { return data[i]; }
    void set(int i, T x) { data[i] = x; }
};

// ----- unique_ptr<T>: an owning pointer that frees its target on destruction --
template<class T>
class unique_ptr {
    T* ptr;
public:
    unique_ptr(T* p) { ptr = p; }
    ~unique_ptr()    { delete ptr; }     // RAII: the owned object is freed
    T*  get()        { return ptr; }
};
