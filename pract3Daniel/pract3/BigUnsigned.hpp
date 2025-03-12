#ifndef BIGUNSIGNED_HPP
#define BIGUNSIGNED_HPP

#include "BigNumber.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>

// Daniel Palenzuela Álvarez alu0101140469

// Clase para números grandes sin signo
template <unsigned char Base>
class BigUnsigned : public BigNumber<Base> {
private:
    // Vector que almacena los dígitos en orden inverso
    // El dígito menos significativo está en el índice 0
    std::vector<unsigned char> digits;

    // Función auxiliar que convierte un carácter en un dígito (verificando la validez para la base)
    unsigned char charToDigit(char c) const {
        if(std::isdigit(c)) {
            int d = c - '0';
            if(d >= Base) throw BigNumberBadDigit(c, Base);
            return d;
        } else if(std::isalpha(c)) {
            int d = std::toupper(c) - 'A' + 10;
            if(d >= Base) throw BigNumberBadDigit(c, Base);
            return d;
        } else {
            throw BigNumberBadDigit(c, Base);
        }
    }

    // Función auxiliar que convierte un dígito en su carácter correspondiente
    char digitToChar(unsigned char d) const {
        return (d < 10) ? ('0' + d) : ('A' + (d - 10));
    }

public:
    // Constructor a partir de una cadena (sin sufijo)
    BigUnsigned(const char* str) {
        std::string s(str);
        // Recorremos la cadena desde el final hasta el principio
        for(auto it = s.rbegin(); it != s.rend(); ++it)
            digits.push_back(charToDigit(*it));
    }

    // Constructor por defecto, inicializa el número en 0
    BigUnsigned() { digits.push_back(0); }

    // Operador de asignación.
    BigUnsigned& operator=(const BigUnsigned& other) {
        if(this != &other)
            digits = other.digits;
        return *this;
    }

    // Operador suma
    BigUnsigned operator+(const BigUnsigned& other) const {
        BigUnsigned result;
        result.digits.clear(); // Se limpia el vector de dígitos del resultado
        unsigned char carry = 0;
        // Se itera hasta el tamaño máximo entre ambos números
        size_t n = std::max(digits.size(), other.digits.size());
        for(size_t i = 0; i < n; i++){
            unsigned char d1 = (i < digits.size()) ? digits[i] : 0;
            unsigned char d2 = (i < other.digits.size()) ? other.digits[i] : 0;
            unsigned char sum = d1 + d2 + carry;
            carry = sum / Base;
            result.digits.push_back(sum % Base);
        }
        if(carry)
            result.digits.push_back(carry);
        return result;
    }

    // Operador resta (se asume que *this es mayor o igual que other)
    BigUnsigned operator-(const BigUnsigned& other) const {
        BigUnsigned result;
        result.digits.clear();
        unsigned char borrow = 0;
        for(size_t i = 0; i < digits.size(); i++){
            int d1 = digits[i];
            int d2 = (i < other.digits.size()) ? other.digits[i] : 0;
            int diff = d1 - d2 - borrow;
            if(diff < 0) { diff += Base; borrow = 1; }
            else borrow = 0;
            result.digits.push_back(diff);
        }
        // Se eliminan los ceros a la izquierda
        while(result.digits.size() > 1 && result.digits.back() == 0)
            result.digits.pop_back();
        return result;
    }

    // Operador multiplicación
    BigUnsigned operator*(const BigUnsigned& other) const {
        BigUnsigned result;
        // Se asigna un vector de tamaño adecuado, inicializado a 0
        result.digits = std::vector<unsigned char>(digits.size() + other.digits.size(), 0);
        for(size_t i = 0; i < digits.size(); i++){
            unsigned char carry = 0;
            for(size_t j = 0; j < other.digits.size() || carry; j++){
                unsigned int current = result.digits[i+j] +
                    digits[i] * (j < other.digits.size() ? other.digits[j] : 0) + carry;
                result.digits[i+j] = current % Base;
                carry = current / Base;
            }
        }
        while(result.digits.size() > 1 && result.digits.back() == 0)
            result.digits.pop_back();
        return result;
    }

    // Operador división (implementación del algoritmo de división larga)
    BigUnsigned operator/(const BigUnsigned& other) const {
        if(other == BigUnsigned(0))
            throw BigNumberDivisionByZero();
        BigUnsigned dividend(*this), divisor(other), quotient, current;
        quotient.digits.clear();
        current.digits.clear();
        for(int i = dividend.digits.size()-1; i >= 0; i--){
            // Se inserta el siguiente dígito en current
            current.digits.insert(current.digits.begin(), dividend.digits[i]);
            unsigned char count = 0;
            // Se resta divisor de current hasta que current < divisor
            while(!(current < divisor)){
                current = current - divisor;
                count++;
            }
            quotient.digits.insert(quotient.digits.begin(), count);
        }
        while(quotient.digits.size() > 1 && quotient.digits.back() == 0)
            quotient.digits.pop_back();
        return quotient;
    }

