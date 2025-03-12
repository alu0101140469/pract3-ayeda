#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "BigNumber.hpp"

// Daniel Palenzuela Álvarez alu0101140469

// Se utiliza para almacenar los operandos (etiqueta, puntero a BigNumber)
// y mantener el orden de inserción (como se leeron del fichero)
template <unsigned char Base>
class Board {
public:
    // Vector de pares, la primera parte es la etiqueta y la segunda es el puntero al objeto
    std::vector<std::pair<std::string, BigNumber<Base>*>> entries;

    // Busca un objeto en el board por su etiqueta y devuelve nullptr si no lo encuentra
    BigNumber<Base>* lookup(const std::string& label) const {
        for(const auto& p : entries)
            if(p.first == label)
                return p.second;
        return nullptr;
    }

    // Inserta o actualiza una entrada en el board
    void insert(const std::string& label, BigNumber<Base>* num) {
        for(auto& p : entries) {
            if(p.first == label) {
                p.second = num;
                return;
            }
        }
        entries.push_back({label, num});
    }
};

template <unsigned char Base>
void processFile(const std::string& inputFilename, const std::string& outputFilename) {
    // Abrir ficheros de entrada y salida
    std::ifstream infile(inputFilename);
    std::ofstream outfile(outputFilename);
    if(!infile.is_open()){
        std::cerr << "No se pudo abrir el fichero de entrada.\n";
        return;
    }
    std::string line;
    // Leer la primera línea, que contiene la base, por ejemplo "Base = 16"
    std::getline(infile, line);
    outfile << line << "\n";  // Escribir la base en la salida

    // Crear el board para almacenar los operandos
    Board<Base> board;

    // Procesar cada línea restante del fichero
    while(std::getline(infile, line)) {
        if(line.empty()) continue;  // Saltar líneas vacías
        std::istringstream iss(line);
        std::string label;
        iss >> label;  // Leer la etiqueta, por ejemplo N1
        char op;
        iss >> op;     // Leer el operador "=" para asignación o "?" para expresión
        if(op == '=') {
            // Línea de asignación, por ejemplo "N1 = 236i"
            std::string value;
            iss >> value;
            try {
                // Crear el objeto usando el método de fábrica de BigNumber
                BigNumber<Base>* bn = BigNumber<Base>::create(value.c_str());
                board.insert(label, bn);
            } catch(const BigNumberException& e) {
                std::cerr << "Error en la línea: " << line << "\n" << e.what() << "\n";
                board.insert(label, BigNumber<Base>::create("0u"));
            }
        } else if(op == '?') {
            // Línea de expresión en notación polaca inversa (RPN)
            std::vector<std::string> tokens;
            std::string token;
            // Leer todos los tokens restantes de la línea
            while(iss >> token)
                tokens.push_back(token);
            // Evaluar la expresión usando una pila
            std::vector<BigNumber<Base>*> stack;
            try {
                // Procesar cada token
                for(const auto& t : tokens) {
                    // Si el token es un operador aritmético
                    if(t == "+" || t == "-" || t == "*" || t == "/") {
                        if(stack.size() < 2)
                            throw BigNumberException();
                        // Se extraen los dos operandos
                        BigNumber<Base>* b = stack.back(); stack.pop_back();
                        BigNumber<Base>* a = stack.back(); stack.pop_back();
                        BigNumber<Base>* res = nullptr;
                        // Se llama al método virtual adecuado
                        if(t == "+")
                            res = &a->add(*b);
                        else if(t == "-")
                            res = &a->subtract(*b);
                        else if(t == "*")
                            res = &a->multiply(*b);
                        else if(t == "/")
                            res = &a->divide(*b);
                        // Se empuja el resultado en la pila
                        stack.push_back(res);
                    } else {
                        // Si el token es una etiqueta, se busca en el board
                        BigNumber<Base>* p = board.lookup(t);
                        if(p == nullptr)
                            throw BigNumberException();
                        stack.push_back(p);
                    }
                }
                if(stack.size() != 1)
                    throw BigNumberException();
                // El resultado final se asocia a la etiqueta de la línea
                board.insert(label, stack.back());
            } catch(const BigNumberException& e) {
                std::cerr << "Error evaluando la expresión en la línea: " << line 
                          << "\n" << e.what() << "\n";
                board.insert(label, BigNumber<Base>::create("0u"));
            }
        }
    }
    // Escribir el contenido del board en el fichero de salida en el orden en que se insertaron
    for(const auto& p : board.entries) {
        outfile << p.first << " = " << *p.second << "\n";
    }
}

