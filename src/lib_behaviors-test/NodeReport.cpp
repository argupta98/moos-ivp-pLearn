//Name: Arjun Gupta
//Date: June 19, 2017

#include "MBUtils.h"
#include <vector>
#include "NodeReport.h"

using namespace std;

NodeReport::NodeReport(string node_report){
  valid=true;
  ProcessNodeReport(node_report);
}


void NodeReport::ProcessNodeReport(string node_report){
  string name, group;
  double nav_x, nav_y, heading; 
  vector<string> elements= parseStringQ(node_report, ',');
  for(int i=0; i<elements.size(); i++){
    string right = elements[i];
    string left = biteStringX(right, '=');
    if(left == "NAME"){
      name = right;
    }
    else if(left == "GROUP"){
      group = right;
    }
    else if(left == "NAV_X"){
      nav_x = stof(right);
    }
    else if(left == "NAV_Y"){
      nav_y = stof(right);
    }
    else if(left =="HEADING"){
      heading = stof(right);
    }
  }
  if(!(name && nav_x && nav_y && heading)){
    valid = false;
  }
}

