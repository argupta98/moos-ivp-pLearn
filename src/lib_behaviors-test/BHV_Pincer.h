/************************************************************/
/*    NAME: Arjun Gupta                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_Pincer.h                                      */
/*    DATE:                                                 */
/************************************************************/

#ifndef Pincer_HEADER
#define Pincer_HEADER

#include <string>
#include "IvPBehavior.h"
#include "NodeReport.h"
#include <vector>

class BHV_Pincer : public IvPBehavior {
public:
  BHV_Pincer(IvPDomain);
  ~BHV_Pincer() {};
  
  bool         setParam(std::string, std::string);
  void         onSetParamComplete();
  void         onCompleteState();
  void         onIdleState();
  void         onHelmStart();
  void         postConfigStatus();
  void         onRunToIdleState();
  void         onIdleToRunState();
  IvPFunction* onRunState();

protected: // Local Utility functions
  std::vector<NodeReport> m_adversaries;
  std::vector<NodeReport> m_allies;
  int critical_point;
  std::string m_group;
  
protected: // Configuration parameters

protected: // State variables
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain) 
  {return new BHV_Pincer(domain);}
}
#endif
