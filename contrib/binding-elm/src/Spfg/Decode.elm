module Spfg.Decode exposing (..)

import Json.Decode exposing (..)
import Spfg.Types exposing (..)


dpValFormat : DpVal -> String
dpValFormat value =
    case value of
        DpValBool True ->
            "true"

        DpValBool False ->
            "false"

        DpValInt value ->
            toString value

        DpValReal value ->
            toString value


dpDecoder : Decoder DpInfo
dpDecoder =
    map5 DpInfo
        (field "id" Json.Decode.int)
        (field "type" Json.Decode.int)
        (field "name" Json.Decode.string)
        (oneOf
            [ map DpValBool (field "value" Json.Decode.bool)
            , map DpValInt (field "value" Json.Decode.int)
            , map DpValReal (field "value" Json.Decode.float)
            ]
        )
        (field "emitted" Json.Decode.bool)


fnDecoder : Decoder FnInfo
fnDecoder =
    map6 FnInfo
        (field "id" Json.Decode.int)
        (field "type" Json.Decode.int)
        (field "name" Json.Decode.string)
        (field "phase" Json.Decode.int)
        (field "in_dp_ids" (Json.Decode.list Json.Decode.int))
        (field "out_dp_ids" (Json.Decode.list Json.Decode.int))


grDecoder : Decoder GrInfo
grDecoder =
    map4 GrInfo
        (field "id" Json.Decode.int)
        (field "name" Json.Decode.string)
        (field "dps" (Json.Decode.list dpDecoder))
        (field "fns" (Json.Decode.list fnDecoder))
