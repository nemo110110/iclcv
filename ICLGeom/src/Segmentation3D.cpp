/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/Segmentation3D.cpp                         **
** Module : ICLGeom                                                **
** Authors: Andre Ueckermann                                       **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#define __CL_ENABLE_EXCEPTIONS //enables openCL error catching
#ifdef HAVE_OPENCL
#include <CL/cl.hpp>
#endif

#include <ICLGeom/Segmentation3D.h>

#include <ICLQt/Quick.h>
#include <ICLGeom/GeomDefs.h>

namespace icl{
  namespace geom{
    //OpenCL kernel code
    static char segmentationKernel[] = 
      "  #pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable                                                           \n"
      "__kernel void                                                                                                                  \n"
      "calculatePointAssignment(__global float4 const * xyz, __global bool * elements, __global int const * assignment, int const radius, int const numFaces, __global bool * neighbours, __global int * assignmentOut, int const w, int const h, float const maxDist)                                                                   \n"
      "{                                                                                                                              \n"
      "  size_t id =  get_global_id(0);                                                                                               \n"
      "  int y = (int)floor((float)id/(float)w);                                                                                      \n"
      "  int x = id-y*w;                                                                                                              \n"
      "  float dist=100000;                                                                                                           \n"
      "  int ass=0;                                                                                                                   \n"
      "  bool assigned=false;                                                                                                         \n"
      "  if(elements[id]==true && assignment[id]==0){                                                                                 \n"
      "    bool adj[100];                                                                                                             \n"
      "    for(int a=0; a<numFaces; a++){                                                                                             \n"
      "      adj[a]=false;                                                                                                            \n"
      "    }                                                                                                                          \n"
      "    for(int xx=-radius; xx<=radius; xx++){                                                                                     \n"
      "      for(int yy=-radius; yy<=radius; yy++){                                                                                   \n"
      "        if(x+xx>=0 && x+xx<w && y+yy>=0 && y+yy<h && assignment[(x+xx)+w*(y+yy)]!=0){                                          \n"
      "          float4 pointa=xyz[id];                                                                                               \n"
      "          pointa.w=1.0;                                                                                                        \n"
      "          float4 pointb=xyz[(x+xx)+w*(y+yy)];                                                                                  \n"
      "          pointb.w=1.0;                                                                                                        \n"
      "          float dist3 = distance(pointa,pointb);                                                                               \n"
      "          if(dist3<maxDist){                                                                                                   \n"
      "            adj[assignment[(x+xx)+w*(y+yy)]-1]=true;                                                                           \n"
      "          }                                                                                                                    \n"
      "          if(dist3<dist && dist3<maxDist){                                                                                     \n"   
      "            dist=dist3;                                                                                                        \n"
      "            ass=assignment[(x+xx)+w*(y+yy)];                                                                                   \n"
      "            assigned=true;                                                                                                     \n"
      "          }                                                                                                                    \n"
      "        }                                                                                                                      \n"
      "      }                                                                                                                        \n"
      "    }                                                                                                                          \n"
      "    for(int a=0; a<numFaces-1; a++){                                                                                           \n"
      "      for (int b=a+1; b<numFaces; b++){                                                                                        \n"
      "        if(adj[a]==true && adj[b]==true){                                                                                      \n"
      "          neighbours[a*numFaces+b]=true;                                                                                       \n"
      "          neighbours[b*numFaces+a]=true;                                                                                       \n"
      "        }                                                                                                                      \n"
      "      }                                                                                                                        \n"
      "    }                                                                                                                          \n"
      "    if(assigned==true){                                                                                                        \n"
      "      assignmentOut[id]=ass;                                                                                                   \n"
      "      elements[id]=false;                                                                                                      \n"
      "    }                                                                                                                          \n"
      "  }                                                                                                                            \n"
      "}                                                                                                                              \n"
      "__kernel void                                                                                                                  \n"
      "checkRANSAC(__global float4 const * xyz, int const RANSACpasses, __global float4 const * n0, __global float const * dist, __global int * countAbove, __global int * countBelow, __global int * countOn, int const RANSACeucl, int const numPoints, __global const int * points, int const subset)                                  \n"
      "{                                                                                                                              \n"
      "  size_t id =  get_global_id(0);                                                                                               \n"
      "  int pass = id%RANSACpasses;                                                                                                  \n"
      "  int point = ((int)floor((float)id/(float)RANSACpasses))*subset;                                                              \n"
      "  int cPoint = points[point];                                                                                                  \n"
      "  float4 selPoint = xyz[cPoint];                                                                                               \n"
      "  float4 seln0 = n0[pass];                                                                                                     \n"
      "  float distance1 = dist[pass];                                                                                                \n"
      "  float s1 = (selPoint.x*seln0.x+selPoint.y*seln0.y+selPoint.z*seln0.z)-distance1;                                             \n"
      "  if((s1>=-RANSACeucl && s1<=RANSACeucl)){                                                                                     \n"
      "    atomic_inc(&countOn[pass]);                                                                                                \n"
      "  }else if(s1>RANSACeucl){                                                                                                     \n"
      "    atomic_inc(&countAbove[pass]);                                                                                             \n"
      "  }else if(s1<-RANSACeucl){                                                                                                    \n"
      "    atomic_inc(&countBelow[pass]);                                                                                             \n"
      "  }                                                                                                                            \n"
      "}                                                                                                                              \n"
      "__kernel void                                                                                                                  \n"
      "assignRANSAC(__global float4 const * xyz, __global bool * elements, __global int * assignment, float4 const n0, float const d, int const euclDist, __global int * oldAssignment, int maxID) \n"
      "{                                                                                                                              \n"
      "  size_t id =  get_global_id(0);                                                                                               \n"
      "  if(oldAssignment[id]==maxID)                                                                                                 \n"
      "  {                                                                                                                            \n"
      "    assignment[id]=1;                                                                                                          \n"
      "    elements[id]=false;                                                                                                        \n"
      "  }                                                                                                                            \n"
      "  else                                                                                                                         \n"
      "  {                                                                                                                            \n"
      "    float4 xyzD=xyz[id];                                                                                                       \n"
      "    float s1 = (xyzD.x*n0.x+xyzD.y*n0.y+xyzD.z*n0.z)-d;                                                                        \n"
      "    if((s1>=-euclDist && s1<=euclDist) && elements[id]==true)                                                                  \n"
      "    {                                                                                                                          \n"
      "      assignment[id]=1;                                                                                                        \n"
      "      elements[id]=false;                                                                                                      \n"
      "    }                                                                                                                          \n"
      "  }                                                                                                                            \n"
      "}                                                                                                                              \n"
      "__kernel void                                                                                                                  \n"
      "segmentColoring(__global int const * assignment, __global uchar * colorR, __global uchar * colorG, __global uchar * colorB)    \n"
      "{                                                                                                                              \n"
      "  size_t id =  get_global_id(0);                                                                                               \n"
      "  if(assignment[id]==0)                                                                                                        \n"
      "  {                                                                                                                            \n"
      "    colorR[id]=128;                                                                                                            \n"
      "    colorG[id]=128;                                                                                                            \n"
      "    colorB[id]=128;                                                                                                            \n"
      "  }                                                                                                                            \n"
      "  else                                                                                                                         \n"
      "  {                                                                                                                            \n"
      "    int H=(int)(assignment[id]*35.)%360;                                                                                       \n"
      "    float S=1.0-assignment[id]*0.01;                                                                                           \n"
      "    float hi=floor((float)H/60.);                                                                                              \n"
			"	   float f=((float)H/60.)-hi;                                                                                                 \n"
			"	   float pp=1.0-S;                                                                                                            \n"
			"	   float qq=1.0-S*f;                                                                                                          \n"
			"	   float tt=1.0-S*(1.-f);                                                                                                     \n"
			"	   float newR=0;                                                                                                              \n"
			"	   float newG=0;                                                                                                              \n"
			"	   float newB=0;                                                                                                              \n"
			"	   if((int)hi==0 || (int)hi==6){                                                                                              \n"
			"	  	 newR=1.0;                                                                                                                \n"
			"	     newG=tt;                                                                                                                 \n"
			"		   newB=pp;                                                                                                                 \n"
			"	   }else if((int)hi==1){                                                                                                      \n"
			"		   newR=qq;                                                                                                                 \n"
			"		   newG=1.0;                                                                                                                \n"
			"		   newB=pp;                                                                                                                 \n"
			"	   }else if((int)hi==2){                                                                                                      \n"
			"		   newR=pp;                                                                                                                 \n"
			"		   newG=1.0;                                                                                                                \n"
			"	     newB=tt;                                                                                                                 \n"
			"	   }else if((int)hi==3){                                                                                                      \n"
			"		   newR=pp;                                                                                                                 \n"
			"	     newG=qq;                                                                                                                 \n"
			"	     newB=1.0;                                                                                                                \n"
			"	   }else if((int)hi==4){                                                                                                      \n"
			"		   newR=tt;                                                                                                                 \n"
			"		   newG=pp;                                                                                                                 \n"
			"		   newB=1.0;                                                                                                                \n"
			"	   }else if((int)hi==5){                                                                                                      \n"
			"	     newR=1.0;                                                                                                                \n"
			"		   newG=pp;                                                                                                                 \n"
			"		   newB=qq;                                                                                                                 \n"
			"	   }                                                                                                                          \n"
		  "    colorR[id]=(unsigned char)(newR*255.);                                                                                     \n"
		  "    colorG[id]=(unsigned char)(newG*255.);                                                                                     \n"
		  "    colorB[id]=(unsigned char)(newB*255.);                                                                                     \n"	
		  "  }                                                                                                                            \n"
		  "}                                                                                                                              \n"
    ;
  
                
    Segmentation3D::Segmentation3D(Size size){
      //set default values
      clReady=false;
      useCL=true;
      w=size.width;
      h=size.height;
      dim=w*h;
      s=size;  
      
      minClusterSize=25;
      useFastGrowing=false;
      assignmentRadius=5;
      assignmentMaxDistance=15;
      
      RANSACeuclDistance=15; 
      RANSACpasses=20; 
      RANSACtolerance=30; 
      RANSACsubset=2;
      BLOBSeuclDistance=15;
            
      useROI=false;
      xMinROI=0, xMaxROI=0, yMinROI=0; yMaxROI=0;
      elements=new bool[w*h];
      assignment=new int[w*h];
      assignmentRemaining=new int[w*h];
      elementsBlobs=new bool[w*h];
      assignmentBlobs=new int[w*h];
      
      normalEdgeImage.setSize(Size(w,h));
      normalEdgeImage.setChannels(1);
      depthImage.setSize(Size(w,h));
      depthImage.setChannels(1);
      
      segmentColorImage.setSize(Size(w,h));
      segmentColorImage.setChannels(3);
      
      region=new RegionDetector(25, 4000000, 254, 255, false);
         
    #ifdef HAVE_OPENCL
      //create openCL context
      segmentColorImageRArray = new cl_uchar[w*h];
      segmentColorImageGArray = new cl_uchar[w*h];
      segmentColorImageBArray = new cl_uchar[w*h];
      
      std::vector<cl::Platform> platformList;//get number of available openCL platforms
      int selectedDevice=0;//initially select platform 0
      try{
        if(cl::Platform::get(&platformList)==CL_SUCCESS){
          std::cout<<"openCL platform found"<<std::endl;
        }else{
          std::cout<<"no openCL platform available"<<std::endl;
        }
        std::cout<<"number of openCL platforms: "<<platformList.size()<<std::endl;
          
        //check devices on platforms
        for(unsigned int i=0; i<platformList.size(); i++){//check all platforms
          std::cout<<"platform "<<i+1<<":"<<std::endl;
          std::vector<cl::Device> deviceList;
          if(platformList.at(i).getDevices(CL_DEVICE_TYPE_GPU, &deviceList)==CL_SUCCESS){
            std::cout<<"GPU-DEVICE(S) FOUND"<<std::endl;
            selectedDevice=i; //if GPU found on platform, select this platform
            clReady=true; //and mark CL context as available
          }else if(platformList.at(i).getDevices(CL_DEVICE_TYPE_CPU, &deviceList)==CL_SUCCESS){
            std::cout<<"CPU-DEVICE(S) FOUND"<<std::endl;
          }else{
            std::cout<<"UNKNOWN DEVICE(S) FOUND"<<std::endl;
          }
          std::cout<<"number of devices: "<<deviceList.size()<<std::endl;
        }
      }catch (cl::Error err) {//catch openCL errors
        std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<< std::endl;
        std::cout<<"OpenCL not ready"<<std::endl;
        clReady=false;//disable openCL on error
      }  
        
      if(clReady==true){//only if CL context is available
        try{
          cl_context_properties cprops[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)(platformList[selectedDevice])(), 0};//get context properties of selected platform
          context = cl::Context(CL_DEVICE_TYPE_GPU, cprops);//select GPU device
          devices = context.getInfo<CL_CONTEXT_DEVICES>();
                
          std::cout<<"selected devices: "<<devices.size()<<std::endl;
                
          cl::Program::Sources sources(1, std::make_pair(segmentationKernel, 0)); //kernel source
          program=cl::Program(context, sources); //program (bind context and source)
          program.build(devices);//build program
  
          //create buffer for memory access and allocation
          segmentColorImageRBuffer = cl::Buffer(
  					  context, 
  					  CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, //
  					  w*h * sizeof(cl_uchar), 
  					  (void *) &segmentColorImageRArray[0]);
  			
  			  segmentColorImageGBuffer = cl::Buffer(
  					  context, 
  					  CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, //
  					  w*h * sizeof(cl_uchar), 
  					  (void *) &segmentColorImageGArray[0]);
  			
  			  segmentColorImageBBuffer = cl::Buffer(
  					  context, 
  					  CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, //
  					  w*h * sizeof(cl_uchar), 
  					  (void *) &segmentColorImageBArray[0]);
  					              
          //create kernels  
          kernelSegmentColoring=cl::Kernel(program, "segmentColoring");  
          kernelPointAssignment=cl::Kernel(program, "calculatePointAssignment");
          kernelCheckRANSAC=cl::Kernel(program, "checkRANSAC");
          kernelAssignRANSAC=cl::Kernel(program, "assignRANSAC");

          queue=cl::CommandQueue(context, devices[0], 0);//create command queue        
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<< std::endl;
          clReady=false;
        }
      }
        
