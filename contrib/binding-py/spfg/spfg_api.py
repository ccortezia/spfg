import json
import ctypes
from ctypes import byref
from . import spfg_types
from .exceptions import SPFGError, SPFGInvalidType


LIB_NAME = 'libspfg.so'

LIB_OBJ = None


def lib_obj():
    """
    """
    global LIB_OBJ
    if not LIB_OBJ:
        LIB_OBJ = ctypes.CDLL(LIB_NAME)
    return LIB_OBJ


def get_runtime_byte_size():
    """
    """
    out_size = ctypes.c_uint32(0)
    lib_obj().spfg_runtime_size(byref(out_size))
    return out_size.value


def raise_for_spfg_err(err):
    """
    """
    if err == 0:
        return
    raise SPFGError(err)


class Runtime(object):
    """
    """

    def __init__(self, name):
        """
        """
        runtime_size = get_runtime_byte_size()
        runtime_bin = ctypes.create_string_buffer(runtime_size)
        err = lib_obj().spfg_rt_init(byref(runtime_bin), name)
        raise_for_spfg_err(err)
        self.runtime_bin = runtime_bin
        self.runtime_ref = byref(runtime_bin)

    def __del__(self):
        err = lib_obj().spfg_rt_finish(self.runtime_ref)
        raise_for_spfg_err(err)

    def create_dp(self, dp_type, name):
        """
        """
        out_dp_id = ctypes.c_uint32(0)
        err = lib_obj().spfg_dp_create(self.runtime_ref, dp_type, name, byref(out_dp_id))
        raise_for_spfg_err(err)
        return out_dp_id.value

    def remove_dp(self, dp_id):
        """
        """
        err = lib_obj().spfg_dp_create(self.runtime_ref, dp_id)
        raise_for_spfg_err(err)

    def create_fn(self, fn_type, phase, in_dp_ids, out_dp_ids, name):
        """
        """
        out_fn_id = ctypes.c_uint32(0)
        in_dp_ids = (ctypes.c_uint32 * len(in_dp_ids))(*in_dp_ids)
        out_dp_ids = (ctypes.c_uint32 * len(out_dp_ids))(*out_dp_ids)
        fn_type = _normalize_fn_type(fn_type)
        err = lib_obj().spfg_fn_create(self.runtime_ref,
                                       fn_type, phase,
                                       byref(in_dp_ids), len(in_dp_ids),
                                       byref(out_dp_ids), len(out_dp_ids),
                                       name,
                                       byref(out_fn_id))
        raise_for_spfg_err(err)
        return out_fn_id.value

    def remove_fn(self, fn_id):
        """
        """
        err = lib_obj().spfg_fn_remove(self.runtime_ref, fn_id)
        raise_for_spfg_err(err)

    def set_dp_bool(self, dp_id, value):
        """
        """
        err = lib_obj().spfg_dp_set_bool(self.runtime_ref, dp_id, value)
        raise_for_spfg_err(err)

    def get_dp_bool(self, dp_id):
        """
        """
        out_value = ctypes.c_uint32(0)
        out_emitted = ctypes.c_uint32(0)
        err = lib_obj().spfg_dp_get_bool(self.runtime_ref, dp_id,
                                         byref(out_value),
                                         byref(out_emitted))
        raise_for_spfg_err(err)
        return out_value.value, out_emitted.value

    def reset(self):
        """
        """
        err = lib_obj().spfg_rt_reset_cycle(self.runtime_ref)
        raise_for_spfg_err(err)

    def run(self, timestamp, callback=None, udata=None):
        """
        """
        def _callback(ptr, fn_id, phase, _):
            return callback(self, fn_id, phase, udata)

        spfg_run_cb_t = ctypes.CFUNCTYPE(ctypes.c_void_p,
                                         ctypes.c_uint32,
                                         ctypes.c_uint32,
                                         ctypes.c_void_p,
                                         ctypes.c_byte)

        c_callback = callback and spfg_run_cb_t(_callback)
        err = lib_obj().spfg_rt_run_cycle(self.runtime_ref, timestamp, c_callback, 0)
        raise_for_spfg_err(err)

    def export_json(self):
        """
        """
        maxlen = 1024 * 500
        out_json = ctypes.create_string_buffer(maxlen)
        out_len = ctypes.c_uint32(0)
        err = lib_obj().spfg_rt_export_json(self.runtime_ref, byref(out_json), maxlen, byref(out_len))
        raise_for_spfg_err(err)
        return json.loads(out_json.value.decode('utf8'))

    def import_json(self, snapshot):
        """
        """
        serialized = json.dumps(snapshot)
        serialized = bytes(serialized.encode('utf8'))
        in_json = ctypes.create_string_buffer(serialized)
        err = lib_obj().spfg_rt_import_json(self.runtime_ref, byref(in_json), len(serialized))
        raise_for_spfg_err(err)


def _normalize_fn_type(fn_type):
    """
    """

    mapping = {
        'inv(bool)->bool': spfg_types.SPFG_FN_INVERT_BOOL_RET_BOOL,
        'and(bool,bool)->bool': spfg_types.SPFG_FN_AND_BOOL_BOOL_RET_BOOL,
    }

    if isinstance(fn_type, str):
        try:
            return mapping[fn_type]
        except KeyError:
            raise SPFGInvalidType('invalid fn type {}.'.format(fn_type))

    return fn_type
