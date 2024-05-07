#!/bin/bash

SERVIDOR="./bin/orchestrator"
CLIENTE="./bin/client"

enviar_pedido()
{
    local args=()
    for arg in "$@"; do
        args+=("$arg")
    done
    "$CLIENTE" "${args[@]}"
}

for ((i=1; i<=200; i++)); do
    if [ $((i % 50)) -eq 0 ]; then
        enviar_pedido "execute" "10000" "-u" "sleep 10"
    elif [ $((i % 4)) -eq 0 ]; then
        enviar_pedido "execute" "10" "-p" "cat example/example.txt | grep ola"
    elif [ $((i % 3)) -eq 0 ]; then
        enviar_pedido "execute" "1" "-u" "ls"
    else
        enviar_pedido "execute" "1" "-u" "ls"
    fi
done

enviar_pedido "status"