    #else
      std::cout<<"no openCL parallelization available"<<std::endl;
      clReady=false;
    #endif   
    }
  
  
    Segmentation3D::~Segmentation3D(){
      delete[] elements;
      delete[] assignment;
      delete[] assignmentRemaining;
      delete[] elementsBlobs;
      delete[] assignmentBlobs;
    #ifdef HAVE_OPENCL
      delete[] segmentColorImageRArray;
      delete[] segmentColorImageGArray;
      delete[] segmentColorImageBArray;
    #endif
    }  
 
    
    Img8u Segmentation3D::segmentation(DataSegment<float,4> xyz, const Img8u &edgeImg, const Img32f &depthImg){
      xyzData=xyz;
      normalEdgeImage=edgeImg;
      depthImage=depthImg;
      clearData();  
      regionGrow();
	    calculatePointAssignmentAndAdjacency();
	    calculateCutfreeMatrix();
	    greedyComposition();
	    calculateRemainingPoints();
	    colorPointcloud();
		  return segmentColorImage;
		}
		
		
		Img8u Segmentation3D::segmentationBlobs(DataSegment<float,4> xyz, const Img8u &edgeImg, const Img32f &depthImg){
		  xyzData=xyz;
      normalEdgeImage=edgeImg;
      depthImage=depthImg;
      clearData();  
      regionGrow();
		  blobSegmentation();
	    colorPointcloud();
		  return segmentColorImage;
		}   
		
		
    void Segmentation3D::setUseCL(bool use){
      useCL=use;
    }
  