    // Operador de comparación, se utiliza en la división
    bool operator<(const BigUnsigned& other) const {
        if(digits.size() != other.digits.size())
            return digits.size() < other.digits.size();
        for(int i = digits.size()-1; i >= 0; i--){
            if(digits[i] != other.digits[i])
                return digits[i] < other.digits[i];
        }
        return false;
    }

    // Convierte el número a un entero de 64 bits
    unsigned long long to_decimal() const {
        unsigned long long result = 0, power = 1;
        for(size_t i = 0; i < digits.size(); i++){
            result += digits[i] * power;
            power *= Base;
        }
        return result;
    }

    // Método auxiliar que devuelve la representación numérica como cadena sin sufijo
    std::string toString() const {
        std::string s;
        // Se recorre el vector de dígitos en orden inverso para formar la cadena
        for (int i = digits.size()-1; i >= 0; i--)
            s.push_back(digitToChar(digits[i]));
        return s;
    }

    // Métodos virtuales de BigNumber implementados utilizando conversiones
    // Se utiliza dynamic_cast para detectar si 'other' es un BigInteger y, en ese caso,
    // se convierte *this a BigInteger para operar con signo
    virtual BigNumber<Base>& add(const BigNumber<Base>& other) const {
        const BigInteger<Base>* pInt = dynamic_cast<const BigInteger<Base>*>(&other);
        if(pInt != nullptr) {
            // Se convierte *this a BigInteger usando el operador de conversión
            BigInteger<Base> left = static_cast<BigInteger<Base>>( *this );
            BigInteger<Base> result = left + *pInt;
            BigInteger<Base>* pres = new BigInteger<Base>(result);
            return *pres;
        } else {
            // En otro caso, se convierte 'other' a BigUnsigned
            BigUnsigned<Base> otherConv = other;
            BigUnsigned<Base>* res = new BigUnsigned<Base>( (*this) + otherConv );
            return *res;
        }
    }
    virtual BigNumber<Base>& subtract(const BigNumber<Base>& other) const {
        const BigInteger<Base>* pInt = dynamic_cast<const BigInteger<Base>*>(&other);
        if(pInt != nullptr) {
            BigInteger<Base> left = static_cast<BigInteger<Base>>( *this );
            BigInteger<Base> result = left - *pInt;
            BigInteger<Base>* pres = new BigInteger<Base>(result);
            return *pres;
        } else {
            BigUnsigned<Base> otherConv = other;
            BigUnsigned<Base>* res = new BigUnsigned<Base>( (*this) - otherConv );
            return *res;
        }
    }
    virtual BigNumber<Base>& multiply(const BigNumber<Base>& other) const {
        const BigInteger<Base>* pInt = dynamic_cast<const BigInteger<Base>*>(&other);
        if(pInt != nullptr) {
            BigInteger<Base> left = static_cast<BigInteger<Base>>( *this );
            BigInteger<Base> result = left * *pInt;
            BigInteger<Base>* pres = new BigInteger<Base>(result);
            return *pres;
        } else {
            BigUnsigned<Base> otherConv = other;
            BigUnsigned<Base>* res = new BigUnsigned<Base>( (*this) * otherConv );
            return *res;
        }
    }
    virtual BigNumber<Base>& divide(const BigNumber<Base>& other) const {
        const BigInteger<Base>* pInt = dynamic_cast<const BigInteger<Base>*>(&other);
        if(pInt != nullptr) {
            BigInteger<Base> left = static_cast<BigInteger<Base>>( *this );
            BigInteger<Base> result = left / *pInt;
            BigInteger<Base>* pres = new BigInteger<Base>(result);
            return *pres;
        } else {
            BigUnsigned<Base> otherConv = other;
            BigUnsigned<Base>* res = new BigUnsigned<Base>( (*this) / otherConv );
            return *res;
        }
    }

    // Operadores de conversión virtuales
    virtual operator BigUnsigned<Base>() const {
        return *this;
    }
    // Se implementa en línea en BigUnsigned.hpp la conversión a BigInteger y BigRational
    virtual operator BigInteger<Base>() const;
    virtual operator BigRational<Base>() const;

    // se imprime la representación sin sufijo toString() y se añade u
    virtual std::ostream& write(std::ostream& out) const {
        out << toString() << "u";
        return out;
    }
    // lee una cadena, crea un BigUnsigned y asigna el valor
    virtual std::istream& read(std::istream& in) {
        std::string s;
        in >> s;
        *this = BigUnsigned(s.c_str());
        return in;
    }

    // Operador de igualdad, usado para la división
    bool operator==(const BigUnsigned& other) const {
        return digits == other.digits;
    }
};

// Implementación de la conversión de BigUnsigned a BigInteger
// Se llama al constructor de BigInteger que recibe un BigUnsigned
template <unsigned char Base>
BigUnsigned<Base>::operator BigInteger<Base>() const {
    return BigInteger<Base>(*this);
}

// Implementación de la conversión de BigUnsigned a BigRational
// Se convierte *this a BigInteger y se usa como numerador
template <unsigned char Base>
BigUnsigned<Base>::operator BigRational<Base>() const {
    return BigRational<Base>( BigInteger<Base>(*this) );
}

#endif
