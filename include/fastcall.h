#pragma once
#include <Python.h>

namespace retracesoftware {

    static inline PyObject * fallback(PyObject *callable, PyObject *const *args, size_t nargsf, PyObject *kwnames) {
        Py_ssize_t nargs = PyVectorcall_NARGS(nargsf);
        return _PyObject_MakeTpCall(PyThreadState_Get(), callable, args, nargs, kwnames);
    }

    static inline vectorcallfunc extract_vectorcall(PyObject *callable)
    {
        PyTypeObject *tp = Py_TYPE(callable);
        if (!PyType_HasFeature(tp, Py_TPFLAGS_HAVE_VECTORCALL)) {
            return fallback;
        }
        Py_ssize_t offset = tp->tp_vectorcall_offset;

        vectorcallfunc ptr;
        memcpy(&ptr, (char *) callable + offset, sizeof(ptr));
        return ptr ? ptr : fallback;
    }

    struct FastCall {
        vectorcallfunc vectorcall;
        PyObject * callable;
    
        FastCall(PyObject * callable) : vectorcall(extract_vectorcall(callable)), callable(callable) {
            assert(PyCallable_Check(callable));
        }

        FastCall() : vectorcall(nullptr), callable(nullptr) {}
        // ~FastCall() { Py_DECREF(callable); }

        inline PyObject * handle_result(PyObject * result) {
            // if (!result) {
            //     assert(PyErr_Occurred());   
            //     throw nullptr;
            // }
            return result;
        }

        inline PyObject * operator()() {
            return handle_result(vectorcall(callable, nullptr, 0, nullptr));
        }

        inline PyObject * operator()(PyObject * arg) {
            return handle_result(vectorcall(callable, &arg, 1, nullptr));
        }

        inline PyObject * operator()(PyObject * arg1, PyObject * arg2) {
            PyObject * args[] = {nullptr, arg1, arg2};

            return handle_result(vectorcall(callable, args + 1, 2 | PY_VECTORCALL_ARGUMENTS_OFFSET, nullptr));
        }

        inline PyObject * operator()(PyObject * arg1, PyObject * arg2, PyObject * arg3) {
            PyObject * args[] = {nullptr, arg1, arg2, arg3};

            return handle_result(vectorcall(callable, args + 1, 3 | PY_VECTORCALL_ARGUMENTS_OFFSET, nullptr));
        }

        inline PyObject * operator()(PyObject *const *args, size_t nargsf, PyObject *kwnames) {
            return handle_result(vectorcall(callable, args, nargsf, kwnames));
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