int main(int argc, char* argv[]) {
    // Verificar que se pasaron los parámetros de entrada y salida
    if(argc < 3) {
        std::cerr << "Uso: calculator <fichero_entrada> <fichero_salida>\n";
        return 1;
    }
    std::string inputFilename = argv[1];
    std::string outputFilename = argv[2];

    // Abrir el fichero de entrada para leer la primera línea (la base)
    std::ifstream infile(inputFilename);
    if(!infile.is_open()){
        std::cerr << "No se pudo abrir el fichero de entrada.\n";
        return 1;
    }
    std::string baseLine;
    std::getline(infile, baseLine);
    infile.close();

    // Extraer la base numérica de la primera línea
    unsigned int baseValue = 10;
    size_t pos = baseLine.find("=");
    if(pos != std::string::npos) {
        std::istringstream iss(baseLine.substr(pos+1));
        iss >> baseValue;
    }
    // Instanciar la función plantilla processFile según la base leída
    switch(baseValue) {
        case 8:
            processFile<8>(inputFilename, outputFilename);
            break;
        case 10:
            processFile<10>(inputFilename, outputFilename);
            break;
        case 16:
            processFile<16>(inputFilename, outputFilename);
            break;
        default:
            std::cerr << "Base no soportada: " << baseValue << "\n";
            return 1;
    }
    return 0;
}

// Función auxiliar que compara dos BigNumber y muestra en consola:
// - El número menor
// - El número mayor
// - La resta (mayor − menor)
// Se asume que ambos números son del mismo tipo (misma base) y se convierten a BigInteger
// para realizar comparaciones y operaciones aritméticas con signo.
template <unsigned char Base>
void compareAndSubtract(const BigNumber<Base>& num1, const BigNumber<Base>& num2) {
    // Convertir ambos números a BigInteger usando el operador de conversión definido.
    // Esto permite trabajar con los valores con signo.
    BigInteger<Base> a = num1;
    BigInteger<Base> b = num2;

    // Obtener el valor decimal de cada número mediante el método to_decimal().
    // Este valor se utiliza para realizar la comparación.
    unsigned long long decA = a.to_decimal();
    unsigned long long decB = b.to_decimal();

    // Imprimir ambos números
    std::cout << "Número 1: " << a << std::endl;
    std::cout << "Número 2: " << b << std::endl;

    // Comparar los valores decimales para determinar cuál es menor y cuál mayor
    if (decA < decB) {
        // Si a es menor que b
        std::cout << "El menor es: " << a << std::endl;
        std::cout << "El mayor es: " << b << std::endl;
        // Calcular la diferencia: mayor - menor
        BigInteger<Base> diff = b - a;
        std::cout << "Resta (mayor - menor): " << diff << std::endl;
    } else if (decB < decA) {
        // Si b es menor que a
        std::cout << "El menor es: " << b << std::endl;
        std::cout << "El mayor es: " << a << std::endl;
        // Calcular la diferencia: mayor - menor
        BigInteger<Base> diff = a - b;
        std::cout << "Resta (mayor - menor): " << diff << std::endl;
    } else {
        // Si ambos números son iguales
        std::cout << "Ambos números son iguales: " << a << std::endl;
        std::cout << "Resta: 0" << std::endl;
    }
}
