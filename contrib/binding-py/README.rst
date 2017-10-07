SPFG Python Bindings
====================

A thin wrapper around core spfg public ABI, implemented using ctypes.

Running the tests
-----------------

To run tests for the core spfg library using the bindings provided by this python package, make sure you have the shared library binary available, then run the following command:

::

    LD_PRELOAD_PATH=path/to/shared/lib/dir python setup.py
