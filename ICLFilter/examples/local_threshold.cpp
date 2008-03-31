#include <iclImg.h>
#include <iclFileGrabber.h>
#include <iclFileWriter.h>
#include <iclLocalThresholdOp.h>
#include <iclTestImages.h>
#include <iclProgArg.h>
#include <stdio.h>


using namespace icl;
using namespace std;


int main (int argc, char **argv) {
  pa_init(argc, argv,"-input(1) -output(1) -masksize(1) -globalthreshold(1)");

  Size s(640,480);
  ImgBase  *I = 0, *R = new Img8u(s,formatRGB);
  

  if(pa_defined("-input")){
    string s = pa_subarg<string>("-input",0,"");
    printf("input file %s \n",s.c_str());
    FileGrabber  gr(s);
    gr.setIgnoreDesiredParams(true);
    gr.grab(&I);
  }
  else{
    printf("working with testimage tree: \n");
    TestImages::create("tree")->convert(I);
  }
  
  unsigned int maskSize = pa_subarg<unsigned int>("-masksize",0,10);
  int gt = pa_subarg<int>("-globalthreshold",0,0);
  printf("mask size was set to %d \n",maskSize);
  printf("global threshold was set to  %d \n",gt);
  
  LocalThresholdOp t(maskSize,gt);
  t.apply(I,&R);
  
  string outputName = pa_subarg<string>("-output",0,"./out.jpg");
  printf("writing output file %s \n", outputName.c_str());
  FileWriter(outputName).write(R);
}