    void Segmentation3D::setUseROI(bool use){
      useROI=use;
    }
    
    
    void Segmentation3D::setROI(float xMin, float xMax, float yMin, float yMax){
      xMinROI=xMin;
      xMaxROI=xMax;
      yMinROI=yMin;
      yMaxROI=yMax;
    }

    
    void Segmentation3D::setMinClusterSize(unsigned int size){
      minClusterSize=size;
    }
    
    
    void Segmentation3D::setUseFastGrowing(bool use){
      useFastGrowing=use;
    }
   
    
    void Segmentation3D::setAssignmentRadius(int radius){
      assignmentRadius=radius;
    }
    
    		
    void Segmentation3D::setAssignmentMaxDistance(float maxDistance){
      assignmentMaxDistance=maxDistance;
    }
    
    
    void Segmentation3D::setRANSACeuclDistance(int distance){
      RANSACeuclDistance=distance;
    }
		
		   
    void Segmentation3D::setRANSACpasses(int passes){
      RANSACpasses=passes;
    }
    
      
    void Segmentation3D::setRANSACtolerance(int tolerance){
      RANSACtolerance=tolerance;
    }
    
      
    void Segmentation3D::setRANSACsubset(int subset){
      RANSACsubset=subset;
    }
    
    
    void Segmentation3D::setBLOBSeuclDistance(int distance){
      BLOBSeuclDistance=distance;
    }

   
    bool Segmentation3D::isCLReady(){
      return clReady;
    }
  
  
    bool Segmentation3D::isCLActive(){
      return useCL;
    }
    
    
    Img8u Segmentation3D::getSegmentColorImage(){
      return segmentColorImage;
    }
    
    
    std::vector<std::vector<int> > Segmentation3D::getCluster(){
      return cluster;
    }
    
    
		std::vector<std::vector<int> > Segmentation3D::getBlobs(){
		  return blobs;
		}
		
		
		DynMatrix<bool> Segmentation3D::getNeigboursMatrix(){
		  return neighbours;
		}
		
		
		DynMatrix<float> Segmentation3D::getProbabilityMatrix(){
		  return probabilities;
		} 
     
      
    void Segmentation3D::setXYZH(DataSegment<float,4> xyz){
      xyzData=xyz;
    }
    
        
    void Segmentation3D::setEdgeImage(const Img8u &edgeImage){
      normalEdgeImage=edgeImage;
    }
     
     
    void Segmentation3D::setDepthImage(const core::Img32f &depth){
      depthImage=depth;
    }
  
     
    void Segmentation3D::clearData(){
		  cluster.clear();
		  blobs.clear();
		  if(useROI){
		    for(int y=0;y<h;++y){
	     		for(int x=0;x<w;++x){
				    int i = x+w*y;
            if( xyzData[i][0]<xMinROI || xyzData[i][0]>xMaxROI || xyzData[i][1]<yMinROI || xyzData[i][1]>yMaxROI){
				      elements[i]=false;
        			assignment[i]=0;
        			assignmentRemaining[i]=0;
        			elementsBlobs[i]=false;
        			assignmentBlobs[i]=0;
        		}else{		
						  elements[i]=true;
		      		assignment[i]=0;
		      		assignmentRemaining[i]=0;
		      		elementsBlobs[i]=true;
		      		assignmentBlobs[i]=0;
		  			}
		      }
		    }
	    }else{
		    for(int y=0;y<h;++y){
	     		for(int x=0;x<w;++x){
				    int i = x+w*y;	
            elements[i]=true;
	        	assignment[i]=0;
	        	assignmentRemaining[i]=0;
	        	elementsBlobs[i]=true;
	        	assignmentBlobs[i]=0;
		  		}
		  	}
		  }
    }
  
  
    void Segmentation3D::regionGrow(){
      if(useFastGrowing){
        int numCluster=0;
        region->setConstraints (minClusterSize, 4000000, 254, 255);
        std::vector<std::vector<int> > remove;
        std::vector<ImageRegion> regions;
        regions = region->detect(&normalEdgeImage); 	
        for(unsigned int i=0; i<regions.size(); i++){
          numCluster++;
          std::vector<utils::Point> ps = regions.at(i).getPixels();
          std::vector<int> data;
          for(unsigned int j=0; j<ps.size(); j++){
            int v=ps.at(j)[0]+w*ps.at(j)[1];
            if(elements[v]==true){
              assignment[v]=numCluster;
              elements[v]=false;
              data.push_back(v);
            }
          }
          if(data.size()<minClusterSize){
			      remove.push_back(data);
			      numCluster--;
			    }
			    else{
			      cluster.push_back(data);
			    }
        }
        for(unsigned int i=0; i<remove.size(); i++){
		      for(unsigned int j=0; j<remove.at(i).size(); j++){
		        elements[remove.at(i).at(j)]=true;
		        assignment[remove.at(i).at(j)]=0;
		      }
		    } 
      }else{
        int numCluster=0;
        std::vector<std::vector<int> > remove;
		    for(int y=0;y<h;++y){
			    for(int x=0;x<w;++x){
				    int i = x+w*y;
				    if(elements[i]==true && normalEdgeImage(x,y,0)==255){
					    elements[i]=false;
					    numCluster++;
					    assignment[i]=numCluster;
					    std::vector<int> data;
					    data.push_back(i);
					    checkNeighbourGrayThreshold(x,y,assignment[i], 255, &data);
					    if(data.size()<minClusterSize){
					      remove.push_back(data);
					      numCluster--;
					    }
					    else{
					      cluster.push_back(data);
					    }
				    }
          }    			
		    }
		    for(unsigned int i=0; i<remove.size(); i++){
		      for(unsigned int j=0; j<remove.at(i).size(); j++){
		        elements[remove.at(i).at(j)]=true;
		        assignment[remove.at(i).at(j)]=0;
		      }
		    }
      }
    }
	  	
    
    void Segmentation3D::calculatePointAssignmentAndAdjacency(){
      if(useCL==true && clReady==true){
    #ifdef HAVE_OPENCL
        try{
          int numFaces=cluster.size();       
          DynMatrix<bool> newMatrix(numFaces,numFaces,false);
          neighbours=newMatrix;
          
          neighboursBuffer = cl::Buffer(
				           context, 
				           CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				           numFaces*numFaces * sizeof(bool), 
				           (void *) &neighbours[0]);
				  xyzBuffer = cl::Buffer(
				           context, 
				           CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
				           w*h * sizeof(cl_float4), 
        				   (void *) &xyzData[0]);
				  elementsBuffer = cl::Buffer(
				           context, 
				           CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				           w*h * sizeof(bool), 
				           (void *) &elements[0]);
				  assignmentBuffer = cl::Buffer(
				           context, 
				           CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
				           w*h * sizeof(int), 
				           (void *) &assignment[0]);
          assignmentOutBuffer = cl::Buffer(
				           context, 
				           CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				           w*h * sizeof(int), 
				           (void *) &assignment[0]);
          
          kernelPointAssignment.setArg(0, xyzBuffer);//set parameter for kernel
          kernelPointAssignment.setArg(1, elementsBuffer);
          kernelPointAssignment.setArg(2, assignmentBuffer);
          kernelPointAssignment.setArg(3, assignmentRadius);
          kernelPointAssignment.setArg(4, numFaces);
          kernelPointAssignment.setArg(5, neighboursBuffer);
          kernelPointAssignment.setArg(6, assignmentOutBuffer);
          kernelPointAssignment.setArg(7, w);
          kernelPointAssignment.setArg(8, h);
          kernelPointAssignment.setArg(9, assignmentMaxDistance);

          queue.enqueueNDRangeKernel(//run kernel
				     kernelPointAssignment, 
				     cl::NullRange, 
				     cl::NDRange(w*h), //input size for get global id
				     cl::NullRange);
    	    
          queue.enqueueReadBuffer(//read output from kernel
			          neighboursBuffer,
			          CL_TRUE, // block 
			          0,
			          numFaces*numFaces * sizeof(bool),
			          (bool*) neighbours.data());
			          
		      queue.enqueueReadBuffer(//read output from kernel
              assignmentOutBuffer,
              CL_TRUE, // block 
              0,
              w*h * sizeof(int),
              (int*) assignment);
          
          queue.enqueueReadBuffer(//read output from kernel
              elementsBuffer,
              CL_TRUE, // block 
              0,
              w*h * sizeof(bool),
              (bool*) elements);
              
          for(int i=0; i<numFaces; i++){///DIAG
            neighbours(i,i)=true;
          }
          cluster.clear();
          for(int x=0; x<w*h; x++){
            if(assignment[x]!=0){
					    if(assignment[x]>(int)cluster.size()){
						    cluster.resize(assignment[x]);
						  }
						  cluster.at(assignment[x]-1).push_back(x);
	          }	 
	        }
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }
    #endif
      }
      else{
        int numFaces=cluster.size();
        DynMatrix<bool> newMatrix(numFaces,numFaces,false);
        neighbours=newMatrix;
        int assignmentOut[w*h];
        for(int x=0; x<w; x++){
          for(int y=0; y<h; y++){
            int i=x+w*y;
            float dist=100000;
            int ass=0;
            bool assigned=false;
            if(elements[i]==true && assignment[i]==0){
              bool adj[numFaces];
              for(int a=0; a<numFaces; a++){
                adj[a]=false;
              }
              for(int xx=-assignmentRadius; xx<=assignmentRadius; xx++){
                for(int yy=-assignmentRadius; yy<=assignmentRadius; yy++){
                  if(x+xx>=0 && x+xx<w && y+yy>=0 && y+yy<h && assignment[(x+xx)+w*(y+yy)]!=0){
                    Vec p1=xyzData[i];
                    Vec p2=xyzData[(x+xx)+w*(y+yy)];
                    float distance=dist3(p1, p2);
                    if(distance<assignmentMaxDistance){
                      adj[assignment[(x+xx)+w*(y+yy)]-1]=true;
                    }
                    if(distance<dist && distance<assignmentMaxDistance){   
                      dist=distance;
                      ass=assignment[(x+xx)+w*(y+yy)];
                      assigned=true;
                    }
                  }
                }
              }
              for(int a=0; a<numFaces-1; a++){
                for (int b=a+1; b<numFaces; b++){
                  if(adj[a]==true && adj[b]==true){
                    neighbours(a,b)=true;
                    neighbours(b,a)=true;
                  }
                }
              }
              if(assigned==true){
                cluster.at(ass-1).push_back(i);
                elements[i]=false;
                assignmentOut[i]=ass;
              }
              else{
                assignmentOut[i]=assignment[i];
              }
            }
            else{
              assignmentOut[i]=assignment[i];
            }
          }
        }
        for(int i=0; i<numFaces; i++){
           neighbours(i,i)=true;
        }
        memcpy(assignment, assignmentOut, sizeof(assignmentOut));
      }
    }
    
    
    void Segmentation3D::calculateCutfreeMatrix(){
      DynMatrix<bool> newMatrix(neighbours.rows(),neighbours.cols(),false);
      cutfree=newMatrix;
    
      for(unsigned int a=0; a<neighbours.rows(); a++){
        #ifdef HAVE_OPENCL
        int numPoints=cluster.at(a).size();
        cl::Buffer RANSACpointsBuffer;
        cl_mem RANSACpointsMem = RANSACpointsBuffer();
        RANSACpointsBuffer = cl::Buffer(
				                   context, 
				                   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
				                   numPoints * sizeof(int), 
				                   (void *) &cluster.at(a)[0]);
			  #endif
        for(unsigned int b=0; b<neighbours.cols(); b++){
          if(a==b){
            cutfree(a,b)=true;
          }else if(neighbours(a,b)==false){
            cutfree(a,b)=false;
          }else{
            int countAcc=0;
            int countNAcc=0;
            
            if(useCL==true && clReady==true){
            #ifdef HAVE_OPENCL
             
              Vec n0[RANSACpasses];
              float dist[RANSACpasses];  
              int cAbove[RANSACpasses];
              int cBelow[RANSACpasses];
              int cOn[RANSACpasses];
              
              
              int cAboveRead[RANSACpasses];
              int cBelowRead[RANSACpasses];
              int cOnRead[RANSACpasses];
              
              for(int i=0; i<RANSACpasses; i++){
                cAbove[i]=0;
                cBelow[i]=0;
                cOn[i]=0;
                
                cAboveRead[i]=0;
                cBelowRead[i]=0;
                cOnRead[i]=0;
                
                
                Vec rPoint1;
                Vec n01;		
                int p0i=cluster.at(b).at(rand()%cluster.at(b).size());
                int p1i=cluster.at(b).at(rand()%cluster.at(b).size());
                int p2i=cluster.at(b).at(rand()%cluster.at(b).size());
                Vec fa1;
                Vec fb1;
                Vec n1;
              	fa1 = xyzData[p1i]-xyzData[p0i];
              	fb1 = xyzData[p2i]-xyzData[p0i];
                n1[0]=fa1[1]*fb1[2]-fa1[2]*fb1[1];
                n1[1]=fa1[2]*fb1[0]-fa1[0]*fb1[2];
                n1[2]=fa1[0]*fb1[1]-fa1[1]*fb1[0];
                n01[0]=n1[0]/norm3(n1);
                n01[1]=n1[1]/norm3(n1);
               	n01[2]=n1[2]/norm3(n1);
            	 	rPoint1 = xyzData[p0i];
                float distance1 = rPoint1[0]*n01[0]+rPoint1[1]*n01[1]+ rPoint1[2]*n01[2];
                dist[i]=distance1;
                n0[i]=n01;
              }
              
              try{         
                cl::Event waitEvent;
                cl::Buffer n0Buffer;
                cl::Buffer distBuffer;
                cl::Buffer countAboveBuffer;
                cl::Buffer countBelowBuffer;
                cl::Buffer countOnBuffer;
      
                cl_mem n0Mem = n0Buffer();
                cl_mem distMem = distBuffer();
                cl_mem countAboveMem = countAboveBuffer();
                cl_mem countBelowMem = countBelowBuffer();
                cl_mem countOnMem = countOnBuffer();
                
                n0Buffer = cl::Buffer(
		                     context, 
		                     CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
		                     RANSACpasses * sizeof(cl_float4), 
		                     (void *) &n0[0]);
		            distBuffer = cl::Buffer(
		                     context, 
		                     CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
		                     RANSACpasses * sizeof(float), 
		                     (void *) &dist[0]); 
		            countAboveBuffer = cl::Buffer(
		                     context, 
		                     CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
		                     RANSACpasses * sizeof(int), 
		                     (void *) &cAbove[0]);
		            countBelowBuffer = cl::Buffer(
		                     context, 
		                     CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
		                     RANSACpasses * sizeof(int), 
		                     (void *) &cBelow[0]);
                countOnBuffer = cl::Buffer(
		                     context, 
		                     CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
		                     RANSACpasses * sizeof(int), 
		                     (void *) &cOn[0]);

                kernelCheckRANSAC.setArg(0, xyzBuffer);//set parameter for kernel
                kernelCheckRANSAC.setArg(1, RANSACpasses);
                kernelCheckRANSAC.setArg(2, n0Buffer);
                kernelCheckRANSAC.setArg(3, distBuffer);
                kernelCheckRANSAC.setArg(4, countAboveBuffer);
                kernelCheckRANSAC.setArg(5, countBelowBuffer);
                kernelCheckRANSAC.setArg(6, countOnBuffer);
                kernelCheckRANSAC.setArg(7, RANSACeuclDistance);
                kernelCheckRANSAC.setArg(8, numPoints);
                kernelCheckRANSAC.setArg(9, RANSACpointsBuffer);
                kernelCheckRANSAC.setArg(10, RANSACsubset);
                              
                int idSize=RANSACpasses*numPoints/RANSACsubset;
                queue.enqueueNDRangeKernel(//run kernel
		               kernelCheckRANSAC, 
		               cl::NullRange, 
		               cl::NDRange(idSize), //input size for get global id
		               cl::NullRange,
		               NULL,
		               &waitEvent);
          	                	    
                queue.enqueueReadBuffer(//read output from kernel
                    countAboveBuffer,
                    CL_TRUE, // block 
                    0,
                    RANSACpasses * sizeof(int),
                    (int*) cAboveRead,
                    NULL,&waitEvent);
                               
                queue.enqueueReadBuffer(//read output from kernel
                    countBelowBuffer,
                    CL_TRUE, // block 
                    0,
                    RANSACpasses * sizeof(int),
                    (int*) cBelowRead,
                    NULL,&waitEvent);
                            
                queue.enqueueReadBuffer(//read output from kernel
                    countOnBuffer,
                    CL_TRUE, // block 
                    0,
                    RANSACpasses * sizeof(int),
                    (int*) cOnRead,
                    NULL,&waitEvent);
                 
                clFinish(queue()); 
                  
                clReleaseMemObject(n0Mem);
                clReleaseMemObject(distMem);
                clReleaseMemObject(countAboveMem);
                clReleaseMemObject(countBelowMem);
                clReleaseMemObject(countOnMem);
          
              }catch (cl::Error err) {//catch openCL errors
                std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
              }
              for(int i=0; i<RANSACpasses; i++){
                if(cAboveRead[i]<RANSACtolerance || cBelowRead[i]<RANSACtolerance){
                countAcc++;
                }else{
                  countNAcc++;
                }
              }
              
              if(countAcc>countNAcc){
                cutfree(a,b)=true;
              }else{
                cutfree(a,b)=false;
              }
            #endif
            }
            else{             
              for(int p=0; p<RANSACpasses; p++){
                Vec rPoint1;
                Vec n01;		
                int p0i=cluster.at(a).at(rand()%cluster.at(a).size());
                int p1i=cluster.at(a).at(rand()%cluster.at(a).size());
                int p2i=cluster.at(a).at(rand()%cluster.at(a).size());
                Vec fa1;
                Vec fb1;
                Vec n1;
                //PlaneFitting
              	fa1 = xyzData[p1i]-xyzData[p0i];
              	fb1 = xyzData[p2i]-xyzData[p0i];
                n1[0]=fa1[1]*fb1[2]-fa1[2]*fb1[1];
                n1[1]=fa1[2]*fb1[0]-fa1[0]*fb1[2];
                n1[2]=fa1[0]*fb1[1]-fa1[1]*fb1[0];
                n01[0]=n1[0]/norm3(n1);
                n01[1]=n1[1]/norm3(n1);
               	n01[2]=n1[2]/norm3(n1);
            	 	rPoint1 = xyzData[p0i];
                float distance1 = rPoint1[0]*n01[0]+rPoint1[1]*n01[1]+ rPoint1[2]*n01[2];
                    
                int countAbove=0;
                int countBelow=0;
                int countOn=0;
                for(unsigned int p=0; p<cluster.at(b).size(); p++){
                  float s1 = (xyzData[cluster.at(b).at(p)][0]*n01[0]+xyzData[cluster.at(b).at(p)][1]*n01[1]+xyzData[cluster.at(b).at(p)][2]*n01[2])-distance1;      
                  if((s1>=-RANSACeuclDistance && s1<=RANSACeuclDistance)){
                    countOn++;
                  }else if(s1>RANSACeuclDistance){
                    countAbove++;
                  }else if(s1<-RANSACeuclDistance){
                    countBelow++;
                  }
                }  
                if(countAbove<RANSACtolerance || countBelow<RANSACtolerance){
                  countAcc++;
                }else{
                  countNAcc++;
                }
              }
              if(countAcc>countNAcc){
                cutfree(a,b)=true;
              }else{
                cutfree(a,b)=false;
              }
            }
          }
        }
        #ifdef HAVE_OPENCL
        clReleaseMemObject(RANSACpointsMem);  
        #endif
      }    
    }
    
    
    void Segmentation3D::greedyComposition(){
      DynMatrix<bool> combinable=DynMatrix<bool>(cluster.size(),cluster.size(),false);
      probabilities=DynMatrix<float>(cluster.size(),cluster.size(),0.0);
      for(unsigned int a=0; a<cutfree.cols(); a++){
        for(unsigned int b=a; b<cutfree.cols(); b++){
          if(a==b){combinable(a,b)=0;}
          else{
            if(cutfree(a,b)==1 && cutfree(b,a)==1){
              combinable(a,b)=1;
              combinable(b,a)=1;
		        }
		        else{
		          combinable(a,b)=0;
		          combinable(b,a)=0;
		        }
		      }
        }
      }

      for(unsigned int a=0;a<combinable.cols(); a++){
        int count=0;
        for(unsigned int b=0; b<combinable.rows(); b++){
          if(combinable(b,a)==true) count++;
        }
        for(unsigned int b=0; b<combinable.rows(); b++){
          if(combinable(b,a)==true) probabilities(b,a)=1./(float)count;
        }
      }

      DynMatrix<float> W = probabilities;

      std::vector<std::vector<int> > facesCom;
      std::vector<float> probsCom;
      for(unsigned int a=0; a<W.cols(); a++){
        for(unsigned int b=0; b<W.cols(); b++){
          if(W(a,b)>0){
            std::vector<int> add;
            add.push_back(a);
            add.push_back(b);
            facesCom.push_back(add);
            probsCom.push_back(W(a,b));
          }
        }
      }
      for(unsigned int a=0; a<facesCom.size(); a++){
        for(unsigned int b=0; b<W.rows(); b++){
          bool breaking=false;
          for(unsigned int c=0; c<facesCom.at(a).size(); c++){
            if(facesCom.at(a).at(c)==(int)b){
              breaking=true;
              break;
            }
            else if(W(facesCom.at(a).at(c),b)==0){
              breaking=true;
              break;
            }
            else{
            }
          }
          if(breaking==false){
            float sum=0.0;
            std::vector<int> add;
            for(unsigned int c=0; c<facesCom.at(a).size(); c++){
              add.push_back(facesCom.at(a).at(c));
            }
            add.push_back(b);
            facesCom.push_back(add);
            for(unsigned int d=1; d<facesCom.at(facesCom.size()-1).size(); d++){
              sum+=W(facesCom.at(facesCom.size()-1).at(0),facesCom.at(facesCom.size()-1).at(d));
            }
            probsCom.push_back(sum);
          }
        }
      }

      while (probsCom.size()>0){
        float maxProb=0;
        int maxProbPos=0;
        for(unsigned int a=0; a<probsCom.size(); a++){
          if((probsCom.at(a)>maxProb) || (probsCom.at(a)==maxProb && facesCom.at(a).size()>facesCom.at(maxProbPos).size())){
            maxProb=probsCom.at(a);
            maxProbPos=a;
          }     
        }
        std::vector<int> faces;
        faces=facesCom.at(maxProbPos);
        blobs.push_back(faces);
        for(unsigned int a=0; a<facesCom.size(); a++){
          bool doubleF=false;
          for(unsigned int b=0; b<facesCom.at(a).size(); b++){  
            for(unsigned int c=0; c<faces.size(); c++){
              if(faces.at(c)==facesCom.at(a).at(b)){
                doubleF=true;
                break;
              }
            }
            if(doubleF==true) break;
          }
          if(doubleF==true){
            probsCom.erase(probsCom.begin()+a);
            facesCom.erase(facesCom.begin()+a);
            a--;
          }
        }
      }
      
      bool alrSet[W.cols()];
      for(unsigned int i=0; i<W.cols(); i++){
        alrSet[i]=false;
      }
      for(unsigned int i=0; i<blobs.size(); i++){
        for(unsigned int j=0; j<blobs.at(i).size(); j++){
          alrSet[blobs.at(i).at(j)]=true;
        }
      }
      for(unsigned int i=0; i<W.cols(); i++){
        if(alrSet[i]==false){
          std::vector<int> f;
          f.push_back(i);
          blobs.push_back(f);
        }
      }
      
      int comp[cluster.size()];
      
      for(unsigned int x=0; x<blobs.size(); x++){
        for(unsigned int y=0; y<blobs.at(x).size(); y++){
          comp[blobs.at(x).at(y)]=x+1;
        }
      }
      for(int i=0; i<w*h; i++){
        if(assignment[i]>0){
          assignment[i]=comp[assignment[i]-1];
        }
      }      
    }
    
    
    void Segmentation3D::calculateRemainingPoints(){          
      int numCluster=cluster.size();
      int currentClusterSize=cluster.size();
      for(int y=0;y<h;++y){
        for(int x=0;x<w;++x){
          int i = x+w*y;
          if(elements[i]==true){ 
	          elements[i]=false;
	          numCluster++;
	          assignmentRemaining[i]=numCluster;
	          std::vector<int> data;
	          data.push_back(i);
	          checkNeighbourDistanceRemaining(x,y,assignmentRemaining[i], &data);
            cluster.push_back(data);
          }
        }    			
      }
      for(unsigned int x=currentClusterSize; x<cluster.size(); x++){ //determine neighbours
        std::vector<int> nb;
        for(unsigned int y=0; y<cluster.at(x).size(); y++){
          if((int)cluster.at(x).at(y)-1>=0){
            if(checkNotExist(assignment[cluster.at(x).at(y)-1], nb) && dist3(xyzData[cluster.at(x).at(y)], xyzData[cluster.at(x).at(y)-1])<BLOBSeuclDistance){
              nb.push_back(assignment[cluster.at(x).at(y)-1]);
            }
          }
          if((int)cluster.at(x).at(y)+1<w*h){
            if(checkNotExist(assignment[cluster.at(x).at(y)+1], nb) && dist3(xyzData[cluster.at(x).at(y)], xyzData[cluster.at(x).at(y)+1])<BLOBSeuclDistance){
              nb.push_back(assignment[cluster.at(x).at(y)+1]);
            }
          }
          if((int)cluster.at(x).at(y)-w>=0){
            if(checkNotExist(assignment[cluster.at(x).at(y)-w], nb) && dist3(xyzData[cluster.at(x).at(y)], xyzData[cluster.at(x).at(y)-w])<BLOBSeuclDistance){
              nb.push_back(assignment[cluster.at(x).at(y)-w]);
            }
          }
          if((int)cluster.at(x).at(y)+w<w*h){
            if(checkNotExist(assignment[cluster.at(x).at(y)+w], nb) && dist3(xyzData[cluster.at(x).at(y)], xyzData[cluster.at(x).at(y)+w])<BLOBSeuclDistance){
              nb.push_back(assignment[cluster.at(x).at(y)+w]);
            }
          }
          if((int)cluster.at(x).at(y)+w-1<w*h){
            if(checkNotExist(assignment[cluster.at(x).at(y)+w-1], nb) && dist3(xyzData[cluster.at(x).at(y)], xyzData[cluster.at(x).at(y)+w-1])<BLOBSeuclDistance){
              nb.push_back(assignment[cluster.at(x).at(y)+w-1]);
            }
          }
          if((int)cluster.at(x).at(y)+w+1<w*h){
            if(checkNotExist(assignment[cluster.at(x).at(y)+w+1], nb) && dist3(xyzData[cluster.at(x).at(y)], xyzData[cluster.at(x).at(y)+w+1])<BLOBSeuclDistance){
              nb.push_back(assignment[cluster.at(x).at(y)+w+1]);
            }
          }
          if((int)cluster.at(x).at(y)-w-1>=0){
            if(checkNotExist(assignment[cluster.at(x).at(y)-w-1], nb) && dist3(xyzData[cluster.at(x).at(y)], xyzData[cluster.at(x).at(y)-w-1])<BLOBSeuclDistance){
              nb.push_back(assignment[cluster.at(x).at(y)-w-1]);
            }
          }
          if((int)cluster.at(x).at(y)-w+1>=0){
            if(checkNotExist(assignment[cluster.at(x).at(y)-w+1], nb) && dist3(xyzData[cluster.at(x).at(y)], xyzData[cluster.at(x).at(y)-w+1])<BLOBSeuclDistance){
              nb.push_back(assignment[cluster.at(x).at(y)-w+1]);
            }
          }
        }

        if(nb.size()==0){ //no neighbours -> new blob
          std::vector<int> blob;
          blob.push_back(x);
          blobs.push_back(blob);
          for(unsigned int i=0; i<cluster.at(x).size(); i++){
            assignment[cluster.at(x).at(i)]=blobs.size();
          }
        }
           
        else if(nb.size()==1 && cluster.at(x).size()<15){ //very small -> assign
          blobs.at(nb.at(0)-1).push_back(x);
          for(unsigned int p=0; p<cluster.at(x).size(); p++){
            assignment[cluster.at(x).at(p)]=nb.at(0);
          }
        }
                    
        else if(nb.size()==1){
          bool assignElement=false;
          int tol=0;
          for(unsigned int a=0; a<cluster.at(x).size(); a++){
            int yy = (int)floor((float)cluster.at(x).at(a)/(float)w);
            int xx = cluster.at(x).at(a)-yy*w;          
            if(cluster.at(x).at(a)<w+1 || cluster.at(x).at(a)>w*h-w-1){
            }
            else{ 
              if(assignment[cluster.at(x).at(a)-1]!=nb.at(0) && assignmentRemaining[cluster.at(x).at(a)-1]!=(int)x+1 && depthImage(xx-1,yy,0)!=2047){
                if(tol<9){
                  tol++;
                }else{
                  assignElement=true;
                  break;
                }
              }
              if(assignment[cluster.at(x).at(a)+1]!=nb.at(0) && assignmentRemaining[cluster.at(x).at(a)+1]!=(int)x+1 && depthImage(xx+1,yy,0)!=2047){
                if(tol<9){
                  tol++;
                }else{
                  assignElement=true;
                  break;
                }
              }
              if(assignment[cluster.at(x).at(a)-w]!=nb.at(0) && assignmentRemaining[cluster.at(x).at(a)-w]!=(int)x+1 && depthImage(xx,yy-1,0)!=2047){
                if(tol<9){
                  tol++;
                }else{
                  assignElement=true;
                  break;
                }
              }
              if(assignment[cluster.at(x).at(a)+w]!=nb.at(0) && assignmentRemaining[cluster.at(x).at(a)+w]!=(int)x+1 && depthImage(xx,yy+1,0)!=2047){
                if(tol<9){
                  tol++;
                }else{
                  assignElement=true;
                  break;
                }
              }
              if(assignment[cluster.at(x).at(a)-w-1]!=nb.at(0) && assignmentRemaining[cluster.at(x).at(a)-w-1]!=(int)x+1 && depthImage(xx-1,yy-1,0)!=2047){
                if(tol<9){
                  tol++;
                }else{
                  assignElement=true;
                  break;
                }
              }
              if(assignment[cluster.at(x).at(a)-w+1]!=nb.at(0) && assignmentRemaining[cluster.at(x).at(a)-w+1]!=(int)x+1 && depthImage(xx+1,yy-1,0)!=2047){
                if(tol<9){
                  tol++;
                }else{
                  assignElement=true;
                  break;
                }
              }
              if(assignment[cluster.at(x).at(a)+w-1]!=nb.at(0) && assignmentRemaining[cluster.at(x).at(a)+w-1]!=(int)x+1 && depthImage(xx-1,yy+1,0)!=2047){
                if(tol<9){
                  tol++;
                }else{
                  assignElement=true;
                  break;
                }
              }
              if(assignment[cluster.at(x).at(a)+w+1]!=nb.at(0) && assignmentRemaining[cluster.at(x).at(a)+w+1]!=(int)x+1 && depthImage(xx+1,yy+1,0)!=2047){
                if(tol<9){
                  tol++;
                }else{
                  assignElement=true;
                  break;
                }
              }
            
            }
          }
          if(assignElement==false){ //new blob
            std::vector<int> blob;
            blob.push_back(x);
            blobs.push_back(blob);
            for(unsigned int i=0; i<cluster.at(x).size(); i++){
              assignment[cluster.at(x).at(i)]=blobs.size();
            }                
          }
          else{ //assign
            blobs.at(nb.at(0)-1).push_back(x);
            for(unsigned int p=0; p<cluster.at(x).size(); p++){
              assignment[cluster.at(x).at(p)]=nb.at(0);
            }
          }
        }    
             
        else if(nb.size()>1){
          int zuws[nb.size()];
          for(unsigned int a=0; a<blobs.size(); a++){
            for(unsigned int b=0; b<blobs.at(a).size(); b++){
              for(unsigned int c=0; c<nb.size(); c++){
                if(blobs.at(a).at(b)==nb.at(c)-1){
                  zuws[c]=a;
                }
              }
            }
          }
          bool same=true;
          for(unsigned int a=1; a<nb.size(); a++){
            if(zuws[a]!=zuws[0]){
              same=false;
            }
          }
          if(same==true){ //same blob->assign
            blobs.at(nb.at(0)-1).push_back(x);
            for(unsigned int p=0; p<cluster.at(x).size(); p++){
              assignment[cluster.at(x).at(p)]=nb.at(0);
            }
          }
          else{ //different blob -> determine best match
            //ToDo: find better solution
            
            /*float zuwsSize[nb.size()];
            for(unsigned int a=0; a<nb.size(); a++){
              zuwsSize[a]=0.0;
              Vec rPoint1;
              Vec n01;		
              int p0i=cluster.at(nb.at(a)-1).at(rand()%cluster.at(nb.at(a)-1).size());
              int p1i=cluster.at(nb.at(a)-1).at(rand()%cluster.at(nb.at(a)-1).size());
              int p2i=cluster.at(nb.at(a)-1).at(rand()%cluster.at(nb.at(a)-1).size());
              Vec fa1;
              Vec fb1;
              Vec n1;
            	fa1 = xyzData[p1i]-xyzData[p0i];
            	fb1 = xyzData[p2i]-xyzData[p0i];
              n1[0]=fa1[1]*fb1[2]-fa1[2]*fb1[1];
              n1[1]=fa1[2]*fb1[0]-fa1[0]*fb1[2];
              n1[2]=fa1[0]*fb1[1]-fa1[1]*fb1[0];
              n01[0]=n1[0]/norm3(n1);
              n01[1]=n1[1]/norm3(n1);
             	n01[2]=n1[2]/norm3(n1);
          	 	rPoint1 = xyzData[p0i];
              float distance1 = rPoint1[0]*n01[0]+rPoint1[1]*n01[1]+ rPoint1[2]*n01[2];
              for(unsigned int p=0; p<cluster.at(x).size(); p++){
                float s1 = (xyzData[cluster.at(x).at(p)][0]*n01[0]+xyzData[cluster.at(x).at(p)][1]*n01[1]+xyzData[cluster.at(x).at(p)][2]*n01[2])-distance1;      
                zuwsSize[a]+=fabs(s1);
              }
            }
            int selection=0;
            for (unsigned int a=1; a<nb.size(); a++){
              if(zuwsSize[a]<zuwsSize[selection]){
                selection=a;
              }
            }
            int selectedBlob=0;
            for(unsigned int a=0; a<blobs.size(); a++){
              for(unsigned int b=0; b<blobs.at(a).size(); b++){
                if(blobs.at(a).at(b)==nb.at(selection)-1){
                  selectedBlob=a;
                }
              }
            }              
            blobs.at(nb.at(selection)-1).push_back(x);
            for(unsigned int p=0; p<cluster.at(x).size(); p++){
              assignment[cluster.at(x).at(p)]=nb.at(selection);
            }
          */
          }
        }
      } 
    }

    
    void Segmentation3D::blobSegmentation(){
      int maxID=0;
      unsigned int maxSize=0;
      for(unsigned int i=0; i<cluster.size(); i++){
        if(cluster.at(i).size()>maxSize){
          maxID=i;
          maxSize=cluster.at(i).size();
        }
      }
     
      int numPoints=cluster.at(maxID).size();
      
      Vec n0[RANSACpasses];
      float dist[RANSACpasses];  
      int cAbove[RANSACpasses];
      int cBelow[RANSACpasses];
      int cOn[RANSACpasses];
      int cAboveRead[RANSACpasses];
      int cBelowRead[RANSACpasses];
      int cOnRead[RANSACpasses];
      
      for(int i=0; i<RANSACpasses; i++){
        cAbove[i]=0;
        cBelow[i]=0;
        cOn[i]=0;   
        cAboveRead[i]=0;
        cBelowRead[i]=0;
        cOnRead[i]=0;
        
        Vec rPoint1;
        Vec n01;		
        int p0i=cluster.at(maxID).at(rand()%cluster.at(maxID).size());
        int p1i=cluster.at(maxID).at(rand()%cluster.at(maxID).size());
        int p2i=cluster.at(maxID).at(rand()%cluster.at(maxID).size());
        Vec fa1;
        Vec fb1;
        Vec n1;
      	fa1 = xyzData[p1i]-xyzData[p0i];
      	fb1 = xyzData[p2i]-xyzData[p0i];
        n1[0]=fa1[1]*fb1[2]-fa1[2]*fb1[1];
        n1[1]=fa1[2]*fb1[0]-fa1[0]*fb1[2];
        n1[2]=fa1[0]*fb1[1]-fa1[1]*fb1[0];
        n01[0]=n1[0]/norm3(n1);
        n01[1]=n1[1]/norm3(n1);
       	n01[2]=n1[2]/norm3(n1);
    	 	rPoint1 = xyzData[p0i];
        float distance1 = rPoint1[0]*n01[0]+rPoint1[1]*n01[1]+ rPoint1[2]*n01[2];
        dist[i]=distance1;
        n0[i]=n01;
      }
        
      if(useCL==true && clReady==true){
      #ifdef HAVE_OPENCL    
        try{         
          cl::Buffer RANSACpointsBuffer;
          cl_mem RANSACpointsMem = RANSACpointsBuffer();
          RANSACpointsBuffer = cl::Buffer(
                     context, 
                     CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                     numPoints * sizeof(int), 
                     (void *) &cluster.at(maxID)[0]);
                     
          cl::Event waitEvent;
          cl::Buffer n0Buffer;
          cl::Buffer distBuffer;
          cl::Buffer countAboveBuffer;
          cl::Buffer countBelowBuffer;
          cl::Buffer countOnBuffer;

          cl_mem n0Mem = n0Buffer();
          cl_mem distMem = distBuffer();
          cl_mem countAboveMem = countAboveBuffer();
          cl_mem countBelowMem = countBelowBuffer();
          cl_mem countOnMem = countOnBuffer();
          
          xyzBuffer = cl::Buffer(
				           context, 
				           CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
				           w*h * sizeof(cl_float4), 
        				   (void *) &xyzData[0]);
          
          n0Buffer = cl::Buffer(
                   context, 
                   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                   RANSACpasses * sizeof(cl_float4), 
                   (void *) &n0[0]);
          distBuffer = cl::Buffer(
                   context, 
                   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
                   RANSACpasses * sizeof(float), 
                   (void *) &dist[0]); 
          countAboveBuffer = cl::Buffer(
                   context, 
                   CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                   RANSACpasses * sizeof(int), 
                   (void *) &cAbove[0]);
          countBelowBuffer = cl::Buffer(
                   context, 
                   CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                   RANSACpasses * sizeof(int), 
                   (void *) &cBelow[0]);
          countOnBuffer = cl::Buffer(
                   context, 
                   CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
                   RANSACpasses * sizeof(int), 
                   (void *) &cOn[0]);

          kernelCheckRANSAC.setArg(0, xyzBuffer);//set parameter for kernel
          kernelCheckRANSAC.setArg(1, RANSACpasses);
          kernelCheckRANSAC.setArg(2, n0Buffer);
          kernelCheckRANSAC.setArg(3, distBuffer);
          kernelCheckRANSAC.setArg(4, countAboveBuffer);
          kernelCheckRANSAC.setArg(5, countBelowBuffer);
          kernelCheckRANSAC.setArg(6, countOnBuffer);
          kernelCheckRANSAC.setArg(7, RANSACeuclDistance/2);
          kernelCheckRANSAC.setArg(8, numPoints);
          kernelCheckRANSAC.setArg(9, RANSACpointsBuffer);
          kernelCheckRANSAC.setArg(10, RANSACsubset);
                        
          int idSize=RANSACpasses*numPoints/RANSACsubset;
          queue.enqueueNDRangeKernel(//run kernel
             kernelCheckRANSAC, 
             cl::NullRange, 
             cl::NDRange(idSize), //input size for get global id
             cl::NullRange,
             NULL,
             &waitEvent);
    	                	    
          queue.enqueueReadBuffer(//read output from kernel
              countAboveBuffer,
              CL_TRUE, // block 
              0,
              RANSACpasses * sizeof(int),
              (int*) cAboveRead,
              NULL,&waitEvent);
                         
          queue.enqueueReadBuffer(//read output from kernel
              countBelowBuffer,
              CL_TRUE, // block 
              0,
              RANSACpasses * sizeof(int),
              (int*) cBelowRead,
              NULL,&waitEvent);
                      
          queue.enqueueReadBuffer(//read output from kernel
              countOnBuffer,
              CL_TRUE, // block 
              0,
              RANSACpasses * sizeof(int),
              (int*) cOnRead,
              NULL,&waitEvent);
           
          clFinish(queue()); 
            
          clReleaseMemObject(n0Mem);
          clReleaseMemObject(distMem);
          clReleaseMemObject(countAboveMem);
          clReleaseMemObject(countBelowMem);
          clReleaseMemObject(countOnMem);
    
          clReleaseMemObject(RANSACpointsMem);  
        
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }       
      #endif
      }
      else{             
        for(int p=0; p<RANSACpasses; p++){
          for(unsigned int q=0; q<cluster.at(maxID).size(); q++){
            Vec n01 = n0[p];
            float s1 = (xyzData[cluster.at(maxID).at(q)][0]*n01[0]+xyzData[cluster.at(maxID).at(q)][1]*n01[1]+xyzData[cluster.at(maxID).at(q)][2]*n01[2])-dist[p];      
            if((s1>=-RANSACeuclDistance/2 && s1<=RANSACeuclDistance/2)){
              cOnRead[p]++;
            }  
          }
        }
      }
      int maxMatch=0;
      int maxMatchID=0;
      for(int i=0;i<RANSACpasses; i++){
        if(cOnRead[i]>maxMatch){
          maxMatch=cOnRead[i];
          maxMatchID=i;
        }
      }
	    
	    if(useCL==true && clReady==true){
      #ifdef HAVE_OPENCL
        try{           
          elementsBlobsBuffer = cl::Buffer(
				           context, 
				           CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 
				           w*h * sizeof(bool), 
				           (void *) &elementsBlobs[0]);
				  assignmentBlobsBuffer = cl::Buffer(
				           context, 
				           CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
				           w*h * sizeof(int), 
				           (void *) &assignmentBlobs[0]);
				           
				  assignmentBuffer = cl::Buffer(
				           context, 
				           CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
				           w*h * sizeof(int), 
				           (void *) &assignment[0]);
                  
          kernelAssignRANSAC.setArg(0, xyzBuffer);//set parameter for kernel
          kernelAssignRANSAC.setArg(1, elementsBlobsBuffer);
          kernelAssignRANSAC.setArg(2, assignmentBlobsBuffer);
          kernelAssignRANSAC.setArg(3, n0[maxMatchID]);
          kernelAssignRANSAC.setArg(4, dist[maxMatchID]);
          kernelAssignRANSAC.setArg(5, RANSACeuclDistance);
          kernelAssignRANSAC.setArg(6, assignmentBuffer);
          kernelAssignRANSAC.setArg(7, maxID+1);
          
         
          queue.enqueueNDRangeKernel(//run kernel
				     kernelAssignRANSAC, 
				     cl::NullRange, 
				     cl::NDRange(w*h), //input size for get global id
				     cl::NullRange);
    	    			          
		      queue.enqueueReadBuffer(//read output from kernel
              assignmentBlobsBuffer,
              CL_TRUE, // block 
              0,
              w*h * sizeof(int),
              (int*) assignmentBlobs);
          
          queue.enqueueReadBuffer(//read output from kernel
              elementsBlobsBuffer,
              CL_TRUE, // block 
              0,
              w*h * sizeof(bool),
              (bool*) elementsBlobs);
        
        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }       
      #endif
      }
      else{                  
        for(int i=0; i<w*h; i++){
          if(assignment[i]==maxID+1){
            assignmentBlobs[i]=1;
            elementsBlobs[i]=false;
          }else{
            Vec n01 = n0[maxMatchID];
            float s1 = (xyzData[i][0]*n01[0]+xyzData[i][1]*n01[1]+xyzData[i][2]*n01[2])-dist[maxMatchID];  
            if((s1>=-RANSACeuclDistance && s1<=RANSACeuclDistance) && elementsBlobs[i]==true){
              assignmentBlobs[i]=1;
              elementsBlobs[i]=false;
            }
          }
        }    
      }
      	    
      regionGrowBlobs();    
      assignment=assignmentBlobs;
    }
    
    
    void Segmentation3D::colorPointcloud(){
  	  if(useCL==true && clReady==true){
      #ifdef HAVE_OPENCL
        try{  
          assignmentBuffer = cl::Buffer(
				         context, 
				         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 
				         w*h * sizeof(int), 
				         (void *) &assignment[0]);
				           
          kernelSegmentColoring.setArg(0, assignmentBuffer);//set parameter for kernel
          kernelSegmentColoring.setArg(1, segmentColorImageRBuffer);
          kernelSegmentColoring.setArg(2, segmentColorImageGBuffer);
          kernelSegmentColoring.setArg(3, segmentColorImageBBuffer);
          
          queue.enqueueNDRangeKernel(//run kernel
				     kernelSegmentColoring, 
				     cl::NullRange, 
				     cl::NDRange(w*h), //input size for get global id
				     cl::NullRange);
				     
          queue.enqueueReadBuffer(//read output from kernel
              segmentColorImageRBuffer,
              CL_TRUE, // block 
              0,
              w*h * sizeof(cl_uchar),
              (cl_uchar*) segmentColorImageRArray);
              
          queue.enqueueReadBuffer(//read output from kernel
              segmentColorImageGBuffer,
              CL_TRUE, // block 
              0,
              w*h * sizeof(cl_uchar),
              (cl_uchar*) segmentColorImageGArray);
              
          queue.enqueueReadBuffer(//read output from kernel
              segmentColorImageBBuffer,
              CL_TRUE, // block 
              0,
              w*h * sizeof(cl_uchar),
              (cl_uchar*) segmentColorImageBArray);
              
          std::vector<icl8u*> data(3);
          data[0] = segmentColorImageRArray; 
          data[1] = segmentColorImageGArray;
          data[2] = segmentColorImageBArray; 
          segmentColorImage = Img8u(Size(w,h),3,data,false);

        }catch (cl::Error err) {//catch openCL errors
          std::cout<< "ERROR: "<< err.what()<< "("<< err.err()<< ")"<<std::endl;
        }
      #endif
  	  }else{
        for(int y=0; y<h; y++){
          for(int x=0; x<w; x++){
            int i=x+w*y;
            if(assignment[i]==0){
              segmentColorImage(x,y,0)=128;
              segmentColorImage(x,y,1)=128;
              segmentColorImage(x,y,2)=128;
            }else{            
              int H=(int)(assignment[i]*35.)%360;
              float S=1.0-assignment[i]*0.01;
              float hi=floor((float)H/60.);
			        float f=((float)H/60.)-hi;
			        float pp=1.0-S;
			        float qq=1.0-S*f;
			        float tt=1.0-S*(1.-f);
			        float newR=0;
			        float newG=0;
			        float newB=0;
			        if((int)hi==0 || (int)hi==6){
			          newR=1.0;
			          newG=tt;
			          newB=pp;
			        }else if((int)hi==1){
			          newR=qq;
			          newG=1.0;
			          newB=pp;
			        }else if((int)hi==2){
			          newR=pp;
			          newG=1.0;
			          newB=tt;
			        }else if((int)hi==3){
			          newR=pp;
			          newG=qq;
			          newB=1.0;
			        }else if((int)hi==4){
			          newR=tt;
			          newG=pp;
			          newB=1.0;
			        }else if((int)hi==5){
			          newR=1.0;
			          newG=pp;
			          newB=qq;
			        }
	            segmentColorImage(x,y,0)=(unsigned char)(newR*255.);
	            segmentColorImage(x,y,1)=(unsigned char)(newG*255.);
	            segmentColorImage(x,y,2)=(unsigned char)(newB*255.);
	          }
	        } 
        }
      }
    }
    
    
    	    
