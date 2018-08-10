/************************************************************/
/*    NAME: Arjun Gupta                                              */
/*    ORGN: MIT                                             */
/*    FILE: PointAssign.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "PointAssign.h"
#include "Point.h"
using namespace std;

//---------------------------------------------------------
// Constructor

PointAssign::PointAssign()
{
  m_num_points = 0;
  m_assign_by_region = false;
  m_last_updated = "MOKAI_RED";
  m_started=false;
  m_ended=false;
  m_done=false;
  m_signal = false;
}

//---------------------------------------------------------
// Destructor

PointAssign::~PointAssign()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool PointAssign::OnNewMail(MOOSMSG_LIST &NewMail)
{
  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key   = msg.GetKey();
    string comm  = msg.GetCommunity();
    string sval  = msg.GetString(); 
    double mtime = msg.GetTime();
    if(key == "VISIT_POINT"){
      if((sval =="firstpoint")){
	m_started=true;
      }
      else if(sval=="lastpoint"){
	m_ended=true;
      }
      else{
	Point new_point(sval);
	m_points_list.push_back(new_point);	
      }
    }
    else if(key == "VISIT_SIGNAL"){
	m_signal=true;
    }
   }	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool PointAssign::OnConnectToServer()
{	
   RegisterVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool PointAssign::Iterate()
{
  if(m_signal){
    if(m_points_list.size()==m_num_points and m_ended and !m_done){
      Notify("VISIT_POINT_MOKAI_RED", "lastpoint");
      Notify("VISIT_POINT_MOKAI_BLUE", "lastpoint");
      m_done=true;
    }
    else if(!m_done){
      Notify("POINT_STAT", "Assigning Points");
      if(m_assign_by_region){
	//Assign According to which region each vehicle is and where the point is
	for(int index=0; index<m_points_list.size(); index++){
	  m_num_points+=1;
	  Point current_point = m_points_list[index];
	  if(current_point.west){
	    Notify("VISIT_POINT_MOKAI_BLUE", current_point.point_string);
	    Notify("POINT_STAT", "sent "+current_point.point_string+" to MOKAI_BLUE");
	  }
	  else{
	    Notify("VISIT_POINT_MOKAI_RED", current_point.point_string);
	    Notify("POINT_STAT", "sent "+current_point.point_string+" point to MOKAI_RED");
	  }
	}
      }
      else{ //Assign Randomly
	for(int index=0; index<m_points_list.size(); index++){
	  m_num_points+=1;
	  Point current_point= m_points_list[index];
	  Notify("POINT_STAT", "Last Updated Was: " + m_last_updated);
	  if(m_last_updated=="MOKAI_RED"){
	    m_last_updated="MOKAI_BLUE";
	    Notify("POINT_STAT", "updated MOKAI_BLUE with "+current_point.point_string);
	    Notify("VISIT_POINT_MOKAI_BLUE", current_point.point_string);
	  }
	  else{
	    m_last_updated = "MOKAI_RED";
	    Notify("POINT_STAT", "updated MOKAI_RED with "+current_point.point_string);
	    Notify("VISIT_POINT_MOKAI_RED", current_point.point_string);
	  }
	}
      }
    }
  }
  //m_points_list.clear();
  return(true);
  
}
//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool PointAssign::OnStartUp()
{
  Notify("POINT_STAT", "started");
  list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(m_MissionReader.GetConfiguration(GetAppName(), sParams)) {
    list<string>::iterator p;
    for(p=sParams.begin(); p!=sParams.end(); p++) {
      string original_line = *p;
      string param = stripBlankEnds(toupper(biteString(*p, '=')));
      string value = stripBlankEnds(*p);
      
      if(param == "FOO") {
        //handled
      }
      else if(param == "BAR") {
        //handled
      }
    }
  }
  
  RegisterVariables();	
  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void PointAssign::RegisterVariables()
{
  Register("VISIT_POINT", 0);
  Notify("UTS_PAUSE", "false");
  Register("VISIT_SIGNAL", 0);
}

