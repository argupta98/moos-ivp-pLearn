/************************************************************/
/*    NAME: Arjun Gupta                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_FrontEstimate.h                                      */
/*    DATE:                                                 */
/************************************************************/

#ifndef FrontEstimate_HEADER
#define FrontEstimate_HEADER

#include <string>
#include "IvPBehavior.h"
#include <vector>

class BHV_CollabEstimate : public IvPBehavior {
public:
  BHV_CollabEstimate(IvPDomain);
  ~BHV_CollabEstimate() {};
  
  bool         setParam(std::string, std::string);
  void         onSetParamComplete();
  void         onCompleteState();
  void         onIdleState();
  void         onHelmStart();
  void         postConfigStatus();
  void         onRunToIdleState();
  void         onIdleToRunState();
  IvPFunction* onRunState();
  IvPFunction* buildFunctionWithZAIC();

protected: // Local Utility functions
  bool m_detected_front;
  bool m_found_extrema;
  bool m_front_running;
  bool m_turn_around;
  double  m_heading;
  double  m_desired_speed;
  vector<double> temp_list;
  double m_last_gradient;
  int m_turn_delay;
  int m_down_delay;
  string m_hostname;
  string m_share;
  string m_last_info;
  string m_last_part_info;
  bool   m_left;
  
protected: // Configuration parameters

protected: // State variables
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain) 
  {return new BHV_CollabEstimate(domain);}
}
#endif
