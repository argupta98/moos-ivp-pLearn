/*****************************y*******************************/
/*    NAME: Arjun Gupta                                      */
/*    ORGN: MIT                                              */
/*    FILE: BHV_Cover.cpp                                    */
/*    DATE: 07/26/2017                                       */
/************************************************************/

#ifdef _WIN32
#pragma warning(disable : 4786)
#pragma warning(disable : 4503)
#endif

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_Cover.h"
#include <stdlib.h>
#include <cmath>
#include <string>
#include <iostream>
#include "ZAIC_PEAK.h"
#include <sstream>
#include "OF_Coupler.h"
#include "XYPoint.h"


#define PI 3.14159265358979323846264338327

using namespace std;

//explicitly define all node reports to register for
string player = "NODE_REPORT_RED_ONE,NODE_REPORT_RED_TWO,NODE_REPORT_RED_THREE,NODE_REPORT_RED_FOUR,NODE_REPORT_BLUE_ONE,NODE_REPORT_BLUE_TWO,NODE_REPORT_BLUE_THREE,NODE_REPORT_BLUE_FOUR";
vector<string> players = parseString(player, ',');

//---------------------------------------------------------------
// Constructor

BHV_Cover::BHV_Cover(IvPDomain domain) :
  IvPBehavior(domain)
{
  // Provide a default behavior name
  //  IvPBehavior::setParam("name", "defaultname");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");
  m_descriptor = "defend";
  m_attX  = 0;
  m_attY  = 0;
  m_protX = 0;
  m_protY = 0;
  m_protH = 0;
  m_speed = 2;
  m_dist_from_flag = 10;
  m_oppX  = 0;
  m_oppY  = 0;
  m_osX = 0;
  m_osY = 0;
  m_destX = 0;
  m_destY = 0;
  m_attack_angle = 0;
  m_attacker = "none";
  m_covered = "";
  m_requestor="";
  m_team ="";
  m_teammate="";
  m_curr_node_report = "";
  m_move = false;
  m_angle = 0;
  m_crit_dist=80;
  m_priority = false;
  
// Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y, TAGGED_VEHICLES, COVER_NOTIFY, "+player);
  postMessage("STAT", "finished initializing");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_Cover::setParam(string param, string val)
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());
  
  if((param == "team")) {
    m_team = val;
    return(true);
  }

  else if((param == "teammate")){
    m_teammate = val;
    return(true);
  }

  else if((param == "requestor")){
    m_requestor=val;
    return(true);
  }
  
  else if((param == "speed") && isNumber(val)) {
    m_speed = double_val;
    return(true);
  }

  else if((param == "self")){
    string name=val;
    for(int i = 0; i < name.size(); i++) {
      name.at(i) = toupper(name.at(i));
    }
    m_self = "NODE_REPORT_"+name;
    m_name = val;
    return(true);
  }
  
  else if((param == "distance_from_flag") && isNumber(val)) {
    m_dist_from_flag = double_val;
    return true;
  }

  else if((param == "enemy_flag")){
    vector<string> flag_coord = parseString(val, ',');
    m_attX = atof(flag_coord[0].c_str());
    m_attY = atof(flag_coord[1].c_str());
    return true;
  }
  // If not handled above, then just return false;
  return(false);
}


//---------------------------------------------------------------
// Procedure: onSetParamComplete()
//   Purpose: Invoked once after all parameters have been handled.
//            Good place to ensure all required params have are set.
//            Or any inter-param relationships like a<b.

