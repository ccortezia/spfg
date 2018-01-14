module Spfg.Encode exposing (..)

import Json.Encode
import Spfg.Types exposing (..)


dpEncoder : DpInfo -> Json.Encode.Value
dpEncoder dpInfo =
    Json.Encode.object
        [ ( "id", Json.Encode.int dpInfo.id )
        , ( "type", Json.Encode.int dpInfo.type_ )
        , ( "name", Json.Encode.string dpInfo.name )
        , ( "value", dpValEncoder dpInfo.value )
        , ( "emitted", Json.Encode.bool dpInfo.emitted )
        ]


dpValEncoder : DpVal -> Json.Encode.Value
dpValEncoder value =
    case value of
        DpValInt value ->
            Json.Encode.int value

        DpValBool value ->
            Json.Encode.bool value

        DpValReal value ->
            Json.Encode.float value


fnEncoder : FnInfo -> Json.Encode.Value
fnEncoder fnInfo =
    Json.Encode.object
        [ ( "id", Json.Encode.int fnInfo.id )
        , ( "type", Json.Encode.int fnInfo.type_ )
        , ( "name", Json.Encode.string fnInfo.name )
        , ( "phase", Json.Encode.int fnInfo.phase )
        , ( "in_dp_ids", Json.Encode.list <| List.map Json.Encode.int fnInfo.inDpIds )
        , ( "out_dp_ids", Json.Encode.list <| List.map Json.Encode.int fnInfo.outDpIds )
        ]


grEncoder : GrInfo -> Json.Encode.Value
grEncoder grInfo =
    Json.Encode.object
        [ ( "id", Json.Encode.int grInfo.id )
        , ( "name", Json.Encode.string grInfo.name )
        , ( "fns", Json.Encode.list <| List.map fnEncoder grInfo.fns )
        , ( "dps", Json.Encode.list <| List.map dpEncoder grInfo.dps )
        ]
