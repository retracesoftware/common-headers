#pragma once
#include <Python.h>

namespace retracesoftware {

    static inline vectorcallfunc extract_vectorcall(PyObject *callable)
    {
        PyTypeObject *tp = Py_TYPE(callable);
        if (!PyType_HasFeature(tp, Py_TPFLAGS_HAVE_VECTORCALL)) {
            return PyObject_Vectorcall;
        }
        Py_ssize_t offset = tp->tp_vectorcall_offset;

        vectorcallfunc ptr;
        memcpy(&ptr, (char *) callable + offset, sizeof(ptr));
        return ptr;
    }

    struct FastCall {
        vectorcallfunc vectorcall;
        PyObject * callable;
    
        FastCall(PyObject * callable) : vectorcall(extract_vectorcall), callable(callable) {
            assert(PyCallable_Check(callable));
        }

        FastCall() : vectorcall(nullptr), callable(nullptr) {}
        // ~FastCall() { Py_DECREF(callable); }

        inline PyObject * throw_on_nullptr(PyObject * result) {
            if (!result) {
                assert(PyErr_Occurred());   
                throw nullptr;
            }
            return result;
        }

        inline PyObject * operator()() {
            return throw_on_nullptr(vectorcall(callable, nullptr, 0, nullptr));
        }

        inline PyObject * operator()(PyObject * arg) {
            return throw_on_nullptr(vectorcall(callable, &arg, 1, nullptr));
        }

        inline PyObject * operator()(PyObject * arg1, PyObject * arg2) {
            PyObject * args[] = {nullptr, arg1, arg2};

            return throw_on_nullptr(vectorcall(callable, args + 1, 2 | PY_VECTORCALL_ARGUMENTS_OFFSET, nullptr));
        }

        inline PyObject * operator()(PyObject * arg1, PyObject * arg2, PyObject * arg3) {
            PyObject * args[] = {nullptr, arg1, arg2, arg3};

            return throw_on_nullptr(vectorcall(callable, args + 1, 3 | PY_VECTORCALL_ARGUMENTS_OFFSET, nullptr));
        }

        inline PyObject * operator()(PyObject *const *args, size_t nargsf, PyObject *kwnames) {
            return throw_on_nullptr(vectorcall(callable, args, nargsf, kwnames));
        }

        // inline nb::object operator()(nb::args args, nb::kwargs kwargs) {
        //     PyObject * result = cached_vectorcall(callable, args.data(), args.size(), kwargs.names().ptr());
        //     if (!result) on_error();
        //     return nb::steal(result);
        // }
    
        // inline nb::object operator()(nb::handle prepend, nb::args args, nb::kwargs kwargs) {
        //     ...
        // }
    };
}