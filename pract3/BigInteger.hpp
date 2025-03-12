#ifndef BIGINTEGER_HPP
#define BIGINTEGER_HPP

#include "BigUnsigned.hpp"
#include <string>

// Daniel Palenzuela Álvarez alu0101140469

// Clase para números grandes con signo.
template <unsigned char Base>
class BigInteger : public BigNumber<Base> {
private:
    // Se utiliza un objeto BigUnsigned para almacenar la parte numérica.
    BigUnsigned<Base> number;
    // Indica si el número es negativo.
    bool isNegative;

public:
    // Constructor a partir de un entero.
    BigInteger(int n = 0)
        : isNegative(n < 0),
          number((n < 0) ? std::to_string(-n).c_str() : std::to_string(n).c_str()) {}

    // Constructor a partir de un BigUnsigned (número no negativo).
    BigInteger(const BigUnsigned<Base>& bigUnsigned)
        : number(bigUnsigned), isNegative(false) {}

    // Constructor a partir de una cadena con signo opcional.
    BigInteger(const char* str) {
        std::string s(str);
        if(!s.empty() && s[0]=='-'){
            isNegative = true;
            s.erase(0,1);  // Se elimina el signo "-"
        } else {
            isNegative = false;
        }
        number = BigUnsigned<Base>(s.c_str());
    }

    // Operador suma
    BigInteger operator+(const BigInteger& other) const {
        // Si ambos números tienen el mismo signo, se suma y se conserva el signo.
        if(isNegative == other.isNegative) {
            BigInteger res(number + other.number);
            res.isNegative = isNegative;
            return res;
        } else {
            // Si los signos son distintos, se efectúa la resta y se asigna el signo del mayor valor absoluto.
            if(number.to_decimal() >= other.number.to_decimal()){
                BigInteger res(number - other.number);
                res.isNegative = isNegative;
                return res;
            } else {
                BigInteger res(other.number - number);
                res.isNegative = other.isNegative;
                return res;
            }
        }
    }

    // Operador resta
    // definido en términos de suma, invirtiendo el signo del segundo operando.
    BigInteger operator-(const BigInteger& other) const {
        BigInteger negOther = other;
        negOther.isNegative = !negOther.isNegative;
        return (*this) + negOther;
    }

    // Operador multiplicación
    BigInteger operator*(const BigInteger& other) const {
        BigInteger res(number * other.number);
        // El resultado es negativo si solo uno de los operandos es negativo.
        res.isNegative = (isNegative != other.isNegative);
        return res;
    }

    // Operador división 
    // asumo que BigUnsigned tiene operator/ implementado
    BigInteger operator/(const BigInteger& other) const {
        BigInteger res(number / other.number);
        res.isNegative = (isNegative != other.isNegative);
        return res;
    }

    // Método para obtener la parte entera en decimal
    unsigned long long to_decimal() const {
        return number.to_decimal();
    }

    // Métodos virtuales de BigNumber utilizando conversión.
    virtual BigNumber<Base>& add(const BigNumber<Base>& other) const {
        // Se utiliza el operador de conversión a BigInteger.
        BigInteger otherConv = other;
        BigInteger* res = new BigInteger((*this) + otherConv);
        return *res;
    }
    virtual BigNumber<Base>& subtract(const BigNumber<Base>& other) const {
        BigInteger otherConv = other;
        BigInteger* res = new BigInteger((*this) - otherConv);
        return *res;
    }
    virtual BigNumber<Base>& multiply(const BigNumber<Base>& other) const {
        BigInteger otherConv = other;
        BigInteger* res = new BigInteger((*this) * otherConv);
        return *res;
    }
    virtual BigNumber<Base>& divide(const BigNumber<Base>& other) const {
        BigInteger otherConv = other;
        BigInteger* res = new BigInteger((*this) / otherConv);
        return *res;
    }

    // Operadores de conversión
    virtual operator BigUnsigned<Base>() const {
        return number;
    }
    virtual operator BigInteger<Base>() const {
        return *this;
    }
    // La conversión a BigRational se implementa en BigInteger
    virtual operator BigRational<Base>() const;

    // Método write: se imprime la parte numérica (usando toString() de BigUnsigned) y se añade "i".
    virtual std::ostream& write(std::ostream& out) const {
        if(isNegative)
            out << "-"; // Imprime el signo negativo si corresponde.
        out << number.toString() << "i"; // Imprime el número sin sufijo y luego "i".
        return out;
    }
    // Método read: lee una cadena y crea un BigInteger
    virtual std::istream& read(std::istream& in) {
        std::string s;
        in >> s;
        *this = BigInteger(s.c_str());
        return in;
    }

    // Sobrecarga del operador << para imprimir BigInteger
    template <unsigned char B>
    friend std::ostream& operator<<(std::ostream& out, const BigInteger<B>& num);
};

template <unsigned char B>
std::ostream& operator<<(std::ostream& out, const BigInteger<B>& num) {
    return num.write(out);
}

// Implementación de la conversión de BigInteger a BigRational
// Crea un BigRational con denominador 1
template <unsigned char Base>
BigInteger<Base>::operator BigRational<Base>() const {
    return BigRational<Base>(*this, BigUnsigned<Base>("1"));
}

#endif
