SPFG Python Bindings
====================

A thin wrapper around the core spfg public ABI, implemented using ctypes.

Running the tests
-----------------

To run the tests for the core spfg library using the bindings provided by this python package, make sure you have the shared library binary available, then run the following command:

::

    LD_PRELOAD_PATH=path/to/shared/lib/dir python3 setup.py test -a "-xs"


Getting Started
---------------

The following code snippet is roughly equivalent to the C snippet in the core library's `README <../../README.md#getting-started>`__

.. code:: python

    import spfg

    # Prepares a runtime instance.
    runtime = spfg.SPFGRuntime('rt0')

    dp1 = runtime.create_dp(spfg.SPFG_DP_BOOL, 'dp1')
    dp2 = runtime.create_dp(spfg.SPFG_DP_BOOL, 'dp2')
    dp3 = runtime.create_dp(spfg.SPFG_DP_BOOL, 'dp3')
    dp4 = runtime.create_dp(spfg.SPFG_DP_BOOL, 'dp4')
    dp5 = runtime.create_dp(spfg.SPFG_DP_BOOL, 'dp5')
    fn1 = runtime.create_fn('and(bool,bool)->bool', 0, [dp1, dp2], [dp4], 'fn1')
    fn2 = runtime.create_fn('and(bool,bool)->bool', 0, [dp4, dp3], [dp5], 'fn2')

    # Runs an evaluation.
    runtime.set_bool(dp1, True);
    runtime.set_bool(dp2, True);
    runtime.set_bool(dp3, True);
    runtime.reset();
    runtime.run(0);
    value, emitted = runtime.get(dp3)
    assert(value)
    assert(emitted)

    # Cleanup allocated memory.
    runtime.finish()


Roadmap
-------

- [ ] Complete API (some methods are still missing).
- [ ] Better adaptable input buffer sizes (through core runtime inspect functions)
