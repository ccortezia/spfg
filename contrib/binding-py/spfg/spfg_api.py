import ctypes

libspfg = ctypes.CDLL('libspfg.so')


def init():
    return libspfg.spfg_init()


def finish():
    return libspfg.spfg_finish()
