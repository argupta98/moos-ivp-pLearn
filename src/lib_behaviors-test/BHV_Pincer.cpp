/************************************************************/
/*    NAME: Arjun Gupta                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_Pincer.cpp                                    */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_Pincer.h"
#include "NodeReport.h"

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_Pincer::BHV_Pincer(IvPDomain domain) :
  IvPBehavior(domain)
{
  // Provide a default behavior name
  IvPBehavior::setParam("name", "PincerAttack");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");

  // Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y, NODE_REPORT");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_Pincer::setParam(string param, string val)
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());
  
  if (param == "team") {
    m_group = param;// return(setBooleanOnString(m_my_bool, val));
    return(true);
  }
  else if (param == "critical_point"){
    m_critical_point = val;
  // If not handled above, then just return false;
  return(false);
}

//---------------------------------------------------------------
// Procedure: onSetParamComplete()
//   Purpose: Invoked once after all parameters have been handled.
//            Good place to ensure all required params have are set.
//            Or any inter-param relationships like a<b.

void BHV_Pincer::onSetParamComplete()
{
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_Pincer::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_Pincer::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_Pincer::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_Pincer::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_Pincer::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_Pincer::onRunToIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onRunState()
//   Purpose: Invoked each iteration when run conditions have been met.

IvPFunction* BHV_Pincer::onRunState()
{
  // Part 1: Build the IvP function
  IvPFunction *ipf = 0;
  //handle new node reports
  //check to see if you are left or right robot and act accordingly by looking at m_allies and deciding where our partner is relative to us
  //if left robot start moving to left edge of field, opposite if right robot
  //after some critical x-coordinate (center line default) start curving in towards the flag



  if(ipf)
    ipf->setPWT(m_priority_wt);

  return(ipf);
}

void BHV_Pincer::handleNodeReport(NodeReport report)
{
  if(!report.valid){
    postWMessage("invalid NODE_REPORT");
  }
  else{ //add the report to the correct group list. 
    bool replaced = false;
    if(report.group != m_group){
      for(int i=0; i<m_adversaries; i++){
	if(m_adversaries[i].name==report.name){
	  m_adversaries[i]=report;
	  replaced = true;
	  break;
	}
      }
      if(!replaced){
	m_adversaries.push_back(report);
      }
    }
    else {
      for(int i=0; i<m_allies; i++){
	if(m_allies[i].name==report.name){
	  m_allies[i]=report;
	  replaced = true;
	  break;
	}
      }
      if(!replaced){
	m_allies.push_back(report);
      }
    }
  }
}


IvPFunction* BHV_Pincer::GenerateObjectiveFunction()
{
  //Set speed to constant 2m/s
  //Set course to m_heading found in OnRunState function
}

