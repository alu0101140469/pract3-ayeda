#ifndef BIGRATIONAL_HPP
#define BIGRATIONAL_HPP

#include "BigInteger.hpp"

// Daniel Palenzuela Álvarez alu0101140469

// Clase para representar números racionales
template <unsigned char Base>
class BigRational : public BigNumber<Base> {
private:
    // Numerador de tipo BigInteger
    BigInteger<Base> numerator;
    // Denominador de tipo BigUnsigned (siempre positivo)
    BigUnsigned<Base> denominator;

public:
    // Constructor por defecto: representa el número 0/1
    BigRational(const BigInteger<Base>& num = 0, const BigUnsigned<Base>& den = BigUnsigned<Base>("1"))
        : numerator(num), denominator(den) {}

    // Operador suma para racionales
    // (a/b) + (c/d) = (a*d + c*b) / (b*d)
    BigRational operator+(const BigRational& other) const {
        // Se convierten los denominadores a BigInteger para la operación
        const BigInteger<Base> temp1(other.denominator);
        const BigInteger<Base> temp2(denominator);
        BigInteger<Base> newNum = numerator * temp1 + other.numerator * temp2;
        BigUnsigned<Base> newDen = denominator * other.denominator;
        return BigRational(newNum, newDen);
    }

    // Operador resta
    BigRational operator-(const BigRational& other) const {
        const BigInteger<Base> temp1(other.denominator);
        const BigInteger<Base> temp2(denominator);
        BigInteger<Base> newNum = numerator * temp1 - other.numerator * temp2;
        BigUnsigned<Base> newDen = denominator * other.denominator;
        return BigRational(newNum, newDen);
    }

    // Operador multiplicación
    BigRational operator*(const BigRational& other) const {
        BigInteger<Base> newNum = numerator * other.numerator;
        BigUnsigned<Base> newDen = denominator * other.denominator;
        return BigRational(newNum, newDen);
    }

    // Para la división se lanza excepción
    virtual BigNumber<Base>& divide(const BigNumber<Base>& other) const {
        throw BigNumberException();
    }

    // Métodos virtuales de BigNumber utilizando conversión
    virtual BigNumber<Base>& add(const BigNumber<Base>& other) const {
        BigRational otherConv = other;  // Se utiliza el operador de conversión a BigRational
        BigRational* res = new BigRational((*this) + otherConv);
        return *res;
    }
    virtual BigNumber<Base>& subtract(const BigNumber<Base>& other) const {
        BigRational otherConv = other;
        BigRational* res = new BigRational((*this) - otherConv);
        return *res;
    }
    virtual BigNumber<Base>& multiply(const BigNumber<Base>& other) const {
        BigRational otherConv = other;
        BigRational* res = new BigRational((*this) * otherConv);
        return *res;
    }

    // Operadores de conversión
    virtual operator BigUnsigned<Base>() const {
        // para la aproximación se devuelve la parte entera utilizando to_decimal() de BigInteger.
        return BigUnsigned<Base>( std::to_string(numerator.to_decimal()).c_str() );
    }
    virtual operator BigInteger<Base>() const {
        return numerator;
    }
    virtual operator BigRational<Base>() const {
        return *this;
    }

    // Imprime el racional en formato "numerador/denominador" seguido del sufijo r
    virtual std::ostream& write(std::ostream& out) const {
        out << numerator << "/" << denominator << "r";
        return out;
    }
    // Lee una cadena en el formato "numerador/denominador"
    virtual std::istream& read(std::istream& in) {
        std::string s;
        in >> s;
        size_t pos = s.find('/');
        if(pos == std::string::npos)
            throw BigNumberException();
        std::string num = s.substr(0, pos);
        std::string den = s.substr(pos+1);
        numerator = BigInteger<Base>(num.c_str());
        denominator = BigUnsigned<Base>(den.c_str());
        return in;
    }

    // Sobrecarga del operador << para imprimir BigRational
    template <unsigned char B>
    friend std::ostream& operator<<(std::ostream& out, const BigRational<B>& num);
};

template <unsigned char B>
std::ostream& operator<<(std::ostream& out, const BigRational<B>& num) {
    return num.write(out);
}

#endif
