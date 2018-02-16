#!/bin/sh

# -mnesia dir "'"$PWD"/Mnesia'"

cd `dirname $0`
exec erl -sname erl_sqlite -config $PWD/sys.config -pa $PWD/_build/default/lib/*/ebin -boot start_sasl -setcookie erl_sqlite