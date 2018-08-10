/************************************************************/
/*    NAME: Arjun Gupta                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_Input.h                                      */
/*    DATE:                                                 */
/************************************************************/

#ifndef Input_HEADER
#define Input_HEADER

#include <string>
#include "IvPBehavior.h"
#include <unordered_map>
#include <iostream>
#include <vector>
#include "NodeReportDefense.h"
#include "python_caller.h"

class BHV_Input : public IvPBehavior {
public:
  BHV_Input(IvPDomain);
  ~BHV_Input() {};
  
  bool         setParam(std::string, std::string);
  string       setup_interpreter();
  string       process_file();
  string       up(std::string);
  string       low(std::string);
  void         onSetParamComplete();
  void         onCompleteState();
  void         onIdleState();
  void         onHelmStart();
  void         postConfigStatus();
  void         onRunToIdleState();
  void         onIdleToRunState();
  void         getVehicleData();
  IvPFunction* onRunState();
  IvPFunction* buildFunctionWithZAIC();
  void         make_action(std::vector<std::string>);
  string       make_state();
  string       setup_python();
  double       process_tagged();
  double       process_flag_captured();
  double       process_dist(double, double, int, double, double);
  double       process_angle(double, double, int, std::string, double, double);
  double       processBound(double, double, std::string);
  void         showPoint(string, string, double, double);

protected: 
  std::unordered_map<std::string, std::string> m_table;
  std::unordered_map<std::string, int> m_state_indeces;
  std::unordered_map<std::string, std::vector<string> > m_state_params;
  std::unordered_map<std::string, NodeReport> m_player_map;
  std::vector<std::string> m_tagged_names;
  std::vector<std::string> m_sVal;
  std::vector<PyObject*> m_pVal;
  std::vector<std::vector<std::string> > m_vVal;
  double m_heading_des;
  double m_heading_abs;
  double m_speed;
  double m_gridsize;
  double m_osX;
  double m_osY;
  double m_bucket_size;
  double m_theta_size;
  int  m_blue_flag[2];
  int  m_red_flag[2];
  int  m_state_size;
  std::string m_infile;
  std::string m_outfile;
  std::string m_blue_cap;
  std::string m_red_cap;
  std::string m_name;
  std::string m_team;
  std::string m_flag;
  std::string m_action;
  std::string m_epsilon;
  bool m_tagged;
  bool m_relative;
  bool m_optimal;
  bool m_module_loaded;
  PyObject *pModelDict;
  PyObject *pResult;
  PythonCaller Py;
};

#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain) 
  {return new BHV_Input(domain);}
}
#endif
