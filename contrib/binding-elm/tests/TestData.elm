module TestData exposing (..)

import Spfg.Types as Spfg


minimalGridJson : String
minimalGridJson =
    """
    {
        "id": 0, "name": "gr0", "fns": [], "dps": []
    }
    """


minimalGridNative =
    { id = 0, name = "gr0", dps = [], fns = [] }


partialGridJson : String
partialGridJson =
    """
    {
        "id": 0,
        "name": "gr0",
        "fns": [],
        "dps": [{"id": 1, "type": 1, "name": "dp0", "value": true, "emitted": false}]
    }
    """


partialGridNative =
    { id = 0
    , name = "gr0"
    , dps =
        [ { id = 1, type_ = 1, name = "dp0", value = Spfg.DpValBool True, emitted = False }
        ]
    , fns = []
    }


completeGridJson : String
completeGridJson =
    """
    {
        "id": 0, "name": "gr0",
        "fns": [
            {"id": 1, "type": 1, "name": "fn0", "phase": 1, "in_dp_ids": [1], "out_dp_ids": [2]}
        ],
        "dps": [
            {"id": 1, "type": 1, "name": "dp0", "value": true, "emitted": false},
            {"id": 2, "type": 1, "name": "dp1", "value": true, "emitted": false}
        ]
    }
    """


completeGridNative =
    { id = 0
    , name = "gr0"
    , dps =
        [ { id = 1, type_ = 1, name = "dp0", value = Spfg.DpValBool True, emitted = False }
        , { id = 2, type_ = 1, name = "dp1", value = Spfg.DpValBool True, emitted = False }
        ]
    , fns =
        [ { id = 1, type_ = 1, name = "fn0", phase = 1, inDpIds = [ 1 ], outDpIds = [ 2 ] }
        ]
    }
