/************************************************************/
/*    NAME: Arjun Gupta                                     */
/*    ORGN: MIT                                             */
/*    FILE: BHV_Pulse.h                                     */
/*    DATE:                                                 */
/************************************************************/

#ifndef ZIG_HEADER
#define ZIG_HEADER

#include <string>
#include "IvPBehavior.h"

class BHV_ZigLeg : public IvPBehavior {   
public:
  BHV_ZigLeg(IvPDomain);
  ~BHV_ZigLeg() {};
  
  bool         setParam(std::string, std::string);
  void         onSetParamComplete();
  void         onCompleteState();
  void         onIdleState();
  void         onHelmStart();
  void         postConfigStatus();
  void         onRunToIdleState();
  void         onIdleToRunState();
  IvPFunction* buildFunctionWithZAIC();
  IvPFunction* onRunState();

protected: // Local Utility functions

protected: // Configuration parameters
  double m_angle;
  double m_duration;
  double m_last_index;
  double m_index_time;
  double m_heading;
  
protected: // State variables
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain) 
  {return new BHV_ZigLeg(domain);}
}
#endif
