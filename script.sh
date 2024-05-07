#!/bin/bash

# Definir o caminho para o executável do servidor e do cliente
SERVIDOR="./bin/orchestrator"
CLIENTE="./bin/client"

# Função para enviar pedidos ao servidor
enviar_pedido() {
    # Use "$@" to represent all arguments passed to the function
    # Loop through each argument and enclose it in double quotes to preserve spaces
    local args=()
    for arg in "$@"; do
        args+=("$arg")
    done
    # Call the client with all arguments preserved
    "$CLIENTE" "${args[@]}"
}

# Teste: Enviar pedido "execute" várias vezes
#echo "Testando comando 'execute'"
for ((i=1; i<=100; i++)); do
    if [ $((i % 10)) -eq 0 ]; then
        enviar_pedido "execute" "100" "-u" "sleep 10"
    elif [ $((i % 4)) -eq 0 ]; then
        enviar_pedido "execute" "100" "-u" "cat ola/results.txt | grep ola"
    elif [ $((i % 3)) -eq 0 ]; then
        enviar_pedido "execute" "100" "-u" "ls"
    else
        enviar_pedido "execute" "100" "-u" "ls"
    fi
done


# Teste: Enviar pedido "status" várias vezes
#echo "Testando comando 'status'"
for ((i=1; i<=1; i++)); do
    enviar_pedido "status"
done

# Adicione mais testes conforme necessário

#echo "Testes concluídos."
