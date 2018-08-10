/************************************************************/
/*    NAME: Arjun Gupta                                     */
/*    ORGN: MIT                                             */
/*    FILE: BHV_FrontEstimate.cpp                           */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_FrontOscillate.h"
#include "ZAIC_PEAK.h"
#include "OF_Coupler.h"
#include <vector>

using namespace std;
double GRAD_LOW = .5;
double GRAD_HIGH = 1.7;
//---------------------------------------------------------------
// Constructor

BHV_FrontOscillate::BHV_FrontOscillate(IvPDomain domain) :
  IvPBehavior(domain)
{
  m_last_gradient = 1;
  m_count = 81;
  m_turn_around=false;
  // Provide a default behavior name
  IvPBehavior::setParam("name", "front_estimate");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");

  // Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y, UCTD_MSMNT_REPORT, OPREG_TRAJECTORY_PERIM_DIST");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_FrontOscillate::setParam(string param, string val)
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());
  
  if((param == "speed") && isNumber(val)) {
    m_desired_speed = double_val;
    return(true);
  }
  else if (param == "bar") {
    // return(setBooleanOnString(m_my_bool, val));
  }

  // If not handled above, then just return false;
  return(false);
}

//---------------------------------------------------------------
// Procedure: onSetParamComplete()
//   Purpose: Invoked once after all parameters have been handled.
//            Good place to ensure all required params have are set.
//            Or any inter-param relationships like a<b.

void BHV_FrontOscillate::onSetParamComplete()
{
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_FrontOscillate::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_FrontOscillate::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_FrontOscillate::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_FrontOscillate::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_FrontOscillate::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_FrontOscillate::onRunToIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_FrontOscillate::onRunState()
{
  // Part 1: Build the IvP function
  IvPFunction *ipf = 0;
  //get info from uFldCTDSensor
  bool ok1, ok2;
  string info= getBufferStringVal("UCTD_MSMNT_REPORT", ok1);
  double distance = getBufferDoubleVal("OPREG_TRAJECTORY_PERIM_DIST", ok2);
  vector<string> parsed_temp = parseString(info, ',');
  string temp_str= parsed_temp[parsed_temp.size()-1];
  biteStringX(temp_str, '=');
  double temp = stod(temp_str);
  if(temp_list.size()==0){
    temp_list.push_back(temp);
  }
  //if we have detected a high gradient already, but not found the max interval range yet keep going down until we find that the change is temperature between consecutive readings is trivial
  m_count++;
  //check if we are near a perimeter
  if(distance<25 && m_detected_front){
    m_desired_speed = 1;
    m_turn_around = !m_turn_around;
  }
  else{
    m_desired_speed = 2;
  }
    int last_temp = temp_list[temp_list.size()-1];
    m_count++;
    if(m_count==50){
      //m_detected_front=false;
	m_turn_around=!m_turn_around;
    }
      if(m_turn_around)
	m_heading = 0;
      else
	m_heading = 180;
      if(((temp-last_temp)> GRAD_HIGH || -1*(temp-last_temp)> GRAD_HIGH) &&  m_count>50){
      m_detected_front =true;
      //m_turn_around=!m_turn_around;
      m_count = 0;
    }
  m_last_gradient = temp-last_temp;
  temp_list.push_back(temp);
  postMessage("STAT", "added " + to_string(temp) + " to message list");
  
  //follow a course in the theta heading until you reach the end and then wrap around and do that again. 

  // generate the IvP function
  ipf=buildFunctionWithZAIC();
  // Part N: Prior to returning the IvP function, apply the priority wt
  // Actual weight applied may be some value different than the configured
  // m_priority_wt, depending on the behavior author's insite.
  if(ipf)
    ipf->setPWT(m_priority_wt);
  return(ipf);
}


IvPFunction *BHV_FrontOscillate::buildFunctionWithZAIC() 
{
  ZAIC_PEAK spd_zaic(m_domain, "speed");
  spd_zaic.setSummit(m_desired_speed);
  spd_zaic.setPeakWidth(0.5);
  spd_zaic.setBaseWidth(1.0);
  spd_zaic.setSummitDelta(0.8);  
  if(spd_zaic.stateOK() == false) {
    string warnings = "Speed ZAIC problems " + spd_zaic.getWarnings();
    postWMessage(warnings);
    return(0);
  }
  
  ZAIC_PEAK crs_zaic(m_domain, "course");
  crs_zaic.setSummit(m_heading);
  crs_zaic.setPeakWidth(0);
  crs_zaic.setBaseWidth(180.0);
  crs_zaic.setSummitDelta(0);  
  crs_zaic.setValueWrap(true);
  if(crs_zaic.stateOK() == false) {
    string warnings = "Course ZAIC problems " + crs_zaic.getWarnings();
    postWMessage(warnings);
    return(0);
  }

  IvPFunction *spd_ipf = spd_zaic.extractIvPFunction();
  IvPFunction *crs_ipf = crs_zaic.extractIvPFunction();

  OF_Coupler coupler;
  IvPFunction *ivp_function = coupler.couple(crs_ipf, spd_ipf, 50, 50);

  return(ivp_function);
}
