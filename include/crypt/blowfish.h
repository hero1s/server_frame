
#pragma once

#include <stdlib.h>
#include <stdio.h>

class CBlowFish {
public:
    enum {
      ECB = 0, CBC = 1, CFB = 2
    };

    //Block Structure
    struct SBlock {
      //Constructors
      SBlock(unsigned int l = 0, unsigned int r = 0)
              :m_uil(l), m_uir(r) { }

      //Copy Constructor
      SBlock(const SBlock& roBlock)
              :m_uil(roBlock.m_uil), m_uir(roBlock.m_uir) { }

      SBlock& operator^=(SBlock& b)
      {
          m_uil ^= b.m_uil;
          m_uir ^= b.m_uir;
          return *this;
      }

      unsigned int m_uil, m_uir;
    };

    //Constructor - Initialize the P and S boxes for a given Key
    CBlowFish(unsigned char* ucKey, size_t n, const SBlock& roChain = SBlock(0UL, 0UL));

    //Resetting the chaining block
    void ResetChain() { m_oChain = m_oChain0; }

    // Encrypt/Decrypt Buffer in Place
    void Encrypt(unsigned char* buf, size_t n, int iMode = ECB);

    void Decrypt(unsigned char* buf, size_t n, int iMode = ECB);

    // Encrypt/Decrypt from Input Buffer to Output Buffer
    void Encrypt(const unsigned char* in, unsigned char* out, size_t n, int iMode = ECB);

    void Decrypt(const unsigned char* in, unsigned char* out, size_t n, int iMode = ECB);

//Private Functions
private:
    unsigned char Byte(unsigned int ui);

    unsigned int F(unsigned int ui);

    void BlockToBytes(SBlock const& b, unsigned char* p);

    void BytesToBlock(unsigned char const* p, SBlock& b);

    void Encrypt(SBlock&);

    void Decrypt(SBlock&);

private:
    //The Initialization Vector, by default {0, 0}
    SBlock                    m_oChain0;
    SBlock                    m_oChain;
    unsigned int              m_auiP[18];
    unsigned int              m_auiS[4][256];
    static const unsigned int scm_auiInitP[18];
    static const unsigned int scm_auiInitS[4][256];
};

//Semi-Portable Byte Shuffling
inline void CBlowFish::BytesToBlock(unsigned char const* p, SBlock& b)
{
    unsigned int y;
    //Left
    b.m_uil = 0;
    y = *p++;
    y <<= 24;
    b.m_uil |= y;
    y = *p++;
    y <<= 16;
    b.m_uil |= y;
    y = *p++;
    y <<= 8;
    b.m_uil |= y;
    y       = *p++;
    b.m_uil |= y;
    //Right
    b.m_uir = 0;
    y = *p++;
    y <<= 24;
    b.m_uir |= y;
    y = *p++;
    y <<= 16;
    b.m_uir |= y;
    y = *p++;
    y <<= 8;
    b.m_uir |= y;
    y = *p++;
    b.m_uir |= y;
}

inline void CBlowFish::BlockToBytes(SBlock const& b, unsigned char* p)
{
    unsigned int y;
    //Right
    y = b.m_uir;
    *--p = Byte(y);
    y = b.m_uir >> 8;
    *--p = Byte(y);
    y = b.m_uir >> 16;
    *--p = Byte(y);
    y = b.m_uir >> 24;
    *--p = Byte(y);
    //Left
    y = b.m_uil;
    *--p = Byte(y);
    y = b.m_uil >> 8;
    *--p = Byte(y);
    y = b.m_uil >> 16;
    *--p = Byte(y);
    y = b.m_uil >> 24;
    *--p = Byte(y);
}

//Extract low order byte
inline unsigned char CBlowFish::Byte(unsigned int ui)
{
    return (unsigned char) (ui & 0xff);
}

//Function F
inline unsigned int CBlowFish::F(unsigned int ui)
{
    return ((m_auiS[0][Byte(ui >> 24)]+m_auiS[1][Byte(ui >> 16)]) ^ m_auiS[2][Byte(ui >> 8)])
            +m_auiS[3][Byte(ui)];
}




