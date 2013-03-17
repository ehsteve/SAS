/*

Pair, Pair3B

These classes are for pairs of floats.  Pair is not particularly more useful
than other alternatives, but Pair3B is critical for writing reduced-precision
pixel location floats to ByteStrings.  Two 4-byte floats are stored in only
3 bytes by rounding to the nearest third of a pixel.

For now, the actual values are kept private, and can be read by using x() and
y().  The only way to set the values after the construction is using the
ByteString insertion operator.

To write a Pair3B:
  Pair3B a(123.45,56.78);
  ByteString bs;
  bs << a;

To write a Pair as a Pair3B:
  Pair p(135.79,246.80);
  bs << Pair3B(p);

To read a Pair3B:
  Pair3B b;
  bs >> b;

*/


#ifndef _TYPES_HPP_
#define _TYPES_HPP_

#include <iostream>
#include <stdint.h>

#include "Packet.hpp"

class Pair;
class Pair3B;

class Pair {
  private:
    float i_x, i_y;

  public:
    Pair(float x = 0, float y = 0);
    Pair(const Pair3B& p3);

    float x() const { return i_x; };
    float y() const { return i_y; };

    friend ByteString& operator<<(ByteString& bs, const Pair& p);
    friend std::ostream& operator<<(std::ostream& os, const Pair& p);

    friend ByteString& operator>>(ByteString& bs, Pair& p);
};

class Pair3B {
  private:
    uint16_t i_a, i_b;

    void initialize(float x, float y);

  public:
    Pair3B(float x = 0, float y = 0);
    Pair3B(const Pair& p);

    float x() const;
    float y() const;

    friend ByteString& operator<<(ByteString& bs, const Pair3B& p3);
    friend std::ostream& operator<<(std::ostream& os, const Pair3B& p3);

    friend ByteString& operator>>(ByteString& bs, Pair3B& p3);
};

#endif
