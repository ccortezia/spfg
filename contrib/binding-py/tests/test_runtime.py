import spfg


def test_get_runtime_byte_size_should_not_raise():
    bytesize = spfg.get_runtime_byte_size()
    assert(bytesize) != 0


def test_runtime_ctor_should_not_raise():
    spfg.Runtime('beta')


def test_runtime_dtor_should_not_raise():
    runtime = spfg.Runtime('beta')
    del runtime


def test_create_remove_dp_should_not_raise():
    runtime = spfg.Runtime('beta')
    dp = runtime.create_dp(spfg.SPFG_DP_INT, 'dp0')
    runtime.remove_dp(dp)


def test_create_remove_fn_should_not_raise():
    runtime = spfg.Runtime('beta')
    dp0 = runtime.create_dp(spfg.SPFG_DP_BOOL, 'dp0')
    dp1 = runtime.create_dp(spfg.SPFG_DP_BOOL, 'dp1')
    dp2 = runtime.create_dp(spfg.SPFG_DP_BOOL, 'dp2')
    fn0 = runtime.create_fn(spfg.SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, [dp0, dp1], [dp2], 'fn0')
    runtime.remove_dp(dp0)
    runtime.remove_dp(dp1)
    runtime.remove_dp(dp2)
    runtime.remove_fn(fn0)


def test_get_set_dp_bool_should_not_raise():
    runtime = spfg.Runtime('beta')
    dp0 = runtime.create_dp(spfg.SPFG_DP_BOOL, 'dp0')
    value, emitted = runtime.get_dp_bool(dp0)
    assert(not value)
    assert(not emitted)
    runtime.set_dp_bool(dp0, True)
    value, emitted = runtime.get_dp_bool(dp0)
    assert(value)
    assert(emitted)


def test_reset_run_should_not_raise():
    runtime = spfg.Runtime('beta')
    dp0 = runtime.create_dp(spfg.SPFG_DP_BOOL, 'dp0')
    dp1 = runtime.create_dp(spfg.SPFG_DP_BOOL, 'dp1')
    dp2 = runtime.create_dp(spfg.SPFG_DP_BOOL, 'dp2')
    fn0 = runtime.create_fn(spfg.SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, [dp0, dp1], [dp2], 'fn0')
    runtime.set_dp_bool(dp0, True)
    runtime.set_dp_bool(dp1, True)
    runtime.set_dp_bool(dp2, False)
    runtime.reset()
    runtime.run(0)
    value, emitted = runtime.get_dp_bool(dp2)
    assert(value)
    assert(emitted)


def test_reset_run_with_callback_should_not_raise():
    local = {'called': False}
    def callback(fn_id, phase, udata):
        local['called'] = True
        return 0

    runtime = spfg.Runtime('beta')
    dp0 = runtime.create_dp(spfg.SPFG_DP_BOOL, 'dp0')
    dp1 = runtime.create_dp(spfg.SPFG_DP_BOOL, 'dp1')
    dp2 = runtime.create_dp(spfg.SPFG_DP_BOOL, 'dp2')
    fn0 = runtime.create_fn(spfg.SPFG_FN_AND_BOOL_BOOL_RET_BOOL, 0, [dp0, dp1], [dp2], 'fn0')
    runtime.set_dp_bool(dp0, True)
    runtime.set_dp_bool(dp1, True)
    runtime.set_dp_bool(dp2, False)

    local['called'] = False
    runtime.reset()
    runtime.run(0, callback)
    value, emitted = runtime.get_dp_bool(dp2)
    assert(value)
    assert(emitted)
    assert(local['called'])
