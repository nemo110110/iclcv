#include <ICLUtils/Rect32f.h>
#include <ICLUtils/Rect.h>



namespace icl{

  const Rect32f Rect32f::null(0,0,0,0);
  
  
  /// ostream operator (x,y)wxy
  std::ostream &operator<<(std::ostream &s, const Rect32f &r){
    return s << r.ul() << r.getSize();
  }
  
  /// istream operator
  std::istream &operator>>(std::istream &s, Rect32f &r){
    Point32f offs;
    Size32f size;
    s >> offs;
    s >> size;
    r = Rect32f(offs,size);
    return s;
  }

  
}