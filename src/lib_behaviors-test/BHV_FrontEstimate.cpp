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
#include "BHV_FrontEstimate.h"
#include "ZAIC_PEAK.h"
#include "OF_Coupler.h"
#include <vector>
#include "NodeMessage.h"

using namespace std;
double GRAD_LOW = .1;
double GRAD_HIGH = 1.3;
//---------------------------------------------------------------
// Constructor

BHV_FrontEstimate::BHV_FrontEstimate(IvPDomain domain) :
  IvPBehavior(domain)
{
  m_last_gradient = 1;
  m_count = 0;
  m_turn_around=false;
  // Provide a default behavior name
  IvPBehavior::setParam("name", "front_estimate");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");

  // Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y, UCTD_MSMNT_REPORT, OPREG_TRAJECTORY_PERIM_DIST, NODE_MESSAGE");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_FrontEstimate::setParam(string param, string val)
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());
  
  if((param == "speed") && isNumber(val)) {
    m_desired_speed = double_val;
    return(true);
  }
  else if (param == "hostname") {
    m_hostname = val;
    return(true);
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

void BHV_FrontEstimate::onSetParamComplete()
{
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_FrontEstimate::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_FrontEstimate::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_FrontEstimate::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_FrontEstimate::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_FrontEstimate::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_FrontEstimate::onRunToIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_FrontEstimate::onRunState()
{
  // Part 1: Build the IvP function
  IvPFunction *ipf = 0;
  
  //get info from uFldCTDSensor
  bool ok1, ok2, ok3;
  string info= getBufferStringVal("UCTD_MSMNT_REPORT", ok1);

  // send info to partner
  if(ok1){
    NodeMessage node_message;
    node_message.setSourceNode(m_hostname);
    node_message.setDestNode("all");
    node_message.setVarName("UCTD_MSMNT_REPORT");
    node_message.setStringVal(info);
    string msg = node_message.getSpec();
    postMessage("NODE_MESSAGE_LOCAL", msg);
    postMessage("STAT", "sent out node_message_local");
  }
  
  //parse report from partner and send to local FrontEstimate
  string part_info = getBufferStringVal("NODE_MESSAGE", ok3);
  if(ok3){
    postMessage("STAT", "recieved NODE_MESSAGE: "+ part_info);
    vector<string> parsed_partner = parseStringQ(part_info, ',');
    string message = parsed_partner[parsed_partner.size()-1];
    biteStringX(message, '=');
    postMessage("STAT", "message = " +message);
    vector<string> message_vector = parseString(message, ',');
    string mod_report = "vname="+m_hostname+",";
    postMessage("STAT", "message size: "+to_string(message_vector.size()));
    for(int i=1; i<message_vector.size();i++){
      mod_report+=message_vector[i];
    }
    postMessage("UCTD_MSMNT_REPORT", mod_report);
    postMessage("STAT", "sent: "+mod_report+" to UCTD_MSMNT_REPORT");
  }

  //parse data from local UCTD_MSMNT
  double distance = getBufferDoubleVal("OPREG_TRAJECTORY_PERIM_DIST", ok2);
  vector<string> parsed_temp = parseString(info, ',');
  string temp_str = parsed_temp[parsed_temp.size()-1];
  biteStringX(temp_str, '=');
  double temp = stod(temp_str);
  if(temp_list.size()==0){
    temp_list.push_back(temp);
  }
  
  if(ok1){
    m_count++;
    if(distance<20 && m_count>40){
      m_count=0;
      m_turn_around = !m_turn_around;
      if(m_heading<180)
	m_heading+=180;
      else
	m_heading-=180;
    }
    else{
      int last_temp = temp_list[temp_list.size()-1];
      if(m_detected_front){
	if(!m_found_extrema){
	  if(m_turn_around)
	    m_heading = 0;
	  else
	    m_heading = 180; //head directly downwards
	  
	  if(temp-last_temp<GRAD_LOW and last_temp-temp<GRAD_LOW){
	    m_found_extrema=true;
	    m_turn_around=false;
	    postMessage("STAT", "found extrema");
	  }
	}	
	//if we have found the max interval, go back up until we reach the area of maximum gradient
	else{
	  if(m_front_running){ //we want to go along the front
	    if(m_turn_around)
	      m_heading = 270; //+theta
	    else
	      m_heading = 90; //+theta
	    // somehow get the current calculated front theta?
	  }
	  else{
	    if(m_turn_around)
	      m_heading=315-190;
	    else
	      m_heading = 315; // 45 degrees off from straight up
	    if(temp>last_temp+GRAD_HIGH || temp<last_temp-GRAD_HIGH){
	      m_front_running=true;
	      m_turn_around = false;
	    }
	  }
	}
      }
      //haven't found wavefront yet
      else{
	if(m_turn_around)
	  m_heading=0;
	else
	  m_heading = 180;
	if((temp-last_temp) > GRAD_HIGH || -(temp-last_temp) > GRAD_HIGH){
	  m_detected_front=true;
	  m_turn_around = false;
	  postMessage("STAT", "detected front");
	}
      }
      m_last_gradient = temp-last_temp;
    }
    temp_list.push_back(temp);
    postMessage("STAT", "added " + to_string(temp) + " to message list");
  }
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


IvPFunction *BHV_FrontEstimate::buildFunctionWithZAIC() 
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
  IvPFunction *ivp_function = coupler.couple(crs_ipf, spd_ipf, 70, 30);

  return(ivp_function);
}
