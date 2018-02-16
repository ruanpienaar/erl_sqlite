-module(erl_sqlite_app).

-behaviour(application).

%% Application callbacks
-export([start/2, stop/1]).

-include("erl_sqlite.hrl").

%% ===================================================================
%% Application callbacks
%% ===================================================================

start(_StartType, _StartArgs) ->
    erl_sqlite_sup:start_link().

stop(_State) ->
    ok.