	  void Segmentation3D::checkNeighbourGrayThreshold(int x, int y, int zuw, int threshold, std::vector<int> *data){
	    std::vector<int> toProcessX;
	    std::vector<int> toProcessY;
	    bool process=true;
	    unsigned int index=0;
		  toProcessX.push_back(x);
		  toProcessY.push_back(y);
		
	    while(process==true){
	      int i = toProcessX.at(index)+w*toProcessY.at(index);
	      x = toProcessX.at(index);
	      y = toProcessY.at(index);
		     
		    if(elements[i+1]==true && x+1<w && normalEdgeImage(x+1,y,0)==threshold){
			    elements[i+1]=false;
			    assignment[i+1]=zuw;
			    toProcessX.push_back(toProcessX.at(index)+1);
			    toProcessY.push_back(toProcessY.at(index));
			    data->push_back(i+1);
		    }
		    if(elements[i-1]==true && x-1>=0 && normalEdgeImage(x-1,y,0)==threshold){
			    elements[i-1]=false;
			    assignment[i-1]=zuw;
			    toProcessX.push_back(toProcessX.at(index)-1);
			    toProcessY.push_back(toProcessY.at(index));
			    data->push_back(i-1);
		    }
		    if(elements[i+w]==true && y+1<h && normalEdgeImage(x,y+1,0)==threshold){
			    elements[i+w]=false;
			    assignment[i+w]=zuw;
			    toProcessX.push_back(toProcessX.at(index));
			    toProcessY.push_back(toProcessY.at(index)+1);
			    data->push_back(i+w);
		    }
		    if(elements[i+w-1]==true && x-1>=0 && y+1<h && normalEdgeImage(x-1,y+1,0)==threshold){
			    elements[i+w-1]=false;
			    assignment[i+w-1]=zuw;
			    toProcessX.push_back(toProcessX.at(index)-1);
			    toProcessY.push_back(toProcessY.at(index)+1);
			    data->push_back(i+w-1);
		    }
		    if(elements[i+w+1]==true && x+1<w && y+1<h && normalEdgeImage(x+1,y+1,0)==threshold){
			    elements[i+w+1]=false;
			    assignment[i+w+1]=zuw;
			    toProcessX.push_back(toProcessX.at(index)+1);
			    toProcessY.push_back(toProcessY.at(index)+1);
			    data->push_back(i+w+1);
		    }
		    if(elements[i-w]==true && y-1>=0 && normalEdgeImage(x,y-1,0)==threshold){
			    elements[i-w]=false;
			    assignment[i-w]=zuw;
			    toProcessX.push_back(toProcessX.at(index));
			    toProcessY.push_back(toProcessY.at(index)-1);
			    data->push_back(i-w);
		    }
		    if(elements[i-w-1]==true && x-1>=0 && y-1>=0 && normalEdgeImage(x-1,y-1,0)==threshold){
			    elements[i-w-1]=false;
			    assignment[i-w-1]=zuw;
			    toProcessX.push_back(toProcessX.at(index)-1);
			    toProcessY.push_back(toProcessY.at(index)-1);
			    data->push_back(i-w-1);
		    }
		    if(elements[i-w+1]==true && x+1<w && y-1>=0 && normalEdgeImage(x+1,y-1,0)==threshold){
			    elements[i-w+1]=false;
			    assignment[i-w+1]=zuw;
			    toProcessX.push_back(toProcessX.at(index)+1);
			    toProcessY.push_back(toProcessY.at(index)-1);
			    data->push_back(i-w+1);
		    } 
		      
        index++;
        if(index>=toProcessX.size()){
          process=false;
        }
      }
	  }
	   
    
    void Segmentation3D::checkNeighbourDistanceRemaining(int x, int y, int zuw, std::vector<int> *data){
	    std::vector<int> toProcessX;
	    std::vector<int> toProcessY;
	    bool process=true;
	    unsigned int index=0;
		  toProcessX.push_back(x);
		  toProcessY.push_back(y);
		
	    while(process==true){
	      int i = toProcessX.at(index)+w*toProcessY.at(index);
	      x = toProcessX.at(index);
	      y = toProcessY.at(index);
	       
		    if(elements[i+1]==true && x+1<w && dist3(xyzData[i],xyzData[i+1])<BLOBSeuclDistance){
			    elements[i+1]=false;
			    assignmentRemaining[i+1]=zuw;
			    toProcessX.push_back(toProcessX.at(index)+1);
			    toProcessY.push_back(toProcessY.at(index));
			    data->push_back(i+1);
		    }
		    if(elements[i-1]==true && x-1>=0 && dist3(xyzData[i],xyzData[i-1])<BLOBSeuclDistance){
			    elements[i-1]=false;
			    assignmentRemaining[i-1]=zuw;
			    toProcessX.push_back(toProcessX.at(index)-1);
			    toProcessY.push_back(toProcessY.at(index));
			    data->push_back(i-1);
		    }
		    if(elements[i+w]==true && y+1<h && dist3(xyzData[i],xyzData[i+w])<BLOBSeuclDistance){
			    elements[i+w]=false;
			    assignmentRemaining[i+w]=zuw;
			    toProcessX.push_back(toProcessX.at(index));
			    toProcessY.push_back(toProcessY.at(index)+1);
			    data->push_back(i+w);
		    }
		    if(elements[i+w-1]==true && x-1>=0 && y+1<h && dist3(xyzData[i],xyzData[i+w-1])<BLOBSeuclDistance){
			    elements[i+w-1]=false;
			    assignmentRemaining[i+w-1]=zuw;
			    toProcessX.push_back(toProcessX.at(index)-1);
			    toProcessY.push_back(toProcessY.at(index)+1);
			    data->push_back(i+w-1);
		    }
		    if(elements[i+w+1]==true && x+1<w && y+1<h && dist3(xyzData[i],xyzData[i+w+1])<BLOBSeuclDistance){
			    elements[i+w+1]=false;
			    assignmentRemaining[i+w+1]=zuw;
			    toProcessX.push_back(toProcessX.at(index)+1);
			    toProcessY.push_back(toProcessY.at(index)+1);
			    data->push_back(i+w+1);
		    }
		    if(elements[i-w]==true && y-1>=0 && dist3(xyzData[i],xyzData[i-w])<BLOBSeuclDistance){
			    elements[i-w]=false;
			    assignmentRemaining[i-w]=zuw;
			    toProcessX.push_back(toProcessX.at(index));
			    toProcessY.push_back(toProcessY.at(index)-1);
			    data->push_back(i-w);
		    }
		    if(elements[i-w-1]==true && x-1>=0 && y-1>=0 && dist3(xyzData[i],xyzData[i-w-1])<BLOBSeuclDistance){
			    elements[i-w-1]=false;
			    assignmentRemaining[i-w-1]=zuw;
			    toProcessX.push_back(toProcessX.at(index)-1);
			    toProcessY.push_back(toProcessY.at(index)-1);
			    data->push_back(i-w-1);
		    }
		    if(elements[i-w+1]==true && x+1<w && y-1>=0 && dist3(xyzData[i],xyzData[i-w+1])<BLOBSeuclDistance){
			    elements[i-w+1]=false;
			    assignmentRemaining[i-w+1]=zuw;
			    toProcessX.push_back(toProcessX.at(index)+1);
			    toProcessY.push_back(toProcessY.at(index)-1);
			    data->push_back(i-w+1);
		    } 
		      
        index++;
        if(index>=toProcessX.size()){
          process=false;
        }
      }
	  }


