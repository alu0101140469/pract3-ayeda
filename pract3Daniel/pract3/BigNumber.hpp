#ifndef BIGNUMBER_HPP
#define BIGNUMBER_HPP

#include <iostream>
#include <exception>
#include <string>

// Daniel Palenzuela Álvarez alu0101140469

// Declaraciones anticipadas para evitar dependencias circulares entre clases
template <unsigned char Base>
class BigUnsigned;
template <unsigned char Base>
class BigInteger;
template <unsigned char Base>
class BigRational;

// Clase abstracta base para representar un número grande
// Esta clase define la interfaz común que deben implementar las clases derivadas
template <unsigned char Base>
class BigNumber {
public:
    // Destructor virtual para permitir eliminación polimórfica
    virtual ~BigNumber() {}

    // Métodos aritméticos virtuales puros
    // Cada clase derivada debe implementar estos métodos para sumar, restar, multiplicar y dividir
    virtual BigNumber<Base>& add(const BigNumber<Base>&) const = 0;
    virtual BigNumber<Base>& subtract(const BigNumber<Base>&) const = 0;
    virtual BigNumber<Base>& multiply(const BigNumber<Base>&) const = 0;
    virtual BigNumber<Base>& divide(const BigNumber<Base>&) const = 0;

    // Operadores de conversión virtuales puros
    // Permiten convertir el objeto a alguno de los tipos concretos (BigUnsigned, BigInteger o BigRational)
    virtual operator BigUnsigned<Base>() const = 0;
    virtual operator BigInteger<Base>() const = 0;
    virtual operator BigRational<Base>() const = 0;

    // Métodos virtuales para escribir y leer el objeto desde un flujo (E/S)
    virtual std::ostream& write(std::ostream&) const = 0;
    virtual std::istream& read(std::istream&) = 0;

    // Sobrecarga de operadores de flujo para facilitar la impresión y lectura
    friend std::ostream& operator<<(std::ostream& out, const BigNumber<Base>& num) {
        return num.write(out);
    }
    friend std::istream& operator>>(std::istream& in, BigNumber<Base>& num) {
        return num.read(in);
    }

    // Crea dinámicamente el objeto concreto a partir de una cadena
    // La cadena debe terminar con un sufijo (u para BigUnsigned, i para BigInteger, r para BigRational)
    static BigNumber<Base>* create(const char* str);
};

// Definición de excepciones
// Excepción base para errores relacionados con BigNumber
class BigNumberException : public std::exception {
public:
    virtual const char* what() const noexcept { return "BigNumber exception"; }
};

// Excepción para dígitos no válidos para la base
class BigNumberBadDigit : public BigNumberException {
    std::string msg;
public:
    BigNumberBadDigit(char digit, unsigned char base) {
         msg = "Bad digit: ";
         msg.push_back(digit);
         msg += " for base " + std::to_string(base);
    }
    virtual const char* what() const noexcept { return msg.c_str(); }
};

// Excepción para división por cero
class BigNumberDivisionByZero : public BigNumberException {
public:
    virtual const char* what() const noexcept { return "Division by zero"; }
};

// Se incluyen los headers de las clases derivadas
#include "BigUnsigned.hpp"
#include "BigInteger.hpp"
#include "BigRational.hpp"

// Implementación del método de fábrica create
// Analiza la cadena de entrada, extrae el sufijo, y crea el objeto concreto
template <unsigned char Base>
BigNumber<Base>* BigNumber<Base>::create(const char* str) {
    std::string s(str);
    if(s.empty())
        return nullptr;
    // El último carácter determina el tipo de número
    char type = s.back();
    s.pop_back(); // Se elimina el sufijo de la cadena
    if(type == 'u') {
        return new BigUnsigned<Base>(s.c_str());
    } else if(type == 'i') {
        return new BigInteger<Base>(s.c_str());
    } else if(type == 'r') {
        // Para BigRational se espera el formato "numerador/denominador"
        size_t pos = s.find('/');
        if(pos == std::string::npos)
            throw BigNumberException();
        std::string num = s.substr(0, pos);
        std::string den = s.substr(pos+1);
        BigInteger<Base> n(num.c_str());
        BigUnsigned<Base> d(den.c_str());
        return new BigRational<Base>(n, d);
    } else {
        throw BigNumberException();
    }
}

#endif
