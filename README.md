# Exemplo de threads persistentes em c++

Este exemplo apresenta duas abordagens para implementar threads persistentes em c++.

## Por que persistente?

Threads tem um custo alto para serem criadas, somente colocar um processo recorrente em uma thread para execução irá diminuir a performance, por causa do overhead de criação. Com os exemplos as threads se mantém vivas e somente executam o processo quando necessário.

## Clonar e configurar o repositório

```{bash}
git clone https://github.com/henrique-jung-isi/thread-testing
cd thread-testing
mkdir build
cd build
cmake .. . -G Ninja 
# ou para release:
# cmake .. . -G Ninja -DCMAKE_BUILD_TYPE:STRING=Release
cmake --build .
sudo cmake --install .
```

# Desinstalação

Utilize o target `uninstall`. Na pasta principal do projeto utilize o comando:

```{bash}
sudo cmake --build build --target uninstall
```
