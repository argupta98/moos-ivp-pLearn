/************************************************************/
/*    NAME: Arjun Gupta                                              */
/*    ORGN: MIT                                             */
/*    FILE: PointAssign.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef PointAssign_HEADER
#define PointAssign_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include <iostream>
#include <vector>
#include "Point.h"

using namespace std;

class PointAssign : public CMOOSApp
{
 public:
   PointAssign();
   ~PointAssign();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected:
   void RegisterVariables();
   bool m_assign_by_region;
   string m_last_updated;
   int m_num_points;
   vector<Point> m_points_list;
   bool m_started;
   bool m_ended;
   bool m_done;
   bool m_signal;
   
 private: // Configuration variables

   

 private: // State variables
};

#endif 
