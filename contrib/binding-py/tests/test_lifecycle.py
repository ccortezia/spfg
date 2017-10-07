import spfg


def test_initialization():
    assert spfg.init() == 0
    assert spfg.init() != 0
    assert spfg.finish() == 0
    assert spfg.finish() != 0
