module Spfg.Types exposing (..)


type DpVal
    = DpValInt Int
    | DpValBool Bool
    | DpValReal Float


type alias DpInfo =
    { id : Int
    , type_ : Int
    , name : String
    , value : DpVal
    , emitted : Bool
    }


type alias FnInfo =
    { id : Int
    , type_ : Int
    , name : String
    , phase : Int
    , inDpIds : List Int
    , outDpIds : List Int
    }


type alias GrInfo =
    { id : Int
    , name : String
    , dps : List DpInfo
    , fns : List FnInfo
    }