void BHV_Cover::onSetParamComplete()
{
  int self_val = 0;
  int team_val = 0;
  for (int i = 0; i < m_name.size(); i++) {
    self_val += m_name[i];
  }
  for (int i = 0; i < m_teammate.size(); i++){
    team_val += m_teammate[i];
  }

  if(self_val > team_val){
    m_priority = true;
  }
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_Cover::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_Cover::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_Cover::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_Cover::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_Cover::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_Cover::onRunToIdleState()
{
}

//---------------------------------------------------------------
// Procedure: getOppCoords()
//   Purpose: Look at opposing node reports to get their position

void BHV_Cover::getOppCoords(string node)
{
  if(m_requestor==""){
    postWMessage("No teammate to guard");
  }
  postMessage("STAT", "starting to get opponent coordinates");
  NodeReport new_report(node);
  
  if(new_report.name=="" || new_report.nav_x==0 || new_report.nav_y==0){
    postWMessage("Invalid Node Report");
  }

  bool tagged = false;
  for(int i = 0; i<m_tagged.size(); i++){
    if(new_report.name==m_tagged[i]){
      tagged = true;
    }
  }
  
  bool found =false;
  for(int i = 0; i<m_opp_list.size(); i++){
    if(m_opp_list[i].name == new_report.name){
      m_opp_list[i]=new_report;
      found= true;
      postMessage("STAT","updating node report: "+node);
    }
  }
  
  if((!found) && (new_report.group != m_team)){
    m_opp_list.push_back(new_report);
    postMessage("STAT", "added "+new_report.name);
  }
  
  if(new_report.name == m_attacker){
    if(hypot(new_report.nav_x-m_protX, new_report.nav_y-m_protY)>m_crit_dist || tagged){
      m_attacker="none";
    }
    else if(!m_priority && m_attacker == m_covered){
      m_attacker="none";
    }
    else{
      m_oppX = new_report.nav_x;
      m_oppY = new_report.nav_y;
    }
  }

  if(new_report.name == m_requestor){
    m_protX = new_report.nav_x;
    m_protY = new_report.nav_y;
    m_protH= new_report.heading;
  }
}



//------------------------------------------------------------
//Procedure: onRunState()
//    Purpose: To calculate the X and Y values of where the robot should be if it is to be between its flag 
//             and the enemy robot (at a set distance from the flag)

IvPFunction* BHV_Cover::onRunState()
{
  postMessage("STAT", "Starting OnRunState()");
  postMessage("STAT", "Guarding: " + m_requestor);
  bool check1, check2, check3;
  double dx, dy=0;
  m_osX = getBufferDoubleVal("NAV_X", check1);
  m_osY = getBufferDoubleVal("NAV_Y", check2);

  if(!check1 || !check2) {
    postWMessage("BHV_DEFENSE ERROR: No X/Y value in info_buffer!");
    return 0;
  }
  string temp = getBufferStringVal("TAGGED_VEHICLES", check2);
  m_tagged = parseString(temp, ',');

  for(int i=0; i<players.size();i++){
    if(players[i] != m_self){
      m_curr_node_report = getBufferStringVal(players[i], check3);
      if(!check3){
	postWMessage("BHV_DEFENSE ERROR: Node_report not found in info_buffer!");
      }
      else{
	getOppCoords(m_curr_node_report);
      }
    }
  }

  m_covered = getBufferStringVal("COVER_NOTIFY", check3);
  string val="src_node="+m_name+",dest_node="+m_teammate+",var_name=COVER_NOTIFY,string_val="+m_attacker;
  postMessage("NODE_MESSAGE_LOCAL", val);
    
  IvPFunction *ipf = 0;
  double deltX,deltY=0;
  
  //we have identified a valid attacker coming within range for the flag
  if(m_attacker!="none"){
    deltX = m_oppX-m_protX;
    deltY = m_oppY-m_protY;
  }
  
  //otherwise try to send the robot out infront of the vehicle
  else{
    deltX = m_attX-m_protX;
    deltY = m_attY-m_protY;
  }
  
  m_attack_angle = atan( abs(deltY) / abs(deltX) );
  //m_attack_angle increases counter clockwise, but resets to 0 every 180 degrees
  
  if (deltX*deltY < 0)
    m_attack_angle = PI - m_attack_angle;
  
  if(deltY>0){   
    m_destX = cos(m_attack_angle)*m_dist_from_flag + m_protX;
    m_destY = sin(m_attack_angle)*m_dist_from_flag + m_protY;
  }
  
  else{
    m_destX = m_protX - cos(m_attack_angle)*m_dist_from_flag;
    m_destY = m_protY - sin(m_attack_angle)*m_dist_from_flag;
  }
 
      
  dx = m_destX-m_osX;
  dy = m_destY-m_osY;

  double dx1 = m_oppX-m_protX;
  double dy1 = m_oppY-m_protY;

  if(hypot(dx1,dy1) < m_dist_from_flag){
    dx = m_oppX-m_osX;
    dy = m_oppY-m_osY;
    m_move=true;
  }
 
  m_angle = 90-atan(abs(dy)/abs(dx))*180/PI;
  
  
  //convert m_angle so that it the ipf function reads it properly
  //(starts at 0 on the positive y-axis and increases counter-clockwise)

  if (dx*dy<0)
    m_angle = 90-m_angle;

  if (dx>0 && dy<0)
    m_angle += 90;
  else if (dx<0 && dy<0)
    m_angle += 180;
  else if (dx<0 && dy>0)
    m_angle += 270;
  



  
  //we have not found a suitble attacker yet 
  if(m_attacker =="none"){
    double min_index=0;
    double min_dist = m_crit_dist+1;
    for(int i=0; i<m_opp_list.size();i++){
      bool tagged = false;
      for(int i = 0; i<m_tagged.size(); i++){
	if(m_opp_list[i].name==m_tagged[i]){
	  tagged = true;
	}
      }
      if(!tagged){
	double delta_x = m_opp_list[i].nav_x-m_protX;
	double delta_y = m_opp_list[i].nav_y-m_protY;
	if(hypot(delta_x, delta_y)<min_dist && (m_opp_list[i].name != m_covered)){
	  min_dist=hypot(delta_x, delta_y);
	  min_index=i;
	}
      }
    }
    if(min_dist<m_crit_dist){
      m_attacker=m_opp_list[min_index].name;
      m_oppX= m_opp_list[min_index].nav_x;
      m_oppY= m_opp_list[min_index].nav_y;
    }
  }
  
  //determines if defending vehicle is close enough to were it should be
  //if it isn't, m_move is set to true which activates the ipf function
  if ((hypot(dx, dy)>5))
    m_move=true;
  
  if (m_move){
    int speed = min(int(5*hypot(dx,dy)/m_dist_from_flag*m_speed), int(m_speed));
    ipf = buildFunctionWithZAIC(speed);
    m_move=false;
  }
  
  // Part N: Prior to returning the IvP function, apply the priority wt
  // Actual weight applied may be some value different than the configured
  // m_priority_wt, depending on the behavior author's insite.
  if(ipf)
    ipf->setPWT(m_priority_wt);

  return(ipf);
}

//---------------------------------------------------------------
// Procedure: buildFunctionWithZAIC()
//     Purpose: Builds and IvP function for the behavior

IvPFunction *BHV_Cover::buildFunctionWithZAIC(double speed)
{
  ZAIC_PEAK spd_zaic(m_domain, "speed");
  spd_zaic.setSummit(speed);
  spd_zaic.setBaseWidth(0.3);
  spd_zaic.setPeakWidth(0.0);
  spd_zaic.setSummitDelta(0.0);
  IvPFunction *spd_of = spd_zaic.extractIvPFunction();

  ZAIC_PEAK crs_zaic(m_domain, "course");
  crs_zaic.setSummit(m_angle);
  crs_zaic.setBaseWidth(180.0);
  crs_zaic.setValueWrap(true);
  IvPFunction *crs_of = crs_zaic.extractIvPFunction();

  OF_Coupler coupler;
  IvPFunction *ipf = coupler.couple(crs_of, spd_of);

  return(ipf);
}
