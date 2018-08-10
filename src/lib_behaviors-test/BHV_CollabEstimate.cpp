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
#include "BHV_CollabEstimate.h"
#include "ZAIC_PEAK.h"
#include "OF_Coupler.h"
#include <vector>
#include "NodeMessage.h"

using namespace std;
double GRAD_LOW = .1;
double GRAD_HIGH = 1.8;
//---------------------------------------------------------------
// Constructor

BHV_CollabEstimate::BHV_CollabEstimate(IvPDomain domain) :
  IvPBehavior(domain)
{
  m_last_gradient = 1;
  m_turn_delay = 0;
  m_down_delay = 0;
  m_turn_around=false;
  m_share = "";
  m_left = false;
  // Provide a default behavior name
  IvPBehavior::setParam("name", "front_estimate");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");

  // Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y, UCTD_MSMNT_REPORT, OPREG_TRAJECTORY_PERIM_DIST, NODE_MESSAGE,UCTD_PARAMETER_ESTIMATE");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_CollabEstimate::setParam(string param, string val)
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
  
  else if(param == "left"){
    //return(setBooleanOnString(m_left, val));
    m_left=true;
    return(true);
    postMessage("STAT", "got parameter :  LEFT");
  }

  // If not handled above, then just return false;
  return(false);
}

//---------------------------------------------------------------
// Procedure: onSetParamComplete()
//   Purpose: Invoked once after all parameters have been handled.
//            Good place to ensure all required params have are set.
//            Or any inter-param relationships like a<b.

void BHV_CollabEstimate::onSetParamComplete()
{
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_CollabEstimate::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_CollabEstimate::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_CollabEstimate::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_CollabEstimate::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_CollabEstimate::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_CollabEstimate::onRunToIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_CollabEstimate::onRunState()
{
  // Part 1: Build the IvP function
  IvPFunction *ipf = 0;
  bool new_info= false;;
  //get info from uFldCTDSensor
  bool ok1, ok2, ok3, ok4;
  string info= getBufferStringVal("UCTD_MSMNT_REPORT", ok1);
  // send info to partner
  if(ok1 and m_last_info!=info){
    new_info = true;
    if(m_share!="")
      m_share+="#";
    m_share+=info;
    vector<string> parsed_info = parseString(info, ',');
    string name = parsed_info[0];
    biteStringX(name, '=');
    if(name== m_hostname){
      NodeMessage node_message;
      node_message.setSourceNode(m_hostname);
      node_message.setDestNode("all");
      node_message.setVarName("UCTD_MSMNT_REPORT");
      node_message.setStringVal(info);
      string msg = node_message.getSpec();
      postMessage("NODE_MESSAGE_LOCAL", msg);
      postMessage("STAT", "sent out: " + msg);
    }
  }
  m_last_info = info;
  //parse report from partner and send to local FrontEstimate to update the local annealer
  string part_info = getBufferStringVal("NODE_MESSAGE", ok3);
  if(ok3){
    vector<string> parsed_partner = parseStringQ(part_info, ',');
    string message = parsed_partner[parsed_partner.size()-1];
    biteStringX(message, '=');
    if(m_last_part_info!=message){
      vector<string> messages = parseString(message, '#');
      for(int i=0;i<messages.size();i++){
      /*
      postMessage("STAT", "message = " +message);
      vector<string> message_vector = parseString(message, ',');
      string mod_report = "vname="+m_hostname;
      postMessage("STAT", "message size: "+to_string(message_vector.size()));
      for(int i=1; i<message_vector.size();i++){
	mod_report+=",";
	mod_report+=message_vector[i];
      }
      */
	postMessage("UCTD_MSMNT_REPORT", message);
	postMessage("STAT", "recieved NODE_MESSAGE: "+ message);
      }
    }
   m_last_part_info=message;
  }
  //parse data from  UCTD_PARAMETER_ESTIMATE
  string estimate= getBufferStringVal("UCTD_PARAMETER_ESTIMATE", ok4);
  vector<string> estimate_vector = parseStringQ(estimate, ',');
  double angle, amplitude, tempnorth, tempsouth;
  for(int i=1; i<estimate_vector.size(); i++){
    string value= estimate_vector[i];
    string key= biteString(value, '=');
    if(key=="angle")
      angle=stod(value);
    else if(key=="amplitude")
      amplitude=stod(value);
    else if(key=="tempnorth")
      tempnorth=stod(value);
    else if(key=="tempsouth")
      tempsouth=stod(value);
  }
  
  //parse data from OPREG_TRAJECTORY_PERIM_DIST
  double distance = getBufferDoubleVal("OPREG_TRAJECTORY_PERIM_DIST", ok2);
  vector<string> parsed_temp = parseString(info, ',');
  string temp_str = parsed_temp[parsed_temp.size()-1];
  biteStringX(temp_str, '=');
  double temp = stod(temp_str);
  if(temp_list.size()==0){
    temp_list.push_back(temp);
  }
  
  if(ok1){
    m_turn_delay++;
    m_down_delay++;
    //handle the case that we are nearing the edge of the boundry box (the count ensures we don't keep changing direction)
    if(distance<20 && m_turn_delay>40){
      m_turn_delay=0;
      m_turn_around = !m_turn_around;
      postMessage("STAT", "turned around");
    }
    //decide which m_heading to take in order to get best data 
    else{
      if(m_down_delay<70){ //we want to go straight down for a while
	   if(m_turn_around)
	      m_heading = 0;
	    else
	      m_heading = 180;
      }
	//if we have found the max interval, go back up 
      else {
        if(m_down_delay>70){ //we want to go along the front
	  if(!m_left){
	    if(m_turn_around)
	      m_heading = 270+angle;
	    else
	      m_heading = 90+angle;
	  }
	  else{
	    if(m_turn_around)
	      m_heading = 90+angle;
	    else
	      m_heading = 270+angle;
	  }
	}
      }
    }  
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


IvPFunction *BHV_CollabEstimate::buildFunctionWithZAIC() 
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
