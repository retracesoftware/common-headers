#pragma once
#include <Python.h>
#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace retracesoftware {
    struct FastCall {
        PyObject * callable;
        vectorcallfunc cached_vectorcall;
    
        FastCall(nb::handle callable_in) {
            // Eagerly check if the object is callable and raise an exception if not.
            if (!PyCallable_Check(callable_in.ptr())) {
                throw nb::type_error("Expected a callable object.");
            }

            // Increment the ref count to own the object's lifetime.
            this->callable = callable_in.inc_ref().ptr();

            // Perform the vectorcall lookup and set the fallback if needed.
            cached_vectorcall = _PyVectorcall_FunctionInline(this->callable);
            if (!cached_vectorcall) {
                cached_vectorcall = PyObject_Vectorcall;
            }
        }

        ~FastCall() { Py_DECREF(callable); }

        void on_error() {
            throw nb::python_error();
        }

        inline nb::object operator()() {
            PyObject * result = cached_vectorcall(callable, nullptr, 0, nullptr);

            if (!result) on_error();
            return nb::steal(result);
        }

        inline nb::object operator()(nb::handle arg) {
            PyObject * args[] = {nullptr, arg.ptr()};

            PyObject * result = cached_vectorcall(callable, args + 1, 1 | PY_VECTORCALL_ARGUMENTS_OFFSET, nullptr);

            if (!result) on_error();
            return nb::steal(result);
        }

        inline nb::object operator()(nb::handle arg1, nb::handle arg2) {
            PyObject * args[] = {nullptr, arg1.ptr(), arg2.ptr()};

            PyObject * result = cached_vectorcall(callable, args + 1, 2 | PY_VECTORCALL_ARGUMENTS_OFFSET, nullptr);

            if (!result) on_error();
            return nb::steal(result);
        }

        inline nb::object operator()(PyObject *const *args, size_t nargsf, PyObject *kwnames) {
            PyObject * result = cached_vectorcall(callable, args, nargsf, kwnames);
            if (!result) on_error();
            return nb::steal(result);
        }

        inline nb::object operator()(nb::args args, nb::kwargs kwargs) {
            PyObject * result = cached_vectorcall(callable, args.data(), args.size(), kwargs.names().ptr());
            if (!result) on_error();
            return nb::steal(result);
        }
    
        inline nb::object operator()(nb::handle prepend, nb::args args, nb::kwargs kwargs) {
            ...
        }
    };
}