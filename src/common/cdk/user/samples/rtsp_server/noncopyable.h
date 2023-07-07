#ifndef _KD_NONCOPYABLE_H
#define _KD_NONCOPYABLE_H

class Noncopyable {
  protected:
    Noncopyable() {}
    ~Noncopyable() {}
  private:  // emphasize the following members are private
    Noncopyable( const Noncopyable& );
    const Noncopyable& operator=( const Noncopyable& );
};

#endif // _KD_NONCOPYABLE_H