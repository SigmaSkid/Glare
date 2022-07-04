if [[ ! -e release ]]; then
    mkdir release
fi

g++ main.cpp utils/utils.cpp -lX11 -lXtst -o release/glare
