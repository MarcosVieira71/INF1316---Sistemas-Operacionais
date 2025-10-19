set -e
echo "Criando diretório de build..."
mkdir -p build
cd build

echo "Gerando arquivos do CMake..."
cmake ..

echo "Compilando projeto..."
cmake --build .

echo "Executando kernel..."
./kernel