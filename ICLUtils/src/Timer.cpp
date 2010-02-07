#include <ICLUtils/Timer.h>
#include <ICLUtils/Time.h>
/*
  Timer.cpp

  Written by: Michael G�tting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/


using namespace std;

//---- ICL in its own namespace ----
namespace icl {

// {{{ Konstruktor/ Destruktor

Timer::Timer(int iTimerMode)
{
  FUNCTION_LOG("Timer mode: " << iTimerMode);
  m_iTimerMode = iTimerMode;

  m_vecTime.clear();
  m_vecTimerName.clear();
}

// }}}

// {{{ Timer functions

void Timer::startTimer()
{
  FUNCTION_LOG("string");
  m_vecTime.push_back(getTime());
  m_vecTimerName.push_back("_START_");
}

void Timer::stopSubTimer(string sName)
{
  FUNCTION_LOG("");
  m_vecTime.push_back(getTime());
  m_vecTimerName.push_back(sName);  
}

void Timer::stopTimer(string sName)
{
  FUNCTION_LOG("string");

  long int lTmpTimeDiff = 0;
  
  m_vecTime.push_back(getTime());
  m_vecTimerName.push_back(sName);  

  cout << endl;
  cout << " ------------------------------------------- " << endl;
  cout << " --             Time measure              -- " << endl;
  
  if (m_vecTime.size() > 2) {
    for (unsigned int i=1;i<m_vecTime.size();i++)
    {
      lTmpTimeDiff = (long int)(m_vecTime[i] - m_vecTime[i-1]);
      
      switch(m_iTimerMode)
      {
        case 0:
          cout << " --  [" << m_vecTimerName[i] << "] -> Time: " 
               << lTmpTimeDiff << " ms" << endl;
          break;
          
        case 1:
          cout << " --  [" << m_vecTimerName[i] << "] -> Time: "
               << lTmpTimeDiff << " �s" << endl;
          break;
      }
    }    
  }
  
  lTmpTimeDiff = (long int)(m_vecTime[m_vecTime.size()-1] - m_vecTime[0]);
  
  switch(m_iTimerMode)
  {
    case 0:
      cout << " ------------------------------------------- " << endl;
      cout << " [ --- ] -> Complete time: " << lTmpTimeDiff << " ms" << endl;
      break;
      
    case 1:
      cout << " ------------------------------------------- " << endl;
      cout << " [ --- ] -> Complete time: " << lTmpTimeDiff << " ns" << endl;
      break;
  }
  cout << endl;
}

long int Timer::stopSilent(){
  return  (long int)(getTime() - m_vecTime[0]);
}

// }}}

// {{{ Misc. functions

Time::value_type Timer::getTime()
{
  FUNCTION_LOG("");
  
  switch (m_iTimerMode)
  {
    case 0: //ms
       return Time::now().toMilliSeconds();
      
    default: //�s
       return Time::now().toMicroSeconds();
  }
}

// }}}

} //namespace