    void Segmentation3D::regionGrowBlobs(){     
      int numCluster=1;
      std::vector<std::vector<int> > remove;
      for(int y=0;y<h;++y){
	      for(int x=0;x<w;++x){
		      int i = x+w*y;
		      if(elementsBlobs[i]==true){
			      elementsBlobs[i]=false;
			      numCluster++;
			      assignmentBlobs[i]=numCluster;
			      std::vector<int> data;
			      data.push_back(i);
			      checkNeighbourDistance(x,y,assignmentBlobs[i], &data);
			      if(data.size()<minClusterSize){
			        remove.push_back(data);
			        numCluster--;
			      }
		      }
        }    			
      }
      for(unsigned int i=0; i<remove.size(); i++){
        for(unsigned int j=0; j<remove.at(i).size(); j++){
          elementsBlobs[remove.at(i).at(j)]=true;
          assignmentBlobs[remove.at(i).at(j)]=0;
        }
      }
    }


    void Segmentation3D::checkNeighbourDistance(int x, int y, int zuw, std::vector<int> *data){
	    std::vector<int> toProcessX;
	    std::vector<int> toProcessY;
	    bool process=true;
	    unsigned int index=0;
		  toProcessX.push_back(x);
		  toProcessY.push_back(y);
		
	    while(process==true){
	      int i = toProcessX.at(index)+w*toProcessY.at(index);
	      x = toProcessX.at(index);
	      y = toProcessY.at(index);
	       
		    if(elementsBlobs[i+1]==true && x+1<w && fabs(depthImage(x,y,0)-depthImage(x+1,y,0))<BLOBSeuclDistance){
			    elementsBlobs[i+1]=false;
			    assignmentBlobs[i+1]=zuw;
			    toProcessX.push_back(toProcessX.at(index)+1);
			    toProcessY.push_back(toProcessY.at(index));
			    data->push_back(i+1);
		    }
		    if(elementsBlobs[i-1]==true && x-1>=0 && fabs(depthImage(x,y,0)-depthImage(x-1,y,0))<BLOBSeuclDistance){
			    elementsBlobs[i-1]=false;
			    assignmentBlobs[i-1]=zuw;
			    toProcessX.push_back(toProcessX.at(index)-1);
			    toProcessY.push_back(toProcessY.at(index));
			    data->push_back(i-1);
		    }
		    if(elementsBlobs[i+w]==true && y+1<h && fabs(depthImage(x,y,0)-depthImage(x,y+1,0))<BLOBSeuclDistance){
			    elementsBlobs[i+w]=false;
			    assignmentBlobs[i+w]=zuw;
			    toProcessX.push_back(toProcessX.at(index));
			    toProcessY.push_back(toProcessY.at(index)+1);
			    data->push_back(i+w);
		    }
		    if(elementsBlobs[i+w-1]==true && x-1>=0 && y+1<h && fabs(depthImage(x,y,0)-depthImage(x-1,y+1,0))<BLOBSeuclDistance){
			    elementsBlobs[i+w-1]=false;
			    assignmentBlobs[i+w-1]=zuw;
			    toProcessX.push_back(toProcessX.at(index)-1);
			    toProcessY.push_back(toProcessY.at(index)+1);
			    data->push_back(i+w-1);
		    }
		    if(elementsBlobs[i+w+1]==true && x+1<w && y+1<h && fabs(depthImage(x,y,0)-depthImage(x+1,y+1,0))<BLOBSeuclDistance){
			    elementsBlobs[i+w+1]=false;
			    assignmentBlobs[i+w+1]=zuw;
			    toProcessX.push_back(toProcessX.at(index)+1);
			    toProcessY.push_back(toProcessY.at(index)+1);
			    data->push_back(i+w+1);
		    }
		    if(elementsBlobs[i-w]==true && y-1>=0 && fabs(depthImage(x,y,0)-depthImage(x,y-1,0))<BLOBSeuclDistance){
			    elementsBlobs[i-w]=false;
			    assignmentBlobs[i-w]=zuw;
			    toProcessX.push_back(toProcessX.at(index));
			    toProcessY.push_back(toProcessY.at(index)-1);
			    data->push_back(i-w);
		    }
		    if(elementsBlobs[i-w-1]==true && x-1>=0 && y-1>=0 && fabs(depthImage(x,y,0)-depthImage(x-1,y-1,0))<BLOBSeuclDistance){
			    elementsBlobs[i-w-1]=false;
			    assignmentBlobs[i-w-1]=zuw;
			    toProcessX.push_back(toProcessX.at(index)-1);
			    toProcessY.push_back(toProcessY.at(index)-1);
			    data->push_back(i-w-1);
		    }
		    if(elementsBlobs[i-w+1]==true && x+1<w && y-1>=0 && fabs(depthImage(x,y,0)-depthImage(x+1,y-1,0))<BLOBSeuclDistance){
			    elementsBlobs[i-w+1]=false;
			    assignmentBlobs[i-w+1]=zuw;
			    toProcessX.push_back(toProcessX.at(index)+1);
			    toProcessY.push_back(toProcessY.at(index)-1);
			    data->push_back(i-w+1);
		    } 
		      
        index++;
        if(index>=toProcessX.size()){
          process=false;
        }
      }
	  }
 

    bool Segmentation3D::checkNotExist(int zw, std::vector<int> &nb){   
      if(zw!=0){
        for(unsigned int z=0; z<nb.size(); z++){
          if(nb.at(z)==zw){
            return false;
          }
        }
        return true;
      }
      return false;
    }


    float Segmentation3D::dist3(const Vec &a, const Vec &b){
      return norm3(a-b);
	  }
 
  } // namespace geom
}
