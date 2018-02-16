-module(erl_sqlite_driver).

-export([
    start/0,
    start/1,
    stop/0,
    init/1
]).
-export([foo/1, bar/1]).
-export([
    query/2
]).

start() ->
    start("erl_sqlite").

start(SharedLib) ->
    case erl_ddll:load_driver(code:priv_dir(erl_sqlite), SharedLib) of
        ok -> 
            ok;
        {error, already_loaded} -> 
            ok;
        E -> 
            io:format("~p~n", [E]),
            exit({error, could_not_load_driver})
    end,
    spawn(?MODULE, init, [SharedLib]).

init(SharedLib) ->
    process_flag(trap_exit, true),
    register(?MODULE, self()),
    Port = open_port({spawn, SharedLib}, []),
    loop(Port).

stop() ->
    ?MODULE ! stop.

foo(X) ->
    call_port({foo, X}).
bar(Y) ->
    call_port({bar, Y}).
query(Db, Query) ->
    call_port({query, Db, Query}).

call_port(Msg) ->
    ?MODULE ! {call, self(), Msg},
    receive
        {?MODULE, Result} ->
            Result
    end.

loop(Port) ->
    receive
        {call, Caller, Msg} ->
            Port ! {self(), {command, encode(Msg)}},
            % Port ! {self(), {command, Msg}},
            receive
                {Port, {data, Data}} ->
                    Caller ! {?MODULE, decode(Data)}
                    % Caller ! {?MODULE, Data}
            end,
            loop(Port);
        stop ->
            Port ! {self(), close},
            receive
                {Port, closed} ->
                    exit(normal)
            end;
        {'EXIT', Port, Reason} ->
            io:format("~p ~n", [Reason]),
            exit(port_terminated)
    end.

encode({foo, X}) -> [1, X];
encode({bar, Y}) -> [2, Y];
encode({query, Db, Query}) -> [3, Db, Query].

decode([Int]) -> Int.